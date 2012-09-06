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

#include "object.h"
#include "string.h"
#include "general/smm.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>


t_hash_table *empty_methods;
t_hash_table *empty_properties;

void object_init(void) {
    empty_methods = ht_create();
    empty_properties = ht_create();
}

void object_fini() {
    ht_destroy(empty_methods);
    ht_destroy(empty_properties);
}


static void unused_object_alloc(t_object *obj) {
    printf("Cannot alloc directly from an object");
    exit(1);
}
static void unused_object_free(t_object *obj) {
     printf("Cannot free directly from an object");
     exit(1);
}
static t_object *unused_object_clone(t_object *obj) {
    printf("Cannot clone directly from an object");
    exit(1);
}


t_object *object_new(void) {
    t_object *obj = smm_malloc(sizeof(t_object));

    obj->header.ref_count = 0;
    obj->header.extends = NULL;
    obj->header.implement_count = 0;
    obj->header.implements = NULL;

    obj->funcs.alloc = unused_object_alloc;
    obj->funcs.free = unused_object_free;
    obj->funcs.clone = unused_object_clone;

    obj->methods = empty_methods;
    obj->properties = empty_properties;

    obj->data = NULL;

    return obj;
}


/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
void *object_call(t_object *obj, char *method) {
    t_hash_table_bucket *htb = ht_find(obj->methods, method);
    if (htb == NULL) {
        printf("Cannot call method '%s' on object %s\n", method, obj->header.fqn);
        return NULL;
    }

    printf("Calling method '%s' on object %s\n", method, obj->header.fqn);
    void *(*func)(t_object *) = htb->data;

    return func(obj);
}

/**
 * Clones an object and returns new object
 */
t_object *object_clone(t_object *obj) {
    return obj->funcs.clone(obj);
}


/**
 * Increase reference to object. Returns new reference count
 */
int object_inc_ref(t_object *obj) {
    obj->header.ref_count++;
    return obj->header.ref_count;
}

/**
 * Decrease reference from object. Returns new reference count
 */
int object_dec_ref(t_object *obj) {
    obj->header.ref_count++;
    return obj->header.ref_count;
}


void test(void) {
    setlocale(LC_ALL,"");

    t_saffire_result *res;
    size_t i;

    object_init();
    object_string_init();

    printf("> obj::ctor\n");
    t_object *obj = object_string_new();
    object_call(obj, "ctor");

    printf("> obj::print\n");
    res = object_call(obj, "print");
    smm_free(res);

    printf("> obj::length\n");
    res = object_call(obj, "length");
    i = (size_t)res->result;
    smm_free(res);
    printf("Length of the string is: %ld\n", i);

    printf("> obj::byte_length\n");
    res = object_call(obj, "byte_length");
    i = (size_t)res->result;
    smm_free(res);
    printf("Byte Length of the string is: %ld\n", i);


    printf("> obj2 = obj::upper\n");
    res = object_call(obj, "upper");
    t_object *obj2 = (t_object *)res->result;
    smm_free(res);
    printf("> obj2::print\n");
    res = object_call(obj2, "print");

    printf("> obj2::sirdoesnotexist\n");
    res = object_call(obj2, "sirdoesnotexist");
    smm_free(res);

    printf("> obj3 = obj2::reverse\n");
    res = object_call(obj2, "reverse");
    t_object *obj3 = (t_object *)res->result;
    smm_free(res);

    printf("> obj3::print\n");
    res = object_call(obj3, "print");
    smm_free(res);

    printf("> obj::print\n");
    res = object_call(obj, "print");
    smm_free(res);
    exit(1);
}

