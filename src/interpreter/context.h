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
#ifndef __CONTEXT_H__
#define __CONTEXT_H__
    #include "general/hashtable.h"
    #include "general/dll.h"
    #include "object/object.h"

    typedef struct _ns_context {
        char *name;                     // Name (or alias) of the context
        int aliased;                    // 0 not aliased, 1 if it is
        union {
            struct _ns_context *alias;  // Pointer to the context that is aliased
            t_hash_table *vars;         // Variables (when not aliased)
        } data;
    } t_ns_context;

    t_hash_table *ht_contexts;         // Hash of all contexts
    t_dll *dll_contexts;               // DLL of all contexts

    void context_init(void);
    void context_fini(void);
    t_ns_context *si_get_current_context(void);

    t_ns_context *si_find_context(const char *name);
    t_hash_table_bucket *si_find_in_context(char *var, t_ns_context *ctx);
    t_ns_context *si_get_context(const char *name);
    t_ns_context *si_create_context(char *name);
    t_ns_context *si_create_context_alias(char *alias, t_ns_context *ctx);
    void si_context_add_object(t_ns_context *ctx, t_object *obj);

#endif
