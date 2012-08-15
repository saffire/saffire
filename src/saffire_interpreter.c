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
#include <string.h>
#include <math.h>
#include "saffire_interpreter.h"
#include "parser.tab.h"
#include "svar.h"
#include "ast.h"

#define SI(p)   (saffire_interpreter(p))
#define SI0(p)  (saffire_interpreter(p->opr.ops[0]))
#define SI1(p)  (saffire_interpreter(p->opr.ops[1]))
#define SI2(p)  (saffire_interpreter(p->opr.ops[2]))



static int svar_idx = 0;

svar *svar_temp_alloc(int type, char *s, long l) {
    char str[512];
    sprintf(str, "TMP-%d", svar_idx++);
    svar *tmp = svar_alloc(type, str, s, l);
    svar_print(tmp);
    return tmp;
}


svar *saffire_interpreter(t_ast_element *p) {
    svar *var, *var1, *var2, *tmp;
    long i;

    if (!p) return svar_temp_alloc(SV_NULL, NULL, 0);

    printf ("interpreting(%d)\n", p->type);

    if (p->type == 1) {
        p->type = 1;
    }

    switch (p->type) {
        case typeStrCon :
            printf ("strcon: %s\n", p->strCon.value);
            return svar_temp_alloc(SV_STRING, p->strCon.value, 0);

        case typeIntCon :
            printf ("intcon: %d\n", p->intCon.value);
            return svar_temp_alloc(SV_LONG, NULL, p->intCon.value);

        case typeVar :
            printf ("var: %s\n", p->var.name);
            return svar_temp_alloc(SV_STRING, p->var.name, 0);

        case nullVar :
            /* nop */
            break;

        case typeOpr :
            printf ("opr.oper(%d)\n", p->opr.oper);
            switch (p->opr.oper) {
                case T_WHILE :
                    while (svar_true(SI0(p))) {
                        SI1(p);
                    }
                    return svar_temp_alloc(SV_NULL, NULL, 0);

                case T_IF:
                    if (svar_true(SI0(p))) {
                        SI1(p);
                    } else if (p->opr.nops > 2) {
                        SI2(p);
                    }
                    return svar_temp_alloc(SV_NULL, NULL, 0);

                case '=' :
                    // Since we are assigning, we know the first operand is the name (string)

                    tmp = SI0(p);
                    var = svar_find(tmp->val.s);
                    if (var == NULL) {
                        // Assign value, since it doesn't exist yet or anymore
                        printf ("Primary allocation for '%s'\n", tmp->val.s);
                        var = svar_alloc(SV_NULL, tmp->val.s, NULL, 0);
                    };

                    var1 = SI1(p);
                    if (var1->type == SV_LONG) {
                        printf("setting long: %ld\n", var1->val.l);
                        var->type = SV_LONG;
                        var->val.l = var1->val.l;
                    } else {
                        printf("setting string: %s\n", var1->val.s);
                        var->type = SV_STRING;
                        var->val.s = strdup(var1->val.s);
                    }

                    svar_print(var);
                    return var;

                case ';' :
                    SI0(p);
                    return SI1(p);

                case '+' :
                        var1 = SI0(p);
                        var2 = SI1(p);

                        svar_print(var1);
                        svar_print(var2);

                        i = (var1->val.l + var2->val.l);
                        printf("ADD: %ld + %ld = %ld\n", var1->val.l, var2->val.l, i);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case '-' :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        return svar_temp_alloc(SV_LONG, NULL, (var1->val.l - var2->val.l));

                case '*' :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        return svar_temp_alloc(SV_LONG, NULL, (var1->val.l * var2->val.l));

                case '/' :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = floor(var1->val.l / var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case '<' :
                        var1 = SI0(p);
                        var2 = SI0(p);
                        i = (var1->val.l < var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case '>' :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = (var1->val.l > var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case T_GE :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = (var1->val.l >= var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case T_LE :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = (var1->val.l <= var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case T_NE :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = (var1->val.l != var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                case T_EQ :
                        var1 = SI0(p);
                        var2 = SI1(p);
                        i = (var1->val.l == var2->val.l);
                        return svar_temp_alloc(SV_LONG, NULL, i);

                default:
                    printf("Unhandled opcode: %d\n", p->opr.oper);
                    exit(1);
                    break;
            }
            break;
    }
    return 0;
}
