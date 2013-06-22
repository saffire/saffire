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
#include <stdio.h>
#include <string.h>
#include "compiler/ast_to_asm.h"
#include "compiler/output/asm.h"
#include "compiler/bytecode.h"
#include "compiler/ast_nodes.h"
#include "compiler/parser.tab.h"
#include "general/output.h"
#include "general/smm.h"
#include "general/stack.h"
#include "general/dll.h"
#include "debug.h"
#include "vm/vm_opcodes.h"
#include "vm/frame.h"
#include "objects/attrib.h"

#define MAX_LABEL_LEN       100

extern char *get_token_string(int token);

#define WALK_LEAF(leaf) __ast_walker(leaf, output, frame, state, append_return_statement)

// state enums
enum context { st_ctx_load, st_ctx_store };
enum type { st_type_id, st_type_const };
enum side { st_side_left, st_side_right, st_none };
enum call_state { st_call_pop, st_call_stay };

/*
 * Structure that holds info for current block in the frame (while, if, foreach etc)
 */
typedef struct _state_frame {
    int type;           // BLOCK_TYPE_* as defined in vm/frame.h
    char label[MAX_LABEL_LEN];
    t_hash_table *labels;       // Defined labels inside this block (so we can tests for duplicates)
} t_state_frame;


/*
 * Structure that holds info about the current state of the frame.
 */
typedef struct _state {
    t_stack *context;     // Load or store context. When a const or id is found, it must either LOAD or STORE
    t_stack *type;        // Are we loading or storing a constant, or an identifier. Must be in sync with state->context
    t_stack *side;        // Are we processing left or right side of an assignment (is this relevant now?)
    t_stack *call_state;  // If we called something, do we need to pop the result, or let it stay on the stack

    int attributes;     // Number of attributes on stack. Used when finding how many attributes a CALL must generate
    int loop_cnt;       // Loop counter. Used for generating labels

    int block_cnt;                            // Last used block number in the blocks[] array
    t_state_frame blocks[BLOCK_MAX_DEPTH];    // Frame blocks
} t_state;

// When > 0, this is a multi-assignment (a = b = c = 3)
static int multi_assignment = 0;

/**
 * Load or store a constant or id. Depends on the state what actually will be done
 */
static void _load_or_store(int lineno, t_asm_opr *opr, t_dll *frame, t_state *state) {
    enum context ctx = (enum context)stack_peek(state->context);
    enum type type  = (enum type)stack_peek(state->type);

    switch (ctx) {
        case st_ctx_load :
            if (type == st_type_id) {
                dll_append(frame, asm_create_codeline(lineno, VM_LOAD_ID, 1, opr));
            } else if (type == st_type_const) {
                dll_append(frame, asm_create_codeline(lineno, VM_LOAD_CONST, 1, opr));
            }
            break;
        case st_ctx_store :
            // Store is always to ID!
            dll_append(frame, asm_create_codeline(lineno, VM_STORE_ID, 1, opr));
            break;
        default :
            fatal_error(1, "Unknown load/store state for %d!", opr->type);
    }
}


static void __ast_walker(t_ast_element *leaf, t_hash_table *output, t_dll *frame, t_state *state, int append_return_statement);
static void _ast_to_frame(t_ast_element *leaf, t_hash_table *output, const char *name, int append_return_statement);



/**
 * Walk the leaf into the frame. Can create new frames if needed (method bodies etc)
 */
static void __ast_walker(t_ast_element *leaf, t_hash_table *output, t_dll *frame, t_state *state, int append_return_statement) {
    char label1[MAX_LABEL_LEN], label2[MAX_LABEL_LEN], label3[MAX_LABEL_LEN];
    char label4[MAX_LABEL_LEN], label5[MAX_LABEL_LEN], label6[MAX_LABEL_LEN];
    t_asm_opr *opr1, *opr2, *opr3;
    t_ast_element *node, *node2;
    t_ast_element *arglist;
    int i, clc, arg_count;

    if (!leaf) return;

    switch (leaf->type) {
        case typeAstProperty :
            stack_push(state->context, st_ctx_load);
            WALK_LEAF(leaf->property.class);
            stack_pop(state->context);

            node = leaf->property.property;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, node->string.value, 0);

            enum context ctx = (enum context)stack_peek(state->context);
            if (ctx == st_ctx_load) {
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_ATTRIB, 1, opr1));
            } else {
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_STORE_ATTRIB, 1, opr1));
            }
            break;

        case typeAstString :
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->string.value, 0);
            stack_push(state->type, (void *)st_type_const);
            _load_or_store(leaf->lineno, opr1, frame, state);
            stack_pop(state->type);
            break;
        case typeAstNumerical :
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->numerical.value);
            stack_push(state->type, (void *)st_type_const);
            _load_or_store(leaf->lineno, opr1, frame, state);
            stack_pop(state->type);
            break;
        case typeAstNull :
            // Store null
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, "null", 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_ID, 1, opr1));
            break;
        case typeAstIdentifier :
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->identifier.name, 0);
            stack_push(state->type, (void *)st_type_id);
            _load_or_store(leaf->lineno, opr1, frame, state);
            stack_pop(state->type);
            break;
        case typeAstNop :
            // Nop does not emit any asmlines
            break;
        case typeAstOperator :
            stack_push(state->type, st_type_id);
            stack_push(state->side, st_side_left);
            WALK_LEAF(leaf->operator.l);
            stack_pop(state->side);
            stack_pop(state->type);

            stack_push(state->type, st_type_id);
            stack_push(state->side, st_side_left);
            WALK_LEAF(leaf->operator.r);
            stack_pop(state->side);
            stack_pop(state->type);

            int op = 0;
            switch (leaf->operator.op) {
                case '+' : op = OPERATOR_ADD; break;
                case '-' : op = OPERATOR_SUB; break;
                case '*' : op = OPERATOR_MUL; break;
                case '/' : op = OPERATOR_DIV; break;
                case '%' : op = OPERATOR_MOD; break;
                case '&' : op = OPERATOR_AND; break;
                case '|' : op = OPERATOR_OR;  break;
                case '^' : op = OPERATOR_XOR; break;
                case T_SHIFT_LEFT : op = OPERATOR_SHL; break;
                case T_SHIFT_RIGHT : op = OPERATOR_SHR; break;
            }

            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_OPERATOR, NULL, op);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_OPERATOR, 1, opr1));
            break;

        case typeAstComparison :
            stack_push(state->call_state, (void *)st_call_stay);

            stack_push(state->context, st_ctx_load);
            stack_push(state->side, (void *)st_side_right);
            WALK_LEAF(leaf->comparison.r);
            stack_pop(state->side);
            stack_pop(state->context);

            stack_push(state->context, st_ctx_load);
            stack_push(state->side, st_side_left);
            WALK_LEAF(leaf->comparison.l);
            stack_pop(state->side);
            stack_pop(state->context);

            stack_pop(state->call_state);

            int tmp = 0;
            switch (leaf->comparison.cmp) {
                case T_EQ : tmp = COMPARISON_EQ; break;
                case T_NE : tmp = COMPARISON_NE; break;
                case '>'  : tmp = COMPARISON_GT; break;
                case '<'  : tmp = COMPARISON_LT; break;
                case T_GE : tmp = COMPARISON_GE; break;
                case T_LE : tmp = COMPARISON_LE; break;
                default :
                    fatal_error(1, "Unknown comparison: %s\n", leaf->comparison.cmp);
            }

            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, tmp);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_COMPARE_OP, 1, opr1));
            break;
        case typeAstAssignment :
            if (leaf->assignment.r->type == typeAstAssignment) {
                multi_assignment++;
            }

            // @TODO: We only handle =, not += -= etc

            stack_push(state->call_state, (void *)st_call_stay);
            stack_push(state->side, (void *)st_side_right);
            stack_push(state->context, st_ctx_load);
            WALK_LEAF(leaf->assignment.r);
            stack_pop(state->context);
            stack_pop(state->side);
            stack_pop(state->call_state);

            if (multi_assignment) {
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_DUP_TOP, 0));
                multi_assignment--;
            }

            stack_push(state->side, st_side_left);
            stack_push(state->context, (void *)st_ctx_store);
            WALK_LEAF(leaf->assignment.l);
            stack_pop(state->context);
            stack_pop(state->side);
            break;

        case typeAstBool :
            state->loop_cnt++;
            clc = state->loop_cnt;

            if (leaf->boolop.op == 0) {
                // Do boolean AND
                sprintf(label1, "and_%03d_end", clc);

                WALK_LEAF(leaf->boolop.l);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                WALK_LEAF(leaf->boolop.r);

                // Skip checks&jumps, since we can safely use the result from RHS as result for boolean op node

                dll_append(frame, asm_create_labelline(label1));
            }

            if (leaf->boolop.op == 1) {
                // Do boolean OR
                sprintf(label1, "or_%03d_end", clc);

                WALK_LEAF(leaf->boolop.l);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_TRUE, 1, opr1));
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                WALK_LEAF(leaf->boolop.r);

                // Skip checks&jumps, since we can safely use the result from RHS as result for boolean op node

                dll_append(frame, asm_create_labelline(label1));
            }

            break;

        case typeAstClass :
            // Iterate body
            WALK_LEAF(leaf->class.body);

            // Push parent class
            stack_push(state->context, st_ctx_load);
            WALK_LEAF(leaf->class.extends);

            // Push implements
            stack_push(state->context, st_ctx_load);
            WALK_LEAF(leaf->class.implements);

            // Push the number of implements we actually found
            node = leaf->class.implements;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, node->group.len);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Push flags
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->class.modifiers);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Push the name
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->class.name, 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Build class code
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, state->attributes);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_BUILD_CLASS, 1, opr1));
            state->attributes = 0;

            // Store class into identifier
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->class.name, 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_STORE_ID, 1, opr1));

            break;

        case typeAstInterface :
            // Iterate body
            WALK_LEAF(leaf->interface.body);

            // Push parent class as NULL
            stack_push(state->context, st_ctx_load);
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, "null", 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Push implements
            stack_push(state->context, st_ctx_load);
            WALK_LEAF(leaf->interface.implements);

            // Push the number of implements we actually found
            node = leaf->interface.implements;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, node->group.len);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Push flags
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->interface.modifiers);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Push the name
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->interface.name, 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Build class code
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, state->attributes);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_BUILD_INTERFACE, 1, opr1));
            state->attributes = 0;

            // Store class into identifier
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->interface.name, 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_STORE_ID, 1, opr1));
            break;

        case typeAstAttribute :
            arg_count = 0;
            if (leaf->attribute.attrib_type == ATTRIB_TYPE_METHOD) {
                // Do arguments
                arglist = leaf->attribute.arguments;
                if (arglist->type == typeAstOpr && arglist->opr.oper == T_ARGUMENT_LIST) {
                    arg_count = arglist->opr.nops;

                    for (int i=arglist->opr.nops-1; i>=0; i--) {
                        t_ast_element *arg = arglist->opr.ops[i];

                        // Iterate method arguments
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(arg->opr.ops[0]);     // type hint
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(arg->opr.ops[1]);     // name
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(arg->opr.ops[2]);     // value
                    }
                }

                // Push method flags
                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.method_flags);
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));


                // Push body
                state->loop_cnt++;
                clc = state->loop_cnt;
                sprintf(label1, "frame_%03d", clc);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_CODE, label1, 0);
                dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

                // Walk the body inside a new frame!
                _ast_to_frame(leaf->attribute.value, output, label1, append_return_statement);
            }

            if (leaf->attribute.attrib_type == ATTRIB_TYPE_CONSTANT) {
                stack_push(state->context, st_ctx_load);
                WALK_LEAF(leaf->attribute.value);
            }

            if (leaf->attribute.attrib_type == ATTRIB_TYPE_PROPERTY) {
                stack_push(state->context, st_ctx_load);
                WALK_LEAF(leaf->attribute.value);
            }


            // Store visibility
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.visibility);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Store access
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.access);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            // Create constant
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, leaf->attribute.attrib_type);
            opr2 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, arg_count);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_BUILD_ATTRIB, 2, opr1, opr2));

            // Name of the id to store the constant into
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->attribute.name, 0);
            dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));

            state->attributes++;
            break;

        case typeAstGroup :
            for (int i=0; i!=leaf->group.len; i++) {
                node = leaf->group.items[i];
                WALK_LEAF(node);
            }
            break;


        case typeAstOpr :
            switch (leaf->opr.oper) {
                case T_PROGRAM :
                    WALK_LEAF(leaf->opr.ops[0]);        // Imports
                    WALK_LEAF(leaf->opr.ops[1]);        // Top statements
                    break;
                case T_IMPORT :
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[2]);

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_IMPORT, 0));

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, node->string.value, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_STORE_ID, 1, opr1));
                    break;

                case T_CALL_ARGUMENT_LIST :
                case T_ARGUMENT_LIST :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        WALK_LEAF(leaf->opr.ops[i]);
                    }
                    break;

                case T_RETURN :
                    stack_push(state->context, st_ctx_load);
                    stack_push(state->call_state, (void *)st_call_stay);
                    WALK_LEAF(leaf->opr.ops[0]);
                    stack_pop(state->call_state);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_RETURN, 0));

                    break;

                case T_IF :
                    state->loop_cnt++;
                    clc = state->loop_cnt;

                    sprintf(label2, "if_%03d_pre_end", clc);
                    sprintf(label5, "if_%03d_end", clc);
                    sprintf(label6, "if_%03d_else", clc);

                    // Comparison first
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label6, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[1]);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));


                    // Always add else-label
                    dll_append(frame, asm_create_labelline(label6));

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(leaf->opr.ops[2]);
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label5));
                    break;

                case T_BREAKELSE :
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_BREAKELSE_LOOP, 0));
                    break;
                case T_BREAK :
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_BREAK_LOOP, 0));
                    break;
                case T_CONTINUE :
                    // We need to find the label1 for the first encountered start state!
                    i = state->block_cnt - 1;
                    while (i >= 0 && state->blocks[i].type != BLOCK_TYPE_LOOP) i--;

                    if (i < 0) {
                        fatal_error(1, "No loop block found to continue too.");
                    }

                    // If we need to break out several loops (ie: break 2) we need to traverse back multiple
                    // loop blocks
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state->blocks[i].label, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_CONTINUE_LOOP, 1, opr1));

                    state->block_cnt = i+1;
                    break;


                case T_DO :
                    state->loop_cnt++;
                    clc = state->loop_cnt;
                    sprintf(state->blocks[state->block_cnt].label, "dowhile_%03d_cmp", clc);
                    state->blocks[state->block_cnt].type = BLOCK_TYPE_LOOP;
                    if (state->blocks[state->block_cnt].labels) {
                        ht_destroy(state->blocks[state->block_cnt].labels);
                        state->blocks[state->block_cnt].labels = ht_create();
                    }
                    state->block_cnt++;

                    sprintf(label3, "dowhile_%03d_cmp", clc);
                    sprintf(label1, "dowhile_%03d", clc);
                    sprintf(label2, "dowhile_%03d_pre_end", clc);
                    sprintf(label5, "dowhile_%03d_end", clc);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_SETUP_LOOP, 1, opr1));
                    dll_append(frame, asm_create_labelline(label1));

                    // Body
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    // Comparison
                    dll_append(frame, asm_create_labelline(label3));
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[1]);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));


                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label2));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_BLOCK, 0));
                    dll_append(frame, asm_create_labelline(label5));

                    state->block_cnt--;
                    break;

                case T_WHILE :
                    state->loop_cnt++;
                    clc = state->loop_cnt;
                    sprintf(state->blocks[state->block_cnt].label, "while_%03d", clc);
                    state->blocks[state->block_cnt].type = BLOCK_TYPE_LOOP;
                    if (state->blocks[state->block_cnt].labels) {
                        ht_destroy(state->blocks[state->block_cnt].labels);
                        state->blocks[state->block_cnt].labels = ht_create();
                    }
                    state->block_cnt++;

                    sprintf(label1, "while_%03d", clc);
                    sprintf(label6, "while_%03d_else", clc);
                    sprintf(label4, "while_%03d_pre_else", clc);
                    sprintf(label2, "while_%03d_pre_end", clc);
                    sprintf(label5, "while_%03d_end", clc);

                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        opr2 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label6, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_SETUP_ELSE_LOOP, 2, opr1, opr2));
                    } else {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_SETUP_LOOP, 1, opr1));
                    }

                    dll_append(frame, asm_create_labelline(label1));

                    // Comparison
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);
                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label4, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FIRST_FALSE, 1, opr1));
                    }
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));


                    // Body
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[1]);

                    // Back to start
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));


                    // Add else (if any)
                    if (leaf->opr.nops == 3) {
                        dll_append(frame, asm_create_labelline(label4));
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_BLOCK, 0));
                        dll_append(frame, asm_create_labelline(label6));

                        // Add else body
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(leaf->opr.ops[2]);

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));
                    }

                    dll_append(frame, asm_create_labelline(label2));

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_BLOCK, 0));
                    dll_append(frame, asm_create_labelline(label5));

                    state->block_cnt--;
                    break;

                case T_FOR :
                    state->loop_cnt++;
                    clc = state->loop_cnt;
                    sprintf(state->blocks[state->block_cnt].label, "for_%03d_incdec", clc);
                    state->blocks[state->block_cnt].type = BLOCK_TYPE_LOOP;
                    if (state->blocks[state->block_cnt].labels) {
                        ht_destroy(state->blocks[state->block_cnt].labels);
                        state->blocks[state->block_cnt].labels = ht_create();
                    }
                    state->block_cnt++;

                    sprintf(label2, "for_%03d_init", clc);
                    sprintf(label3, "for_%03d_test", clc);
                    sprintf(label4, "for_%03d_pre_end", clc);
                    sprintf(label5, "for_%03d_end", clc);
                    sprintf(label6, "for_%03d_incdec", clc);

                    // Add init
                    dll_append(frame, asm_create_labelline(label2));
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_SETUP_LOOP, 1, opr1));

                    // Add comparison
                    dll_append(frame, asm_create_labelline(label3));
                    stack_push(state->context, st_ctx_load);

                    if (leaf->opr.ops[1]->type == typeAstNop) {
                        // Make sure that when we have no expression-statement inside the compare, we still
                        // push TRUE, so the VM_JUMP_IF_FALSE won't crash
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, "true", 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_CONST, 1, opr1));
                    } else {
                        WALK_LEAF(leaf->opr.ops[1]);
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label4, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Add body
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[3]);

                    // Add incdec
                    dll_append(frame, asm_create_labelline(label6));
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[2]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label3, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label4));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_BLOCK, 0));

                    dll_append(frame, asm_create_labelline(label5));

                    state->block_cnt--;
                    break;

                case T_CALL :
                    stack_push(state->context, st_ctx_load);

                    stack_push(state->call_state, (void *)st_call_stay);
                    WALK_LEAF(leaf->opr.ops[1]);       // Do argument list (including last item being NullObject or ListObject varargs)
                    stack_pop(state->call_state);

                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);       // Load callable

                    // Push number of arguments
                    int arg_count = leaf->opr.ops[1]->group.len;
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, arg_count-1);

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_CALL, 1, opr1));

                    // Pop the item after the call, but only when we need so.
                    enum call_state cs = (enum call_state)stack_peek(state->call_state);
                    if (!cs || cs == st_call_pop) {
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    }

                    break;

                case T_SUBSCRIPT :
                    node = leaf->opr.ops[1];
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(node->group.items[0]);
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(node->group.items[1]);

                    // load "splice" attrib
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);       // Load object to subscribe

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, "splice", 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_ATTRIB, 1, opr1));

                    // Call splice
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, 2);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_CALL, 1, opr1));
                    break;

                case T_DATASTRUCT :
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    // Iterate elements
                    node = leaf->opr.ops[1];
                    for (int i=0; i!=node->group.len; i++) {
                        node2 = node->group.items[i];
                        // Iterate attributes
                        for (int j=0; j!=node2->group.len; j++) {
                            stack_push(state->context, st_ctx_load);
                            WALK_LEAF(node2->group.items[j]);
                        }
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, node2->group.len);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_BUILD_TUPLE, 1, opr1));
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, node->group.len);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_BUILD_DATASTRUCT, 1, opr1));
                    break;

                case T_GOTO :
                    node = leaf->opr.ops[0];
                    sprintf(label1, "userlabel_%s", node->string.value);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1   , 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));
                    break;

                case T_LABEL :
                    node = leaf->opr.ops[0];
                    // Check for duplicate label inside current block
                    sprintf(label1, "userlabel_%s", node->string.value);
                    if (ht_exists(state->blocks[state->block_cnt].labels, label1)) {
                        fatal_error(1, "Duplicate label '%s' found\n", node->string.value);
                    }
                    ht_add(state->blocks[state->block_cnt].labels, label1, (void *)1);

                    dll_append(frame, asm_create_labelline(label1));
                    break;

                case T_FOREACH :
                    break;
                case T_TRY :
                    state->loop_cnt++;
                    clc = state->loop_cnt;

                    sprintf(label5, "try_%03d_finally", clc);
                    sprintf(label2, "try_%03d_end_try", clc);
                    sprintf(label1, "try_%03d_end_finally", clc);

                    // Setup exception
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    opr2 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    opr3 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_SETUP_EXCEPT, 3, opr1, opr2, opr3));

                    // Try block
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_FORWARD, 1, opr1));

                    dll_append(frame, asm_create_labelline(label2));

                    // Iterate all catches
                    node = leaf->opr.ops[1];
                    for (int i=0; i!=node->group.len; i++) {
                        sprintf(label3, "try_%03d_match_%03d", clc, i);
                        sprintf(label4, "try_%03d_nomatch_%03d", clc, i);

                        if (i != 0) dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_DUP_TOP, 0));

                        node2 = node->group.items[i];       // Catch group
                        t_ast_element *node3 = node2->group.items[0];
                        if (node3->opr.oper != T_CATCH) {
                            fatal_error(1, "Expected a T_CATCH operator node\n");
                        }
                        t_ast_element *exception = node3->opr.ops[0];
                        t_ast_element *identifier = node3->opr.ops[1];

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, exception->string.value, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_LOAD_ID, 1, opr1));
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, COMPARISON_EX);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_COMPARE_OP, 1, opr1));

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label4, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, identifier->string.value, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_STORE_ID, 1, opr1));

                        dll_append(frame, asm_create_labelline(label3));

                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(node2->group.items[1]);

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_FORWARD, 1, opr1));

                        dll_append(frame, asm_create_labelline(label4));
                    }

                    // Unhandled exception, just forward to finally.
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_FORWARD, 1, opr1));


                    // Setup finally block
                    dll_append(frame, asm_create_labelline(label5));

                    if (leaf->opr.ops[2]->type != typeAstNop) {
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(leaf->opr.ops[2]);

                    }
                    dll_append(frame, asm_create_labelline(label1));
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_END_FINALLY, 0));

                    break;
                case T_SWITCH :
                    break;
                case T_COALESCE :
                    state->loop_cnt++;
                    clc = state->loop_cnt;

                    sprintf(label1, "coalesce_%03d_default", clc);
                    sprintf(label2, "coalesce_%03d_end", clc);

                    // Walk default operand expression
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    // Duplicate identifier
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_DUP_TOP, 0));

                    // Go to default if value == null
                    opr3 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr3));

                    // Clean up comparison?
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Jump to end
                    opr3 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr3));

                    // Value was null
                    dll_append(frame, asm_create_labelline(label1));

                    // Clean up comparison?
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Clean up dupped identifier value
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Walk the expression in the second operand, and store the result
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[1]);

                    // Jump to end
                    opr3 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr3));

                    // End
                    dll_append(frame, asm_create_labelline(label2));

                    break;
                case '?' :
                    // @TODO: This is the same code as if-else.
                    state->loop_cnt++;
                    clc = state->loop_cnt;

                    sprintf(label2, "ternaryif_%03d_pre_end", clc);
                    sprintf(label5, "ternaryif_%03d_end", clc);
                    sprintf(label6, "ternaryif_%03d_else", clc);

                    // Comparison first
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label6, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_IF_FALSE, 1, opr1));

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));
                    stack_push(state->context, st_ctx_load);
                    WALK_LEAF(leaf->opr.ops[1]);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));


                    // Always add else-label
                    dll_append(frame, asm_create_labelline(label6));

                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_POP_TOP, 0));

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        stack_push(state->context, st_ctx_load);
                        WALK_LEAF(leaf->opr.ops[2]);
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(leaf->lineno, VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label5));
                    break;



                default :
                    fatal_error(1, "Unknown AST Operator: %s\n", get_token_string(leaf->opr.oper));
            }
            break;
        default :
            fatal_error(1, "Unknown AST type : %d\n", leaf->type);
    }
}


/**
 * Initialize a new state structure
 */
t_state *_ast_state_init() {
    t_state *state = (t_state *)smm_malloc(sizeof(t_state));

    state->context = stack_init();
    state->side = stack_init();
    state->call_state = stack_init();
    state->type = stack_init();

    state->attributes = 0;
    state->loop_cnt = 0;
    state->block_cnt = 0;

    for (int i=0; i!=BLOCK_MAX_DEPTH; i++) {
        state->blocks[i].labels = ht_create();
    }

    return state;
}

/**
 * Finalizes a state structure
 */
void _ast_state_fini(t_state *state) {
    // @TODO: Troubles when trying to free the stack (dll_free in endless loop). Needs fixing! It's probably already fixed!
/*
    stack_free(state->context);
    stack_free(state->side);
    stack_free(state->call_state);
    stack_free(state->type);
*/
    for (int i=0; i!=BLOCK_MAX_DEPTH; i++) {
        if (state->blocks[i].labels) {
            ht_destroy(state->blocks[i].labels);
        }
    }

    smm_free(state);
}

/**
 * Initialize a new frame and walk the leaf into this frame
 */
static void _ast_to_frame(t_ast_element *leaf, t_hash_table *output, const char *name, int append_return_statement) {
    // Initialize state structure
    t_state *state = _ast_state_init();

    // Create frame and add it to our main output
    t_dll *frame = dll_init();
    ht_add(output, name, frame);

    // Walk the leaf and store in the frame
    __ast_walker(leaf, output, frame, state, append_return_statement);

    if (append_return_statement) {
        // Add precaution return statement. Will be "self", but main-frame will return numerical(0) (the OS exit code)
        t_asm_opr *opr1;
        if (strcmp(name, "main") == 0) {
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, 0);
            dll_append(frame, asm_create_codeline(0, VM_LOAD_CONST, 1, opr1));
        } else {
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, "self", 0);
            dll_append(frame, asm_create_codeline(0, VM_LOAD_ID, 1, opr1));
        }
        dll_append(frame, asm_create_codeline(0, VM_RETURN, 0));
    }

    // Clean up state structure
    _ast_state_fini(state);
}


/**
 * Walk a complete AST (all frames, starting with the main frame and root of the AST)
 */
t_hash_table *ast_to_asm(t_ast_element *ast, int append_return_statement) {
    t_hash_table *output = ht_create();

    _ast_to_frame(ast, output, "main", append_return_statement);

    return output;
}

