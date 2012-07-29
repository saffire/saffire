#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "saffire_parser.h"
#include "saffire_compile.h"
#include "node.h"
#include "parser.tab.h"
#include "svar.h"

extern void yyerror(const char *err);


void saffire_execute(nodeType *p) {
    saffire_compile(p);
}


//#define SIZEOF_NODETYPE ((char *)&p->con - (char *)p)

nodeType *saffire_strCon(char *value) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeStrCon;
    p->strCon.value = strdup(value);

    return p;
}

nodeType *saffire_intCon(int value) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeIntCon;
    p->intCon.value = value;

    return p;
}

nodeType *saffire_var(char *var_name) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeVar;
    p->var.name = strdup(var_name);

    return p;
}

nodeType *saffire_opr(int opr, int nops, ...) {
    va_list ap;
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }
    if ((p->opr.ops = malloc (nops * sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeOpr;
    p->opr.oper = opr;
    p->opr.nops = nops;

    va_start(ap, nops);
    for (int i =0; i < nops; i++) {
        p->opr.ops[i] = va_arg(ap, nodeType *);
    }
    va_end(ap);

    return p;
}

void saffire_free_node(nodeType *p) {
    if (!p) return;

    // @TODO: If it's a strConOpr, we must free our char as well!
    // @TODO: If it's a varOpr, we must free our svar as well

    if (p->type == typeOpr) {
        for (int i=0; i < p->opr.nops; i++) {
            saffire_free_node(p->opr.ops[i]);
        }
        free(p->opr.ops);
    }
    free(p);
}


/*
 * This file should hold all information to parse the tree into bytecode probably.
 * For now, it pretty much interpret the code during parsing.
 */

void saffire_do_program_begin(char *title) {
}

void saffire_do_program_end() {
}

void saffire_do_assign(char *var_name, char *val) {
    // Default to a string
    char type = SV_STRING;

    // Check if the WHOLE val is a numerical value
    char *endptr;
    long num = strtol(val, &endptr, 10);
    if (endptr == val+strlen(val)) {
        // only when the whole value is converted, it's a long
        // "20" is a long, "20A" isn't.
        type = SV_LONG;
    }

    // Try and search for this value
    svar *var = svar_find(var_name);
    if (var == NULL) {
        //  Not found, initial assignment
        var = svar_alloc(type, var_name, type == SV_STRING ? (void *)val : (void *)num);
    } else {
        // Found, check if types match: cannot change from string to int for example
        if (var->type != type) {
            printf("We cannot switch types!");
            exit(1);
        }
        // add number
        if (type == SV_STRING) {
            var->val.s = strdup(val);
        } else if (type == SV_LONG) {
            var->val.l = num;
        }
    }
}

void saffire_do_print(char *str) {
    if (str[0] == '$') {
        svar *var = svar_find(str);
        if (var == NULL) {
            printf("Cannot find variable %s", str);
            exit(1);
        }
        if (var->type == SV_STRING) {
            printf("%s", var->val.s);
        } else if (var->type == SV_LONG) {
            printf("%ld", var->val.l);
        } else if (var->type == SV_DOUBLE) {
            printf("%g", var->val.d);
        }
    } else {
        //str[strlen(str)-1] = '\0';
        //printf("%s", str+1);
        printf("%s", str);
    }
}

void _do_incdec(char *var_name, int inc) {
    svar *var = svar_find(var_name);
    if (var == NULL) {
        printf("Warning: var is not initialized.");
        var = svar_alloc(SV_LONG, var_name, 0);
    }

    if (var->type != SV_LONG) {
        printf("Warning: var is not a number");
        return;
    }

    if (inc) {
        var->val.l++;
    } else {
        var->val.l--;
    }

    //print_var(var);
}

void saffire_do_pre_inc(char *var_name) {
    _do_incdec(var_name, 1);
}

void saffire_do_post_inc(char *var_name) {
    _do_incdec(var_name, 1);
}

void saffire_do_pre_dec(char *var_name) {
    _do_incdec(var_name, 0);
}

void saffire_do_post_dec(char *var_name) {
    _do_incdec(var_name, 0);
}