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
#include "general/hashtable.h"
#include "general/smm.h"

extern t_hashfuncs chained_hf;

#define HT_INITIAL_BUCKET_COUNT    16           // Initial hash size
#define HT_LOAD_FACTOR           1.25           // Above this load, we will increase the hash size (it's above 1.00
                                                // because we use chaining instead of linear probing or other means)
#define HT_RESIZE_FACTOR         1.75           // Factor to resize to (current size * ht_resize_factor)

#define DEFAULT_HASHFUNCS        &chained_hf     // Default hashtable functionality



/**
 * Creates new hash table
 */
static t_hash_table *_ht_create(int bucket_count, float load_factor, float resize_factor, t_hashfuncs *hashfuncs) {
    // Allocate memory for table and buckets
    t_hash_table *ht = (t_hash_table *)smm_malloc(sizeof(t_hash_table));

    ht->bucket_count = 0;
    ht->element_count = 0;
    ht->load_factor = load_factor;
    ht->resize_factor = resize_factor;
    ht->copy_on_write = 0;
    ht->hashfuncs = hashfuncs;
    ht->head = NULL;
    ht->tail = NULL;

    ht->hashfuncs->resize(ht, bucket_count);
    return ht;
}


/**
 * Create a new hash table with all default value
 */
t_hash_table *ht_create(void) {
    return _ht_create(HT_INITIAL_BUCKET_COUNT, HT_LOAD_FACTOR, HT_RESIZE_FACTOR, DEFAULT_HASHFUNCS);
}


/**
 * Create a new hash table with customized values
 */
t_hash_table *ht_create_custom(int bucket_count, float load_factor, float resize_factor, t_hashfuncs *hashfuncs) {
    return _ht_create(bucket_count, load_factor, resize_factor, hashfuncs);
}


t_hash_table *ht_copy(t_hash_table *ht, int copy_on_write) {
    t_hash_table *copy = smm_malloc(sizeof(t_hash_table));

    memcpy(copy, ht, sizeof(t_hash_table));

    copy->copy_on_write = copy_on_write;

    if(!copy_on_write) {
        ht->hashfuncs->deep_copy(copy);
    }

    return copy;
}

/**
 * Free a hash table
 */
void ht_destroy(t_hash_table *ht) {
    t_hash_table_bucket *bucket, *next_bucket;

    // Nothing to free
    if (! ht) return;

    if (!ht->copy_on_write) {
        // Destroy buckets
        bucket = ht->head;
        while (bucket) {
            // Find next bucket
            next_bucket = bucket->next_element;

            // Now, we can safely remove bucket
            smm_free(bucket->key); // strdupped
            smm_free(bucket);

            // goto next bucket
            bucket = next_bucket;
        }

        // Free bucket list array
        smm_free(ht->bucket_list);
    }


    // Destroy hash table
    smm_free(ht);
    return;
}


///**
// * Return the hash of a certain key
// */
//unsigned int ht_hash(t_hash_table *ht, const char *key) {
//    return ht->hashfuncs.hash(ht, key);
//}

/**
 * Return value of key, or NULL when nothing found
 */
void *ht_find(t_hash_table *ht, const char *key) {
    return ht->hashfuncs->find(ht, key);
}

/**
 * Return value of key, or NULL when nothing found
 */
void *ht_num_find(t_hash_table *ht, long index) {
    char key[32];
    snprintf(key, 31, "%ld", index);
    return ht_find(ht, key);
}

/**
 * Return 0 when key is not found, 1 otherwise
 */
int ht_exists(t_hash_table *ht, const char *key) {
    return ht->hashfuncs->exists(ht, key);
}

/**
 * Add key/value into hashtable
 */
int ht_add(t_hash_table *ht, const char *key, void *value) {
    if (ht->copy_on_write) {
        ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->add(ht, key, value);
}

int ht_num_add(t_hash_table *ht, long index, void *value) {
    char key[32];
    snprintf(key, 31, "%ld", index);
    return ht_add(ht, key, value);
}

/**
 * Replace key/value into hashtable
 */
void *ht_replace(t_hash_table *ht, const char *key, void *value) {
    if (ht->copy_on_write) {
            ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->replace(ht, key, value);
}


/**
 * Removes key from hashtable
 */
void *ht_remove(t_hash_table *ht, const char *key) {
    if (ht->copy_on_write) {
            ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->remove(ht, key);
}



/*
 * ITERATOR FUNCTIONALITY
 */


int ht_iter_init(t_hash_iter *iter, t_hash_table *ht) {
    iter->ht = ht;
    iter->bucket_idx = 0;
    iter->bucket = ht->head;
    return 1;
}

/**
 * Rewind hash table (or initialize a new table)
 */
int ht_iter_rewind(t_hash_iter *iter, t_hash_table *ht) {
    if (iter == NULL) return 0;

    // No hash table set, and no hash table given
    if (iter->ht == NULL && ht == NULL) return 0;

    // Set hash table
    if (ht != NULL) iter->ht = ht;

    iter->bucket_idx = 0;
    iter->bucket = ht->head;

    return 1;
}


/**
 * Return 0 when iterator is not valid (no more elements)
 */
int ht_iter_valid(t_hash_iter *iter) {
    return (iter->bucket != NULL);
}


/**
 * Goto next element (either inside the bucket, or the next bucket)
 */
int ht_iter_next(t_hash_iter *iter) {
    // Nothing found (or no more items)
    if (iter->bucket == NULL) return 0;

    iter->bucket = iter->bucket->next_element;
    return (iter->bucket != NULL);
}


/**
 * Fetch key from current element
 */
char *ht_iter_key(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->key;
}


/**
 * Fetch value from current element
 */
void *ht_iter_value(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->value;
}
