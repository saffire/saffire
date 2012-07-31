#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

#define HT_INITIAL_BUCKET_SIZE   16             // Number of initial buckets


/**
 * Create a new hash table of initial size
 */
t_hash_table *ht_create(void) {
    // Allocate memory for table and buckets
    t_hash_table *ht = (t_hash_table *)malloc(sizeof(t_hash_table));
    if (! ht) {
        return NULL;
    }

    ht->bucket_list = (t_hash_table_bucket **)malloc(sizeof(t_hash_table_bucket *) * HT_INITIAL_BUCKET_SIZE);
    if (! ht->bucket_list) {
        free(ht);
        return NULL;
    }

    // Initialize buckets
    for (int i=0; i != HT_INITIAL_BUCKET_SIZE; i++) {
        ht->bucket_list[i] = NULL;
    }
    ht->bucket_size = HT_INITIAL_BUCKET_SIZE;
    ht->element_count = 0;

    return ht;
}


/*
 * Create a hash from a specific string. Will return a value between 0 and ht->bucket_size
 */
unsigned int ht_hash(t_hash_table *ht, char *str) {
    unsigned int hash_value = 0;

    for (; *str; str++) {
        hash_value = *str + (hash_value << 5) - hash_value;
    }

    return hash_value % ht->bucket_size;
}



/*
 * Find key in hash table
 */
t_hash_table_bucket *ht_find(t_hash_table *ht, char *str) {
    unsigned int hash_value = ht_hash(ht, str);

    // Locate the hash value in the bucket list.
    if (ht->bucket_list[hash_value] == NULL) {
        // Not found
        return NULL;
    }

    // Found bucket. Try and find the key. Traverse linked list if needed.
    t_hash_table_bucket *htb = ht->bucket_list[hash_value];
    while (htb && strcmp(str, htb->key)) htb = htb->next;

    // Traversed list, nothing left so not found.
    if (! htb) return NULL;

    return htb;
}

/*
 * @TODO: When we have more elements, we should resize our array.
 */
int ht_add(t_hash_table *ht, char *str, void *data) {
    unsigned int hash_value = ht_hash(ht, str);

    // Create bucket for new variable
    t_hash_table_bucket *htb = (t_hash_table_bucket *)malloc(sizeof(t_hash_table_bucket));
    htb->key = strdup(str);
    htb->data = data;
    htb->next = NULL;
    htb->prev = NULL;

    // Locate the hash value in the bucket list.
    if (ht->bucket_list[hash_value] == NULL) {
        // Unallocated yet. Insert bucket here
        ht->bucket_list[hash_value] = htb;
    } else {
        // Allocated, add to end
        t_hash_table_bucket *cur_htb = ht->bucket_list[hash_value];
        while (cur_htb->next) cur_htb = cur_htb->next;

        // Make sure prev / next links are correct
        cur_htb->next = htb;
        htb->prev = cur_htb;
    }

    // Increase element count
    ht->element_count++;
}

/*
 *
 */
void ht_remove(t_hash_table *ht, char *str) {
    t_hash_table_bucket *prev, *next;
    unsigned int hash_value = ht_hash(ht, str);

    // Return when hash is null
    if (ht->bucket_list[hash_value] == NULL) return;

    // Traverse list to find actual element
    t_hash_table_bucket *htb = ht->bucket_list[hash_value];
    while (htb && strcmp(str, htb->key)) htb = htb->next;

    // Key still not found
    if (!htb) return;

    prev = htb->prev;
    next = htb->next;

    if (! prev && ! next) {
        // Only one item
        ht->bucket_list[hash_value] = NULL;
    } else if (! htb->prev && htb->next) {
        // First item
        ht->bucket_list[hash_value] = next;
        htb->next->prev = NULL;
    } else if (htb->prev && ! htb->next) {
        // Last item
        htb->prev->next = NULL;
    } else if (htb->prev && htb->next) {
        // Middle item
        htb->prev->next = htb->next;
        htb->next->prev = htb->prev;
    }

    // Free key and bucket
    free(htb->key);
    free(htb);


    // Decrease element count
    ht->element_count--;

    return;
}


/**
 * Free a hash table
 */
void ht_destroy(t_hash_table *ht) {
    t_hash_table_bucket *htb;

    // Nothing to free
    if (! ht) return;

    // Destroy buckets
    for (int i=0; i!=ht->bucket_size; i++) {
        // Unallocated, so continue with the next one
        if (! ht->bucket_list[i]) continue;

        if (ht->bucket_list[i]->next) {
            // Seek end of linked list
            htb = ht->bucket_list[i];
            while (htb->next) htb = htb->next;

            // Free items in reverse order
            while (htb->prev) {
                htb = htb->prev;

                free(htb->next->key);
                free(htb->next);
            }
        }

        free(ht->bucket_list[i]->key);
        free(ht->bucket_list[i]);
    }

    // Free bucket list array
    free(ht->bucket_list);

    // Destroy hash table
    free(ht);
}
