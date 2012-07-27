#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.tab.h"

extern int yyparse();
extern FILE *yyin;


const VT_NULL   = 0;
const VT_LONG   = 1;
const VT_STRING = 2;

typedef struct var {
    char type;
    char *name;
    union {
        long num;
        char *str;

    } val;
} t_var;

#define MAX_VARS 50
t_var *vars[MAX_VARS];

void print_var(t_var *var) {
    printf("VAR:\n");
    printf("  Name: %s\n", var->name);
    printf("  Type: %d\n", var->type);
    if (var->type == VT_LONG) {
        printf("  Val:  %d\n", var->val.num);
    }
    if (var->type == VT_STRING) {
        printf("  Val:  %s\n", var->val.str);
    }
    printf("\n");
}





int var_find_slot() {
    int i;
    for (i=0; i!=MAX_VARS; i++) {
        if (vars[i] == NULL) {
            return i;
        }
    }
    return -1;
}

t_var *var_find(char *name) {
    int i;
    for (i=0; i!=MAX_VARS; i++) {
        if (vars[i] == NULL) continue;

        if (strcmp(vars[i]->name, name) == 0) {
            return vars[i];
        }
    }

    return NULL;
}

t_var *var_alloc(char type, char *name, void *val) {
    int var_idx = var_find_slot();
    if (var_idx == -1) {
        fprintf(stderr, "No more room for variables!");
        exit(1);
    }
    t_var *var = (t_var *)malloc(sizeof(t_var));

    var->type = type;
    var->name = strdup(name);

    if (var->type == VT_LONG) {
        var->val.num = (long)val;
    } else if (var->type == VT_STRING) {
        var->val.str = strdup((char *)val);
    }

    vars[var_idx] = var;
    return var;
}

var_free(t_var *var) {
    free(var->name);
    if (var->type == VT_STRING) {
        free(var->val.str);
    }
}

void _do_incdec(char *var_name, int inc) {
    t_var *var = var_find(var_name);
    if (var == NULL) {
        printf("Warning: var is not initialized.");
        var = var_alloc(VT_LONG, var_name, 0);
    }

    if (var->type != VT_LONG) {
        printf("Warning: var is not a number");
        return;
    }

    if (inc) {
        var->val.num++;
    } else {
        var->val.num--;
    }

    //print_var(var);
}

void saffire_do_program_begin(char *title) {
    printf("do_program_begin: %s \n", title);
}
void saffire_do_program_end() {
    printf("do_program_end\n");
}

void saffire_ext_info() {
    printf("extinfo()\n");
}
void saffire_do_assign(char *var_name, char *val) {
    printf("assign(%s => %s)\n", var_name, val);


    char type = VT_STRING;
    char *endptr;
    long num = strtol(val, &endptr, 10);
    if (endptr == val+strlen(val)) {
        type = VT_LONG;
    }


    t_var *var = var_find(var_name);
    if (var == NULL) {
        printf("Initial assign\n");
        var = var_alloc(type, var_name, type == VT_STRING ? (void *)val : (void *)num);
    } else {
        if (var->type != type) {
            printf("We cannot switch types!");
            exit(1);
        }
        if (type == VT_STRING) {
            var->val.str = strdup(val);
        } else if (type = VT_LONG) {
            var->val.num = num;
        }
    }

    //print_var(var);
}


void saffire_do_print(char *str) {
    printf("print(%s)\n", str);

    if (str[0] == '$') {
        t_var *var = var_find(str);
        if (var == NULL) {
            printf("Cannot find variable %s", str);
            exit(1);
        }
        if (var->type == VT_STRING) {
            printf("print_var_str(%s)", var->val.str);
        } else if (var->type = VT_LONG) {
            printf("print_var_num(%d)", var->val.num);
        }
    } else {
        printf(str);
    }
}

void saffire_do_pre_inc(char *var_name) {
    printf("do_pre_inc()\n");
    _do_incdec(var_name, 1);
}

void saffire_do_post_inc(char *var_name) {
    printf("do_post_inc()\n");
    _do_incdec(var_name, 1);
}

void saffire_do_pre_dec(char *var_name) {
    printf("do_pre_dec()\n");
    _do_incdec(var_name, 0);
}

void saffire_do_post_dec(char *var_name) {
    printf("do_post_dec()\n");
    _do_incdec(var_name, 0);
}


void saffire_inner_statement() {
    printf("inner_statement();\n");
}

void saffire_do_expr() {
    printf("expr\n");
}


int main(int argc, char *argv[]) {
    int i;
    for (i=0; i!=MAX_VARS; i++) {
        vars[i] = NULL;
    }

    if (argc < 2) {
        fprintf(stderr, "Please specify source file\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    yyin = fp;
    yyparse();

    return 0;
}
