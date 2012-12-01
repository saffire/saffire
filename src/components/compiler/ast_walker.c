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

#define SI0(leaf, state)  (_ast_walker(leaf->opr.ops[0], state, output))
#define SI1(leaf, state)  (_ast_walker(leaf->opr.ops[1], state, output))
#define SI2(leaf, state)  (_ast_walker(leaf->opr.ops[2], state, output))
#define SI3(leaf, state)  (_ast_walker(leaf->opr.ops[3], state, output))

typedef struct _state {
    char state;
    char type;
    int  labelcount;
    char start_label[100];
    char else_label[100];
    char pre_end_label[100];
    char end_label[100];
} t_state;


#define ST_NULL     0
#define ST_LOAD     1
#define ST_STORE    2

#define ST_T_ID     1
#define ST_T_CONST  2


static int loop_cnt = 0;

static void _load_or_store(t_state state, t_asm_opr *opr, t_dll *output) {
    switch (state.state) {
        case ST_LOAD :
            if (state.type == ST_T_ID) {
                dll_append(output, asm_create_codeline(VM_LOAD_ID, 1, opr));
            } else if (state.type == ST_T_CONST) {
                dll_append(output, asm_create_codeline(VM_LOAD_CONST, 1, opr));
            }
            break;
        case ST_STORE :
            // Store is always to ID!
            dll_append(output, asm_create_codeline(VM_STORE_ID, 1, opr));
            break;
        default :
            error_and_die(1, "Unknown state!");
    }
}

/**
 *
 */
static void _ast_walker(t_ast_element *leaf, t_state state, t_dll *output) {
    t_asm_opr *opr1, *opr2;
    t_ast_element *node;
    if (!leaf) return;

    switch (leaf->type) {
        case typeAstString :
            state.type = ST_T_CONST;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, leaf->string.value, 0);
            _load_or_store(state, opr1, output);
            break;
        case typeAstNumerical :
            state.type = ST_T_CONST;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_NUM, NULL, leaf->numerical.value);
            _load_or_store(state, opr1, output);
            break;
        case typeAstNull :
            // Do nothing
            break;
        case typeAstIdentifier :
            state.type = ST_T_ID;
            opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, leaf->identifier.name, 0);
            _load_or_store(state, opr1, output);
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
                    SI0(leaf, state);  // Imports
                    SI1(leaf, state);  // Top statements
                    break;
                case T_TOP_STATEMENTS :
                case T_STATEMENTS :
                case T_EXPRESSIONS :
                case T_USE_STATEMENTS :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        _ast_walker(leaf->opr.ops[i], state, output);
                    }
                    break;
                case T_IMPORT :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    SI2(leaf, state);

                    dll_append(output, asm_create_codeline(VM_IMPORT, 0));

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_ID, node->string.value, 0);
                    dll_append(output, asm_create_codeline(VM_STORE_ID, 1, opr1));
                    break;

                case T_ARGUMENT_LIST :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        _ast_walker(leaf->opr.ops[i], state, output);
                    }
                    break;

                case '+' :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    SI1(leaf, state);
                    dll_append(output, asm_create_codeline(VM_BINARY_ADD, 0));
                    break;

                case T_EQ :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    SI1(leaf, state);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, COMPARISON_EQ);
                    dll_append(output, asm_create_codeline(VM_COMPARE_OP, 1, opr1));

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.pre_end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    break;

                case '<' :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    SI1(leaf, state);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_COMPARE, NULL, COMPARISON_LT);
                    dll_append(output, asm_create_codeline(VM_COMPARE_OP, 1, opr1));

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.pre_end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_IF_FALSE, 1, opr1));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    break;

                case T_IF :
                    loop_cnt++;
                    state.labelcount = loop_cnt;

                    sprintf(state.start_label, "if_%03d", state.labelcount);
                    sprintf(state.pre_end_label, "else_if_%03d", state.labelcount);
                    sprintf(state.end_label, "end_of_%03d", state.labelcount);
                    sprintf(state.else_label, "else_if_%03d", state.labelcount);

                    dll_append(output, asm_create_labelline(state.start_label));

                    // Comparison first
                    SI0(leaf, state);

                    SI1(leaf, state);
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.end_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(output, asm_create_labelline(state.else_label));
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        SI2(leaf, state);
                    }

                    dll_append(output, asm_create_labelline(state.end_label));

                    break;

                case T_WHILE :
                    loop_cnt++;
                    state.labelcount = loop_cnt;

                    sprintf(state.start_label, "while_%03d", state.labelcount);
                    sprintf(state.pre_end_label, "end_while_%03d_pb", state.labelcount);
                    sprintf(state.end_label, "end_while_%03d", state.labelcount);

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.end_label, 0);
                    dll_append(output, asm_create_codeline(VM_SETUP_LOOP, 1, opr1));

                    dll_append(output, asm_create_labelline(state.start_label));

                    // Comparison
                    SI0(leaf, state);

                    // Body
                    SI1(leaf, state);

                    // Add else in SI2 (if any) ??

                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_LABEL, state.start_label, 0);
                    dll_append(output, asm_create_codeline(VM_JUMP_ABSOLUTE, 1, opr1));

                    dll_append(output, asm_create_labelline(state.pre_end_label));
                    dll_append(output, asm_create_codeline(VM_POP_BLOCK, 0));
                    dll_append(output, asm_create_labelline(state.end_label));

                    break;

                case T_METHOD_CALL :
                    state.state = ST_LOAD;
                    SI2(leaf, state);       // Do argument list
                    SI0(leaf, state);       // Load object to call

                    node = leaf->opr.ops[1];
                    opr1 = asm_create_opr(ASM_LINE_TYPE_OP_STRING, node->string.value, 0);
                    opr2 = asm_create_opr(ASM_LINE_TYPE_OP_REALNUM, NULL, leaf->opr.ops[2]->opr.nops);
                    dll_append(output, asm_create_codeline(VM_CALL_METHOD, 2, opr1, opr2));

                    state.state = ST_STORE;
                    dll_append(output, asm_create_codeline(VM_POP_TOP, 0));
                    break;

                case T_ASSIGNMENT :
                    state.state = ST_LOAD;
                    SI2(leaf, state);

                    state.state = ST_STORE;
                    SI0(leaf, state);
                    break;
                default :
                    error_and_die(1, "Unknown AST Operator: %02X (%d)", leaf->opr.oper, leaf->opr.oper);
            }
            break;
        default :
            error_and_die(1, "Unknown AST type");
    }
}

/**
 *
 */
t_dll *ast_walker(t_ast_element *ast) {
    t_dll *output = dll_init();

    t_state state;
    state.state = ST_NULL;
    _ast_walker(ast, state, output);

    return output;
}