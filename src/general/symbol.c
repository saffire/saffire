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
#include "symbol.h"
#include "hashtable/hash.h"

/**
 * Symbol tables are "stacked" hashtables. Whenever we enter a new scope, a new symboltable will be created.
 * When trying to find variables, it will check the symbol table of the current scope, and if not found, drops
 * down to the previous level until we hit the "global" scope. This allows us to have multiple variables with
 * the same name, but in different scopes.
 */

/**
 * Creates initial symbol table (the global scope table)
 */
t_symbol_table *symbol_init_table(void) {
    return symbol_new_table(NULL);
}

/**
 * Create a new symbol table on top of another table (new scope)
 */
t_symbol_table *symbol_new_table(t_symbol_table *prev_st) {
    t_symbol_table *st = (t_symbol_table *)smm_malloc(sizeof(t_symbol_table));

    if (prev_st) {
        // Increases scope level
        st->level = prev_st->level + 1;
    } else {
        st->level = 0;  // Global scope level
    }
    st->prev = prev_st;
    st->ht = ht_create();

    return st;
}

void symbol_put(t_symbol_table *st, char *s, void *sym) {
    ht_add(st->ht, s, sym);
}


/**
 * Find string
 */
void *symbol_get(t_symbol_table *st, char *s) {
    t_symbol_table *dst;
    t_hash_table_bucket *htb;

    // Browse all symbol-tables until we hit the global scope
    for (dst = st, dst != null, dst = dst->prev) {
        // Try and find the string in the hashtable of this scope
        htb = ht_find(dst->ht, s);
        if (htb) return htb;
    }

    // Variable not found in any scope
    return NULL;
}