#ifndef __SYMBOL_H__
#define __SYMBOL_H__

    typedef struct symbol_table {
        t_symbol_table *prev;   // Pointer to previous scope, or NULL when global scope
        int level;              // Which level is this scope

        t_hash_table *ht;       // Hash table with actual symbols
    } t_symbol_table;

    t_symbol_table *symbol_init_table(void);
    t_symbol_table *symbol_new_table(t_symbol_table *prev_st);
    void symbol_put(t_symbol_table *st, char *s, void *sym);
    void *symbol_get(t_symbol_table *st, char *s);

#endif

