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
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

    // Hashd value
    typedef unsigned long hash_t;

    // Hash table bucket
    typedef struct _hash_table_bucket {
        char *key;                              // Key of the variable
        void *value;                            // Actual variable stored

        hash_t hash;                            // The calculated hash for the item (for rehashing)

        struct _hash_table_bucket *prev_element; // Link to next element (any bucket)
        struct _hash_table_bucket *next_element; // Link to next element (any bucket)

        struct _hash_table_bucket *next_in_list; // Link to next entry in this bucket
    } t_hash_table_bucket;


    struct _hashfuncs;

    // Hash table structure
    typedef struct _hash_table {
        unsigned int bucket_count;              // Number of available buckets
        unsigned int element_count;             // Number of elements in the hash table
        float load_factor;                      // ratio that has to be filled before resizing
        float resize_factor;                    // Resize factor
        char copy_on_write;                     // Copy the buckets when written to

        struct _hashfuncs *hashfuncs;           // Pointer to actual hash functions
        t_hash_table_bucket *head;              // DLL head (for iteration)
        t_hash_table_bucket *tail;              // DLL head (for appending elements)

        t_hash_table_bucket **bucket_list;      // Actual bucket list array
    } t_hash_table;


    // Actual hash functions
    typedef struct _hashfuncs {
        hash_t (*hash)(t_hash_table *ht, const char *str);                  // Returns hash of string (0..bucket_count)
        void *(*find)(t_hash_table *ht, const char *key);                   // Find value for key in hashtable
        int (*exists)(t_hash_table *ht, const char *key);                   // Find if a key exists in a hashtable
        int (*add)(t_hash_table *ht, const char *key, void *value);         // Add value to key
        void *(*replace)(t_hash_table *ht, const char *key, void *value);   // Replace value to key
        void *(*remove)(t_hash_table *ht, const char *key);                 // Remove key
        void (*resize)(t_hash_table *ht, int new_bucket_count);             // Resize (and rehash) hashtable to new size
        void (*deep_copy)(t_hash_table *ht);                                // Makes a deep copy of the buckets
    } t_hashfuncs;


    t_hash_table *ht_create(void);
    t_hash_table *ht_create_custom(int bucket_count, float load_factor, float resize_factor, t_hashfuncs *hashfuncs);
    t_hash_table *ht_copy(t_hash_table *ht, int copy_on_write);

    int ht_exists(t_hash_table *ht, const char *key);
    void *ht_find(t_hash_table *ht, const char *key);
    int ht_add(t_hash_table *ht, const char *key, void *value);
    void *ht_replace(t_hash_table *ht, const char *key, void *value);
    void *ht_remove(t_hash_table *ht, const char *key);
    void ht_destroy(t_hash_table *ht);

    void *ht_num_find(t_hash_table *ht, unsigned long index);
    int ht_num_add(t_hash_table *ht, unsigned long index, void *value);

    // Functionality for iterating a hash table (forward only)
    typedef struct _hash_iter {
        t_hash_table *ht;
        unsigned long bucket_idx;
        t_hash_table_bucket *bucket;
    } t_hash_iter;

    int ht_iter_init(t_hash_iter *iter, t_hash_table *ht);
    int ht_iter_rewind(t_hash_iter *iter, t_hash_table *ht);
    int ht_iter_valid(t_hash_iter *iter);
    int ht_iter_next(t_hash_iter *iter);
    char *ht_iter_key(t_hash_iter *iter);
    void *ht_iter_value(t_hash_iter *iter);


#endif

