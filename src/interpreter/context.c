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
#include "debug.h"
#include "interpreter/saffire_interpreter.h"
#include "interpreter/context.h"
#include "interpreter/errors.h"
#include "general/hashtable.h"
#include "general/smm.h"

/**
 * Like strstr but backwards search
 */
char *strrstr(char *x,char *y) {
    int m = strlen(x);
    int n = strlen(y);
    char *X = smm_malloc(m+1);
    char *Y = smm_malloc(n+1);
    int i;
    for (i=0; i<m; i++) X[m-1-i] = x[i];
    X[m] = 0;
    for (i=0; i<n; i++) Y[n-1-i] = y[i];
    Y[n] = 0;
    char *Z = strstr(X,Y);
    if (Z) {
        int ro = Z-X;
        int lo = ro+n-1;
        int ol = m-1-lo;
        Z = x+ol;
    }
    smm_free(X);
    smm_free(Y);
    return Z;
}

// These are our context separators
#define NS_SEPARATOR "::"
#define DOUBLE_NS_SEPARATOR NS_SEPARATOR NS_SEPARATOR

// What context are we at this moment inside    (does this even make sense? Probably need the top context from a stack?)
t_ns_context *current_context = NULL;


/**
 * Returns the current (latest) context
 */
t_ns_context *si_get_current_context(void) {
    return current_context;
}


/**
 * Initializes the context engine
 */
void context_init(void) {
    // Create context hash and DLL for iteration and fast lookups
    ht_contexts = ht_create();
    dll_contexts = dll_init();

    // Create default global context
    current_context = si_create_context(NS_SEPARATOR);
}


/**
 * Destroys the context engine
 */
void context_fini(void) {
#ifdef __DEBUG
    t_dll_element *e = DLL_HEAD(dll_contexts);
    while (e) {
        t_ns_context *ctx = (t_ns_context *)e->data;
        DEBUG_PRINT("Context: '%s'\n", ctx->name);
        DEBUG_PRINT("Aliased: '%s'", ctx->aliased ? "yes" : "no");
        if (ctx->aliased) {
            t_ns_context *actx = ctx->data.alias;
            DEBUG_PRINT(" => '%s'",  actx->name);
        }
        DEBUG_PRINT("\n");

        DEBUG_PRINT("  Vars : %d\n", ctx->data.vars->element_count);
        e = DLL_NEXT(e);
    }
#endif
    // Free context hash and dll
    ht_destroy(ht_contexts);
    dll_free(dll_contexts);
}


/**
 * Creates a new context or alias to the context
 */
static t_ns_context *_si_create_ctx(char *name, int aliased, t_ns_context *ctx) {
    char *new_ctx_name;
    int prefix = 0;

    int len = strlen(name);
    t_ns_context *cur_ctx = si_get_current_context();

    // Relative name, concat it with the current context
    if (strncmp(name, NS_SEPARATOR, strlen(NS_SEPARATOR)) != 0) {
        prefix = 1;
        len += (strlen(cur_ctx->name) + 1);
    }

    new_ctx_name = (char *)smm_malloc(len);

    // Add ending name separator if needed
    if (prefix) {
        strcpy(new_ctx_name, cur_ctx->name);
    } else {
        strcpy(new_ctx_name, "");
    }

    strcat(new_ctx_name, name);

    DEBUG_PRINT("Creating context: %s\n", new_ctx_name);
    t_ns_context *new_ctx = smm_malloc(sizeof(t_ns_context));

    // Populate new context
    new_ctx->name = smm_strdup(new_ctx_name);
    new_ctx->aliased = aliased;
    if (aliased) {
        new_ctx->data.alias = ctx;
    } else {
        new_ctx->data.vars = ht_create();                     // This context starts with a clean variable slate.
    }

    // Save the name in our global context list if it does not exists already
    if (! ht_find(ht_contexts, new_ctx_name)) {
        DEBUG_PRINT(">>>>>> Added context: '%s'\n", new_ctx_name);
        ht_add(ht_contexts, new_ctx_name, new_ctx);     // Save in context hash
        dll_append(dll_contexts, new_ctx);        // Save in DLL as well
    }

    return new_ctx;
}


/**
 * Creates a new context
 */
t_ns_context *si_create_context(char *name) {
    return _si_create_ctx(name, 0, NULL);
}


/**
 * Creates a new alias to an existing context
 */
t_ns_context *si_create_context_alias(char *alias, t_ns_context *ctx) {
    return _si_create_ctx(alias, 1, ctx);
}


/**
 *
 */
char *si_create_fqn(const char *var) {
    char *fqn;
    t_ns_context *ctx = si_get_current_context();

    if (strncmp(var, NS_SEPARATOR, 2) == 0) {
        // Qualified
        fqn = smm_strdup(var);
        return fqn;
    }

    // Unqualified
    int fqn_len = strlen(var) + strlen(ctx->name);
    fqn = smm_malloc(fqn_len + 1);
    strcpy(fqn, ctx->name);
    strcat(fqn, var);
    return fqn;
}


/**
 * Splits a fully qualified variable name into context and variable separated.
 */
void si_split_var(t_ns_context *current_ctx, char *var, char **fqn_ctx, char **fqn_var) {
    DEBUG_PRINT("si_split_var : '%s' ", var);
    char *fqn = si_create_fqn(var);

    // We now have a fully qualified name. We now can split the variable from the name, and check for presence
    DEBUG_PRINT("FQN: '%s'  ", fqn);

    // TODO: Check for separator!
    char *ch = strrstr(fqn, NS_SEPARATOR);
    if (ch == NULL) {
        saffire_error("Cannot find last %s in fully qualified name '%s'!", NS_SEPARATOR, fqn);
    }

    // Strip context and variable
    int len = ch - fqn;
    if (len == 0) len = strlen(NS_SEPARATOR);
    *fqn_var = smm_strdup(ch + strlen(NS_SEPARATOR));
    *fqn_ctx = smm_strdup(fqn);
    (*fqn_ctx)[len] = '\0';

    DEBUG_PRINT("CTX : '%s' ", *fqn_ctx);
    DEBUG_PRINT("VAR : '%s' ", *fqn_var);

    smm_free(fqn);

    DEBUG_PRINT("\n");
}


/**
 * Find context, or NULL when does not exist
 */
t_ns_context *si_find_context(const char *name) {
    char *ctx_name = si_create_fqn(name);

    DEBUG_PRINT("t_ns_context *si_find_context(%s) {\n", ctx_name);
    t_hash_table_bucket *htb = ht_find(ht_contexts, (char *)ctx_name);
    if (!htb) {
        smm_free(ctx_name);
        return NULL;
    }
    t_ns_context *ctx = (t_ns_context *)htb->data;


    // Check if context is aliased, if so, goto the alias
    int i=10;
    while (i--) {
        if (! ctx->aliased) {
            smm_free(ctx_name);
            return ctx;
        }
        ctx = ctx->data.alias;
        DEBUG_PRINT("CTX aliased to '%s'\n", ctx->name);
    }


    // Prevent endless loops:    context1 -> context2 -> context1
    smm_free(ctx_name);
    saffire_error("Context nesting too deep!");
}


/**
 * Returns t_ns_context of the name. Errors when not found
 */
t_ns_context *si_get_context(const char *name) {
    t_ns_context *ns = si_find_context(name);
    if (ns == NULL) {
        saffire_error("Unknown context '%s'", name);
    }
    return ns;
}


/**
 *
 */
t_hash_table_bucket *si_create_in_context(char *var, t_ns_context *ctx) {
    char *fqn_ctx, *fqn_var;

    if (ctx == NULL) {
        ctx = si_get_current_context();
    }
    si_split_var(ctx, var, &fqn_ctx, &fqn_var);

    t_ns_context *ns_ctx = si_get_context(fqn_ctx);
    if (ns_ctx == NULL) {
        saffire_error("Cannot find context '%s'", fqn_ctx);
    }

    t_hash_table_bucket *htb = ht_find(ns_ctx->data.vars, fqn_var);
    if (htb) {
        saffire_error("Variable %s already exists inside %s", fqn_var, ns_ctx->name);
    }

    DEBUG_PRINT("Creating a new entry for '%s' in '%s'\n", fqn_var, fqn_ctx);
    ht_add(ns_ctx->data.vars, fqn_var, NULL);

    return ht_find(ns_ctx->data.vars, fqn_var);
}

/**
 * Returns the bucket of the variable. Will take care of namespacing depending on the given context
 */
t_hash_table_bucket *si_find_in_context(char *var, t_ns_context *ctx) {
    char *fqn_ctx, *fqn_var;

    if (ctx == NULL) {
        ctx = si_get_current_context();
    }

    DEBUG_PRINT("si_find_in_context (%s) : '%s'\n", ctx->name, var);

    // Create fqn from our variables
    si_split_var(ctx, var, &fqn_ctx, &fqn_var);

    DEBUG_PRINT("CTX: %s\n", fqn_ctx);

    t_ns_context *ns_ctx = si_get_context(fqn_ctx);
    if (ns_ctx == NULL) {
        saffire_error("Cannot find context '%s'", fqn_ctx);
    }

    // Find variable in name
    t_hash_table_bucket *htb = ht_find(ns_ctx->data.vars, fqn_var);
    if (! htb) {
        htb = si_create_in_context(fqn_var, ns_ctx);
    }

    // Free temp vars
    smm_free(fqn_var);
    smm_free(fqn_ctx);

    return htb;
}


void si_context_add_object(t_ns_context *ctx, t_object *obj) {
    DEBUG_PRINT("   Adding object %s to %s\n", obj->name, ctx->name);
    ht_add(ctx->data.vars, obj->name, obj);
}


