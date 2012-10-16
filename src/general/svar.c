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
#include <stdlib.h>
#include "debug.h"
#include "general/hashtable.h"
#include "general/svar.h"
#include "general/smm.h"

t_hash_table *variable_table;

const int SV_NULL   = 0;
const int SV_LONG   = 1;
const int SV_STRING = 2;
const int SV_DOUBLE = 3;



/**
 * Print debug svar info
 */
void svar_print(svar *var) {
    DEBUG_PRINT("VAR:\n");
    DEBUG_PRINT("  Name: %s\n", var->name);
    DEBUG_PRINT("  Type: %d\n", var->type);
    if (var->type == SV_LONG) {
        DEBUG_PRINT("  Val:  %ld\n", var->val.l);
    }
    if (var->type == SV_STRING) {
        DEBUG_PRINT("  Val:  %s\n", var->val.s);
    }
    if (var->type == SV_DOUBLE) {
        DEBUG_PRINT("  Val:  %g\n", var->val.d);
    }
    DEBUG_PRINT("\n");
}

/**
 * Find a svar based on name
 */
svar *svar_find(char *name) {
    return (svar *)ht_find(variable_table, name);
}

/**
 * Allocate and initialize an svar and place it into the variable table
 */
svar *svar_alloc(char type, char *name, char *s, long l) {
    // Allocate memory and set standard info
    svar *var = (svar *)smm_malloc(sizeof(svar));
    var->type = type;
    var->name = smm_strdup(name);

    // Set additional values if needed
    if (var->type == SV_LONG) {
        var->val.l = l;
    } else if (var->type == SV_STRING) {
        var->val.s = smm_strdup(s);
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
    smm_free(var->name);

    // Free additional memory if needed
    if (var->type == SV_STRING) {
        smm_free(var->val.s);
    }

    ht_remove(variable_table, var->name);

    smm_free(var);
}

/**
 * Initialize room for a limited number of svars
 */
void svar_init_table() {
    variable_table = ht_create();
}

void svar_fini_table() {
    ht_destroy(variable_table);
}

int svar_true(svar *var) {
    return (var->type == SV_LONG && var->val.l);
}