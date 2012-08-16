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

