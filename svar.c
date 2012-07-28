#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "svar.h"

svar *vars[MAX_VARS];

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
 * Find a free svar slot.
 *
 * Returns index of slot, or -1 on none found
 */
int svar_find_free_slot() {
    for (int i = 0; i !=MAX_VARS; i++) {
        if (vars[i] == NULL) {
            return i;
        }
    }
    return -1;
}

/**
 * Find a svar based on name
 */
svar *svar_find(char *name) {
    for (int i=0; i!=MAX_VARS; i++) {
        // Empty slot, check the next
        if (vars[i] == NULL) continue;

        // Check if name matches
        if (strcmp(vars[i]->name, name) == 0) {
            // return svar in slot
            return vars[i];
        }
    }

    // No var found with this name
    return NULL;
}

/**
 * Allocate and initialize an svar
 */
svar *svar_alloc(char type, char *name, void *val) {
    // Find a free slot to add our svar into
    int idx = svar_find_free_slot();
    if (idx == -1) {
        fprintf(stderr, "No more room for variables!");
        exit(1);
    }

    // Allocate memory and set standard info
    svar *var = (svar *)malloc(sizeof(svar));
    var->type = type;
    var->name = strdup(name);

    // Set additional values if needed
    if (var->type == SV_LONG) {
        var->val.l = (long)val;
    } else if (var->type == SV_STRING) {
        var->val.s = strdup((char *)val);
    }

    // Store this variable in our main lookup table
    vars[idx] = var;
    return var;
}

/**
 * Free an allocated svar
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
}

/**
 * Initialize room for a limited number of svars
 */
void svar_init_table() {
    for (int i=0; i!=MAX_VARS; i++) {
        vars[i] = NULL;
    }
}
