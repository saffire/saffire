/*
 Copyright (c) 2012-2013, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
 * Neither the name of the Saffire Group the
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


#define NEW_KEY_STR(hk, key)  \
                t_hash_key *hk = (t_hash_key *)malloc(sizeof(t_hash_key)); \
                hk->type = HASH_KEY_STR; \
                hk->val.s = smm_strdup(key);

#define NEW_KEY_NUM(hk, key)  \
                t_hash_key *hk = (t_hash_key *)malloc(sizeof(t_hash_key)); \
                hk->type = HASH_KEY_NUM; \
                hk->val.n = key;

#define NEW_KEY_PTR(hk, key)  \
                t_hash_key *hk = (t_hash_key *)malloc(sizeof(t_hash_key)); \
                hk->type = HASH_KEY_PTR; \
                hk->val.p = (void *)key;



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
    t_hash_table *ht = (t_hash_table *) smm_malloc(sizeof (t_hash_table));

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

/**
 *
 * @param ht
 * @param copy_on_write
 * @return
 */
t_hash_table *ht_copy(t_hash_table *ht, int copy_on_write) {
    t_hash_table *copy = smm_malloc(sizeof (t_hash_table));

    memcpy(copy, ht, sizeof (t_hash_table));

    copy->copy_on_write = copy_on_write;

    if (!copy_on_write) {
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
    if (!ht) return;

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

/**
 *
 * @param ht
 * @param s
 * @return
 */
void *ht_find(t_hash_table *ht, t_hash_key *key) {
    return ht->hashfuncs->find(ht, key);
}
void *ht_find_str(t_hash_table *ht, char *key) {
    NEW_KEY_STR(hkey, key);
    return ht_find(ht, hkey);
}
void *ht_find_num(t_hash_table *ht, unsigned long key) {
    NEW_KEY_NUM(hkey, key);
    return ht_find(ht, hkey);
}
void *ht_find_obj(t_hash_table *ht, t_object *key) {
    NEW_KEY_PTR(hkey, key);
    return ht_find(ht, hkey);
}

/**
 * Return 0 when key is not found, 1 otherwise
 */
int ht_exists(t_hash_table *ht, t_hash_key *key) {
    return ht->hashfuncs->exists(ht, key);
}
int ht_exists_str(t_hash_table *ht, char *key) {
    NEW_KEY_STR(hkey, key);
    return ht_exists(ht, hkey);
}
int ht_exists_num(t_hash_table *ht, unsigned long key) {
    NEW_KEY_NUM(hkey, key);
    return ht_exists(ht, hkey);
}
int ht_exists_obj(t_hash_table *ht, t_object *key) {
    NEW_KEY_PTR(hkey, key);
    return ht_exists(ht, hkey);
}

/**
 *
 * @param ht
 * @param key
 * @param value
 * @return
 */
int ht_add(t_hash_table *ht, t_hash_key *key, void *value) {
    if (ht->copy_on_write) {
        ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->add(ht, key, value);
}
int ht_add_str(t_hash_table *ht, char *key, void *value) {
    NEW_KEY_STR(hkey, key);
    return ht_add(ht, hkey, value);
}
int ht_add_num(t_hash_table *ht, unsigned long key, void *value) {
    NEW_KEY_NUM(hkey, key);
    return ht_add(ht, hkey, value);
}
int ht_add_obj(t_hash_table *ht, t_object *key, void *value) {
    NEW_KEY_PTR(hkey, key);
    return ht_add(ht, hkey, value);
}

/**
 * Replace key/value into hashtable
 */
void *ht_replace(t_hash_table *ht, t_hash_key *key, void *value) {
    if (ht->copy_on_write) {
        ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->replace(ht, key, value);
}
void *ht_replace_str(t_hash_table *ht, char *key, void *value) {
    NEW_KEY_STR(hkey, key);
    return ht_replace(ht, hkey, value);
}
void *ht_replace_num(t_hash_table *ht, unsigned long key, void *value) {
    NEW_KEY_NUM(hkey, key);
    return ht_replace(ht, hkey, value);
}
void *ht_replace_obj(t_hash_table *ht, t_object *key, void *value) {
    NEW_KEY_PTR(hkey, key);
    return ht_replace(ht, hkey, value);
}

/**
 * Removes key from hashtable
 */
void *ht_remove(t_hash_table *ht, t_hash_key *key) {
    if (ht->copy_on_write) {
        ht->hashfuncs->deep_copy(ht);
    }
    return ht->hashfuncs->remove(ht, key);
}
void *ht_remove_str(t_hash_table *ht, char *key) {
    NEW_KEY_STR(hkey, key);
    return ht_remove(ht, hkey);
}
void *ht_remove_num(t_hash_table *ht, unsigned long key) {
    NEW_KEY_NUM(hkey, key);
    return ht_remove(ht, hkey);
}
void *ht_remove_obj(t_hash_table *ht, t_object *key) {
    NEW_KEY_PTR(hkey, key);
    return ht_remove(ht, hkey);
}

/*
 * ITERATOR FUNCTIONALITY
 */


int ht_iter_init(t_hash_iter *iter, t_hash_table *ht) {
    iter->ht = ht;
    iter->bucket_idx = 0;
    iter->bucket = ht ? ht->head : NULL;
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
t_hash_key *ht_iter_key(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->key;
}

char *ht_iter_key_str(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->key->val.s;
}

unsigned long ht_iter_key_num(t_hash_iter *iter) {
    if (iter->bucket == NULL) return 0;
    return iter->bucket->key->val.n;
}

t_object *ht_iter_key_obj(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->key->val.p;
}



/**
 * Fetch value from current element
 */
void *ht_iter_value(t_hash_iter *iter) {
    if (iter->bucket == NULL) return NULL;
    return iter->bucket->value;
}
