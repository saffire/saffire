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
#include <saffire/vm/vm.h>
#include <saffire/vm/stackframe.h>
#include <saffire/vm/thread.h>
#include <saffire/vm/context.h>
#include <saffire/vm/import.h>
#include <saffire/compiler/bytecode.h>
#include <saffire/vm/vm_opcodes.h>
#include <saffire/memory/smm.h>
#include <saffire/objects/string.h>
#include <saffire/objects/numerical.h>
#include <saffire/objects/hash.h>
#include <saffire/objects/null.h>
#include <saffire/objects/boolean.h>
#include <saffire/objects/callable.h>
#include <saffire/debug.h>
#include <saffire/general/output.h>

#include <saffire/general/hashtable.h>


/**
 * Returns the current context of the given frame
 */
t_vm_context *vm_frame_get_context(t_vm_stackframe *frame)
{
    return frame->codeblock->context;
}


/**
 * Returns the current line number based on the instruction pointer in the current frame.
 *
 * @param frame
 * @return long
 */
long vm_frame_get_source_line(t_vm_stackframe *frame) {
    // If IP is within current bounds, just return the current line number
    if (frame->lineno_upperbound != 0 &&
        frame->lineno_lowerbound <= frame->ip &&
        frame->ip < frame->lineno_upperbound
    ) {
        return frame->lineno_current_line;
    }

    long delta_lino = 0;
    long delta_line = 0;

    // Check if current offset isn't larger then the bytecode length.
    if (frame->lineno_current_lino_offset >= frame->codeblock->bytecode->lino_length) {
        return frame->lineno_current_line;
    }

    // Peek if the first entry is a 0, if so, the offset is actually negative. This is to compensate the fact
    // that sometimes byte code is generated on different places (for instance, in while() blocks)
    long negate = 0;
    if (frame->codeblock->bytecode->lino[frame->lineno_current_lino_offset] == 0) {
        frame->lineno_current_lino_offset++;
        negate = 1;
    }

    // Decompress delta linenumber. A lineno byte > 127 indicates the next block is also part of this.
    long i;
    do {
        i = (frame->codeblock->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_line += i;
    } while (i > 127);

    // Decompress delta lino (which bytecode offset marks the upper bound). A lineno byte > 127 indicates the
    // next block is also part of this.
    do {
        i = (frame->codeblock->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_lino += i;
    } while (i > 127);

    if (negate) {
        delta_line = 0 - delta_line;
    }

    frame->lineno_lowerbound = frame->lineno_upperbound;
    frame->lineno_upperbound += delta_lino;
    frame->lineno_current_line += delta_line;

    return frame->lineno_current_line;
}

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
 * Pops an object from the stack. If the resolve_attrib == 1 and object is an attribute object, fetch the actual
 * data of that attribute. Will error when the stack is empty.
 */
t_object *vm_frame_stack_pop(t_vm_stackframe *frame, int resolve_attrib) {
#if __DEBUG_STACK
    DEBUG_PRINT_CHAR(ANSI_BRIGHTYELLOW "STACK POP (%d): %08X '%s'{%d}\n" ANSI_RESET, frame->sp, (uintptr_t)frame->stack[frame->sp], object_debug(frame->stack[frame->sp]), ((t_object *)(frame->stack[frame->sp]))->ref_count);
#endif

    if (frame->sp >= frame->codeblock->bytecode->stack_size) {
        fatal_error(1, "Trying to pop from an empty stack");        /* LCOV_EXCL_LINE */
    }

    t_object *obj = frame->stack[frame->sp];
    frame->stack[frame->sp] = NULL;
    frame->sp++;


    if (resolve_attrib == 1 && OBJECT_IS_ATTRIBUTE(obj)) return ((t_attrib_object *)obj)->data.attribute;
    return obj;
}

/**
 * Pushes an object onto the stack. Errors when the stack is full
 */
void vm_frame_stack_push(t_vm_stackframe *frame, t_object *obj) {
#if __DEBUG_STACK
        DEBUG_PRINT_STRING_ARGS(ANSI_BRIGHTYELLOW "STACK PUSH(%d): %s %08lX {%d}\n" ANSI_RESET, frame->sp-1, object_debug(obj), (unsigned long)obj, obj->ref_count);
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
t_object *vm_frame_stack_fetch_top(t_vm_stackframe *frame, int resolve_attrib) {
    t_object *obj = frame->stack[frame->sp];

    if (resolve_attrib == 1 && OBJECT_IS_ATTRIBUTE(obj)) return ((t_attrib_object *)obj)->data.attribute;
    return obj;
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
void vm_frame_set_local_identifier(t_vm_stackframe *frame, char *class, t_object *new_obj) {
    t_vm_context *ctx = vm_frame_get_context(frame);
    char *fqcn = vm_context_create_fqcn_from_context(ctx, class);
    t_object *old_obj = (t_object *) ht_replace_str(frame->local_identifiers->data.ht, fqcn, new_obj);
    smm_free(fqcn);

    // Increase object before decreasing old object. Otherwise, the object might expire
    // in the mean time when the object has ref-count 1 and old_obj == new_obj.
    if (new_obj != NULL && new_obj != OBJECT_NEEDS_RESOLVING) {
        object_inc_ref(new_obj);
    }

    if (old_obj != NULL && old_obj != OBJECT_NEEDS_RESOLVING) {
        object_release(old_obj);
    }
}

void vm_frame_set_alias_identifier(t_vm_stackframe *frame, char *class, char *target_fqcn) {
    if (vm_frame_find_identifier(frame, class)) {
        fatal_error(1, "Alias %s already imported", class);
    }

    t_vm_context *ctx = vm_frame_get_context(frame);
    char *fqcn = vm_context_create_fqcn_from_context(ctx, class);

    // Store alias id under the FQCN id, and store that we still need to resolve the object
    vm_frame_set_local_identifier(frame, fqcn, OBJECT_NEEDS_RESOLVING);

    // Add to the alias table so we know that  alias -> \some\module\class
    char *val = string_strdup0(target_fqcn);
    ht_add_str(frame->object_aliases, fqcn, val);

    smm_free(fqcn);
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
    char *alias_qcn = ht_find_str(frame->object_aliases, fqcn);
    if (! alias_qcn) {
        // We need to resolve, but we don't know what.. Should not happen
        fatal_error(1, "Cannot find alias for '%s'\n", fqcn);
    }

    // Resolve and update the local identifiers
    t_object *obj = vm_class_resolve(frame, alias_qcn);
    return obj;
}

t_object *vm_frame_identifier_exists(t_vm_stackframe *frame, char *id) {
    t_object *obj;
    char *fqcn = NULL;
    t_exception_object *exception = thread_save_exception();

    /*
     *  Find in local identifiers (without FQCN)
     */
    obj = ht_find_str(frame->local_identifiers->data.ht, id);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        obj = _vm_frame_object_resolve(frame, id);
        if (obj) {
            vm_frame_set_local_identifier(frame, id, obj);
        }
    }
    if (obj || thread_exception_thrown()) goto done;

    fqcn = vm_context_create_fqcn_from_context(vm_frame_get_context(frame), id);

    /*
     *  Check local identifiers, but on FQCN
     */
    obj = ht_find_str(frame->local_identifiers->data.ht, fqcn);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        obj = _vm_frame_object_resolve(frame, fqcn);
        if (obj) {
            vm_frame_set_local_identifier(frame, fqcn, obj);
        }
    }
    if (obj || thread_exception_thrown()) goto done;


    /*
     * Check global identifiers
     */
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
    if (obj || thread_exception_thrown()) goto done;


    /*
     * Check global identifiers on FQCN
     */
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
    if (obj || thread_exception_thrown()) goto done;


    /*
     * Check built-ins, only on UQCN!
     */
    obj = ht_find_str(frame->builtin_identifiers->data.ht, id);
    if (obj == OBJECT_NEEDS_RESOLVING) {
        fatal_error(1, "A built-in object should always be resolved!");
    }
    if (obj || thread_exception_thrown()) goto done;

    // Nothing found :(
    obj = NULL;

done:
    //If an exception has been thrown during resolving, use that one
    if (thread_exception_thrown()) {
        thread_dump_exception(exception);
        obj = NULL;
    } else {
        // Otherwise restore any current exception
        thread_restore_exception(exception);
    }

    if (fqcn) smm_free(fqcn);
    return obj;
}

/**
 * Same as vm_frame_get_identifier, but does not halt on error (but returns NULL)
 */
t_object *vm_frame_find_identifier(t_vm_stackframe *frame, char *id) {
    if (! frame) return NULL;

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
    bzero(frame, sizeof(t_vm_stackframe));

    frame->parent = parent_frame;
    frame->codeblock = codeblock;

    frame->ip = 0;

    frame->trace_class = NULL;
    frame->trace_method = NULL;

    frame->sp = codeblock->bytecode->stack_size;
    frame->stack = smm_malloc(codeblock->bytecode->stack_size * sizeof(t_object *));
    bzero(frame->stack, codeblock->bytecode->stack_size * sizeof(t_object *));


    //    DEBUG_PRINT_CHAR("Increasing builtin_identifiers refcount\n");
    frame->builtin_identifiers = builtin_identifiers;
    object_inc_ref((t_object *)builtin_identifiers);

    // Create new empty local identifier hash
    DEBUG_PRINT_CHAR("Creating local ID table for frame %08x\n", frame);
    frame->local_identifiers = (t_hash_object *)object_alloc_instance(Object_Hash, 0);
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
    t_vm_context *ctx = vm_frame_get_context(frame);
    DEBUG_PRINT_CHAR("\n\n\n\n\n============================ STACKFRAME DESTROY: %s ================================\n", ctx ? ctx->module.full : "<root>");

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

//    // Remove codeblock reference (don't mind cleanup, since we still have it on the codeblock stack)
//    frame->codeblock = NULL;
    if (frame->trace_class) smm_free(frame->trace_class);
    if (frame->trace_method) smm_free(frame->trace_method);

    t_hash_iter iter;
    ht_iter_init_tail(&iter, frame->local_identifiers->data.ht);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key_str(&iter);
        t_object *val = ht_iter_value(&iter);

        // Fetch previous item before removing key from the hashtable
        ht_iter_prev(&iter);

        if (val == OBJECT_NEEDS_RESOLVING) continue;

        DEBUG_PRINT_STRING_ARGS("Frame destroy: Releasing => %s => %s [%p]\n", key, object_debug(val), val);

        // Release values, as it's no longer needed.
        object_release(val);

        ht_remove_str(frame->local_identifiers->data.ht, key);
    }

    // Free identifiers
    object_release((t_object *)frame->global_identifiers);
    object_release((t_object *)frame->local_identifiers);
    object_release((t_object *)frame->builtin_identifiers);


    if (frame->parent == NULL) {
        // If we are the lowest frame, remove object aliases
        ht_iter_init(&iter, frame->object_aliases);
        while (ht_iter_valid(&iter)) {
            char *val = ht_iter_value(&iter);
            smm_free(val);
            ht_iter_next(&iter);
        }
        ht_destroy(frame->object_aliases);
    }

    smm_free(frame->stack);

    smm_free(frame);
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


t_vm_stackframe *vm_create_empty_stackframe(void) {
    t_bytecode *bytecode = smm_malloc(sizeof(t_bytecode));
    bytecode->stack_size = 142;
    bytecode->code_len = 0;
    bytecode->code = NULL;
    bytecode->constants_len = 0;
    bytecode->constants = NULL;
    bytecode->identifiers_len = 0;
    bytecode->identifiers = NULL;
    bytecode->lino_offset = 0;
    bytecode->lino_length = 0;
    bytecode->lino = NULL;

    t_vm_context *ctx = vm_context_new("\\", "");
    t_vm_codeblock *codeblock = vm_codeblock_new(bytecode, ctx);
    return vm_stackframe_new(NULL, codeblock);
}
