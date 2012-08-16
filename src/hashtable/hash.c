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
#include "hashtable.h"

void print_it (t_hash_table *ht) {
    int i = 0;
    for (i=0; i!=ht->bucket_size; i++) {
        int size = 0;
        t_hash_table_bucket *htb = ht->bucket_list[i];
        while (htb) { size++; htb = htb->next; }
        printf("Bucket: %d : %d\n", i, size);
    }
}

void main(void) {
    t_hash_table_bucket *htb;
    t_hash_table *ht = ht_create();

    htb = ht_find(ht, "foo");
    printf ("ht_find: %08X\n", htb);
    if (htb) printf("key: %s  val: %s\n", htb->key, (char *)htb->data);

    print_it(ht);

    ht_add(ht, "foo", "bar");
    ht_add(ht, "foo1", "bar1");
    ht_add(ht, "foo2", "bar2");
    ht_add(ht, "foo3", "bar3");
    ht_add(ht, "foo4", "bar4");
    ht_add(ht, "foo5", "bar5");
    ht_add(ht, "foo6", "bar6");
    ht_add(ht, "foo7", "bar7");
    ht_add(ht, "foo8", "bar8");
    ht_add(ht, "foo9", "bar9");
    ht_add(ht, "foo10", "bar10");
    ht_add(ht, "foo11", "bar11");
    ht_add(ht, "foo12", "bar12");
    ht_add(ht, "foo13", "bar13");
    ht_add(ht, "foo14", "bar14");
    ht_add(ht, "foo15", "bar15");
    ht_add(ht, "foo16", "bar16");
    ht_add(ht, "foo17", "bar17");
    ht_add(ht, "foo18", "bar18");
    ht_add(ht, "foo19", "bar19");
    ht_add(ht, "foo20", "bar20");

    htb = ht_find(ht, "foo");
    printf ("ht_find: %08X\n", htb);
    if (htb) printf("key: %s  val: %s\n", htb->key, (char *)htb->data);

    print_it(ht);

    ht_remove(ht, "foo");

    htb = ht_find(ht, "foo");
    printf ("ht_find: %08X\n", htb);
    if (htb) printf("key: %s  val: %s\n", htb->key, (char *)htb->data);

    print_it(ht);

    ht_remove(ht, "foo12");     print_it(ht);
    ht_remove(ht, "foo5");      print_it(ht);
    ht_remove(ht, "foo3");      print_it(ht);
    ht_remove(ht, "foo2");      print_it(ht);
    ht_remove(ht, "foo1");      print_it(ht);
    ht_remove(ht, "foo");       print_it(ht);
    ht_remove(ht, "foo6");      print_it(ht);
    ht_remove(ht, "foo7");      print_it(ht);
    ht_remove(ht, "foo6");      print_it(ht);
    ht_remove(ht, "foo7");      print_it(ht);
    ht_remove(ht, "foo6");      print_it(ht);
    ht_remove(ht, "foo8");      print_it(ht);
    ht_remove(ht, "foo9");      print_it(ht);
    ht_remove(ht, "foo10");      print_it(ht);
    ht_remove(ht, "foo11");      print_it(ht);
    ht_remove(ht, "foo12");      print_it(ht);
    ht_remove(ht, "foo13");      print_it(ht);
    ht_remove(ht, "foo14");      print_it(ht);
    ht_remove(ht, "foo15");      print_it(ht);
    ht_remove(ht, "foo16");      print_it(ht);
    ht_remove(ht, "foo17");      print_it(ht);

    ht_remove(ht, "blaat");      print_it(ht);


    ht_destroy(ht);
}

