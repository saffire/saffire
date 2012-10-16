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

t_ns_context *si_create_context(t_ns_context *ctx, char *namespace) {
    if (strncmp(namespace, "::", strlen("::"))) {
        saffire_error("Context should start with ::");
    }

    DEBUG_PRINT("Creating context: %s\n", namespace);
    t_ns_context *new_ctx = smm_malloc(sizeof(t_ns_context));

    // Populate new context
    t_dll_element *e = DLL_TAIL(contexts);
    new_ctx->parent = e == NULL ? NULL : (t_ns_context *)e->data;     // Parent is the last pushed context (or NULL on the first)

    new_ctx->namespace = smm_strdup(namespace+2);       // Remove the :: in the stored namespace
    new_ctx->vars = ht_create();                        // This context starts with a clean variable slate.

    // Save the namespace in our global namespace list if it does not exists already
    if (! ht_find(namespaces, namespace)) {
        ht_add(namespaces, namespace, new_ctx);
    }

    return new_ctx;
}

void context_init(void) {
    // Create namespace hash
    namespaces = ht_create();

    contexts = dll_init();

    // Create default context
    t_ns_context *ctx = si_create_context(NULL, "::");
    dll_append(contexts, ctx);  // Initial context

    // @TODO: Remove these contexts
    t_ns_context *ioctx = si_create_context(ctx, "::io::");
    si_create_context(ioctx, "::io::console::");
    si_create_context(ctx, "::saffire::");
}

void context_fini(void) {
    // Free namespaces hash
    ht_destroy(namespaces);
}


/**
 * Splits a fully qualified variable name into namespace and variable separated.
 */
void si_split_fqn(t_ns_context *current_ctx, char *var, char **fqn_ns, char **fqn_var) {
    DEBUG_PRINT("si_split_fqn (::%s) : '%s'\n", current_ctx->namespace, var);
    int fqn_len = 0;
    char *fqn;

    if (var[0] == ':' && var[1] == ':') {
        // Already a fully qualified name
        fqn_len = strlen(var);
        fqn = smm_malloc(fqn_len);
        strcpy(fqn, var);
    } else {
        fqn_len = strlen(var) + strlen(current_ctx->namespace) + strlen("::");
        fqn = smm_malloc(fqn_len);
        strcpy(fqn, current_ctx->namespace);
        strcat(fqn, "::");
        strcat(fqn, var);
    }

    // We now have a fully qualified name. We now can split the variable from the namespace, and check for presence
    DEBUG_PRINT("FQN: '%s'\n", fqn);

    char *ch = strrchr(fqn, ':') + 1;
    if (ch == NULL) {
        saffire_error("Cannot find last : in fully qualified name '%s'!", fqn);
    }
    *fqn_var = smm_strdup(ch);

    *fqn_ns = smm_strdup(fqn);
    ch = strrchr(*fqn_ns, ':')+1 ;
    *ch = '\0';

    DEBUG_PRINT("NS  : '%s'\n", *fqn_ns);
    DEBUG_PRINT("VAR : '%s'\n", *fqn_var);

    smm_free(fqn);
}



/**
 * Returns t_ns_context of the namespace. Errors when not found
 */
t_ns_context *si_get_namespace(const char *namespace) {
    // Find the namespace
    t_hash_table_bucket *htb = ht_find(namespaces, (char *)namespace);
    if (htb == NULL) {
        saffire_error("Unknown namespace '%s'", namespace);
    }
    return (t_ns_context *)htb->data;
}

/**
 * Returns the bucket of the variable. Will take care of namespacing depending on the given context
 */
t_hash_table_bucket *si_find_in_context(char *var) {
    char *fqn_ns, *fqn_var;

    t_dll_element *e = DLL_TAIL(contexts);
    t_ns_context *ctx = (t_ns_context *)e->data;

    DEBUG_PRINT("si_find_in_context (::%s) : '%s'\n", ctx->namespace, var);

    // Create fqn from our variables
    si_split_fqn(ctx, var, &fqn_ns, &fqn_var);

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
