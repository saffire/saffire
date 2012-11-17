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
#include "objects/object.h"
#include "objects/boolean.h"
#include "objects/string.h"
#include "objects/numerical.h"
#include "objects/hash.h"
#include "objects/null.h"
#include "interpreter/errors.h"
#include "modules/module_api.h"

#define OBJ2STR(_obj_) smm_strdup(((t_string_object *)_obj_)->value)

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
                obj1 = stack_pop(); \
                object_dec_ref(obj1); \
                obj2 = stack_pop(); \
                object_dec_ref(obj2); \
                \
                if (obj1->type != obj2->type) { \
                    saffire_vm_error(err_str); \
                } \
                obj3 = object_operator(obj2, opr, in_place, 1, obj1); \
                \
                object_inc_ref(obj3);   \
                stack_push(obj3);   \


#define NOT_IMPLEMENTED  printf("opcode %d is not implemented yet", opcode); exit(1); break;

typedef struct _vm_context {
    t_bytecode *bc;                   // Global bytecode array
    unsigned int ip;                  // Instruction pointer

    t_object **stack;                 // Local context stack
    unsigned int sp;                  // Stack pointer
    t_hash_object *local_identifiers;     // Local identifiers
    t_hash_object *global_identifiers;    // Global identifiers
    t_hash_object *builtin_identifiers;   // Builtin identifiers

    unsigned int time;                // Total time spend in this bytecode block
    unsigned int executions;          // Number of total executions (opcodes processed)
} t_vm_context;


t_hash_object *global_identifiers = NULL;
t_hash_object *builtin_identifiers = NULL;
t_dll *contexts;


/**
 *
 */
void vm_init(void) {
    object_init();
    builtin_identifiers = (t_hash_object *)object_new(Object_Hash);
    module_init();
}


/**
 * Adds a context onto the context-stack
 */
static void push_context(t_vm_context *ctx) {
    dll_append(contexts, ctx);
}

/**
 * Pops a context from the context-stack
 */
static void pop_context(void) {
    t_dll_element *e = DLL_TAIL(contexts);
    dll_remove(contexts, e);
}


/**
 * Returns the current context (tail of the context-stack)
 */
static t_vm_context *get_current_vm_context(void) {
    t_dll_element *e = DLL_TAIL(contexts);
    return e->data;
}


/**
 * Creates and initializes a new context
 */
static t_vm_context *create_context(t_bytecode *bc) {
    t_vm_context *ctx = smm_malloc(sizeof(t_vm_context));
    bzero(ctx, sizeof(t_vm_context));

    ctx->bc = bc;
    ctx->ip = 0;

    //
    ctx->stack = smm_malloc(bc->stack_size * sizeof(t_object *));
    bzero(ctx->stack, bc->stack_size * sizeof(t_object *));
    ctx->sp = bc->stack_size-1;

    // Identifiers
    ctx->local_identifiers = (t_hash_object *)object_new(Object_Hash);

    // Set global and builtin identifiers
    ctx->global_identifiers = global_identifiers;
    ctx->builtin_identifiers = builtin_identifiers;

    return ctx;
}


/**
 * Returns the next opcode
 */
static unsigned char get_next_opcode(void) {
    t_vm_context *ctx = get_current_vm_context();

    if (ctx->ip < 0 || ctx->ip > ctx->bc->code_len) {
        saffire_vm_error("Trying to reach outside code length!");
    }

    if (ctx->ip == ctx->bc->code_len) {
        return VM_STOP;
    }

    char op = ctx->bc->code[ctx->ip];
    ctx->ip++;

    return op;
}

/**
 * Returns he next operand. Does not do any sanity checks if it actually is an operand.
 */
static int get_operand(void) {
    t_vm_context *ctx = get_current_vm_context();

    // Read operand
    char *ptr = ctx->bc->code + ctx->ip;

    uint32_t ret = *ptr;
    ctx->ip += sizeof(uint32_t);

    int tmp = ret;
    return tmp;
}


/**
 * Pops an object from the stack. Errors when the stack is empty
 */
static t_object *stack_pop(void) {
    t_vm_context *ctx = get_current_vm_context();

    printf("STACK POP(%d)\n", ctx->sp);

    if (ctx->sp == ctx->bc->stack_size) {
        saffire_vm_error("Trying to pop from an empty stack");
    }
    t_object *ret = ctx->stack[ctx->sp];
    ctx->sp++;

    return ret;
}


/**
 * Pushes an object onto the stack. Errors when the stack is full
 */
static void stack_push(t_object *obj) {
    t_vm_context *ctx = get_current_vm_context();

    printf("STACK PUSH(%d)\n", ctx->sp);

    if (ctx->sp < 0) {
        saffire_vm_error("Trying to push to a full stack");

    }
    ctx->sp--;
    ctx->stack[ctx->sp] = obj;

}


/**
 * Fetches the top of the stack. Does not pop anything.
 */
static t_object *stack_fetch_top(void) {
    t_vm_context *ctx = get_current_vm_context();

    return ctx->stack[ctx->sp];
}


/**
 * Fetches a non-top element form the stack. Does not pop anything.
 */
static t_object *stack_fetch(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->stack_size) {
        saffire_vm_error("Trying to fetch from outside stack range");
    }

    return ctx->stack[idx];
}


/**
 * Return a constant literal, without converting to an object
 */
static void *get_constant_literal(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->constants_len) {
        saffire_vm_error("Trying to fetch from outside constant range");
    }

    t_bytecode_constant *c = ctx->bc->constants[idx];
    return c->data.ptr;
}


/**
  *
  */
static t_object *get_constant(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->constants_len) {
        saffire_vm_error("Trying to fetch from outside constant range");
    }

    t_bytecode_constant *c = ctx->bc->constants[idx];
    switch (c->type) {
        case BYTECODE_CONST_OBJECT :
            return ctx->bc->constants[idx]->data.obj;
            break;

        case BYTECODE_CONST_STRING :
            RETURN_STRING(ctx->bc->constants[idx]->data.s);
            break;

        case BYTECODE_CONST_NUMERICAL :
            RETURN_NUMERICAL(ctx->bc->constants[idx]->data.l);
            break;
    }

    saffire_vm_error("Cannot convert constant type %d to an object\n", idx);
    return NULL;
}


/**
 * Store object into the global identifier table
 */
static void set_global_identifier(char *id, t_object *obj) {
    t_vm_context *ctx = get_current_vm_context();

    if (obj == NULL) {
        ht_remove(ctx->global_identifiers->ht, id);
    } else {
        ht_add(ctx->global_identifiers->ht, id, obj);
    }
}


/**
 * Return object from the global identifier table
 */
static t_object *get_global_identifier(int idx) {
    t_vm_context *ctx = get_current_vm_context();
    t_object *obj = ht_num_find(ctx->global_identifiers->ht, idx);
    if (obj == NULL) RETURN_NULL;
    return obj;
}


/**
 * Store object into either the local or global identifier table
 */
static void set_identifier(char *id, t_object *obj) {
    t_vm_context *ctx = get_current_vm_context();

    ht_add(ctx->local_identifiers->ht, id, obj);
}


/**
 * Return object from either the local or the global identifier table
 */
static t_object *get_identifier(char *id) {
    t_object *obj;
    t_vm_context *ctx = get_current_vm_context();

    obj = ht_find(ctx->local_identifiers->ht, id);
    if (obj != NULL) return obj;

    if (ctx->global_identifiers && ctx->global_identifiers->ht) {
        obj = ht_find(ctx->global_identifiers->ht, id);
        if (obj != NULL) return obj;
    }

    obj = ht_find(ctx->builtin_identifiers->ht, id);
    if (obj != NULL) return obj;

    saffire_vm_error("Cannot find variable: %s\n", id);
    return NULL;
}


/**
 *
 */
static char *get_name(int idx) {
    t_vm_context *ctx = get_current_vm_context();

    if (idx < 0 || idx >= ctx->bc->identifiers_len) {
        saffire_vm_error("Trying to fetch from outside identifier range");
    }
    return ctx->bc->identifiers[idx]->s;
}


/**
 *
 */
static t_object *_import(char *module, char *class) {
    char tmp[100];
    snprintf(tmp, 99, "%s::%s", module, class);

    t_object *obj = get_identifier(tmp);
    return obj;
}


/**
 *
 */
int vm_execute(t_bytecode *source_bc) {
    register t_object *obj1, *obj2, *obj3, *obj4;
    register unsigned int opcode, oparg1, oparg2;
    register char *s1;

    // Create new context
    contexts = dll_init();
    t_vm_context *tmp = create_context(source_bc);
    push_context(tmp);

    // If we don't have a global_identifier list yet, set this so next contexts will use it as it's globals
    if (! global_identifiers) {
        global_identifiers = tmp->local_identifiers;
    }

    // Prefetch the current context
    t_vm_context *ctx = get_current_vm_context();


    for (;;) {
        // Room for some other stuff
dispatch:
        // Increase number of executions done
        ctx->executions++;

        // Get opcode and additional argument
        opcode = get_next_opcode();
        if (opcode == VM_STOP) break;
        if (opcode == VM_RESERVED) {
            saffire_vm_error("VM: Reached reserved (0xFF) opcode. Halting.\n");
        }

        // If high bit is set, get operand
        oparg1 = ((opcode & 0x80) == 0x80) ? get_operand() : 0;
        oparg2 = ((opcode & 0xC0) == 0xC0) ? get_operand() : 0;

#ifdef __DEBUG
        printf("Opcode: 0x%02X (0x%02X, 0x%02X)\n", opcode, oparg1, oparg2);
#endif

        switch (opcode) {
            // Removes SP-0
            case VM_POP_TOP :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                goto dispatch;
                break;

            // Rotate / swap SP-0 and SP-1
            case VM_ROT_TWO :
                obj1 = stack_pop();
                obj2 = stack_pop();
                stack_push(obj1);
                stack_push(obj2);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-2
            case VM_ROT_THREE :
                obj1 = stack_pop();
                obj2 = stack_pop();
                obj3 = stack_pop();
                stack_push(obj1);
                stack_push(obj2);
                stack_push(obj3);
                goto dispatch;
                break;

            // Duplicate SP-0
            case VM_DUP_TOP :
                obj1 = stack_fetch_top();
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-3
            case VM_ROT_FOUR :
                obj1 = stack_pop();
                obj2 = stack_pop();
                obj3 = stack_pop();
                obj4 = stack_pop();
                stack_push(obj1);
                stack_push(obj2);
                stack_push(obj3);
                stack_push(obj4);
                goto dispatch;
                break;

            // No operation
            case VM_NOP :
                goto dispatch;
                break;

            // Load a global identifier
            case VM_LOAD_GLOBAL :
                obj1 = get_global_identifier(oparg1);
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            // store SP+0 as a global identifier
            case VM_STORE_GLOBAL :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                s1 = get_name(oparg1);
                set_global_identifier(s1, obj1);
                goto dispatch;
                break;

            // Remove global identifier
            case VM_DELETE_GLOBAL :
                s1 = get_name(oparg1);
                set_global_identifier(s1, NULL);
                goto dispatch;
                break;

            // Load and push constant onto stack
            case VM_LOAD_CONST :
                obj1 = get_constant(oparg1);
                object_inc_ref(obj1);
                stack_push(obj1);
                goto dispatch;
                break;

            // Store SP+0 into identifier (either local or global)
            case VM_STORE_ID :
                obj1 = stack_pop();
                object_dec_ref(obj1);
                s1 = get_name(oparg1);
                set_identifier(s1, obj1);
                goto dispatch;
                break;
                // @TODO: If string(obj1) exists in local store it there, otherwise, store in global

            // Load and push identifier onto stack (either local or global)
            case VM_LOAD_ID :
                s1 = get_name(oparg1);
                obj1 = get_identifier(s1);
                object_inc_ref(obj1);
                stack_push(obj1);
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
                ctx->ip += oparg1;
                goto dispatch;
                break;

            // Conditional jump on SP-0 is true
            case VM_JUMP_IF_TRUE :
                obj1 = stack_pop();
                if (! OBJECT_IS_BOOLEAN(obj1)) {
                    // Cast to boolean
                    obj2 = object_find_method(obj1, "boolean");
                    obj1 = object_call(obj1, obj2, 0);
                }

                if (IS_TRUE(obj1)) {
                    ctx->ip += oparg1;
                }

                goto dispatch;
                break;

            // Conditional jump on SP-0 is false
            case VM_JUMP_IF_FALSE :
                obj1 = stack_pop();
                if (! OBJECT_IS_BOOLEAN(obj1)) {
                    // Cast to boolean
                    obj2 = object_find_method(obj1, "boolean");
                    obj1 = object_call(obj1, obj2, 0);
                }

                if (IS_FALSE(obj1)) {
                    ctx->ip += oparg1;
                }
                goto dispatch;
                break;

            // Unconditional absolute jump
            case VM_JUMP_ABSOLUTE :
                ctx->ip = oparg1;
                goto dispatch;
                break;

            // Duplicates the SP+0 a number of times
            case VM_DUP_TOPX :
                obj1 = stack_fetch_top();
                for (int i=0; i!=oparg1; i++) {
                    object_inc_ref(obj1);
                    stack_push(obj1);
                }
                goto dispatch;
                break;

            // Calls method OP+0 SP+0 from object SP+1 with OP+1 args starting from SP+2.
            case VM_CALL_METHOD :
                obj1 = stack_pop();   // Self
                obj2 = object_find_method(obj1, (char *)get_constant_literal(oparg1));

                // Create argument list
                t_dll *dll = dll_init();
                for (int i=0; i!=oparg2; i++) {
                    obj3 = stack_pop();
                    object_dec_ref(obj3);
                    dll_append(dll, obj3);
                }

                // Call object and push result onto stack
                obj3 = object_call_args(obj1, obj2, dll);
                object_inc_ref(obj3);
                stack_push(obj3);

                dll_free(dll);

                goto dispatch;
                break;

            case VM_IMPORT :
                // Fetch the module to import
                obj1 = stack_pop();
                object_dec_ref(obj1);
                char *module = OBJ2STR(obj1);

                // Fetch class
                obj1 = stack_pop();
                object_dec_ref(obj1);
                char *class = OBJ2STR(obj1);

                printf("Importing %s from %s\n", class, module);

                obj3 = _import(module, class);
                object_inc_ref(obj3);
                stack_push(obj3);

                goto dispatch;
                break;

        } // switch(opcode) {
    } // for (;;)

    // @TODO: We should return "something"
    return 0;
}
