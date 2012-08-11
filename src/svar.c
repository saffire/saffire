#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hashtable/hashtable.h"
#include "svar.h"

t_hash_table *variable_table;

const int SV_NULL   = 0;
const int SV_LONG   = 1;
const int SV_STRING = 2;
const int SV_DOUBLE = 3;



/**
 * Print debug svar info
 */
void svar_print(svar *var) {
    printf("VAR:\n");
    printf("  Name: %s\n", var->name);
    printf("  Type: %d\n", var->type);
    if (var->type == SV_LONG) {
        printf("  Val:  %ld\n", var->val.l);
    }
    if (var->type == SV_STRING) {
        printf("  Val:  %s\n", var->val.s);
    }
    if (var->type == SV_DOUBLE) {
        printf("  Val:  %g\n", var->val.d);
    }
    printf("\n");
}

/**
 * Find a svar based on name
 */
svar *svar_find(char *name) {
    return ht_find(variable_table, name);
}

/**
 * Allocate and initialize an svar and place it into the variable table
 */
svar *svar_alloc(char type, char *name, char *s, long l) {
    // Allocate memory and set standard info
    svar *var = (svar *)malloc(sizeof(svar));
    var->type = type;
    var->name = strdup(name);

    // Set additional values if needed
    if (var->type == SV_LONG) {
        var->val.l = l;
    } else if (var->type == SV_STRING) {
        var->val.s = strdup(s);
    }

    // Store this variable in our main lookup table
    ht_add(variable_table, name, var);
    return var;
}

/**
 * Free an allocated svar and remove it from the variable table
 */
void svar_free(svar *var) {
    if (var == NULL) {
        fprintf(stderr, "svar is NULL, not freeing");
        return;
    }

    // Free the name
    free(var->name);

    // Free additional memory if needed
    if (var->type == SV_STRING) {
        free(var->val.s);
    }

    ht_remove(variable_table, var->name);
}

/**
 * Initialize room for a limited number of svars
 */
void svar_init_table() {
    variable_table = ht_create();
}


int svar_true(svar *var) {
    return (var->type == SV_LONG && var->val.l);
}