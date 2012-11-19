/*
 Copyright (c) 2012, The Saffire Group
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
#include "compiler/bytecode.h"
#include "vm/vm_opcodes.h"
#include "vm/frame.h"
#include "vm/block.h"
#include "general/dll.h"
#include "general/smm.h"
#include "objects/object.h"
#include "objects/boolean.h"
#include "objects/string.h"
#include "objects/numerical.h"
#include "objects/hash.h"
#include "objects/null.h"
#include "interpreter/errors.h"
#include "modules/module_api.h"

#define OBJ2STR(_obj_) smm_strdup(((t_string_object *)_obj_)->value)

#define REASON_RETURN       0
#define REASON_CONTINUE     1
#define REASON_BREAK        2
#define REASON_BREAKELSE    3

/**
 *
 */
static void saffire_vm_warning(char *str, ...) {
    va_list args;
    va_start(args, str);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, str, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/**
 *
 */
static void saffire_vm_error(char *str, ...) {
    va_list args;
    va_start(args, str);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, str, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}


#define CALL_OP(opr, in_place, err_str) \
                obj1 = vm_frame_stack_pop(frame); \
                object_dec_ref(obj1); \
                obj2 = vm_frame_stack_pop(frame); \
                object_dec_ref(obj2); \
                \
                if (obj1->type != obj2->type) { \
                    saffire_vm_error(err_str); \
                } \
                obj3 = object_operator(obj2, opr, in_place, 1, obj1); \
                \
                object_inc_ref(obj3);   \
                vm_frame_stack_push(frame, obj3);


//#define NOT_IMPLEMENTED  printf("opcode %d is not implemented yet", opcode); exit(1); break;

t_hash_object *global_identifiers = NULL;
t_hash_object *builtin_identifiers = NULL;

/**
 *
 */
void vm_init(void) {
    object_init();
    builtin_identifiers = (t_hash_object *)object_new(Object_Hash);
    module_init();
}


/**
 *
 */
static t_object *_import(t_vm_frame *frame, char *module, char *class) {
    char tmp[100];
    snprintf(tmp, 99, "%s::%s", module, class);

    t_object *obj = vm_frame_get_identifier(frame, tmp);
    return obj;
}


/**
 * Calls a method by creating a new frame.
 */
t_object *vm_call_method(t_object *object, t_object *method, t_dll *args) {
    t_vm_frame *method_frame = vm_frame_create(method->bytecode);

    t_object *vm_execute(method_frame);

    vm_push_block(frame, BLOCK_TYPE_FUNCTION, frame->ip + oparg1, frame->sp);

    obj2

    // Call object and push result onto stack
    obj3 = object_call_args(obj1, obj2, args);

    vm_pop_block();
}


/**
 *
 */
t_object *vm_execute(t_vm_frame *frame) {
    register t_object *obj1, *obj2, *obj3, *obj4, *ret;
    register unsigned int opcode, oparg1, oparg2;
    register char *s1;
    int reason;
    t_vm_frame *tfr;

    // If we don't have a global_identifier list yet, set this so next frames will use it as it's globals
    if (! global_identifiers) {
        global_identifiers = tfr->local_identifiers;
    }

    for (;;) {
        // Room for some other stuff
dispatch:
        // Increase number of executions done
        frame->executions++;


        unsigned long cip = frame->ip;

        // Get opcode and additional argument
        opcode = vm_frame_get_next_opcode(frame);
        if (opcode == VM_STOP) break;
        if (opcode == VM_RESERVED) {
            saffire_vm_error("VM: Reached reserved (0xFF) opcode. Halting.\n");
        }

        // If high bit is set, get operand
        oparg1 = ((opcode & 0x80) == 0x80) ? vm_frame_get_operand(frame) : 0;
        oparg2 = ((opcode & 0xC0) == 0xC0) ? vm_frame_get_operand(frame) : 0;

#ifdef __DEBUG
        if ((opcode & 0xC0) == 0xC0) {
            printf("%08lX : 0x%02X (0x%02X, 0x%02X)\n", cip, opcode, oparg1, oparg2);
        } else if ((opcode & 0x80) == 0x80) {
            printf("%08lX : 0x%02X (0x%02X)\n", cip, opcode, oparg1);
        } else {
            printf("%08lX : 0x%02X\n", cip, opcode);
        }
#endif

        switch (opcode) {
            // Removes SP-0
            case VM_POP_TOP :
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                goto dispatch;
                break;

            // Rotate / swap SP-0 and SP-1
            case VM_ROT_TWO :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-2
            case VM_ROT_THREE :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                obj3 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                vm_frame_stack_push(frame, obj3);
                goto dispatch;
                break;

            // Duplicate SP-0
            case VM_DUP_TOP :
                obj1 = vm_frame_stack_fetch_top(frame);
                object_inc_ref(obj1);
                vm_frame_stack_push(frame, obj1);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-3
            case VM_ROT_FOUR :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                obj3 = vm_frame_stack_pop(frame);
                obj4 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                vm_frame_stack_push(frame, obj3);
                vm_frame_stack_push(frame, obj4);
                goto dispatch;
                break;

            // No operation
            case VM_NOP :
                goto dispatch;
                break;

            // Load a global identifier
            case VM_LOAD_GLOBAL :
                obj1 = vm_frame_get_global_identifier(frame, oparg1);
                object_inc_ref(obj1);
                vm_frame_stack_push(frame, obj1);
                goto dispatch;
                break;

            // store SP+0 as a global identifier
            case VM_STORE_GLOBAL :
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                s1 = vm_frame_get_name(frame, oparg1);
                vm_frame_set_global_identifier(frame, s1, obj1);
                goto dispatch;
                break;

            // Remove global identifier
            case VM_DELETE_GLOBAL :
                s1 = vm_frame_get_name(frame, oparg1);
                vm_frame_set_global_identifier(frame, s1, NULL);
                goto dispatch;
                break;

            // Load and push constant onto stack
            case VM_LOAD_CONST :
                obj1 = vm_frame_get_constant(frame, oparg1);
                object_inc_ref(obj1);
                vm_frame_stack_push(frame, obj1);
                goto dispatch;
                break;

            // Store SP+0 into identifier (either local or global)
            case VM_STORE_ID :
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                s1 = vm_frame_get_name(frame, oparg1);
                vm_frame_set_identifier(frame, s1, obj1);
                goto dispatch;
                break;
                // @TODO: If string(obj1) exists in local store it there, otherwise, store in global

            // Load and push identifier onto stack (either local or global)
            case VM_LOAD_ID :
                s1 = vm_frame_get_name(frame, oparg1);
                obj1 = vm_frame_get_identifier(frame, s1);
                object_inc_ref(obj1);
                vm_frame_stack_push(frame, obj1);
                goto dispatch;
                break;

            //
            case VM_BINARY_ADD :
                CALL_OP(OPERATOR_ADD, 0, "Can only add equal types");
                goto dispatch;
                break;

            case VM_BINARY_SUB :
                CALL_OP(OPERATOR_SUB, 0, "Can only subtract equal types");
                goto dispatch;
                break;

            case VM_BINARY_MUL :
                CALL_OP(OPERATOR_MUL, 0, "Can only multiply equal types");
                goto dispatch;
                break;

            case VM_BINARY_DIV :
                CALL_OP(OPERATOR_DIV, 0, "Can only divide equal types");
                goto dispatch;
                break;

            case VM_BINARY_SHL :
                CALL_OP(OPERATOR_SHL, 0, "Can only shift equal types");
                goto dispatch;
                break;

            case VM_BINARY_SHR :
                CALL_OP(OPERATOR_SHR, 0, "Can only shift equal types");
                goto dispatch;
                break;

            case VM_BINARY_OR :
                CALL_OP(OPERATOR_OR, 0, "Can only 'binary or' equal types");
                goto dispatch;
                break;

            case VM_BINARY_AND :
                CALL_OP(OPERATOR_AND, 0, "Can only 'binary and' equal types");
                goto dispatch;
                break;

            case VM_BINARY_XOR :
                CALL_OP(OPERATOR_XOR, 0, "Can only 'binary xor' equal types")
                goto dispatch;
                break;
            case VM_INPLACE_ADD :
                CALL_OP(OPERATOR_ADD, 1, "Can only add equal types");
                goto dispatch;
                break;

            case VM_INPLACE_SUB :
                CALL_OP(OPERATOR_SUB, 1, "Can only subtract equal types");
                goto dispatch;
                break;

            case VM_INPLACE_MUL :
                CALL_OP(OPERATOR_MUL, 1, "Can only multiply equal types");
                goto dispatch;
                break;

            case VM_INPLACE_DIV :
                CALL_OP(OPERATOR_DIV, 1, "Can only divide equal types");
                goto dispatch;
                break;

            case VM_INPLACE_SHL :
                CALL_OP(OPERATOR_SHL, 1, "Can only shift equal types");
                goto dispatch;
                break;

            case VM_INPLACE_SHR :
                CALL_OP(OPERATOR_SHR, 1, "Can only shift equal types");
                goto dispatch;
                break;

            case VM_INPLACE_OR :
                CALL_OP(OPERATOR_OR, 1, "Can only 'binary or' equal types");
                goto dispatch;
                break;

            case VM_INPLACE_AND :
                CALL_OP(OPERATOR_AND, 1, "Can only 'binary and' equal types");
                goto dispatch;
                break;

            case VM_INPLACE_XOR :
                CALL_OP(OPERATOR_XOR, 1, "Can only 'binary xor' equal types")
                goto dispatch;
                break;

            // Unconditional relative jump forward
            case VM_JUMP_FORWARD :
                frame->ip += oparg1;
                goto dispatch;
                break;

            // Conditional jump on SP-0 is true
            case VM_JUMP_IF_TRUE :
                obj1 = vm_frame_stack_pop(frame);
                if (! OBJECT_IS_BOOLEAN(obj1)) {
                    // Cast to boolean
                    obj2 = object_find_method(obj1, "boolean");
                    obj1 = object_call(obj1, obj2, 0);
                }

                if (IS_TRUE(obj1)) {
                    frame->ip += oparg1;
                }

                goto dispatch;
                break;

            // Conditional jump on SP-0 is false
            case VM_JUMP_IF_FALSE :
                obj1 = vm_frame_stack_pop(frame);
                if (! OBJECT_IS_BOOLEAN(obj1)) {
                    // Cast to boolean
                    obj2 = object_find_method(obj1, "boolean");
                    obj1 = object_call(obj1, obj2, 0);
                }

                if (IS_FALSE(obj1)) {
                    frame->ip += oparg1;
                }
                goto dispatch;
                break;

            // Unconditional absolute jump
            case VM_JUMP_ABSOLUTE :
                frame->ip = oparg1;
                goto dispatch;
                break;

            // Duplicates the SP+0 a number of times
            case VM_DUP_TOPX :
                obj1 = vm_frame_stack_fetch_top(frame);
                for (int i=0; i!=oparg1; i++) {
                    object_inc_ref(obj1);
                    vm_frame_stack_push(frame, obj1);
                }
                goto dispatch;
                break;

            // Calls method OP+0 SP+0 from object SP+1 with OP+1 args starting from SP+2.
            case VM_CALL_METHOD :
                obj1 = vm_frame_stack_pop(frame);   // Self
                obj2 = object_find_method(obj1, (char *)vm_frame_get_constant_literal(frame, oparg1));

                // TODO: Maybe this should just be a tuple?
                // Create argument list
                t_dll *args = dll_init();
                for (int i=0; i!=oparg2; i++) {
                    obj3 = vm_frame_stack_pop(frame);
                    object_dec_ref(obj3);
                    dll_append(args, obj3);
                }

                obj3 = vm_call_method(obj1, obj2, args);

                object_inc_ref(obj3);
                vm_frame_stack_push(frame, obj3);

                dll_free(args);

                goto dispatch;
                break;

            // Import X as Y from Z
            case VM_IMPORT :
                // Fetch the module to import
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                char *module = OBJ2STR(obj1);

                // Fetch class
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                char *class = OBJ2STR(obj1);

                //printf("Importing %s from %s\n", class, module);

                obj3 = _import(frame, module, class);
                object_inc_ref(obj3);
                vm_frame_stack_push(frame, obj3);

                goto dispatch;
                break;


            case VM_SETUP_LOOP :
                tfr = dup_frame(oparg1);
                vm_push_block(frame, BLOCK_TYPE_LOOP, frame->ip + oparg1, frame->sp);
                push_frame(tfr, "loop");
                goto dispatch;
                break;
            case VM_POP_BLOCK :
                pop_frame();
                goto dispatch;
                break;

            case VM_CONTINUE_LOOP :
                reason = REASON_CONTINUE;
                goto block_end;
                break;
            case VM_BREAK_LOOP :
                reason = REASON_BREAK;
                goto block_end;
                break;
            case VM_BREAKELSE_LOOP :
                reason = REASON_BREAKELSE;
                goto block_end;
                break;

            case VM_COMPARE_OP :
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                obj2 = vm_frame_stack_pop(frame);
                object_dec_ref(obj2);

                if (obj1->type != obj2->type) {
                    saffire_vm_error("Cannot compare non-identical object types");
                }
                obj3 = object_comparison(obj1, oparg1, obj2);

                object_inc_ref(obj3);
                vm_frame_stack_push(frame, obj3);
                goto dispatch;
                break;

            case VM_SETUP_FINALLY :
                vm_create_block(frame, BLOCK_TYPE_FINALLY, frame->ip + oparg1, frame->sp);
                tfr = dup_frame(oparg1);
                push_frame(tfr, "finally");
                goto dispatch;
                break;
            case VM_SETUP_EXCEPT :
                vm_create_block(frame, BLOCK_TYPE_EXCEPTION, frame->ip + oparg1, frame->sp);
                goto dispatch;
                break;
            case VM_END_FINALLY :
                vm_create_block(frame, BLOCK_TYPE_END_FINALLY, frame->ip + oparg1, frame->sp);
                goto dispatch;
                break;


            case VM_BUILD_CLASS :
                goto dispatch;
                break;
            case VM_MAKE_METHOD :
                goto dispatch;
                break;
            case VM_RETURN_VALUE :
                // fetch retval
                ret = vm_frame_stack_pop(frame);
                reason = REASON_RETURN;
                goto block_end;
                break;

//            case VM_LOAD_LOCALS :
//                goto dispatch;
//                break;

        } // switch(opcode) {


block_end:
        // A block has ended. We have to "jump" to another place. Figure out why this is, and how to deal with it
        switch (reason) {
            case REASON_RETURN :
                // Return from a function
                break;
            case REASON_CONTINUE :
                // Continue a loop
                break;
            case REASON_BREAK :
                // Break from a loop
                break;
            case REASON_BREAKELSE :
                // Break into the else part of a loop
                break;
        }


    } // for (;;)


    // Return last entry on the stack
    obj1 = vm_frame_stack_pop(frame);
    return obj1;
}