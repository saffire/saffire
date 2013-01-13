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
     * Neither the name of the <organization> nor the
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
    DEBUG_PRINT("STACK POP(%d): %08lX %s\n", frame->sp, (unsigned long)frame->stack[frame->sp], object_debug(frame->stack[frame->sp]));

    if (frame->sp >= frame->bytecode->stack_size) {
        error_and_die(1, "Trying to pop from an empty stack");
    }
    t_object *ret = frame->stack[frame->sp];
    frame->stack[frame->sp] = NULL;
    frame->sp++;

    return ret;
}


/**
 * Pushes an object onto the stack. Errors when the stack is full
 */
void vm_frame_stack_push(t_vm_frame *frame, t_object *obj) {
    DEBUG_PRINT("DBG PUSH: %s %08lX \n", object_debug(obj), (unsigned long)obj);

    if (frame->sp < 0) {
        error_and_die(1, "Trying to push to a full stack");

    }
    frame->sp--;
    frame->stack[frame->sp] = obj;

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
        error_and_die(1, "Trying to fetch from outside stack range");
    }

    return frame->stack[idx];
}


/**
 * Return a constant literal, without converting to an object
 */
void *vm_frame_get_constant_literal(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->constants_len) {
        error_and_die(1, "Trying to fetch from outside constant range");
    }

    t_bytecode_constant *c = frame->bytecode->constants[idx];
    return c->data.ptr;
}


/**
 * Returns an object from the constant table
 */
t_object *vm_frame_get_constant(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->constants_len) {
        error_and_die(1, "Trying to fetch from outside constant range");
    }

    return frame->constants_objects[idx];
}


/**
 * Store object into the global identifier table
 */
void vm_frame_set_global_identifier(t_vm_frame *frame, char *id, t_object *obj) {
    if (obj == NULL) {
        ht_remove(frame->global_identifiers->ht, id);
    } else {
        ht_add(frame->global_identifiers->ht, id, obj);
    }
}


/**
 * Return object from the global identifier table
 */
t_object *vm_frame_get_global_identifier(t_vm_frame *frame, int idx) {
    t_object *obj = ht_num_find(frame->global_identifiers->ht, idx);
    if (obj == NULL) RETURN_NULL;
    return obj;
}


/**
 * Store object into either the local or global identifier table
 */
void vm_frame_set_identifier(t_vm_frame *frame, char *id, t_object *obj) {
    t_object *old_obj = ht_replace(frame->local_identifiers->ht, id, obj);
    if (old_obj) {
        object_dec_ref((t_object *)old_obj);
    }
}


/**
 * Return object from either the local or the global identifier table
 */
t_object *vm_frame_get_identifier(t_vm_frame *frame, char *id) {
    DEBUG_PRINT("vm_frame_get_identifier(%s)\n", id);
    t_object *obj = vm_frame_find_identifier(frame, id);
    if (obj != NULL) return obj;

    error_and_die(1, "Cannot find attribute: %s\n", id);
    return NULL;
}

/**
 * Same as get, but does not halt on error (but returns NULL)
 */
t_object *vm_frame_find_identifier(t_vm_frame *frame, char *id) {
    t_object *obj;


//#ifdef __DEBUG
//    t_hash_iter iter;
//    ht_iter_init(&iter, frame->local_identifiers->ht);
//    while (ht_iter_valid(&iter)) {
//        char *k = ht_iter_key(&iter);
//        t_object *v = ht_iter_value(&iter);
//        DEBUG_PRINT("  K: %-20s %s\n", k, object_debug(v));
//        ht_iter_next(&iter);
//    }
//#endif

    // Check locals first
    obj = ht_find(frame->local_identifiers->ht, id);
    if (obj != NULL) return obj;

    // If file identifiers are present, check them
    if (frame->file_identifiers) {
        obj = ht_find(frame->file_identifiers->ht, id);
        if (obj != NULL) return obj;
    }

    // Check globals
    obj = ht_find(frame->global_identifiers->ht, id);
    if (obj != NULL) return obj;

    // Last, check builtins
    obj = ht_find(frame->builtin_identifiers->ht, id);
    if (obj != NULL) return obj;

    // @TODO: We should throw an exception instead of just returning
    return NULL;
}


/**
 * Returns an identifier name as string
 */
char *vm_frame_get_name(t_vm_frame *frame, int idx) {
    if (idx < 0 || idx >= frame->bytecode->identifiers_len) {
        error_and_die(1, "Trying to fetch from outside identifier range");
    }
    return frame->bytecode->identifiers[idx]->s;
}



/**
* Creates and initializes a new frame
*/
t_vm_frame *vm_frame_new(t_vm_frame *parent_frame, t_bytecode *bytecode) {
    t_vm_frame *cfr = smm_malloc(sizeof(t_vm_frame));
    bzero(cfr, sizeof(t_vm_frame));

    cfr->parent = parent_frame;
    cfr->bytecode = bytecode;
    cfr->ip = 0;
    cfr->sp = bytecode->stack_size;

    // Setup variable stack
    cfr->stack = smm_malloc(bytecode->stack_size * sizeof(t_object *));
    bzero(cfr->stack, bytecode->stack_size * sizeof(t_object *));

    // Set the variable hashes
    if (parent_frame == NULL) {
        // Initial frame, so create a new global identifier hash
        cfr->global_identifiers = (t_hash_object *)object_new(Object_Hash, 0);
        cfr->local_identifiers = cfr->global_identifiers;
    } else {
        // otherwise link globals from the parent
        cfr->global_identifiers = parent_frame->global_identifiers;
        // And create new local identifier hash
        cfr->local_identifiers = (t_hash_object *)object_new(Object_Hash, 0);

        // By default, don't create file identifiers
        cfr->file_identifiers = NULL;
    }

    // Create builtin-identifiers
    cfr->builtin_identifiers = (t_hash_object *)object_new(Object_Hash, 1, builtin_identifiers);

    // Create constants @TODO: Rebuild on every frame (ie: method call?). Can we reuse them?
    cfr->constants_objects = smm_malloc(bytecode->constants_len * sizeof(t_object *));
    for (int i=0; i!=bytecode->constants_len; i++) {
        t_object *obj = Object_Null;

        t_bytecode_constant *c = bytecode->constants[i];
        switch (c->type) {
            case BYTECODE_CONST_CODE :
                obj = object_new(Object_Callable, 4, CALLABLE_CODE_EXTERNAL, bytecode->constants[i]->data.code, NULL, NULL);
                break;
            case BYTECODE_CONST_STRING :
                obj = object_new(Object_String, 1, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                obj = object_new(Object_Numerical, 1, bytecode->constants[i]->data.l);
                break;
            default :
                error_and_die(1, "Cannot convert constant type into object!");
                break;
        }
        object_inc_ref(obj);
        cfr->constants_objects[i] = obj;
    }

    return cfr;
}

/**
 *
 */
void vm_frame_destroy(t_vm_frame *frame) {
    // @TODO: Remove identifiers in the local_identifiers hash object
    object_free((t_object *)frame->local_identifiers);

    // Destroy global identifiers when this frame is the initial one
    if (! frame->parent) {
        // @TODO: We should free global id's, but this results in errors
        //object_free((t_object *)frame->global_identifiers);
    }

    // @TODO: remove constants objects.

    // @TODO: Should we unwind the stack first
    smm_free(frame->stack);
    smm_free(frame);
}


#ifdef __DEBUG
void vm_frame_stack_debug(t_vm_frame *frame) {
    printf("\nFRAME STACK\n");
    printf("=======================\n");
    for (int i=0; i!=frame->bytecode->stack_size; i++) {
        printf("  %s%02d %08X %s\n", (i == frame->sp - 1) ? ">" : " ", i, (unsigned int)frame->stack[i], frame->stack[i] ? object_debug(frame->stack[i]) : "");
    }
    printf("\n");
}
#endif
