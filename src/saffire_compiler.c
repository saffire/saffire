/*
 Copyright (c) 2012, Joshua Thijssen
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
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include "saffire_compiler.h"
#include "parser.tab.h"
#include "ast.h"

static int lbl = 0;

#define SC0(p) saffire_compiler(p->opr.ops[0])
#define SC1(p) saffire_compiler(p->opr.ops[1])
#define SC2(p) saffire_compiler(p->opr.ops[2])


void saffire_compiler(t_ast_element *p) {
    int lbl1, lbl2;

    if (!p) return;

    switch (p->type) {
        case typeStrCon :
            printf("\tpush\t\"%s\"\n", p->strCon.value);
            break;

        case typeIntCon :
            printf("\tpush\t%d\n", p->intCon.value);
            break;

        case typeVar :
            printf("\tpush\t~\"%s\"\n", p->var.name);
            break;

        case typeOpr :
            switch (p->opr.oper) {
                case T_USE :
                    printf("\tuse_alias\t\"%s\"\t\"%s\"\n", p->opr.ops[0]->var.name, p->opr.ops[1]->var.name);
                    break;

                case T_WHILE :
                    printf("L%03d:\n", lbl1 = lbl++);
                    SC0(p);
                    printf("\tjz\tL%03d\n", lbl2 = lbl++);
                    SC1(p);
                    printf("\tjmp\tL%03d\n", lbl1);
                    printf("L%03d:\n", lbl2);
                    break;

                case T_IF:
                    SC0(p);
                    if (p->opr.nops > 2) {
                        // if else
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        SC1(p);
                        printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                        printf("L%03d:\n", lbl1);
                        SC2(p);
                        printf("L%03d:\n", lbl2);
                    } else {
                        // if
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        SC1(p);
                        printf("L%03d:\n", lbl1);
                    }
                    break;

                case '=' :
                    SC1(p);
                    printf("\tpop\t~\"%s\"\n", p->opr.ops[0]->var.name);
                    break;

                default :
                    SC0(p);
                    SC1(p);
                    switch (p->opr.oper) {
                        case '+' : printf("\tadd\n"); break;
                        case '-' : printf("\tsub\n"); break;
                        case '*' : printf("\tmul\n"); break;
                        case '/' : printf("\tdiv\n"); break;
                        case '^' : printf("\tpow\n"); break;
                        case '<' : printf("\tcompLT\n"); break;
                        case '>' : printf("\tcompGT\n"); break;
                        case T_GE : printf("\tcompGE\n"); break;
                        case T_LE : printf("\tcompLE\n"); break;
                        case T_NE : printf("\tcompNE\n"); break;
                        case T_EQ : printf("\tcompEQ\n"); break;
                        case ';' :
                                printf("\n");
                                // End of statement?
                                break;
                        default:
                            printf("Unhandled opcode: %d\n", p->opr.oper);
                            break;
                    }
            }
    }
}