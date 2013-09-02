/*
 Copyright (c) 2012-2013, The Saffire Group
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
#include "vm/frame.h"
#include "vm/context.h"
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


/**
 * Returns the next opcode
 */
unsigned char vm_frame_get_next_opcode(t_vm_frame *frame) {
    // Sanity stop
    if (frame->ip >= frame->bytecode->code_len) {
        DEBUG_PRINT("Running outside bytecode!\n\n\n");
        return VM_STOP;
    }

    unsigned char op = frame->bytecode->code[frame->ip];
    frame->ip++;

    return op;
}


/**
 * Returns he next operand. Does not do any sanity checks if it actually is an operand.
 */
unsigned int vm_frame_get_operand(t_vm_frame *frame) {
    // Read operand
    uint16_t *ptr = (uint16_t *)(frame->bytecode->code + frame->ip);
    unsigned int ret = (*ptr & 0xFFFF);

    frame->ip += sizeof(uint16_t);
    return ret;
}


/**
 * Pops an object from the stack. Errors when the stack is empty
 */
t_object *vm_frame_stack_pop(t_vm_frame *frame) {
    DEBUG_PRINT(ANSI_BRIGHTYELLOW "STACK POP (%d): %08lX %s\n" ANSI_RESET, frame->sp, (unsigned long)frame->stack[frame->sp], object_debug(frame->stack[frame->sp]));

    if (frame->sp >= frame->bytecode->stack_size) {
        fatal_error(1, "Trying to pop from an empty stack");
    }
    t_object *ret = frame->stack[frame->sp];
    frame->stack[frame->sp] = NULL;
    frame->sp++;


    object_dec_ref(ret);
    return ret;
}


/**
 * Pushes an object onto the stack. Errors when the stack is full
 */
void vm_frame_stack_push(t_vm_frame *frame, t_object *obj) {
    DEBUG_PRINT(ANSI_BRIGHTYELLOW "STACK PUSH(%d): %s %08lX \n" ANSI_RESET, frame->sp-1, object_debug(obj), (unsigned long)obj);


    if (frame->sp < 0) {
        fatal_error(1, "Trying to push to a full stack");
    }
    frame->sp--;
    frame->stack[frame->sp] = obj;
    object_inc_ref(obj);
}

void vm_frame_stack_modify(t_vm_frame *frame, int idx, t_object *obj) {
    DEBUG_PRINT(ANSI_BRIGHTYELLOW "STACK CHANGE(%d): %s %08lX \n" ANSI_RESET, idx, object_debug(obj), (unsigned long)obj);
    frame->stack[idx] = obj;
}


/**
 * Fetches the top of the stack. Does not pop anything.
 */
t_object *vm_frame_stack_fetch_top(t_vm_frame *frame) {
    return frame->stack[frame->sp];
}


/**
 * Fetches a non-top element form the stack. Does not pop anything.
 */
t_object *vm_frame_stack_fetch(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->stack_size) {
        fatal_error(1, "Trying to fetch from outside stack range");
    }

    return frame->stack[idx];
}


/**
 * Return a constant literal, without converting to an object
 */
void *vm_frame_get_constant_literal(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->constants_len) {
        fatal_error(1, "Trying to fetch from outside constant range");
    }

    t_bytecode_constant *c = frame->bytecode->constants[idx];
    return c->data.ptr;
}


/**
 * Returns an object from the constant table
 */
t_object *vm_frame_get_constant(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->constants_len) {
        fatal_error(1, "Trying to fetch from outside constant range");
    }

    return frame->constants_objects[idx];
}


/**
 * Store object into the global identifier table. When obj == NULL, it will remove the actual reference (plus object)
 */
void vm_frame_set_global_identifier(t_vm_frame *frame, char *id, t_object *obj) {
    t_object *key = object_alloc(Object_String, 1, id);

    if (obj == NULL) {
        t_object *old = ht_remove_obj(frame->global_identifiers->ht, key);
        object_release(key);

        if (old) object_release(old);
        return;
    }

    ht_add_obj(frame->global_identifiers->ht, key, obj);
    object_inc_ref(obj);
}


/**
* Return object from the global identifier table
*/
t_object *vm_frame_get_global_identifier(t_vm_frame *frame, char *id) {
    t_object *key = object_alloc(Object_String, 1, id);

    t_object *obj = (t_object *)ht_find_obj(frame->global_identifiers->ht, key);

    object_release(key);
    if (obj == NULL) RETURN_NULL;
    return obj;
}


/**
 * Store object into either the local or global identifier table
 */
void vm_frame_set_identifier(t_vm_frame *frame, char *id, t_object *obj) {
    t_object *old_obj = (t_object *)ht_replace_obj(frame->local_identifiers->ht, object_alloc(Object_String, 1, id), obj);
    object_release(old_obj);
    object_inc_ref(obj);
}

void vm_frame_set_builtin_identifier(t_vm_frame *frame, char *id, t_object *obj) {
    t_object *old_obj = ht_replace_obj(frame->builtin_identifiers->ht, object_alloc(Object_String, 1, id), obj);

    object_release(old_obj);
    object_inc_ref(obj);
}


/**
 * Return object from either the local or the global identifier table
 */
t_object *vm_frame_get_identifier(t_vm_frame *frame, char *id) {
    DEBUG_PRINT("vm_frame_get_identifier(%s)\n", id);
    t_object *obj = vm_frame_find_identifier(frame, id);
    return obj;
}


#ifdef __DEBUG
void print_debug_table(t_hash_table *ht, char *prefix) {
    t_hash_iter iter;

    if (! ht) return;

    ht_iter_init(&iter, ht);
    while (ht_iter_valid(&iter)) {
        t_object *key = ht_iter_key_obj(&iter);
        t_object *val = ht_iter_value(&iter);
        printf("%-10s KEY: [%08X] '%-40s'{%d} ", prefix, (unsigned int)key, object_debug(key), key->ref_count);
        printf("=> [%08X] %s{%d}\n", (unsigned int)val, object_debug(val), val->ref_count);

        ht_iter_next(&iter);
    }

}
#endif


/**
 * Same as get, but does not halt on error (but returns NULL)
 */
t_object *vm_frame_find_identifier(t_vm_frame *frame, char *id) {
    t_object *obj;

    t_object *key = object_alloc(Object_String, 1, id);

    // Check locals first
    obj = ht_find_obj(frame->local_identifiers->ht, key);
    if (obj != NULL) {
        object_release(key);
        return obj;
    }

    // Check globals
    obj = ht_find_obj(frame->global_identifiers->ht, key);
    if (obj != NULL) {
        object_release(key);
        return obj;
    }

    // Last, check builtins
    obj = ht_find_obj(frame->builtin_identifiers->ht, key);
    if (obj != NULL) {
        object_release(key);
        return obj;
    }

    // @TODO: We should throw an exception instead of just returning

    DEBUG_PRINT("VM_FRAME_FIND_IDENTIFIER(%08X): '%s' NOT FOUND!\n", frame, id);

//    DEBUG_PRINT("Builtimns\n");
//    print_debug_table(frame->builtin_identifiers->ht);

    object_release(key);
    return NULL;
}



/**
 * Returns an identifier name as string
 */
char *vm_frame_get_name(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->identifiers_len) {
        fatal_error(1, "Trying to fetch from outside identifier range");
    }
    return frame->bytecode->identifiers[idx]->s;
}


void vm_detach_bytecode(t_vm_frame *frame) {
    if (frame->bytecode == NULL) return;

    vm_context_free_context(frame);

    smm_free(frame->stack);

    printf("vm_detach_bytecode: freeing constants init\n");
    // Free constants objects
    for (int i=0; i!=frame->bytecode->constants_len; i++) {
        printf("Freeing: %s\n", object_debug((t_object *)frame->constants_objects[i]));
        object_release((t_object *)frame->constants_objects[i]);
    }
    printf("vm_detach_bytecode: freeing constants fini\n");
    smm_free(frame->constants_objects);

    frame->bytecode = NULL;
}


extern t_dll *all_objects;


void vm_attach_bytecode(t_vm_frame *frame, char *context, t_bytecode *bytecode) {
    if (frame->context) {
        vm_context_free_context(frame);
    }
    vm_context_set_context(frame, context);

    frame->bytecode = bytecode;
    frame->ip = 0;
    frame->sp = bytecode->stack_size;

    // Setup variable stack
    frame->stack = smm_malloc(bytecode->stack_size * sizeof(t_object *));
    bzero(frame->stack, bytecode->stack_size * sizeof(t_object *));

    // Create constants @TODO: Rebuild on every frame (ie: method call?). Can we reuse them?
    frame->constants_objects = smm_malloc(bytecode->constants_len * sizeof(t_object *));
    for (int i=0; i!=bytecode->constants_len; i++) {
        t_object *obj;
        t_bytecode_constant *c = bytecode->constants[i];
        switch (c->type) {
            case BYTECODE_CONST_CODE :
//                object_release(obj);    // Decrease NULL object usage

                // We create a reference to the source filename of the original bytecode name.
                bytecode->constants[i]->data.code->source_filename = smm_strdup(bytecode->source_filename);
                obj = object_alloc(Object_Callable, 3, CALLABLE_CODE_EXTERNAL, bytecode->constants[i]->data.code, /* arguments */ NULL);
                break;
            case BYTECODE_CONST_STRING :
//                object_release(obj);    // Decrease NULL object usage

                obj = object_alloc(Object_String, 1, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
//                object_release(obj);    // Decrease NULL object usage

                obj = object_alloc(Object_Numerical, 1, bytecode->constants[i]->data.l);
                break;
            default :
                fatal_error(1, "Cannot convert constant type into object!");
                break;
        }
        frame->constants_objects[i] = obj;
    }



//    printf("\n\n---- AttachBytecode ----\n");
//    t_dll_element *e = DLL_HEAD(all_objects);
//    while (e) {
//        t_object *obj = (t_object *)e->data;
//        printf("%-20s %d (%s)\n", obj->name, obj->ref_count, object_debug(obj));
//        e = DLL_NEXT(e);
//    }
//    printf("----------------------------\n");
}

/**
* Creates and initializes a new frame
*/
t_vm_frame *vm_frame_new(t_vm_frame *parent_frame, char *context, t_bytecode *bytecode) {
    DEBUG_PRINT("\n\n\n\n\n============================ VM frame new ('%s' -> parent: '%s') ============================\n", context, parent_frame ? parent_frame->context : "none");
    t_vm_frame *frame = smm_malloc(sizeof(t_vm_frame));
    bzero(frame, sizeof(t_vm_frame));

    frame->parent = parent_frame;


    frame->created_objects = dll_init();

    printf("Increasing builtin_identifiers refcount\n");
    frame->builtin_identifiers = builtin_identifiers;
    object_inc_ref((t_object *)builtin_identifiers);

    vm_context_set_context(frame, context);

    frame->bytecode = NULL;

    // Create new local identifier hash
    frame->local_identifiers = (t_hash_object *)object_alloc(Object_Hash, 0);

    // By default, don't create file identifiers
    frame->constants_objects = NULL;

    // Set the variable hashes
    if (frame->parent == NULL) {
        // global identifiers are the same as the local identifiers for the initial frame
        frame->global_identifiers = frame->local_identifiers;
        //ht_add_obj(frame->local_identifiers->ht, object_alloc(Object_String, 1, "superglobalvar"), object_alloc(Object_Null, 0));
    } else {
        // if not the initial frame, link globals from the parent frame
        frame->global_identifiers = frame->parent->global_identifiers;
    }
    object_inc_ref((t_object *)frame->global_identifiers);

    if (bytecode) {
        vm_attach_bytecode(frame, context, bytecode);
    }


//#ifdef __DEBUG
//    DEBUG_PRINT("----- [START FRAME: %s (%08X)] ----\n", frame->context, frame);
//    if (frame->local_identifiers) print_debug_table(frame->local_identifiers->ht, "Locals");
//    if (frame->global_identifiers) print_debug_table(frame->global_identifiers->ht, "Globals");
//    if (frame->builtin_identifiers) print_debug_table(frame->builtin_identifiers->ht, "Builtins");
//#endif


    return frame;
}

/**
 *
 */
void vm_frame_destroy(t_vm_frame *frame) {
    printf("FRAME DESTROY: %s\n", frame->context);

#ifdef __DEBUG
    printf("----- [END FRAME: %s (%08X)] ----\n", frame->context, (unsigned int)frame);
    if (frame->local_identifiers) print_debug_table(frame->local_identifiers->ht, "Locals");
    if (frame->global_identifiers) print_debug_table(frame->global_identifiers->ht, "Globals");
//    if (frame->builtin_identifiers) print_debug_table(frame->builtin_identifiers->ht, "Builtins");
#endif


    printf("detach bytecode init\n");
    if (frame->bytecode) {
        vm_detach_bytecode(frame);
    }
    printf("detach bytecode fini\n");

    t_hash_iter iter;
    ht_iter_init(&iter, frame->local_identifiers->ht);
    while (ht_iter_valid(&iter)) {
        t_object *key = ht_iter_key_obj(&iter);
        t_object *val = ht_iter_value(&iter);

        // Because we MIGHT change the hash-table, we need to fetch the next
        // element PRIOR to changing the table. Otherwise we might end up in
        // the crapper.
        ht_iter_next(&iter);
        printf("Frame destroy: Releasing => %s [%08X]\n", object_debug(val), (unsigned int)val);

        object_release(val);
        object_release(key);
        ht_remove_obj(frame->local_identifiers->ht, key);
    }

    // Free created objects
    t_dll_element *e = DLL_HEAD(frame->created_objects);
    while (e) {
        object_release((t_object *)e->data);
        e = DLL_NEXT(e);
    }
    dll_free(frame->created_objects);

    // Free identifiers
    object_release((t_object *)frame->global_identifiers);
    object_release((t_object *)frame->local_identifiers);
//    printf("Decreasing builtin_identifiers refcount\n");
    object_release((t_object *)frame->builtin_identifiers);

    smm_free(frame->context);

    smm_free(frame);
}


void vm_frame_add_created_object(t_vm_frame *frame, t_object *obj) {
    dll_append(frame->created_objects, obj);
}


#ifdef __DEBUG
void vm_frame_stack_debug(t_vm_frame *frame) {
    if (frame->sp == frame->bytecode->stack_size) {
        //printf("\nEmpty framestack\n");
        return;
    }

    printf("\nFRAME STACK\n");
    printf("=======================\n");
    for (int i=frame->sp; i<=frame->bytecode->stack_size-1; i++) {
        printf("  %s%02d %08X %s\n", (i == frame->sp - 1) ? ">" : " ", i, (unsigned int)frame->stack[i], frame->stack[i] ? object_debug(frame->stack[i]) : "");
    }
    printf("\n");
}
#endif
