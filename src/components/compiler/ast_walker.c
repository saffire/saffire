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

extern char *get_token_string(int token);

#define SI0(leaf)  (_ast_walker(leaf->opr.ops[0], output))
#define SI1(leaf)  (_ast_walker(leaf->opr.ops[1], output))
#define SI2(leaf)  (_ast_walker(leaf->opr.ops[2], output))
#define SI3(leaf)  (_ast_walker(leaf->opr.ops[3], output))

enum _blocktype { st_bt_none, st_bt_loop };

typedef struct _state_frame {
    int type;           // BLOCK_TYPE_* as defined in vm/frame.h
    char label[100];
} t_state_frame;


typedef struct _state {
    enum { st_load, st_store } state;
    enum { st_type_id, st_type_const } type;

    int block_cnt;                            // Last used block number
    t_state_frame blocks[BLOCK_MAX_DEPTH];    // Frame blocks
} t_state;

t_state state;

static int loop_cnt = 0;

static void _load_or_store(t_asm_opr *opr, t_dll *output) {
    switch (state.state) {
        case st_load :
            if (state.type == st_type_id) {
                dll_append(output, asm_create_codeline(VM_LOAD_ID, 1, opr));
            } else if (state.type == st_type_const) {
                dll_append(output, asm_create_codeline(VM_LOAD_CONST, 1, opr));
            }
            break;
        case st_store :
            // Store is always to ID!
            dll_append(output, asm_create_codeline(VM_STORE_ID, 1, opr));
            break;
        default :
            error_and_die(1, "Unknown load/store state for %d!", opr->type);
    }
}

/**
 *
 */
static void _ast_walker(t_ast_element *leaf, t_dll *output) {
    char start_label[100], pre_end_label[100], cmp_label[100], pre_else_label[100];
    char end_label[100], else_label[100];
    int tmp;
    t_asm_opr *opr1, *opr2;
    t_ast_element *node;
    int i;


    if (!leaf) return;

    switch (leaf->type) {
        case typeAstString :
            state.type = st_type_const;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->string.value, 0);
            _load_or_store(opr1, output);
            break;
        case typeAstNumerical :
            state.type = st_type_const;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->numerical.value);
            _load_or_store(opr1, output);
            break;
        case typeAstNull :
            // Do nothing
            break;
        case typeAstIdentifier :
            state.type = st_type_id;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->identifier.name, 0);
            _load_or_store(opr1, output);
            break;
        case typeAstClass :
            break;
        case typeAstInterface :
            break;
        case typeAstMethod :
            break;
        case typeAstOpr :
            switch (leaf->opr.oper) {
                case T_PROGRAM :
                    SI0(leaf);  // Imports
                    SI1(leaf);  // Top statements
                    break;
                case T_TOP_STATEMENTS :
                case T_STATEMENTS :
                case T_EXPRESSIONS :
                case T_USE_STATEMENTS :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        _ast_walker(leaf->opr.ops[i], output);
                    }
                    break;
                case T_IMPORT :
                    state.state = st_load;
                    SI0(leaf);
                    SI2(leaf);

                    dll_append(output, asm_create_codeline(VM_IMPORT, 0));

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, node->string.value, 0);
                    dll_append(output, asm_create_codeline(VM_STORE_ID, 1, opr1));
                    break;

                case T_ARGUMENT_LIST :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        _ast_walker(leaf->opr.ops[i], output);
                    }
                    break;

            /* Operators */
                case '+' :
                    tmp = OPERATOR_ADD;
                    goto operator;
                case '-' :
                    tmp = OPERATOR_SUB;
                    goto operator;
                case '*' :
                    tmp = OPERATOR_MUL;
                    goto operator;
                case '/' :
                    tmp = OPERATOR_DIV;
                    goto operator;
                case '%' :
                    tmp = OPERATOR_MOD;
                    goto operator;
                case T_AND :
                    tmp = OPERATOR_AND;
                    goto operator;
                case T_OR :
                    tmp = OPERATOR_OR;
                    goto operator;
                case '^' :
                    tmp = OPERATOR_XOR;
                    goto operator;
                case T_SHIFT_LEFT :
                    tmp = OPERATOR_SHL;
                    goto operator;
                case T_SHIFT_RIGHT :
                    tmp = OPERATOR_SHR;
                    goto operator;


operator:
                    state.state = st_load;
                    SI0(leaf);
                    SI1(leaf);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_OPERATOR, NULL, tmp);
                    dll_append(output, asm_create_codeline(VM_OPERATOR, 1, opr1));
                    break;

            /* Comparisons */
                case T_EQ :
                    tmp = COMPARISON_EQ;
                    goto compare;
                case T_NE :
                    tmp = COMPARISON_NE;
                    goto compare;
                case '>' :
                    tmp = COMPARISON_GT;
                    goto compare;
                case '<' :
                    tmp = COMPARISON_LT;
                    goto compare;
                case T_GE :
                    tmp = COMPARISON_GE;
                    goto compare;
                case T_LE :
                    tmp = COMPARISON_LE;
                    goto compare;
                case T_IN :
                    tmp = COMPARISON_IN;
                    goto compare;
compare:
                    state.state = st_load;
                    SI0(leaf);
                    SI1(leaf);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, tmp);
                    dll_append(output, asm_create_codeline(VM_COMPARE_OP, 1, opr1));
                    break;

                case T_IF :
                    loop_cnt++;
                    int clc = loop_cnt;

                    sprintf(pre_end_label, "if_%03d_pre_end", clc);
                    sprintf(end_label, "if_%03d_end", clc);
                    sprintf(else_label, "if_%03d_else", clc);

                    //dll_append(output, asm_create_labelline(start_label));

                    // Comparison first
                    SI0(leaf);
                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, else_label, 0);
                    } else {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, pre_end_label, 0);
                    }
                    dll_append(output, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));


                    SI1(leaf);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        dll_append(output, asm_create_labelline(else_label));
                        dll_append(output, asm_create_codeline(VM_POP_TOP, 0));

                        SI2(leaf);
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                        dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    }

                    if (leaf->opr.nops != 3) {
                        dll_append(output, asm_create_labelline(pre_end_label));
                        dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    }

                    dll_append(output, asm_create_labelline(end_label));
                    break;

                case T_BREAKELSE :
                    dll_append(output, asm_create_codeline(VM_BREAKELSE_LOOP, 0));
                    break;
                case T_BREAK :
                    dll_append(output, asm_create_codeline(VM_BREAK_LOOP, 0));
                    break;
                case T_CONTINUE :
                    // We need to find the start_label for the first encountered start state!
                    i = state.block_cnt - 1;
                    while (i >= 0 && state.blocks[i].type != BLOCK_TYPE_LOOP) i--;

                    if (i < 0) {
                        error_and_die(1, "No loop block found to continue too.");
                    }

                    // If we need to break out several loops (ie: break 2) we need to traverse back multiple
                    // loop blocks
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.blocks[i].label, 0);
                    dll_append(output, asm_create_codeline(VM_CONTINUE_LOOP, 1, opr1));

                    state.block_cnt = i+1;
                    break;


                case T_DO :
                    loop_cnt++;
                    clc = loop_cnt;
                    sprintf(state.blocks[state.block_cnt].label, "dowhile_%03d_cmp", clc);
                    state.blocks[state.block_cnt].type = BLOCK_TYPE_LOOP;
                    state.block_cnt++;

                    sprintf(cmp_label, "dowhile_%03d_cmp", clc);
                    sprintf(start_label, "dowhile_%03d", clc);
                    sprintf(pre_end_label, "dowhile_%03d_pre_end", clc);
                    sprintf(end_label, "dowhile_%03d_end", clc);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                    dll_append(output, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));
                    dll_append(output, asm_create_labelline(start_label));

                    // Body
                    SI0(leaf);

                    // Comparison
                    dll_append(output, asm_create_labelline(cmp_label));
                    state.state = st_load;
                    SI1(leaf);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, pre_end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));


                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, start_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(output, asm_create_labelline(pre_end_label));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    dll_append(output, asm_create_codeline(VM_POP_BLOCK, 0));
                    dll_append(output, asm_create_labelline(end_label));

                    state.block_cnt--;
                    break;

                case T_WHILE :
                    loop_cnt++;
                    clc = loop_cnt;
                    sprintf(state.blocks[state.block_cnt].label, "while_%03d", clc);
                    state.blocks[state.block_cnt].type = BLOCK_TYPE_LOOP;
                    state.block_cnt++;

                    sprintf(start_label, "while_%03d", clc);
                    sprintf(else_label, "while_%03d_else", clc);
                    sprintf(pre_else_label, "while_%03d_pre_else", clc);
                    sprintf(pre_end_label, "while_%03d_pre_end", clc);
                    sprintf(end_label, "while_%03d_end", clc);

                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                        opr2 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, else_label, 0);
                        dll_append(output, asm_create_codeline(VM_SETUP_ELSE_LOOP, 2, opr1, opr2));
                    } else {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                        dll_append(output, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));
                    }

                    dll_append(output, asm_create_labelline(start_label));

                    // Comparison
                    state.state = st_load;
                    SI0(leaf);
                    if (leaf->opr.nops == 3) {
                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, pre_else_label, 0);
                        dll_append(output, asm_create_codeline(VM_JUMP_IF_FIRST_FALSE, 1, opr1));
                    }
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, pre_end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));


                    // Body
                    SI1(leaf);

                    // Back to start
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, start_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));


                    // Add else in SI2 (if any) ??
                    if (leaf->opr.nops == 3) {
                        dll_append(output, asm_create_labelline(pre_else_label));
                        dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                        dll_append(output, asm_create_codeline(VM_POP_BLOCK, 0));
                        dll_append(output, asm_create_labelline(else_label));

                        // Add else body
                        SI2(leaf);

                        opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, end_label, 0);
                        dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));
                    }

                    dll_append(output, asm_create_labelline(pre_end_label));

                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    dll_append(output, asm_create_codeline(VM_POP_BLOCK, 0));
                    dll_append(output, asm_create_labelline(end_label));

                    state.block_cnt--;
                    break;

                case T_METHOD_CALL :
                    state.state = st_load;
                    SI2(leaf);       // Do argument list
                    SI0(leaf);       // Load object to call

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, node->string.value, 0);
                    opr2 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, leaf->opr.ops[2]->opr.nops);
                    dll_append(output, asm_create_codeline(VM_CALL_METHOD, 2, opr1, opr2));

                    state.state = st_store;
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    break;

                case T_ASSIGNMENT :
                    state.state = st_load;
                    SI2(leaf);

                    state.state = st_store;
                    SI0(leaf);
                    break;
                default :
                    error_and_die(1, "Unknown AST Operator: %s\n", get_token_string(leaf->opr.oper));
            }
            break;
        default :
            error_and_die(1, "Unknown AST type\n");
    }
}

/**
 *
 */
t_dll *ast_walker(t_ast_element *ast) {
    t_dll *output = dll_init();

    state.block_cnt = 0;
    _ast_walker(ast, output);

    return output;
}