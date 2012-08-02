#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "saffire_interpreter.h"
#include "parser.tab.h"
#include "svar.h"

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


svar *saffire_interpreter(nodeType *p) {
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

                case T_PRINT :
                    var1 = SI0(p);
                    printf("*** PRINTING DATA!!!!\n");
                    if (var1->type == SV_STRING) {
                        if (var1->val.s[0] == '$') {
                            // found $<val>
                            printf("doing string!\n");
                            var2 = svar_find(var1->val.s);
                            svar_print(var2);
                            if (var2->type == SV_STRING) {
                                printf("print: '%s'\n", var2->val.s);
                            } else {
                                printf("print: '%ld'\n", var2->val.l);
                            }
                        } else {
                            printf("print: '%s'\n", var1->val.s);
                        }
                    } else {
                        printf("print: '%ld'\n", var1->val.l);
                    }
                    return var1;

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