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
#include <stdio.h>
#include <string.h>
#include "compiler/ast_walker.h"
#include "compiler/assembler.h"
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "compiler/parser.tab.h"
#include "general/output.h"
#include "compiler/assembler.h"
#include "general/smm.h"
#include "general/dll.h"
#include "debug.h"
#include "vm/vm_opcodes.h"
#include "vm/frame.h"
#include "objects/attrib.h"

#define MAX_LABEL_LEN       100

extern char *get_token_string(int token);

#define WALK_LEAF(leaf) __ast_walker(leaf, output, frame, state)

enum _blocktype { st_bt_none, st_bt_loop };

typedef struct _state_frame {
    int type;           // BLOCK_TYPE_* as defined in vm/frame.h
    char label[MAX_LABEL_LEN];
    t_hash_table *labels;       // Defined labels inside this block (so we can tests for duplicates)
} t_state_frame;

typedef struct _state {
    enum { st_load, st_store } state;
    enum { st_type_id, st_type_const } type;
    enum { st_left, st_right, st_none } side;

    int attributes;     // Number of attributes on stack
    int loop_cnt;       // Loop counter
    int block_cnt;                            // Last used block number
    t_state_frame blocks[BLOCK_MAX_DEPTH];    // Frame blocks
} t_state;



static void _load_or_store(t_asm_opr *opr, t_dll *frame, t_state *state) {
    switch (state->state) {
        case st_load :
            if (state->type == st_type_id) {
                dll_append(frame, asm_create_codeline(VM_LOAD_ID, 1, opr));
            } else if (state->type == st_type_const) {
                dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr));
            }
            break;
        case st_store :
            // Store is always to ID!
            dll_append(frame, asm_create_codeline(VM_STORE_ID, 1, opr));
            break;
        default :
            error_and_die(1, "Unknown load/store state for %d!", opr->type);
    }
}


static void __ast_walker(t_ast_element *leaf, t_hash_table *output, t_dll *frame, t_state *state);
static void _ast_walker(t_ast_element *leaf, t_hash_table *output, const char *name);


/**
 * Cleanup callback for assignments
 */
static void ast_walker_call_method_clean_handler(t_dll *frame) {
    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
}

static int multi_assignment = 0;

/**
 * Walk the leaf into the frame. Can create new frames if needed (method bodies etc)
 */
static void __ast_walker(t_ast_element *leaf, t_hash_table *output, t_dll *frame, t_state *state) {
    char label1[MAX_LABEL_LEN], label2[MAX_LABEL_LEN], label3[MAX_LABEL_LEN];
    char label4[MAX_LABEL_LEN], label5[MAX_LABEL_LEN], label6[MAX_LABEL_LEN];
    t_asm_opr *opr1, *opr2;
    t_ast_element *node, *node2;
    t_ast_element *arglist;
    int i, clc, arg_count;

    if (!leaf) return;

    switch (leaf->type) {
        case typeAstProperty :
            state->state = st_load;
            WALK_LEAF(leaf->property.class);

            node = leaf->property.property;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, node->string.value, 0);
            if (state->side == st_left) {
                dll_append(frame, asm_create_codeline(VM_STORE_ATTRIB, 1, opr1));
            } else {
                dll_append(frame, asm_create_codeline(VM_LOAD_ATTRIB, 1, opr1));
            }
            break;

        case typeAstString :
            state->type = st_type_const;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->string.value, 0);
            _load_or_store(opr1, frame, state);
            break;
        case typeAstNumerical :
            state->type = st_type_const;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->numerical.value);
            _load_or_store(opr1, frame, state);
            break;
        case typeAstNull :
            // Store null
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, "null", 0);
            dll_append(frame, asm_create_codeline(VM_LOAD_ID, 1, opr1));
            break;
        case typeAstIdentifier :
            state->type = st_type_id;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->identifier.name, 0);
            _load_or_store(opr1, frame, state);
            break;
        case typeAstNop :
            // Nop does not emit any asmlines
            break;
        case typeAstOperator :
            state->state = st_load;
            WALK_LEAF(leaf->operator.l);
            state->state = st_load;
            WALK_LEAF(leaf->operator.r);

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
            dll_append(frame, asm_create_codeline(VM_OPERATOR, 1, opr1));
            break;

        case typeAstComparison :
            state->state = st_load;
            WALK_LEAF(leaf->comparison.r);
            state->state = st_load;
            WALK_LEAF(leaf->comparison.l);

            int tmp = 0;
            switch (leaf->comparison.cmp) {
                case T_EQ : tmp = COMPARISON_EQ; break;
                case T_NE : tmp = COMPARISON_NE; break;
                case '>'  : tmp = COMPARISON_GT; break;
                case '<'  : tmp = COMPARISON_LT; break;
                case T_GE : tmp = COMPARISON_GE; break;
                case T_LE : tmp = COMPARISON_LE; break;
                default :
                    error_and_die(1, "Unknown comparison: %s\n", leaf->comparison.cmp);
            }

            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, tmp);
            dll_append(frame, asm_create_codeline(VM_COMPARE_OP, 1, opr1));
            break;
        case typeAstAssignment :

            if (leaf->assignment.r->type == typeAstAssignment) {
                multi_assignment++;
            }
            // @TODO: We only handle =, not += -= etc
            state->state = st_load;
            WALK_LEAF(leaf->assignment.r);

            if (multi_assignment) {
                dll_append(frame, asm_create_codeline(VM_DUP_TOP, 0));
                multi_assignment--;
            }

            state->state = st_store;
            state->side = st_left;
            WALK_LEAF(leaf->assignment.l);
            state->side = st_none;
            state->state = st_load;
            break;

        case typeAstBool :
            state->loop_cnt++;
            clc = state->loop_cnt;

            if (leaf->boolop.op == 0) {
                // Do boolean AND
                sprintf(label1, "and_%03d_end", clc);

                WALK_LEAF(leaf->boolop.l);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                dll_append(frame, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));

                WALK_LEAF(leaf->boolop.r);

                // Skip checks&jumps, since we can safely use the result from RHS as result for boolean op node

                dll_append(frame, asm_create_labelline(label1));
            }

            if (leaf->boolop.op == 1) {
                // Do boolean OR
                sprintf(label1, "or_%03d_end", clc);

                WALK_LEAF(leaf->boolop.l);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                dll_append(frame, asm_create_codeline(VM_JUMP_IF_TRUE, 1, opr1));
                dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));

                WALK_LEAF(leaf->boolop.r);

                // Skip checks&jumps, since we can safely use the result from RHS as result for boolean op node

                dll_append(frame, asm_create_labelline(label1));
            }

            break;

        case typeAstClass :
            // Iterate body
            WALK_LEAF(leaf->class.body);

            // Push parent class
            state->state = st_load;
            WALK_LEAF(leaf->class.extends);

            // Push flags
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->class.modifiers);
            dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

            // Push the name
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->class.name, 0);
            dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

            // Build class code
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, state->attributes);
            dll_append(frame, asm_create_codeline(VM_BUILD_CLASS, 1, opr1));
            state->attributes = 0;

            // Store class into identifier
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->class.name, 0);
            dll_append(frame, asm_create_codeline(VM_STORE_ID, 1, opr1));

            break;
        case typeAstInterface :
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
                        state->state = st_load;
                        WALK_LEAF(arg->opr.ops[0]);     // type hint
                        WALK_LEAF(arg->opr.ops[1]);     // name
                        WALK_LEAF(arg->opr.ops[2]);     // value
                    }
                }

                // Push method flags
                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.method_flags);
                dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));


                // Push body
                state->loop_cnt++;
                clc = state->loop_cnt;
                sprintf(label1, "frame_%03d", clc);

                opr1 = asm_create_opr(ASM_LINE_TYPE_OP_CODE, label1, 0);
                dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

                // Walk the body inside a new frame!
                _ast_walker(leaf->attribute.value, output, label1);
            }

            if (leaf->attribute.attrib_type == ATTRIB_TYPE_CONSTANT) {
                state->state = st_load;
                WALK_LEAF(leaf->attribute.value);
            }

            if (leaf->attribute.attrib_type == ATTRIB_TYPE_PROPERTY) {
                state->state = st_load;
                WALK_LEAF(leaf->attribute.value);
            }


            // Store visibility
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.visibility);
            dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

            // Store access
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->attribute.access);
            dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

            // Create constant
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, leaf->attribute.attrib_type);
            opr2 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, arg_count);
            dll_append(frame, asm_create_codeline(VM_BUILD_ATTRIB, 2, opr1, opr2));

            // Name of the id to store the constant into
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->attribute.name, 0);
            dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));

            state->attributes++;
            break;

        case typeAstGroup :
//            for (int i=0; i!=leaf->opr.nops; i++) {
//                WALK_LEAF(leaf->opr.ops[i]);
//                if(leaf->opr.ops[i]->opr.oper != T_ASSIGNMENT && leaf->opr.ops[i]->clean_handler) {
//                    leaf->opr.ops[i]->clean_handler(frame);
//                }
//            }
//            break;
            for (int i=0; i!=leaf->group.len; i++) {
                WALK_LEAF(leaf->group.items[i]);
            }
            break;


        case typeAstOpr :
            switch (leaf->opr.oper) {
                case T_PROGRAM :
                    WALK_LEAF(leaf->opr.ops[0]);        // Imports
                    WALK_LEAF(leaf->opr.ops[1]);        // Top statements
                    break;
                case T_IMPORT :
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[0]);
                    WALK_LEAF(leaf->opr.ops[2]);

                    dll_append(frame, asm_create_codeline(VM_IMPORT, 0));

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, node->string.value, 0);
                    dll_append(frame, asm_create_codeline(VM_STORE_ID, 1, opr1));
                    break;

                case T_CALL_ARGUMENT_LIST :
                case T_ARGUMENT_LIST :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        WALK_LEAF(leaf->opr.ops[i]);
                    }
                    break;

                case T_RETURN :
                    WALK_LEAF(leaf->opr.ops[0]);
                    dll_append(frame, asm_create_codeline(VM_RETURN, 0));

                    break;

                case T_IF :
                    state->loop_cnt++;
                    clc = state->loop_cnt;

                    sprintf(label2, "if_%03d_pre_end", clc);
                    sprintf(label5, "if_%03d_end", clc);
                    sprintf(label6, "if_%03d_else", clc);

                    // Comparison first
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label6, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));

                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
                    WALK_LEAF(leaf->opr.ops[1]);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));


                    // Always add else-label
                    dll_append(frame, asm_create_labelline(label6));

                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        WALK_LEAF(leaf->opr.ops[2]);
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label5));
                    break;

                case T_BREAKELSE :
                    dll_append(frame, asm_create_codeline(VM_BREAKELSE_LOOP, 0));
                    break;
                case T_BREAK :
                    dll_append(frame, asm_create_codeline(VM_BREAK_LOOP, 0));
                    break;
                case T_CONTINUE :
                    // We need to find the label1 for the first encountered start state!
                    i = state->block_cnt - 1;
                    while (i >= 0 && state->blocks[i].type != BLOCK_TYPE_LOOP) i--;

                    if (i < 0) {
                        error_and_die(1, "No loop block found to continue too.");
                    }

                    // If we need to break out several loops (ie: break 2) we need to traverse back multiple
                    // loop blocks
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state->blocks[i].label, 0);
                    dll_append(frame, asm_create_codeline(VM_CONTINUE_LOOP, 1, opr1));

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
                    dll_append(frame, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));
                    dll_append(frame, asm_create_labelline(label1));

                    // Body
                    WALK_LEAF(leaf->opr.ops[0]);

                    // Comparison
                    dll_append(frame, asm_create_labelline(label3));
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[1]);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));


                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label2));
                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(VM_POP_BLOCK, 0));
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
                        dll_append(frame, asm_create_codeline(VM_SETUP_ELSE_LOOP, 2, opr1, opr2));
                    } else {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        dll_append(frame, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));
                    }

                    dll_append(frame, asm_create_labelline(label1));

                    // Comparison
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[0]);
                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label4, 0);
                        dll_append(frame, asm_create_codeline(VM_JUMP_IF_FIRST_FALSE, 1, opr1));
                    }
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label2, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));


                    // Body
                    WALK_LEAF(leaf->opr.ops[1]);

                    // Back to start
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));


                    // Add else (if any)
                    if (leaf->opr.nops == 3) {
                        dll_append(frame, asm_create_labelline(label4));
                        dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
                        dll_append(frame, asm_create_codeline(VM_POP_BLOCK, 0));
                        dll_append(frame, asm_create_labelline(label6));

                        // Add else body
                        WALK_LEAF(leaf->opr.ops[2]);

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                        dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));
                    }

                    dll_append(frame, asm_create_labelline(label2));

                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(VM_POP_BLOCK, 0));
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
                    WALK_LEAF(leaf->opr.ops[0]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label5, 0);
                    dll_append(frame, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));

                    // Add comparison
                    dll_append(frame, asm_create_labelline(label3));
                    WALK_LEAF(leaf->opr.ops[1]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label4, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));

                    // Add body
                    WALK_LEAF(leaf->opr.ops[3]);

                    // Add incdec
                    dll_append(frame, asm_create_labelline(label6));
                    WALK_LEAF(leaf->opr.ops[2]);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label3, 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(frame, asm_create_labelline(label4));
                    dll_append(frame, asm_create_codeline(VM_POP_TOP, 0));
                    dll_append(frame, asm_create_codeline(VM_POP_BLOCK, 0));

                    dll_append(frame, asm_create_labelline(label5));

                    state->block_cnt--;
                    break;

                case T_CALL :
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[1]);       // Do argument list
                    WALK_LEAF(leaf->opr.ops[0]);       // Load callable

                    int arg_count = leaf->opr.ops[1]->group.len;

                    // @TODO: Dummy. Remove me! (@TODO: is this true?)
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, arg_count);
                    dll_append(frame, asm_create_codeline(VM_CALL, 1, opr1));

                    leaf->clean_handler = &ast_walker_call_method_clean_handler;
                    break;

                case T_SUBSCRIPT :
                    state->state = st_load;
                    node = leaf->opr.ops[1];
                    WALK_LEAF(node->group.items[0]);
                    WALK_LEAF(node->group.items[1]);

                    // load "splice" attrib
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[0]);       // Load object to subscribe

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, "splice", 0);
                    dll_append(frame, asm_create_codeline(VM_LOAD_ATTRIB, 1, opr1));

                    // Call splice
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, 2);
                    dll_append(frame, asm_create_codeline(VM_CALL, 1, opr1));

                    leaf->clean_handler = &ast_walker_call_method_clean_handler;
                    break;

                case T_DATASTRUCT :
                    state->state = st_load;
                    WALK_LEAF(leaf->opr.ops[0]);

                    // Iterate elements
                    node = leaf->opr.ops[1];
                    for (int i=0; i!=node->group.len; i++) {
                        node2 = node->group.items[i];
                        // Iterate attributes
                        for (int j=0; j!=node2->group.len; j++) {
                            state->state = st_load;
                            WALK_LEAF(node2->group.items[j]);
                        }
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, node2->group.len);
                        dll_append(frame, asm_create_codeline(VM_BUILD_TUPLE, 1, opr1));
                    }

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, node->group.len);
                    dll_append(frame, asm_create_codeline(VM_BUILD_DATASTRUCT, 1, opr1));
                    break;

                case T_GOTO :
                    node = leaf->opr.ops[0];
                    sprintf(label1, "userlabel_%s", node->string.value);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, label1   , 0);
                    dll_append(frame, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));
                    break;

                case T_LABEL :
                    node = leaf->opr.ops[0];
                    // Check for duplicate label inside current block
                    sprintf(label1, "userlabel_%s", node->string.value);
                    if (ht_exists(state->blocks[state->block_cnt].labels, label1)) {
                        error_and_die(1, "Duplicate label '%s' found\n", label1);
                    }
                    ht_add(state->blocks[state->block_cnt].labels, label1, (void *)1);

                    dll_append(frame, asm_create_labelline(label1));
                    break;

                case T_FOREACH :
                    break;

                default :
                    error_and_die(1, "Unknown AST Operator: %s\n", get_token_string(leaf->opr.oper));
            }
            break;
        default :
            error_and_die(1, "Unknown AST type : %d\n", leaf->type);
    }
}


/**
 * Initialize a new frame and walk the leaf into this frame
 */
static void _ast_walker(t_ast_element *leaf, t_hash_table *output, const char *name) {
    // Initialize frame state
    t_state state;
    state.loop_cnt = 0;
    state.block_cnt = 0;
    state.attributes = 0;
    for (int i=0; i!=BLOCK_MAX_DEPTH; i++) {
        state.blocks[i].labels = ht_create();
    }

    // Create frame and add to output
    t_dll *frame = dll_init();
    ht_add(output, name, frame);

    // Walk the leaf and store in the frame
    __ast_walker(leaf, output, frame, &state);

    // Destroy all the hashtables that might be available
    for (int i=0; i!=BLOCK_MAX_DEPTH; i++) {
        if (state.blocks[i].labels) {
            ht_destroy(state.blocks[i].labels);
        }
    }


    // Add precaution return statement. Will be "self", but main-frame will return numerical(0) (the OS exit code)
    t_asm_opr *opr1;
    if (strcmp(name, "main") == 0) {
        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, 0);
    } else {
        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, "self", 0);
    }
    dll_append(frame, asm_create_codeline(VM_LOAD_CONST, 1, opr1));
    dll_append(frame, asm_create_codeline(VM_RETURN, 0));
}


/**
 * Walk a complete AST (all frames, starting with the main frame and root of the AST)
 */
t_hash_table *ast_walker(t_ast_element *ast) {
    t_hash_table *output = ht_create();

    _ast_walker(ast, output, "main");

    return output;
}