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
#include <ctype.h>
#include "object.h"
#include "general/smm.h"


#define STRINGDATA(x) ((t_object_data_string *)(x->data))

t_hash_table *string_methods;
t_hash_table *string_properties;




// @TODO: Define macro SAFFIRE_METHOD(object_string_method_length) ?
void *object_string_method_length(t_object *self) {
    return (void *)(STRINGDATA(self)->length);
}

void *object_string_method_upper(t_object *self) {
    t_object *obj = object_clone(self);

    // @TODO: UTF8!
    for (int i=0; i!=STRINGDATA(obj)->length; i++) {
        STRINGDATA(obj)->value[i] = toupper(STRINGDATA(obj)->value[i]);
    }

    // @TODO recalc hash!

    return (t_object *)obj;
}

void *object_string_method_lower(t_object *self) {
    t_object *obj = object_clone(self);

    // @TODO: UTF8!
    for (int i=0; i!=STRINGDATA(obj)->length; i++) {
        STRINGDATA(obj)->value[i] = tolower(STRINGDATA(obj)->value[i]);
    }

    // @TODO recalc hash!

    return (t_object *)obj;
}

void *object_string_method_reverse(t_object *self) {
    t_object *obj = object_clone(self);

    char *str = STRINGDATA(obj)->value;
    char *p1, *p2;

    // @TODO: UTF8!
    if (! str || ! *str) return obj;

    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }

    // @TODO recalc hash!

    return (t_object *)obj;
}



void *object_string_method_ctor(t_object *self) {
    STRINGDATA(self)->value = "foobar";
    STRINGDATA(self)->length = strlen(STRINGDATA(self)->value);
    STRINGDATA(self)->hash = 0xdeadbeef;
    return NULL;
}

void *object_string_method_dtor(t_object *self) {
    return NULL;
}

void *object_string_method_print(t_object *self) {
    printf("%s\n", STRINGDATA(self)->value);
    return NULL;
}


void object_string_init(void) {
    string_methods = ht_create();
    ht_add(string_methods, "ctor", object_string_method_ctor);
    ht_add(string_methods, "dtor", object_string_method_dtor);

    ht_add(string_methods, "length", object_string_method_length);
    ht_add(string_methods, "upper", object_string_method_upper);
    ht_add(string_methods, "lower", object_string_method_lower);
    ht_add(string_methods, "reverse", object_string_method_reverse);
    ht_add(string_methods, "print", object_string_method_print);

    string_properties = ht_create();
}

void object_string_fini() {
    ht_destroy(string_methods);
    ht_destroy(string_properties);
}

void object_string_alloc(t_object *obj) {
    obj->data = (t_object_data_string *)smm_malloc(sizeof(t_object_data_string));
}

void object_string_free(t_object *obj) {
    smm_free(STRINGDATA(obj)->value);
    smm_free(obj->data);
}

t_object *object_string_clone(t_object *obj) {
    // Create new object and copy all info
    t_object *new = object_new();
    memcpy(new, obj, sizeof(t_object));

    // New separated object, so refcount = 0
    obj->header->ref_count = 0;

    object_string_alloc(new);
    STRINGDATA(new)->length = STRINGDATA(obj)->length;
    STRINGDATA(new)->hash = STRINGDATA(obj)->hash;
    STRINGDATA(new)->value = smm_strdup(STRINGDATA(obj)->value);

    return new;
}


t_object *object_string_new(void) {
    t_object *obj = object_new();

    obj->header->name = "string";
    obj->header->fqn = "::string";

    obj->methods = string_methods;
    obj->properties = string_properties;

    obj->funcs.alloc = object_string_alloc;
    obj->funcs.free = object_string_free;
    obj->funcs.clone = object_string_clone;

    obj->funcs.alloc(obj);
    return obj;
}
