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
#include "general/dll.h"
#include "general/smm.h"
#include "object/object.h"
#include "object/string.h"
#include "object/numerical.h"

#define NOT_IMPLEMENTED  printf("opcode %d is not implemented yet", opcode); exit(1); break;

typedef struct _vm_context {
    t_bytecode *bc;             // Global bytecode array
    int ip;                     // Instruction pointer

    t_object **stack;           // Local context stack
    int sp;                     // Stack pointer
    t_object **variables;       // Local variables
} t_vm_context;

t_dll *contexts;

static void push_context(t_vm_context *ctx) {
    dll_append(contexts, ctx);
}

static void pop_context(void) {
    t_dll_element *e = DLL_TAIL(contexts);
    dll_remove(contexts, e);
}

static t_vm_context *get_current_vm_context(void) {
    t_dll_element *e = DLL_TAIL(contexts);
    return e->data;
}


static t_vm_context *create_context(t_bytecode *bc) {
    t_vm_context *ctx = smm_malloc(sizeof(t_vm_context));

    ctx->bc = bc;
    ctx->ip = 0;

    //
    ctx->stack = smm_malloc(bc->stack_size * sizeof(t_object *));
    bzero(ctx->stack, bc->stack_size * sizeof(t_object *));
    ctx->sp = bc->stack_size-1;

    // Variables
    ctx->variables = smm_malloc(bc->variables_len * sizeof(t_object *));
    bzero(ctx->variables, bc->variables_len * sizeof(t_object *));

    return ctx;
}


static char get_next_opcode(void) {
    t_vm_context *ctx = get_current_vm_context();

    if (ctx->ip < 0 || ctx->ip > ctx->bc->code_len) {
        printf("Trying to reach outside code length!");
        exit(1);
    }

    if (ctx->ip == ctx->bc->code_len) {
        return VM_STOP_CODE;
    }

    char op = ctx->bc->code[ctx->ip];
    ctx->ip++;

    return op;
}

static int get_operand(void) {
    t_vm_context *ctx = get_current_vm_context();

    // Read operand
    char *ptr = ctx->bc->code + ctx->ip;

    uint32_t ret = *ptr;
    ctx->ip += sizeof(uint32_t);

    int tmp = ret;
    return tmp;
}


static t_object *stack_pop(void) {
    t_vm_context *ctx = get_current_vm_context();

    printf("STACK POP(%d)\n", ctx->sp);

    if (ctx->sp == ctx->bc->stack_size) {
        printf("Trying to pop from an empty stack");
        exit(1);
    }
    t_object *ret = ctx->stack[ctx->sp];
    ctx->sp++;

    return ret;
}


static void stack_push(t_object *obj) {
    t_vm_context *ctx = get_current_vm_context();

    printf("STACK PUSH(%d)\n", ctx->sp);

    if (ctx->sp < 0) {
        printf("Trying to push to a full stack");
        exit(1);
    }
    ctx->sp--;
    ctx->stack[ctx->sp] = obj;

}


static t_object *stack_fetch_top(void) {
    t_vm_context *ctx = get_current_vm_context();

    return ctx->stack[ctx->sp];
}


static t_object *stack_fetch(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->stack_size) {
        printf("Trying to fetch from outside stack range");
        exit(1);
    }

    return ctx->stack[idx];
}


/**
  *
  */
static t_object *get_constant(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->constants_len) {
        printf("Trying to fetch from outside constant range");
        exit(1);
    }

    t_bytecode_constant *c = ctx->bc->constants[idx];
    switch (c->type) {
        case BYTECODE_CONST_STRING :
            RETURN_STRING(ctx->bc->constants[idx]->data.s);
            break;

        case BYTECODE_CONST_NUMERICAL :
            RETURN_NUMERICAL(ctx->bc->constants[idx]->data.l);
            break;

        default :
            printf("Cannot convert constant type %d to an object\n");
            exit(1);
    }

}


static void set_variable(int idx, t_object *obj) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->variables_len) {
        printf("Trying to fetch from outside variable range");
        exit(1);
    }

    ctx->variables[idx] = obj;
}


/**
  *
  */
static t_object *get_variable(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->variables_len) {
        printf("Trying to fetch from outside variable range");
        exit(1);
    }
    return ctx->variables[idx];
}

/**
  *
  */
static t_object *get_name(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->variables_len) {
        printf("Trying to fetch from outside variable range");
        exit(1);
    }
    RETURN_STRING(ctx->bc->variables[idx]->s);
}

/**
 *
 */
int vm_execute(t_bytecode *source_bc) {
    int opcode, oparg;

    contexts = dll_init();
    t_vm_context *tmp = create_context(source_bc);
    push_context(tmp);

    t_object *obj1, *obj2, *obj3, *obj4;


    while (1) {
        // Room for some other stuff
dispatch:

        // Get opcode and additional argument
        opcode = get_next_opcode();
        if (opcode  == VM_STOP_CODE) break;

        oparg = (opcode >= HAVE_ARGUMENT) ? get_operand() : 0;
        printf("Opcode: %02X (%02X)\n", opcode, oparg);

        switch (opcode) {
            // @TODO: DEBUG OPCODES
            case VM_PRINT_VAR :
                obj1 = stack_pop();
                obj2 = object_find_method(obj1, "print");
                object_call(obj1, obj2, 0);
                goto dispatch;
                break;

            case VM_POP_TOP :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                goto dispatch;
                break;

            case VM_ROT_TWO :
                obj1 = stack_pop();
                obj2 = stack_pop();
                stack_push(obj1);
                stack_push(obj2);
                goto dispatch;
                break;

            case VM_ROT_THREE :
                obj1 = stack_pop();
                obj2 = stack_pop();
                obj3 = stack_pop();
                stack_push(obj1);
                stack_push(obj2);
                stack_push(obj3);
                goto dispatch;
                break;

            case VM_DUP_TOP :
                obj1 = stack_fetch_top();
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            case VM_ROT_FOUR :
                obj1 = stack_fetch_top();
                obj2 = stack_fetch_top();
                obj3 = stack_fetch_top();
                obj4 = stack_fetch_top();
                stack_push(obj1);
                stack_push(obj2);
                stack_push(obj3);
                stack_push(obj4);
                goto dispatch;
                break;

            case VM_NOP :
                goto dispatch;
                break;

            case VM_LOAD_CONST :
                obj1 = get_constant(oparg);
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            case VM_STORE_VAR :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                set_variable(oparg, obj1);
                goto dispatch;
                break;
                // @TODO: If string(obj1) exists in local store it there, otherwise, store in global

            case VM_LOAD_VAR :
                obj1 = get_variable(oparg);
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            case VM_BINARY_ADD :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                obj2 = stack_pop();
                object_dec_ref(obj2);

                if (obj1->type != obj2->type) {
                    saffire_error("Can only add equal types :/");
                }
                obj3 = object_operator(obj2, OPERATOR_ADD, 0, 1, obj1);

                object_inc_ref(obj3);
                stack_push(obj3);
                goto dispatch;
                break;

            case VM_BINARY_SUBTRACT :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                obj2 = stack_pop();
                object_dec_ref(obj2);

                if (obj1->type != obj2->type) {
                    saffire_error("Can only sub equal types :/");
                }
                obj3 = object_operator(obj2, OPERATOR_SUB, 0, 1, obj1);

                object_inc_ref(obj3);
                stack_push(obj3);
                goto dispatch;
                break;


        }

    }

    // @TODO: We should return "something"
    return 0;
}