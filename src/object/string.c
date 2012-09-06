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
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
#include "object.h"
#include "string.h"
#include "general/smm.h"
#include "general/md5.h"


#define STRINGDATA(x) ((t_object_data_string *)(x->data))

t_hash_table *string_methods;       // Pointer to hash table with string object methods
t_hash_table *string_properties;    // pointer to hash table with string object properties



/**
 * Recalculate MD5 hash for current string in object.
 */
static void recalc_hash(t_object *obj) {
    char utf8_char_buf[MB_LEN_MAX];
    md5_state_t state;

    md5_init(&state);
    for (int i=0; i!=STRINGDATA(obj)->char_length; i++) {
        // Convert wide character to UTF8 character
        int utf8_char_len = wctomb(utf8_char_buf, STRINGDATA(obj)->value[i]);

        // Add character to md5
        md5_append(&state, utf8_char_buf, utf8_char_len);
    }
    md5_finish(&state, STRINGDATA(obj)->hash);

    printf("Recalc_Hash:");
    for (int i=0; i!=16; i++) {
        printf("%02X", (unsigned char)STRINGDATA(obj)->hash[i]);
    }
    printf("\n");
}


SAFFIRE_METHOD(string, length) {
    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = (size_t *)(STRINGDATA(self)->char_length);
    return result;
}

SAFFIRE_METHOD(string, byte_length) {
    char utf8_char_buf[MB_LEN_MAX];
    size_t byte_len = 0;

    // Calculate length for each character, and add to total
    for (int i=0; i!=STRINGDATA(self)->char_length; i++) {
        int l = wctomb(utf8_char_buf, STRINGDATA(self)->value[i]);
        byte_len += (l == -1 ? 1 : l);
    }

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = (size_t *)byte_len;
    return result;
}


SAFFIRE_METHOD(string, upper) {
    t_object *obj = object_clone(self);

    for (int i=0; i!=STRINGDATA(obj)->char_length; i++) {
        STRINGDATA(obj)->value[i] = towupper(STRINGDATA(obj)->value[i]);
    }

    recalc_hash(obj);

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = (t_object *)obj;
    return result;
}

SAFFIRE_METHOD(string, lower) {
    t_object *obj = object_clone(self);

    for (int i=0; i!=STRINGDATA(obj)->char_length; i++) {
        STRINGDATA(obj)->value[i] = towlower(STRINGDATA(obj)->value[i]);
    }

    recalc_hash(obj);

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = (t_object *)obj;
    return result;
}

SAFFIRE_METHOD(string, reverse) {
    t_object *obj = object_clone(self);

    wchar_t *str = STRINGDATA(obj)->value;
    wchar_t *p1, *p2;

    // @TODO: UTF8!
    if (! str || ! *str) {
        t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
        result->result = (t_object *)obj;
        return result;
    }

    wchar_t p3;
    for (p1 = str, p2 = str + STRINGDATA(obj)->char_length - 1; p2 > p1; ++p1, --p2) {
        p3 = *p2;
        *p2 = *p1;
        *p1 = p3;
    }

    recalc_hash(obj);

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = (t_object *)obj;
    return result;
}



SAFFIRE_METHOD(string, ctor) {
    char utf8_char_buf[MB_LEN_MAX];

    wchar_t s[] = L"BjörkBjörk Guðmundsdóttir";
    STRINGDATA(self)->value = wcsdup(s);
    STRINGDATA(self)->char_length = wcslen(STRINGDATA(self)->value);

    // Calculate length for each character, and add to total
    STRINGDATA(self)->byte_length = 0;
    for (int i=0; i!=STRINGDATA(self)->char_length; i++) {
        int l = wctomb(utf8_char_buf, STRINGDATA(self)->value[i]);
        STRINGDATA(self)->byte_length += l;
    }

    recalc_hash(self);

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = self;
    return result;
}

SAFFIRE_METHOD(string, dtor) {
    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = NULL;
    return result;
}

SAFFIRE_METHOD(string, print) {
    char utf8_char_buf[MB_LEN_MAX];

    for (int i=0; i!=STRINGDATA(self)->char_length; i++) {
        int l = wctomb(utf8_char_buf, STRINGDATA(self)->value[i]);
        for (int j=0; j!=l; j++) {
            printf("%c", utf8_char_buf[j]);
        }
    }
    printf("\n");

    t_saffire_result *result = smm_malloc(sizeof(t_saffire_result));
    result->result = NULL;
    return result;
}


void object_string_init(void) {
    string_methods = ht_create();
    ht_add(string_methods, "ctor", object_string_method_ctor);
    ht_add(string_methods, "dtor", object_string_method_dtor);

    ht_add(string_methods, "byte_length", object_string_method_byte_length);
    ht_add(string_methods, "length", object_string_method_length);
    ht_add(string_methods, "upper", object_string_method_upper);
    ht_add(string_methods, "lower", object_string_method_lower);
    ht_add(string_methods, "reverse", object_string_method_reverse);
    ht_add(string_methods, "print", object_string_method_print);

    string_properties = ht_create();
}

static void object_string_fini() {
    ht_destroy(string_methods);
    ht_destroy(string_properties);
}

static void object_string_alloc(t_object *obj) {
    obj->data = (t_object_data_string *)smm_malloc(sizeof(t_object_data_string));
}

static void object_string_free(t_object *obj) {
    smm_free(STRINGDATA(obj)->value);
    smm_free(obj->data);
}

static t_object *object_string_clone(t_object *obj) {
    // Create new object and copy all info
    t_object *new = object_new();
    memcpy(new, obj, sizeof(t_object));

    // New separated object, so refcount = 1
    obj->header.ref_count = 1;

    object_string_alloc(new);
    STRINGDATA(new)->char_length = STRINGDATA(obj)->char_length;
    STRINGDATA(new)->byte_length = STRINGDATA(obj)->byte_length;
    memcpy(STRINGDATA(new)->hash, STRINGDATA(obj)->hash, 16);
    STRINGDATA(new)->value = wcsdup(STRINGDATA(obj)->value);

    return new;
}


SAFFIRE_NEW_OBJECT(string) {
    t_object *obj = object_new();

    obj->header.name = "string";
    obj->header.fqn = "::string";

    obj->methods = string_methods;
    obj->properties = string_properties;

    obj->funcs.alloc = object_string_alloc;
    obj->funcs.free = object_string_free;
    obj->funcs.clone = object_string_clone;

    obj->funcs.alloc(obj);
    return obj;
}
