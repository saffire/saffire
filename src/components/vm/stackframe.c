/*
 Copyright (c) 2012-2015, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>
#include "vm/vm.h"
#include "vm/stackframe.h"
#include "vm/thread.h"
#include "vm/context.h"
#include "vm/import.h"
#include "compiler/bytecode.h"
#include "vm/vm_opcodes.h"
#include "general/smm.h"
#include "objects/string.h"
#include "objects/numerical.h"
#include "objects/hash.h"
#include "objects/null.h"
#include "objects/boolean.h"
#include "objects/callable.h"
#include "debug.h"
#include "general/output.h"

#include "general/hashtable.h"

/**
 * Returns the next opcode
 */
unsigned char vm_frame_get_next_opcode(t_vm_stackframe *frame) {
    // Sanity stop
    if (frame->ip >= frame->codeblock->bytecode->code_len) {
        DEBUG_PRINT_CHAR("Running outside bytecode!\n\n\n");
        return VM_STOP;
    }

    unsigned char op = frame->codeblock->bytecode->code[frame->ip];
    frame->ip++;

    return op;
}

/**
 * Returns he next operand. Does not do any sanity checks if it actually is an operand.
 */
unsigned int vm_frame_get_operand(t_vm_stackframe *frame) {
    // Read operand
    uint16_t *ptr = (uint16_t *)(frame->codeblock->bytecode->code + frame->ip);
    unsigned int ret = (*ptr & 0xFFFF);

    frame->ip += sizeof(uint16_t);
    return ret;
}

/**
 * Pops an object from the stack. If the object is an attribute, fetch the actual data of that attribute.
 * Errors when the stack is empty
 */
t_object *vm_frame_stack_pop(t_vm_stackframe *frame) {
    t_object *obj = vm_frame_stack_pop_attrib(frame);
    if (OBJECT_IS_ATTRIBUTE(obj)) return ((t_attrib_object *)obj)->data.attribute;
    return obj;
}

/**
 * Pops an object from the stack. Errors when the stack is empty
 */
t_object *vm_frame_stack_pop_attrib(t_vm_stackframe *frame) {
#if __DEBUG_STACK
    DEBUG_PRINT_CHAR(ANSI_BRIGHTYELLOW "STACK POP (%d): %08lX %s\n" ANSI_RESET, frame->sp, (unsigned long)frame->stack[frame->sp], object_debug(frame->stack[frame->sp]));
#endif

    if (frame->sp >= frame->codeblock->bytecode->stack_size) {
        fatal_error(1, "Trying to pop from an empty stack");        /* LCOV_EXCL_LINE */
    }
    t_object *ret = frame->stack[frame->sp];
    frame->stack[frame->sp] = NULL;
    frame->sp++;

    return ret;
}

/**
 * Pushes an object onto the stack. Errors when the stack is full
 */
void vm_frame_stack_push(t_vm_stackframe *frame, t_object *obj) {
#if __DEBUG_STACK
        DEBUG_PRINT_STRING_ARGS(ANSI_BRIGHTYELLOW "STACK PUSH(%d): %s %08lX \n" ANSI_RESET, frame->sp-1, object_debug(obj), (unsigned long)obj);
#endif


    if (frame->sp < 0) {
        fatal_error(1, "Trying to push to a full stack"); /* LCOV_EXCL_LINE */
    }
    frame->sp--;
    frame->stack[frame->sp] = obj;
}

void vm_frame_stack_modify(t_vm_stackframe *frame, int idx, t_object *obj) {
    DEBUG_PRINT_STRING_ARGS(ANSI_BRIGHTYELLOW "STACK CHANGE(%d): %s %08lX \n" ANSI_RESET, idx, object_debug(obj), (unsigned long)obj);
    frame->stack[idx] = obj;
}

/**
 * Fetches the top of the stack. Does not pop anything.
 */
t_object *vm_frame_stack_fetch_top(t_vm_stackframe *frame) {
    return frame->stack[frame->sp];
}

/**
 * Fetches a non-top element form the stack. Does not pop anything.
 */
t_object *vm_frame_stack_fetch(t_vm_stackframe *frame, int idx) {
    if (idx < 0 || idx >= frame->codeblock->bytecode->stack_size) {
        fatal_error(1, "Trying to fetch from outside stack range");     /* LCOV_EXCL_LINE */
    }

    return frame->stack[idx];
}

/**
 * Return a constant literal, without converting to an object
 */
void *vm_frame_get_constant_literal(t_vm_stackframe *frame, int idx) {
    if (idx < 0 || idx >= frame->codeblock->bytecode->constants_len) {
        fatal_error(1, "Trying to fetch from outside constant range");      /* LCOV_EXCL_LINE */
    }

    t_bytecode_constant *c = frame->codeblock->bytecode->constants[idx];
    return c->data.ptr;
}

/**
 * Returns an object from the constant table
 */
t_object *vm_frame_get_constant(t_vm_stackframe *frame, int idx) {
    if (idx < 0 || idx >= frame->codeblock->bytecode->constants_len) {
        fatal_error(1, "Trying to fetch from outside constant range");      /* LCOV_EXCL_LINE */
    }

    return frame->codeblock->constants_objects[idx];
}

/**
 * Store object into the global identifier table. When obj == NULL, it will remove the actual reference (plus object)
 */
void vm_frame_set_global_identifier(t_vm_stackframe *frame, char *id, t_object *obj) {
    if (obj == NULL) {
        t_object *old = ht_remove_str(frame->global_identifiers->data.ht, id);

        if (old) object_release(old);
        return;
    }

    if (! ht_exists_str(frame->global_identifiers->data.ht, id)) {
        ht_add_str(frame->global_identifiers->data.ht, id, obj);
        object_inc_ref(obj);
    } else {
        // @TODO: Overwrite, or throw error?
    }
}

/**
 * Return object from the global identifier table
 */
t_object *vm_frame_get_global_identifier(t_vm_stackframe *frame, char *id) {
    t_object *obj = (t_object *)ht_find_str(frame->global_identifiers->data.ht, id);

    if (obj == NULL) RETURN_NULL;
    return obj;
}

/**
 * Store object into the local identifier table
 */
void vm_frame_set_local_identifier(t_vm_stackframe *frame, char *fqcn, t_object *obj) {
    t_object *old_obj = (t_object *) ht_replace_str(frame->local_identifiers->data.ht, fqcn, obj);
    if (obj != NULL && obj != OBJECT_NEEDS_RESOLVING) {
        object_release(old_obj);
    }

    if (obj != NULL && obj != OBJECT_NEEDS_RESOLVING) {
        object_inc_ref(obj);
    }
}

void vm_frame_set_alias_identifier(t_vm_stackframe *frame, char *fqcn, char *target_fqcn) {
    // Store alias id under the FQCN id, and store that we still need to resolve the object
    vm_frame_set_local_identifier(frame, fqcn, OBJECT_NEEDS_RESOLVING);

    // Add to the alias table so we know that  alias -> ::some::module::class
    char *val = string_strdup0(target_fqcn);
    ht_add_str(frame->object_aliases, fqcn, val);
}

void vm_frame_set_builtin_identifier(t_vm_stackframe *frame, char *uqcn, t_object *obj) {
    // Builtin objects do not have a FQCN. They are stored as "numeric", "false", "null" etc..
    t_object *old_obj = ht_replace_str(frame->builtin_identifiers->data.ht, uqcn, obj);

    object_release(old_obj);
    if (obj != NULL && obj != OBJECT_NEEDS_RESOLVING) {
        object_inc_ref(obj);
    }
}


#ifdef __DEBUG

void print_debug_table(t_hash_table *ht, char *prefix) {
    t_hash_iter iter;

    if (! ht) return;

    ht_iter_init(&iter, ht);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key_str(&iter);
        t_object *val = ht_iter_value(&iter);
        DEBUG_PRINT_STRING_ARGS("%-10s KEY: '%s' ", prefix, key);


        if (val == NULL) {
            DEBUG_PRINT_CHAR("=> [NULL]\n");        // @TODO: Why is this happening!? Who has released this!!!
        } else if (val == OBJECT_NEEDS_RESOLVING) {
            DEBUG_PRINT_CHAR("=> [UNRESOLVED]\n");
        } else {
            DEBUG_PRINT_STRING_ARGS("=> [%08X] %s{%d}\n", (intptr_t)val, object_debug(val), val->ref_count);
        }

        ht_iter_next(&iter);
    }

}
#endif

t_object *_vm_frame_object_resolve(t_vm_stackframe *frame, char *fqcn) {
#ifdef __DEBUG
    ht_debug_keys(frame->object_aliases);
#endif

    char *alias_fqcn = ht_find_str(frame->object_aliases, fqcn);
    if (! alias_fqcn) {
        // We need to resolve, but we don't know what.. Should not happen
        // @TODO: Throw hard error
    }

    // Resolve and update the local identifiers
    t_object *obj = vm_class_resolve(frame, alias_fqcn);
    return obj;
}

t_object *vm_frame_identifier_exists(t_vm_stackframe *frame, char *id) {
    t_object *obj;

    // Find local identifiers (without FQCN)
    obj = ht_find_str(frame->local_identifiers->data.ht, id);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        obj = _vm_frame_object_resolve(frame, id);
        if (obj) {
            vm_frame_set_local_identifier(frame, id, obj);
        }
    }
    if (obj) return obj;

    // Check local identifiers, but on FQCN
    char *fqcn;
    fqcn = vm_context_fqcn(frame->codeblock->context, id);

    obj = ht_find_str(frame->local_identifiers->data.ht, fqcn);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        obj = _vm_frame_object_resolve(frame, fqcn);
        if (obj) {
            vm_frame_set_local_identifier(frame, fqcn, obj);
        }
    }
    if (obj) {

        smm_free(fqcn);
        return obj;
    }



    // @TODO: THIS SHOULD NOT BE... IMPORTS SHOULD BE ON LOCAL LIST!
    obj = ht_find_str(frame->global_identifiers->data.ht, id);
    if (obj == OBJECT_NEEDS_RESOLVING) {
#ifdef __DEBUG
        ht_debug_keys(frame->object_aliases);
#endif
        obj = _vm_frame_object_resolve(frame, id);
        if (obj) {
            vm_frame_set_global_identifier(frame, id, obj);
        }
    }
    if (obj) {
        smm_free(fqcn);
        return obj;
    }

    obj = ht_find_str(frame->global_identifiers->data.ht, fqcn);
    if (obj == OBJECT_NEEDS_RESOLVING) {
#ifdef __DEBUG
        ht_debug_keys(frame->object_aliases);
#endif
        obj = _vm_frame_object_resolve(frame, fqcn);
        if (obj) {
            vm_frame_set_global_identifier(frame, fqcn, obj);
        }
    }
    if (obj) {
        smm_free(fqcn);
        return obj;
    }


    // Check built-ins, but on UQCN
    obj = ht_find_str(frame->builtin_identifiers->data.ht, id);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        // Resolve and update the builtin identifiers (this should never happen?)
        obj = vm_class_resolve(frame, id);
        if (obj) {
            vm_frame_set_builtin_identifier(frame, id, obj);
        }
    }
    if (obj) {
        smm_free(fqcn);
        return obj;
    }

    smm_free(fqcn);
    return NULL;
}

/**
 * Same as vm_frame_get_identifier, but does not halt on error (but returns NULL)
 */
t_object *vm_frame_find_identifier(t_vm_stackframe *frame, char *id) {
    if (! frame) return NULL;

//    char *fqcn = vm_context_fqcn(frame->codeblock->context, id);
//    printf("*** FIND : %-20s  (%s)\n", id, fqcn);
//    smm_free(fqcn);

    t_object *obj = vm_frame_identifier_exists(frame, id);
    if (obj) return obj;

    return NULL;
}

/**
 * Returns an identifier name as string
 */
char *vm_frame_get_name(t_vm_stackframe *frame, int idx) {
    if (idx < 0 || idx >= frame->codeblock->bytecode->identifiers_len) {
        fatal_error(1, "Trying to fetch from outside identifier range");        /* LCOV_EXCL_LINE */
    }

    //#ifdef __DEBUG
    //    DEBUG_PRINT_CHAR("---------------------\n");
    //    DEBUG_PRINT_CHAR("frame identifiers:\n");
    //    for (int i=0; i!=frame->codeblock->bytecode->identifiers_len; i++) {
    //        DEBUG_PRINT_CHAR("ID %d: %s\n", i, frame->codeblock->bytecode->identifiers[i]->s);
    //    }
    //    print_debug_table(frame->local_identifiers->ht, "Locals");
    //#endif

    return frame->codeblock->bytecode->identifiers[idx]->s;
}

/**
 * Creates and initializes a new frame
 */
t_vm_stackframe *vm_stackframe_new(t_vm_stackframe *parent_frame, t_vm_codeblock *codeblock) {
    DEBUG_PRINT_CHAR("\n\n\n\n\n============================ VM frame new ('%s' -> parent: '%s') ============================\n", codeblock->context->module.full, parent_frame ? parent_frame->codeblock->context->module.full : "<root>");
    DEBUG_PRINT_CHAR("THIS FRAME IS BASED ON %08X\n", parent_frame);

    t_vm_stackframe *frame = smm_malloc(sizeof(t_vm_stackframe));

    DEBUG_PRINT_CHAR("THIS FRAME IS %08X\n", frame);
    bzero(frame, sizeof (t_vm_stackframe));

    frame->parent = parent_frame;
    frame->codeblock = codeblock;

    frame->ip = 0;

    frame->trace_class = NULL;
    frame->trace_method = NULL;

    frame->sp = codeblock->bytecode->stack_size;
    frame->stack = smm_malloc(codeblock->bytecode->stack_size * sizeof(t_object *));
    bzero(frame->stack, codeblock->bytecode->stack_size * sizeof(t_object *));


    frame->created_user_objects = dll_init();

    //    DEBUG_PRINT_CHAR("Increasing builtin_identifiers refcount\n");
    frame->builtin_identifiers = builtin_identifiers;
    object_inc_ref((t_object *)builtin_identifiers);

    // Create new empty local identifier hash
    DEBUG_PRINT_CHAR("Creating local ID table for frame %08x\n", frame);
    frame->local_identifiers = (t_hash_object *)object_alloc(Object_Hash, 0);
    object_inc_ref((t_object *)frame->local_identifiers);

    if (frame->parent == NULL) {
        frame->object_aliases = ht_create();
    } else {
        frame->object_aliases = parent_frame->object_aliases;
    }

    // Set the variable hashes
    if (frame->parent == NULL) {
        // global identifiers are the same as the local identifiers for the initial frame
        frame->global_identifiers = frame->local_identifiers;
    } else {
        // if not the initial frame, link globals from the parent frame
        frame->global_identifiers = frame->parent->global_identifiers;
    }
    object_inc_ref((t_object *)frame->global_identifiers);

    DEBUG_PRINT_CHAR("INIT LOCAL  ADDR: %08X\n", frame->local_identifiers);
    DEBUG_PRINT_CHAR("INIT GLOBAL ADDR: %08X\n", frame->global_identifiers);

    return frame;
}

/**
 *
 */
void vm_stackframe_destroy(t_vm_stackframe *frame) {

#ifdef __DEBUG_STACKFRAME
    DEBUG_PRINT_CHAR("\n\n\n\n\n============================ STACKFRAME DESTROY: %s ================================\n",
        frame->codeblock->context ? frame->codeblock->context->module.full : "<root>");

    DEBUG_PRINT_CHAR("FINI LOCAL  ADDR: %08X\n", frame->local_identifiers);
    DEBUG_PRINT_CHAR("FINI GLOBAL ADDR: %08X\n", frame->global_identifiers);

#if __DEBUG_STACKFRAME_DESTROY
    if (frame->local_identifiers) print_debug_table(frame->local_identifiers->data.ht, "Locals");
    if (frame->global_identifiers && frame->global_identifiers != frame->local_identifiers) {
        print_debug_table(frame->global_identifiers->data.ht, "Globals");
    }
    //if (frame->builtin_identifiers) print_debug_table(frame->builtin_identifiers->data.ht, "Builtins");
#endif
#endif


    // Remove codeblock reference (don't mind cleanup, since we still have it on the codeblock stack)
    frame->codeblock = NULL;
    if (frame->trace_class) smm_free(frame->trace_class);
    if (frame->trace_method) smm_free(frame->trace_method);

    t_hash_iter iter;
    ht_iter_init(&iter, frame->local_identifiers->data.ht);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key_str(&iter);
        t_object *val = ht_iter_value(&iter);

        // Because we MIGHT change the hash-table, we need to fetch the next
        // element PRIOR to changing the table. Otherwise we might end up in
        // the crapper.
        ht_iter_next(&iter);

        if (val == OBJECT_NEEDS_RESOLVING) continue;

        DEBUG_PRINT_STRING_ARGS("Frame destroy: Releasing => %s => %s [%08X]\n", key, object_debug(val), (intptr_t)val);

        // Release value, as it's no longer needed.
        object_release(val);

        ht_remove_str(frame->local_identifiers->data.ht, key);
    }

    // Free created user objects
    t_dll_element *e = DLL_HEAD(frame->created_user_objects);
    while (e) {
        t_object *obj = (t_object *)e->data;

        object_release(obj);
        e = DLL_NEXT(e);
    }
    dll_free(frame->created_user_objects);

    // Free identifiers
    object_release((t_object *)frame->global_identifiers);
    object_release((t_object *)frame->local_identifiers);
    object_release((t_object *)frame->builtin_identifiers);

    smm_free(frame->stack);

    smm_free(frame);
}

///**
// * Register a user-created class. This way we always keep a reference onto the stack, until we actually remove it.
// *
// * WARNING:
// * What happens now is that we globally register our objects. As long as the object is registered here, we have at least
// * a refcount. So when we push/pop it from the stack, we cannot accidentally free it (because we pop it, and in the same
// * pass, we push it again).
// *
// * We should change the system so we don't really need this way of working. An object can have multiple states:
// *
// *  - born      An object has been generated, but not yet pushed onto the stack. This is basically the same state as
// *              "is use", but once an object gets born, it can never return to this state. Maybe possible for some
// *              initialization etc.
// *  - stacked   The object is not in use by the VM, but it has at least 1 reference on (a) stack.
// *  - in use    The object is currently in use by the VM. It *MIGHT* not have any references onto the stack (refcount = 0).�
// *
// *  - died      The object has died. There is no more refcounts AND the object is not in use. It might be possible to reanimate
// *              the object for other purposes. In that case, the state becomes stacked, or in-use again.
// *  - buried    The object has died, and the garbage collector has freed its memory. When the object needs to be generated
// *              again, it must be newly allocated. In this case, the object state because "born" again.
// *
// */

// register a user object. This way it is known in the stack frame, AND has a refcount of 1. This is to make sure
// that we always have a reference to the user-object.
void vm_frame_register_userobject(t_vm_stackframe *frame, t_object *obj) {
    dll_append(frame->created_user_objects, obj);
    object_inc_ref(obj);
}


#ifdef __DEBUG
void vm_frame_stack_debug(t_vm_stackframe *frame) {
    // Nothing on the stack
    if (frame->sp == frame->codeblock->bytecode->stack_size) {
        return;
    }

    DEBUG_PRINT_CHAR("\nFRAME STACK\n");
    DEBUG_PRINT_CHAR("=======================\n");
    for (int i=frame->sp; i<=frame->codeblock->bytecode->stack_size-1; i++) {
        DEBUG_PRINT_CHAR("  %s%02d %08X %s\n", (i == frame->sp - 1) ? ">" : " ", i, (intptr_t)frame->stack[i], frame->stack[i] ? object_debug(frame->stack[i]) : "");
    }
    DEBUG_PRINT_CHAR("\n");
}
#endif
