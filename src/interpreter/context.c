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


char *strrstr(char *x,char *y) {
    int m = strlen(x);
    int n = strlen(y);
    char *X = malloc(m+1);
    char *Y = malloc(n+1);
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
    free(X);
    free(Y);
    return Z;
}


#define NS_SEPARATOR "::"
#define DOUBLE_NS_SEPARATOR "::::"


t_ns_context *current_context = NULL;

/**
 * Returns the current (latest) context
 */
t_ns_context *si_get_current_context(void) {
  return current_context;
}


/**
 * Creates a new context
 */
t_ns_context *si_create_context(char *namespace) {
    char *new_ns;
    int prefix = 0;
    int postfix = 0;

    int len = strlen(namespace);
    t_ns_context *ctx = si_get_current_context();

    // Relative namespace, concat it with the current namespace
    if (strncmp(namespace, NS_SEPARATOR, strlen(NS_SEPARATOR)) != 0) {
        prefix = 1;
    }
//    if (strncmp(namespace+strlen(namespace)-strlen(NS_SEPARATOR), NS_SEPARATOR, strlen(NS_SEPARATOR)) != 0) {
//        postfix = 1;
//    }

    if (prefix) {
        len =  + strlen(ctx->namespace) + 1;
    }
//    if (postfix) {
//        len += strlen(NS_SEPARATOR);
//    }

    new_ns = (char *)smm_malloc(len);

    // Add ending namespace separator if needed
    if (prefix) {
        strcpy(new_ns, ctx->namespace);
    } else {
        strcpy(new_ns, "");
    }

    strcat(new_ns, namespace);

    // Add ending namespace separator if needed
//    if (postfix) {
//        strcat(new_ns, NS_SEPARATOR);
//    }


//    // Global namespace is a special case, otherwise we get double ::'s
//    if (! strcmp(new_ns, DOUBLE_NS_SEPARATOR)) {
//        new_ns[2] = '\0';
//    }


    DEBUG_PRINT("Creating context: %s\n", new_ns);
    t_ns_context *new_ctx = smm_malloc(sizeof(t_ns_context));

    // Populate new context
    new_ctx->namespace = smm_strdup(new_ns);
    new_ctx->vars = ht_create();                     // This context starts with a clean variable slate.

    // Save the namespace in our global namespace list if it does not exists already
    if (! ht_find(namespaces, new_ns)) {
        DEBUG_PRINT(">>>>>> Added namespace: '%s'\n", new_ns);
        ht_add(namespaces, new_ns, new_ctx);     // Save in namespace hash
        dll_append(contexts, new_ctx);              // Save in DLL as well
    }

    return new_ctx;
}

void context_init(void) {
    // Create namespace hash and DLL for iteration and fast lookups
    namespaces = ht_create();
    contexts = dll_init();

    // Create default global context
    current_context = si_create_context(NS_SEPARATOR);
}

void context_fini(void) {
#ifdef __DEBUG
    t_dll_element *e = DLL_HEAD(contexts);
    while (e) {
        t_ns_context *ctx = (t_ns_context *)e->data;
        printf("Context: '%s'\n", ctx->namespace);
        printf("  Vars : %d\n", ctx->vars->element_count);
        e = DLL_NEXT(e);
    }
#endif
    // Free namespaces hash and dll
    ht_destroy(namespaces);
    dll_free(contexts);
}


/**
 * Splits a fully qualified variable name into namespace and variable separated.
 */
void si_split_var(t_ns_context *current_ctx, char *var, char **fqn_ns, char **fqn_var) {
    DEBUG_PRINT("si_split_var (%s) : '%s'\n", current_ctx->namespace, var);
    int fqn_len = 0;
    char *fqn;

    // TODO: Check for separator!
    if (strncmp(var, NS_SEPARATOR, 2) == 0) {
        // Already a fully qualified name
        fqn_len = strlen(var);
        fqn = smm_malloc(fqn_len);
        strcpy(fqn, var);
    } else {
        fqn_len = strlen(var) + strlen(current_ctx->namespace);
        fqn = smm_malloc(fqn_len);
        strcpy(fqn, current_ctx->namespace);
        strcat(fqn, var);
    }

    // We now have a fully qualified name. We now can split the variable from the namespace, and check for presence
    DEBUG_PRINT("FQN: '%s'\n", fqn);

    // TODO: Check for separator!
    char *ch = strrstr(fqn, NS_SEPARATOR);
    if (ch == NULL) {
        saffire_error("Cannot find last %s in fully qualified name '%s'!", NS_SEPARATOR, fqn);
    }

    // Strip namespace and variable
    int len = ch - fqn;
    if (len == 0) len = strlen(NS_SEPARATOR);
    *fqn_var = smm_strdup(ch + strlen(NS_SEPARATOR));
    *fqn_ns = smm_strdup(fqn);
    (*fqn_ns)[len] = '\0';


    DEBUG_PRINT("NS  : '%s'\n", *fqn_ns);
    DEBUG_PRINT("VAR : '%s'\n", *fqn_var);

    smm_free(fqn);
}


/**
 * Find namespace, or NULL when does not exist
 */
t_ns_context *si_find_namespace(const char *namespace) {
    printf("t_ns_context *si_find_namespace(%s) {\n", namespace);
    t_hash_table_bucket *htb = ht_find(namespaces, (char *)namespace);
    if (!htb) return NULL;
    return (t_ns_context *)htb->data;
}

/**
 * Returns t_ns_context of the namespace. Errors when not found
 */
t_ns_context *si_get_namespace(const char *namespace) {
    t_ns_context *ns = si_find_namespace(namespace);
    if (ns == NULL) {
        saffire_error("Unknown namespace '%s'", namespace);
    }
    return ns;
}

/**
 * Returns the bucket of the variable. Will take care of namespacing depending on the given context
 */
t_hash_table_bucket *si_find_in_context(char *var) {
    char *fqn_ns, *fqn_var;

    t_ns_context *ctx = si_get_current_context();

    DEBUG_PRINT("si_find_in_context (%s) : '%s'\n", ctx->namespace, var);

    // Create fqn from our variables
    si_split_var(ctx, var, &fqn_ns, &fqn_var);

    DEBUG_PRINT("NS: %s\n", fqn_ns);

    t_ns_context *ns_ctx = si_get_namespace(fqn_ns);
    if (ns_ctx == NULL) {
        saffire_error("Cannot find namespace '%s'", fqn_ns);
    }

    // Find variable in namespace
    t_hash_table_bucket *htb = ht_find(ns_ctx->vars, fqn_var);

    if (! htb) {
        DEBUG_PRINT("Creating a new entry for '%s' in '%s'\n", fqn_var, fqn_ns);
        ht_add(ns_ctx->vars, fqn_var, NULL);                            // set to NULL by default
        htb = ht_find(ns_ctx->vars, fqn_var);
    }


    // Free temp vars
    smm_free(fqn_var);
    smm_free(fqn_ns);

    return htb;
}
