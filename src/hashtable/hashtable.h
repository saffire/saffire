#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

    typedef struct _hash_table_bucket {
        char *key;                              // Key of the variable
        void *data;                             // Actual variable stored
        struct _hash_table_bucket *next;        // Link to next variable in bucket (if any)
        struct _hash_table_bucket *prev;        // Link to previous variable in bucket (if any)
    } t_hash_table_bucket;

    typedef struct _hash_table {
        int bucket_size;                        // Number of available buckets
        int element_count;                      // Number of elements in the hashtable
        t_hash_table_bucket **bucket_list;      // Actual bucket list array
    } t_hash_table;


    t_hash_table *ht_create(void);
    unsigned int ht_hash(t_hash_table *ht, char *str);
    t_hash_table_bucket *ht_find(t_hash_table *ht, char *str);
    int ht_add(t_hash_table *ht, char *str, void *data);
    void ht_remove(t_hash_table *ht, char *str);
    void ht_destroy(t_hash_table *ht);

#endif

