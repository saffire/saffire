/*
 Copyright (c) 2012-2015, The Saffire Group
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
#include <string.h>
#include <math.h>
#include <saffire/objects/object.h>
#include <saffire/general/hashtable.h>
#include <saffire/general/hash/hash_funcs.h>
#include <saffire/general/smm.h>

/**
 * These hash tables are not reentrant, nor threadsafe!
 */

static hash_t ht_hash(t_hash_table *ht, t_hash_key *key) {
    hash_t hash_value = 0;
    if (! ht) return 0;      // Not a hash table

    switch (key->type) {
        case HASH_KEY_STR :
            hash_value = ht->hashfuncs->hash(ht, (const char *)key->val.s);
            break;
        case HASH_KEY_NUM :
            hash_value = key->val.n;
            break;
        case HASH_KEY_OBJ :
            hash_value = ht->hashfuncs->hash(ht, (const char *)object_get_hash((t_object *)(key->val.o)));
            break;
    }

    return hash_value;
}

/**
 * Resize the hashtable, and rehash all values
 */
static void chf_resize(t_hash_table *ht, int new_bucket_count) {
    if (ht->bucket_count == 0) {
        // Initial allocation.
        ht->bucket_count = new_bucket_count;
        ht->bucket_list = (t_hash_table_bucket **)smm_malloc(sizeof(t_hash_table_bucket *) * ht->bucket_count);
        bzero(ht->bucket_list, sizeof(t_hash_table_bucket *) * ht->bucket_count);
        return;
    }

    // Resize bucket index
    t_hash_table_bucket **new_bucket_list = smm_malloc(sizeof(t_hash_table_bucket *) * new_bucket_count);
    bzero(new_bucket_list, sizeof(t_hash_table_bucket *) * new_bucket_count);

    // We only have to rehash the first elements for each bucket // @TODO: NO WE CANT. REHASH ALL THE ELEMENTS!
    t_hash_table_bucket *htb = ht->head;
    while (htb) {
        hash_t hash_capped = htb->hash % new_bucket_count;
        htb->next_in_bucket = new_bucket_list[hash_capped];       // just add the element in front of the line
        new_bucket_list[hash_capped] = htb;
        htb = htb->next_element;
    }

    // Free our "old" bucket-list
    smm_free(ht->bucket_list);

    // Set the new bucket-list
    ht->bucket_list = new_bucket_list;
    ht->bucket_count = new_bucket_count;

    // @TODO: If bucket count reaches a certain limit, don't increase expansion, but use a constant one. Basically
    //        it means that after a threshold, the resize_factor should become 1.00
}


/**
 * Return bucket for specified key
 */
static t_hash_table_bucket *find_bucket(t_hash_table *ht, t_hash_key *key) {
    // Locate the hash value in the bucket list.
    hash_t hash_value = ht_hash(ht, key);
    hash_t hash_value_capped = hash_value % ht->bucket_count;
    if (ht->bucket_list[hash_value_capped] == NULL) {
        // Not found
        return NULL;
    }

    // Found bucket. Try and find the key. Traverse linked list if needed.
    int found = 0;
    t_hash_table_bucket *htb = ht->bucket_list[hash_value_capped];


    // Cache the key hashval when we are dealing with objects.
    char *key_hash_val = NULL;
    if (key->type == HASH_KEY_OBJ) {
        key_hash_val = object_get_hash((t_object *)(key->val.o));
    }


    while (htb) {
        switch (key->type) {
            case HASH_KEY_STR :
                if (strcmp(htb->key->val.s, key->val.s) == 0) found = 1;
                break;
            case HASH_KEY_NUM :
                if (htb->key->val.n == key->val.n) found = 1;
                break;
            case HASH_KEY_OBJ :
                if (strcmp(object_get_hash((t_object *)(htb->key->val.o)), key_hash_val) == 0) found = 1;
                break;
        }
        if (found) return htb;
        htb = htb->next_in_bucket;
    }

    return NULL;
}


/**
 * Find key in hash table
 */
static void *chf_find(t_hash_table *ht, t_hash_key *key) {
    if (! ht) return NULL;      // Not a hash table

    t_hash_table_bucket *htb = find_bucket(ht, key);
    if (! htb) return NULL;

    return htb->value;
}


/**
 * Check if a key exists in a hashtable
 */
static int chf_exists(t_hash_table *ht, t_hash_key *key) {
    if (! ht) return 0;      // Not a hash table

    t_hash_table_bucket *htb = find_bucket(ht, key);
    if (! htb) return 0;

    return 1;
}


/**
 * Add key/value pair to the hash
 */
static int chf_add(t_hash_table *ht, t_hash_key *key, void *value) {
    if (! ht) return 0;      // Not a hash table

    hash_t hash_value = ht_hash(ht, key);
    hash_t hash_value_capped = hash_value % ht->bucket_count;

    // Create bucket for new variable
    t_hash_table_bucket *htb = (t_hash_table_bucket *)smm_malloc(sizeof(t_hash_table_bucket));
    htb->hash = hash_value;         // Store original hash value (for quick rehashing)
    htb->key = key;
    htb->value = value;
    htb->next_in_bucket = NULL;

    // Set this element to head if no head is found yet
    if (ht->head == NULL) {
        ht->head = htb;
    }

    // Append this element to the end of the list
    htb->next_element = NULL;
    htb->prev_element = ht->tail;
    if (htb->prev_element) {
        htb->prev_element->next_element = htb;
    }
    ht->tail = htb;

    // Locate the hash value in the bucket list.
    if (ht->bucket_list[hash_value_capped] == NULL) {
        // Unallocated yet. Insert bucket here
        ht->bucket_list[hash_value_capped] = htb;
    } else {
        // Allocated, added to end
        t_hash_table_bucket *cur_htb = ht->bucket_list[hash_value_capped];
        while (cur_htb->next_in_bucket) cur_htb = cur_htb->next_in_bucket;

        // Make sure links are correct
        cur_htb->next_in_bucket = htb;
    }

    // Increase element count
    ht->element_count++;

    // Calculate load_factor and resize if needed
    float lf = ht->element_count / (float)ht->bucket_count;
    if (lf > ht->load_factor) {
        chf_resize(ht, floor(ht->bucket_count * ht->resize_factor));
    }

    return 1;
}


/**
 * Add key/value pair to the hash
 */
static void *chf_replace(t_hash_table *ht, t_hash_key *key, void *value) {
    if (! ht) return 0;      // Not a hash table

    t_hash_table_bucket *htb = find_bucket(ht, key);
    if (! htb) {
        chf_add(ht, key, value);
        return NULL;
    }

    void *val = htb->value;
    htb->value = value;
    return val;
}


/**
 * Remove key from hash table
 */
static void *chf_remove(t_hash_table *ht, t_hash_key *key) {
    void *val;

    if (! ht) return 0;      // Not a hash table

    t_hash_table_bucket *prev, *next;

    hash_t hash_value = ht_hash(ht, key);
    hash_t hash_value_capped = hash_value % ht->bucket_count;

    // Nothing to remove if nothing was found
    if (ht->bucket_list[hash_value_capped] == NULL) return 0;

    // Traverse list to find actual element
    t_hash_table_bucket *prev_htb = NULL;   // Keep a reference to the previous element in the bucket
    t_hash_table_bucket *htb = ht->bucket_list[hash_value_capped];
    while (htb) {
        int found = 0;
        switch (htb->key->type) {
            case HASH_KEY_STR :
                found = strcmp(htb->key->val.s, key->val.s) == 0;
                break;
            case HASH_KEY_NUM :
                found = htb->key->val.n == key->val.n;
                break;
            case HASH_KEY_OBJ :
                found = strcmp(object_get_hash((t_object *)(htb->key->val.o)), object_get_hash((t_object *)(key->val.o)));
                break;
        }
        if (found) break;
        prev_htb = htb;
        htb = htb->next_in_bucket;
    }

    // Key is still not found
    if (!htb) return 0;

    val = htb->value;


    // If the element was at the head, set the next element to become the head
    if (ht->head == htb) {
        ht->head = htb->next_element;
    }

    // If the element was at the tail, set the previous element to become the tail
    if (ht->tail == htb) {
        ht->tail = htb->prev_element;
    }

    // Check if we removed the first element of the bucket
    if (prev_htb == NULL) {
        // First element, so we need to "relink" the bucket
        ht->bucket_list[hash_value_capped] = ht->bucket_list[hash_value_capped]->next_in_bucket;
    } else {
        // Otherwise, it's in the middle or at the end of the bucket, make sure it's connected correctly
        prev_htb->next_in_bucket = htb->next_in_bucket;
    }

    // Remove the element from the element LL
    prev = htb->prev_element;
    next = htb->next_element;
    if (prev) prev->next_element = next;
    if (next) next->prev_element = prev;


    // Free key and bucket
    ht_key_free(htb->key);
    smm_free(htb);

    // Decrease element count
    ht->element_count--;

    // @TODO: Should we be able to "shrink" as well? For instance, when we drop below a load factor of 0.1, for instance?

    return val;
}

t_hash_table_bucket *_copy_bucket(t_hash_table_bucket *bucket) {
    t_hash_table_bucket *copy = smm_malloc(sizeof(t_hash_table_bucket));

    copy->key = ht_key_copy(bucket->key);
    copy->value = bucket->value;
    copy->hash = bucket->hash;

    copy->prev_element = bucket->prev_element;
    copy->next_element = bucket->next_element;
    copy->next_in_bucket = bucket->next_in_bucket;

    return copy;
}

void _place_bucket(t_hash_table *ht, t_hash_table_bucket *bucket) {
    hash_t capped_hash = bucket->hash % ht->bucket_count;
    bucket->next_in_bucket = ht->bucket_list[capped_hash];
    ht->bucket_list[capped_hash] = bucket;
}

void chf_deep_copy(t_hash_table *ht) {
    ht->copy_on_write = 0;

    int bucket_count = ht->bucket_count;
    ht->bucket_count = 0;
    chf_resize(ht, bucket_count);

    t_hash_table_bucket *old_current = ht->head;
    ht->head = _copy_bucket(old_current);

    t_hash_table_bucket *new_current = ht->head;
    _place_bucket(ht, new_current);
    t_hash_table_bucket *new_bucket;
    old_current = old_current->next_element;

    while(old_current) {
        new_bucket = _copy_bucket(old_current);
        new_bucket->prev_element = new_current;
        new_current->next_element = new_bucket;
        _place_bucket(ht, new_bucket);

        new_current = new_bucket;
        old_current = old_current->next_element;
    }

    ht->tail = new_current;
}


// Hash structure with our function definitions
t_hashfuncs chained_hf = {
    hash_native,                // Use the native hashing method
    //hash_djbx33a,
    chf_find,
    chf_exists,
    chf_add,
    chf_replace,
    chf_remove,
    chf_resize,
    chf_deep_copy,
};
