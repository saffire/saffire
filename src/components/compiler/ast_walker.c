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
#include "compiler/ast_walker.h"
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "compiler/parser.tab.h"
#include "general/output.h"
#include "debug.h"

#define SI0(leaf, state)  (_ast_walker(leaf->opr.ops[0], state))
#define SI1(leaf, state)  (_ast_walker(leaf->opr.ops[1], state))
#define SI2(leaf, state)  (_ast_walker(leaf->opr.ops[2], state))
#define SI3(leaf, state)  (_ast_walker(leaf->opr.ops[3], state))

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


void emitraw(char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void emit(t_state state, char *format, ...) {
    va_list args;

    if (state.state == ST_LOAD) {
        if (state.type == ST_T_ID) {
            emitraw("\tLOAD_ID\t\t");
        } else if (state.type == ST_T_CONST) {
            emitraw("\tLOAD_CONST\t");
        }
        emitraw("\t");
    } else if (state.state == ST_STORE) {
        emitraw("\tSTORE_ID\t\t");
    }

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

static int loop_cnt = 0;

t_bytecode *_ast_walker(t_ast_element *leaf, t_state state) {
//    printf("astwalk: %d  (ST: %d  TP:%d)\n", leaf->type, state.state, state.type);
    if (!leaf) return NULL;

    switch (leaf->type) {
        case typeAstString :
            state.type = ST_T_CONST;
            emit(state, "\"%s\"", leaf->string.value);
            break;
        case typeAstNumerical :
            state.type = ST_T_CONST;
            emit(state, "%d", leaf->numerical.value);
            break;
        case typeAstNull :
//            state.type = ST_T_CONST;
//            emit(state, "Null");
            break;
        case typeAstIdentifier :
            state.type = ST_T_ID;
            emit(state, "%s", leaf->identifier.name);
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
                        _ast_walker(leaf->opr.ops[i], state);
                    }
                    break;
                case T_IMPORT :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    emitraw("\n");
                    SI2(leaf, state);
                    emitraw("\n");

                    t_ast_element *node2 = leaf->opr.ops[1];
                    emitraw("\tIMPORT\n");
                    emitraw("\tSTORE_ID\t\t%s\n", node2->string.value);
                    emitraw("\n");

                    emitraw("\n");
                    break;

                case T_ARGUMENT_LIST :
                    for (int i=0; i!=leaf->opr.nops; i++) {
                        _ast_walker(leaf->opr.ops[i], state);
                    }
                    break;

                case '+' :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    emitraw("\n");
                    SI1(leaf, state);
                    emitraw("\n");
                    emitraw("\tBINARY_ADD\n");
                    break;

                case T_EQ :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    emitraw("\n");
                    SI1(leaf, state);
                    emitraw("\n");
                    emitraw("\tCOMPARE_OP\t\tOP_EQ\n");
                    emitraw("\tJUMP_IF_FALSE\t%s\n", state.pre_end_label, state.labelcount);
                    emitraw("\tPOP_TOP\n");
                    emitraw("\n");
                    break;

                case '<' :
                    state.state = ST_LOAD;
                    SI0(leaf, state);
                    emitraw("\n");
                    SI1(leaf, state);
                    emitraw("\n");
                    emitraw("\tCOMPARE_OP\t\tOP_LT\n");
                    emitraw("\tJUMP_IF_FALSE\t%s\n", state.pre_end_label, state.labelcount);
                    emitraw("\tPOP_TOP\n");
                    emitraw("\n");
                    break;

                case T_IF :
                    loop_cnt++;
                    state.labelcount = loop_cnt;
                    sprintf(state.start_label, "#if_%03d", state.labelcount);
                    sprintf(state.pre_end_label, "#else_if_%03d", state.labelcount);
                    sprintf(state.end_label, "#end_of_%03d", state.labelcount);
                    sprintf(state.else_label, "#else_if_%03d", state.labelcount);

                    emitraw("%s:\n", state.start_label);

                    // Comparison first
                    SI0(leaf, state);
                    emitraw("\n");

                    SI1(leaf, state);
                    emitraw("\n");
                    emitraw("\tJUMP_ABSOLUTE\t%s\n", state.end_label);
                    emitraw("%s:\n", state.else_label);
                    emitraw("\tPOP_TOP\n");
                    emitraw("\n");

                    // Do else body, if there is one
                    if (leaf->opr.nops == 3) {
                        SI2(leaf, state);
                    }

                    emitraw("%s:\n", state.end_label);
                    emitraw("\n");

                    break;

                case T_WHILE :
                    loop_cnt++;
                    state.labelcount = loop_cnt;

                    sprintf(state.start_label, "#while_%03d", state.labelcount);
                    sprintf(state.pre_end_label, "#end_while_%03d_pb", state.labelcount);
                    sprintf(state.end_label, "#end_while_%03d", state.labelcount);

                    emitraw("\tSETUP_LOOP\t\t%s\n", state.end_label);
                    emitraw("%s:\n", state.start_label);

                    // Comparison
                    SI0(leaf, state);

                    // Body
                    SI1(leaf, state);

                    // Add else in SI2 (if any)

                    emitraw("\tJUMP_ABSOLUTE\t%s\n", state.start_label);
                    emitraw("%s:\n", state.pre_end_label);
                    emitraw("\tPOP_BLOCK\n");
                    emitraw("%s:\n", state.end_label);
                    emitraw("\n");

                    break;

                case T_METHOD_CALL :
                    state.state = ST_LOAD;
                    SI2(leaf, state);  // Do argument list
                    emitraw("\n");
                    SI0(leaf, state);
                    emitraw("\n");

                    emitraw("\tCALL_METHOD\t\t\"");
                    int old_state = state.state;
                    state.state = ST_NULL;
                    SI1(leaf, state);
                    state.state = old_state;
                    emitraw("\", $%d", leaf->opr.ops[2]->opr.nops);
                    emitraw("\n");

                    state.state = ST_STORE;
                    emitraw("\tPOP_TOP\n");
                    emitraw("\n");
                    break;

                case T_ASSIGNMENT :
                    state.state = ST_LOAD;
                    SI2(leaf, state);
                    emitraw("\n");

                    state.state = ST_STORE;
                    SI0(leaf, state);
                    emitraw("\n");

                    emitraw("\n");
                    break;
                default :
                    error_and_die(1, "Unknown AST Operator: %02X (%d)", leaf->opr.oper, leaf->opr.oper);
            }
            break;
        default :
            error_and_die(1, "Unknown AST type");
    }
}

t_bytecode *ast_walker(t_ast_element *ast) {
    t_state state;

    emitraw(";\n");
    emitraw("; Bytecode generation\n");
    emitraw(";\n");
    emitraw("\n");

    state.state = ST_NULL;
    return _ast_walker(ast, state);
}