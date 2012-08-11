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
    t_symbol_table *st = (t_symbol_table *)malloc(sizeof(t_symbol_table));

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