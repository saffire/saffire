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
#include <stdarg.h>
#include <ctype.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/md5.h"
#include "general/output.h"
#include "debug.h"

t_hash_table *string_cache;


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */

/**
 * Returns the number of characters inside a (UTF8) string
 */
static int utf8_len(char *s) {
    int i = 0;
    int j = 0;

    while (s[i]) {
        if ((s[i] & 0xC0) != 0x80) j++;
        i++;
    }
    return j;
}


/**
 * Returns a byte[16] md5 hash of the given widestring
 */
static void hash_string(char *str, md5_byte_t *hash) {
    md5_state_t state;

    md5_init(&state);
    md5_append(&state, (md5_byte_t *)str, strlen(str));
    md5_finish(&state, hash);
}

/**
 * Returns a char[32] (+\0) md5 hash of the given widestring in text.
 */
static void hash_string_text(char *str, char *strhash) {
    md5_byte_t hash[16];

    hash_string(str, hash);

    for (int i=0; i!=16; i++) {
        sprintf(strhash+(i*2), "%02X", (unsigned char)hash[i]);
    }
    strhash[32] = '\0';
}

/**
 * Recalculate MD5 hash for current string in object.
 */
static void recalc_hash(t_string_object *obj) {
    hash_string(obj->value, obj->hash);
}


static long find_utf_idx(t_string_object *obj, long idx) {
    long utf_idx = 0;

    for (long i=0; i!=idx; i++) {
        while ((obj->value[utf_idx] & 0xC0) == 0x80) utf_idx++;
        utf_idx++;
    }

    return utf_idx;
}

/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(string, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(string, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns length of the string (in characters)
 */
SAFFIRE_METHOD(string, length) {
    RETURN_NUMERICAL(self->char_length);
}

/**
 * Saffire method: Returns length of the string (in bytes)
 */
SAFFIRE_METHOD(string, byte_length) {
    RETURN_NUMERICAL(self->byte_length);
}

/**
 * Saffire method: Returns uppercased string object
 */
SAFFIRE_METHOD(string, upper) {
    // @TODO: UTF8 - UPPER!
    RETURN_SELF;
}

/**
 * Saffire method: Returns lowercased string object
 */
SAFFIRE_METHOD(string, lower) {
    // @TODO: UTF8 - LOWER!
    RETURN_SELF;
}

/**
 * Saffire method: Returns reversed string object
 */
SAFFIRE_METHOD(string, reverse) {
    // @TODO: UTF8 - REVERSE!
    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_METHOD(string, conv_boolean) {
    if (self->char_length == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_null) {
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_numerical) {
    // Convert wide into char
    long value = strtol(self->value, NULL, 0);

    RETURN_NUMERICAL(value);
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_string) {
    RETURN_SELF;
}

SAFFIRE_METHOD(string, splice) {
    // @TODO: use parse_args
    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    t_object *obj1 = e->data;
    e = DLL_NEXT(e);
    t_object *obj2 = e->data;

    // We can just return self when we want to do a whole range?
    if (OBJECT_IS_NULL(obj1) && OBJECT_IS_NULL(obj2)) RETURN_SELF;

    // @TODO: make sure obj1 is numerical!
    long min = OBJECT_IS_NULL(obj1) ? 0 : OBJ2NUM(obj1);
    // @TODO: make sure obj1 is numerical!
    long max = OBJECT_IS_NULL(obj2) ? self->char_length : OBJ2NUM(obj2);

    if (min > self->char_length) min = self->char_length;
    if (max > self->char_length) max = self->char_length;

    // Sanity check
    if (max < min) {
        object_raise_exception(Object_SystemException, "max < min!");
        return NULL;
    }

    long min_idx = find_utf_idx(self, min);
    long max_idx = find_utf_idx(self, max);

    long new_size = max_idx - min_idx + 1;

    // Make a new copy of length. Allocate worst-case scenario bytes.
    char *new_string = smm_malloc(new_size + 1);
    memcpy(new_string, self->value + min_idx, new_size);
    new_string[new_size] = '\0';

    t_object *obj = object_new(Object_String, 1, new_string);
    smm_free(new_string);

    RETURN_OBJECT(obj);
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(string, add) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    int new_length = self->byte_length + other->byte_length;
    char *str = smm_malloc(new_length + 1);
    strcpy(str, self->value);
    strcat(str, other->value);

    t_object *obj = object_new(Object_String, 1, str);
    smm_free(str);

    RETURN_OBJECT(obj);
}

/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(string, eq) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    // @TODO: Assuming that every unique string will return the same object, we could do a object check instead of a strcmp
    (strcmp(self->value, other->value) == 0) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, ne) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    // @TODO: Assuming that every unique string will return the same object, we could do a object check instead of a strcmp
    (strcmp(self->value, other->value) != 0) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, lt) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strcmp(self->value, other->value) == -1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, gt) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strcmp(self->value, other->value) == 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, le) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strcmp(self->value, other->value) <= 0) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, ge) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strcmp(self->value, other->value) >= 0) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, in) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strstr(self->value, other->value) != NULL) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, ni) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    (strstr(self->value, other->value) == NULL) ? (RETURN_TRUE) : (RETURN_FALSE);
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes string methods and properties, these are used
 */
void object_string_init(void) {
    Object_String_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_String_struct, "__ctor",           CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_ctor);
    object_add_internal_method((t_object *)&Object_String_struct, "__dtor",           CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_dtor);

    object_add_internal_method((t_object *)&Object_String_struct, "__boolean",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_String_struct, "__null",           CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_null);
    object_add_internal_method((t_object *)&Object_String_struct, "__numerical",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_String_struct, "__string",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_string);

    object_add_internal_method((t_object *)&Object_String_struct, "byte_length",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_byte_length);
    object_add_internal_method((t_object *)&Object_String_struct, "length",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_length);
    object_add_internal_method((t_object *)&Object_String_struct, "upper",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_upper);
    object_add_internal_method((t_object *)&Object_String_struct, "lower",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_lower);
    object_add_internal_method((t_object *)&Object_String_struct, "reverse",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_reverse);

    object_add_internal_method((t_object *)&Object_String_struct, "splice",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_splice);

    object_add_internal_method((t_object *)&Object_String_struct, "__opr_add",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_add);
//    object_add_internal_method((t_object *)&Object_String_struct, "__opr_sl",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sl);
//    object_add_internal_method((t_object *)&Object_String_struct, "__opr_sr",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sr);

    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_eq",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ne",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ne);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_lt",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_lt);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_gt",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_gt);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_le",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_le);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ge",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ge);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_in",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_in);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ni",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ni);

    // Create string cache
    string_cache = ht_create();

    vm_populate_builtins("string", (t_object *)&Object_String_struct);
}

/**
 * Frees memory for a string object
 */
void object_string_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_String_struct);
    ht_destroy(Object_String_struct.attributes);

    // Destroy string cache
    ht_destroy(string_cache);
}



static t_object *obj_new(t_object *self) {
    t_string_object *obj = smm_malloc(sizeof(t_string_object));
    memcpy(obj, Object_String, sizeof(t_string_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_string_object *str_obj = (t_string_object *)obj;

    // Get the widestring from the argument list
    t_dll_element *e = DLL_HEAD(arg_list);
    char *value = (char *)e->data;

    // Create a hash from the string
    char strhash[33];
    hash_string_text(value, strhash);

    // Check for and return cached object
    t_object *cache_obj = ht_find(string_cache, strhash);
    if (cache_obj) {
        obj = cache_obj;
    }

    // No cache, populate the new object

    // Set internal data
    str_obj->value = smm_strdup(value);
    str_obj->char_length = strlen(value);

    // Calculate length for each character, and add to total
    str_obj->byte_length = utf8_len(value);
    recalc_hash(str_obj);

    // Add to string cache
    ht_add(string_cache, strhash, str_obj);
}

static void obj_free(t_object *obj) {
   t_string_object *str_obj = (t_string_object *)obj;

   if (str_obj->value) {
       smm_free(str_obj->value);
   }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    char *s = ((t_string_object *)obj)->value;
    snprintf(global_buf, 1023, "string(%s)", s);
    return global_buf;
}
#endif


// String object management functions
t_object_funcs string_funcs = {
        obj_new,              // Allocate a new string object
        obj_populate,         // Populate a string object
        obj_free,             // Free a string object
        obj_destroy,          // Destroy a string object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

// Intial object
t_string_object Object_String_struct = {
    OBJECT_HEAD_INIT("string", objectTypeString, OBJECT_TYPE_CLASS, &string_funcs),
    '\0',
    0,
    0,
    ""
};
