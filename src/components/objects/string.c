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
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <saffire/general/string.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>
#include <saffire/general/md5.h>
#include <saffire/general/output.h>
#include <saffire/debug.h>
#include <saffire/vm/thread.h>

/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */
/**
 * Returns a byte[16] md5 hash of the given widestring
 */
static void calculate_hash(t_string_object *str_obj) {
    md5_state_t state;

    md5_init(&state);
    md5_append(&state, (md5_byte_t *)STROBJ2CHAR0(str_obj), STROBJ2CHAR0LEN(str_obj));
    md5_finish(&state, str_obj->data.hash);
}


static void string_change_locale(t_string_object *str_obj, char *locale) {
    if (str_obj->data.locale) {
        smm_free(str_obj->data.locale);
    }
    str_obj->data.locale = string_strdup0(locale);
}

static t_string_object *string_create_new_object(t_string *str, char *locale) {
    t_string_object *uc_obj = (t_string_object *)object_alloc_instance(Object_String, 0);
    object_inc_ref((t_object *)uc_obj);

    uc_obj->data.value = str;
    uc_obj->data.locale = string_strdup0(locale);

    uc_obj->data.needs_hashing = 1;

    return uc_obj;
}

t_string *object_string_cat(t_string_object *s1, t_string_object *s2) {
    t_string *dst = string_strdup(s1->data.value);
    string_strcat(dst, s2->data.value);

    return dst;
}

int object_string_compare(t_string_object *s1, t_string_object *s2) {
    return utf8_strcmp(s1->data.value, s2->data.value);
}

/* ======================================================================
 *   Object methods
 * ======================================================================
 */


 int object_string_hash_compare(t_string_object *s1, t_string_object *s2) {
    int c = 0;

    // Do a complete hash check to counter timing attacks
    for (int i=0; i!=16; i++) {
        c += (s1->data.hash[i] ^ s2->data.hash[i]);
    }
    return (c == 0);
 }

/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(string, ctor) {
    t_string_object *str_obj, *locale_obj;
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s|s", &str_obj, &locale_obj)) {
        return NULL;
    }

    self->data.value = string_strdup(str_obj->data.value);
    if (locale_obj) {
        self->data.locale = string_to_char(locale_obj->data.value);
    } else {
        t_thread *thread = thread_get_current();
        self->data.locale = thread->locale ? string_strdup0(thread->locale) : NULL;
    }
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
    RETURN_NUMERICAL(self->data.value->len);
}

/**
 * Saffire method: Returns uppercased string object
 */
SAFFIRE_METHOD(string, upper) {
    t_string *dst = utf8_toupper(self->data.value, self->data.locale);

    // Create new object
    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}


/**
 * Saffire method: Returns lowercased string object
 */
SAFFIRE_METHOD(string, lower) {
    t_string *dst = utf8_tolower(self->data.value, self->data.locale);

    // Create new object
    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}

/**
 * Saffire method: Returns reversed string object
 */
SAFFIRE_METHOD(string, reverse) {
    t_string *dst = string_strdup(self->data.value);

    // Reverse all chars
    for (int i=0; i!=dst->len; i++) {
        dst->val[i] = self->data.value->val[dst->len - i];
    }
    utf8_free_unicode(dst);

    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}


/**
 *
 */
SAFFIRE_METHOD(string, conv_boolean) {
    if (self->data.value->len == 0) {
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
    long value = strtol(STROBJ2CHAR0(self), NULL, 0);
    RETURN_NUMERICAL(value);
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_string) {
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_METHOD(string, splice) {
    // @TODO: We probably want to just change the length and the offset of the t_string. Not doing any real copies
    t_object *min_obj;
    t_object *max_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "oo", &min_obj, &max_obj)) {
        return NULL;
    }

    signed long min = OBJECT_IS_NULL(min_obj) ? 0 : OBJ2NUM(min_obj);
    signed long max = OBJECT_IS_NULL(max_obj) ? self->data.value->len : OBJ2NUM(max_obj);

    if (min == 0 && max == 0) RETURN_SELF;

    // Below 0, means we have to seek from the end of the string
    if (min < 0) min = self->data.value->len + min - 1;
    if (max < 0) max = self->data.value->len + max - 1;

    if (min > self->data.value->len) min = self->data.value->len;
    if (max > self->data.value->len || max == 0) max = self->data.value->len;

    // Sanity check
    if (max < min) {
        object_raise_exception(Object_SystemException, 1, "max < min!");
        return NULL;
    }

    long new_size = (max - min) + 1;

    if (new_size == 0) {
        object_raise_exception(Object_SystemException, 1, "lenght == 0");
        return NULL;
    }


    t_string *dst = string_copy_partial(self->data.value, min, new_size);

    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}


/**
 *
 */
SAFFIRE_METHOD(string, to_locale) {
    t_string_object *str_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", (t_object *)&str_obj)) {
        return NULL;
    }

    // Clone as new string
    t_string_object *dst = (t_string_object *)object_clone((t_object *)self);

    // Set new locale
    string_change_locale(dst, STROBJ2CHAR0(str_obj));

    RETURN_OBJECT(dst);
}


/**
 *
 */
SAFFIRE_METHOD(string, get_locale) {
    RETURN_STRING_FROM_CHAR(self->data.locale);
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


    t_string *dst = object_string_cat(self, other);

    t_string_object *uc_obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(uc_obj);
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

    if (self->data.value->len != other->data.value->len) {
        RETURN_FALSE;
    }

    // @TODO: Assuming that every unique string will be at the same address, we could do a simple address check
    //        instead of a memcmp. However, it means that we MUST make sure that the value_len's are also matching,
    //        otherwise "foo" would match "foobar", as they both have the same start address
    if (object_string_compare(self, other) == 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, ne) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    if (self->data.value->len != other->data.value->len) {
        RETURN_TRUE;
    }

    // @TODO: Assuming that every unique string will be at the same address, we could do a simple address check
    //        instead of a memcmp. However, it means that we MUST make sure that the value_len's are also matching,
    //        otherwise "foo" would match "foobar", as they both have the same start address
    if (object_string_compare(self, other) != 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, lt) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    if (object_string_compare(self, other) < 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, gt) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    if (object_string_compare(self, other) > 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, le) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    if (object_string_compare(self, other) <= 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;

}

SAFFIRE_COMPARISON_METHOD(string, ge) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    if (object_string_compare(self, other) >= 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, in) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    utf8_strstr(self->data.value, other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, ni) {
    t_string_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other)) {
        return NULL;
    }

    utf8_strstr(self->data.value, other->data.value) ? (RETURN_FALSE) : (RETURN_TRUE);
}




SAFFIRE_METHOD(string, __iterator) {
    RETURN_SELF;
}

SAFFIRE_METHOD(string, __key) {
    RETURN_NUMERICAL(self->data.iter);
}

SAFFIRE_METHOD(string, __value) {
    t_string *dst = string_copy_partial(self->data.value, self->data.iter, 1);
    t_string_object *dst_obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(dst_obj);
}

SAFFIRE_METHOD(string, __next) {
    self->data.iter++;
    RETURN_SELF;
}

SAFFIRE_METHOD(string, __rewind) {
    self->data.iter = 0;
    RETURN_SELF;
}

SAFFIRE_METHOD(string, __hasNext) {
    if (self->data.iter < self->data.value->len) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_METHOD(string, __add) {
    RETURN_SELF;
}
SAFFIRE_METHOD(string, __remove) {
    RETURN_SELF;
}
SAFFIRE_METHOD(string, __get) {
    t_object *idx_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &idx_obj)) {
        return NULL;
    }

    long idx = OBJ2NUM(idx_obj);
    if (idx < 0 || idx > self->data.value->len) {
        object_raise_exception(Object_IndexException, 1, "Index out of range");
        return NULL;
    }


    t_string *dst = string_copy_partial(self->data.value, idx, 1);
    t_string_object *dst_obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(dst_obj);
}


SAFFIRE_METHOD(string, __has) {
    t_object *idx_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &idx_obj)) {
        return NULL;
    }

    long idx = OBJ2NUM(idx_obj);
    if (idx < 0 || idx > self->data.value->len) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
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
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__ctor",           ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_string_method_ctor);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__dtor",           ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_string_method_dtor);

    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__boolean",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_boolean);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__null",           ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_null);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__numerical",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_numerical);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__string",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_string);

//    object_add_internal_method((t_object *)&Object_String_struct, "byte_length",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_byte_lengthx);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "length",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_length);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "upper",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_upper);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "lower",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_lower);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "reverse",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_reverse);

    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "toLocale",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_to_locale);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "getLocale",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_get_locale);


    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "splice",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_splice);

    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__opr_add",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_add);
//    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__opr_sl",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sl);
//    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__opr_sr",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sr);

    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_eq",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_eq);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_ne",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ne);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_lt",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_lt);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_gt",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_gt);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_le",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_le);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_ge",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ge);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_in",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_in);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__cmp_ni",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ni);


    // Iterator interface
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__iterator",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___iterator);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__key",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___key);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__value",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___value);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__rewind",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___rewind);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__next",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___next);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__hasNext",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___hasNext);
    object_add_interface((t_object *)&Object_String_struct, Object_Iterator);

    // Subscription interface
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_length);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__add",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___add);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__remove",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___remove);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__get",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___get);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__has",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___has);
    object_add_internal_method(Object_String_struct.attributes, (t_object *)&Object_String_struct, "__splice",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_splice);
    object_add_interface((t_object *)&Object_String_struct, Object_Subscription);

    vm_populate_builtins("string", (t_object *)&Object_String_struct);
}

/**
 * Frees memory for a string object
 */
void object_string_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_String_struct);
}



static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_string_object *str_obj = (t_string_object *)obj;

    if (arg_list->size == 1) {
        // 1 element: it's already a string
        t_dll_element *e = DLL_HEAD(arg_list);
        str_obj->data.value = (t_string *)e->data.p;
    } else if (arg_list->size > 1) {
        // 2 (or more) elements: it's a size + char0 string

        // Get length of string
        t_dll_element *e = DLL_HEAD(arg_list);
        int value_len = (int)e->data.l;

        // Get actual binary safe and non-encoded string
        e = DLL_NEXT(e);
        char *value = (char *)e->data.p;

        // Convert our stream to UTF8
        str_obj->data.value = char_to_string(value, value_len);
    }

    t_thread *thread = thread_get_current();
    str_obj->data.locale = thread->locale ? string_strdup0(thread->locale) : NULL;
}

static void obj_free(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;
    if (str_obj->data.value) string_free(str_obj->data.value);
    if (str_obj->data.locale) smm_free(str_obj->data.locale);
}


static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG

/**
 * Object debug doesn't output binary safe strings
 */
static char *obj_debug(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;

    if (! str_obj->data.value) {
        snprintf(str_obj->__debug_info, 199, "string()");
    } else {
        snprintf(str_obj->__debug_info, 199, "string(%d):\"%s\"", str_obj->data.value->len, str_obj->data.value->val);
    }
    return str_obj->__debug_info;
}
#endif

static char *obj_hash(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;

    char *s = (char *)smm_malloc(17);

    if (str_obj->data.needs_hashing == 1) {
        // Generate hash
        calculate_hash(str_obj);
        str_obj->data.needs_hashing = 0;
    }
    memcpy(s, str_obj->data.hash, 16);
    s[16] = '\0';

    return s;
}



// String object management functions
t_object_funcs string_funcs = {
        obj_populate,         // Populate a string object
        obj_free,             // Free a string object
        obj_destroy,          // Destroy a string object
        NULL,                 // Clone
        NULL,                 // Object cache
        obj_hash,             // Hash
#ifdef __DEBUG
        obj_debug,
#endif
};


// Intial object
t_string_object Object_String_struct = {
    OBJECT_HEAD_INIT("string", objectTypeString, OBJECT_TYPE_CLASS, &string_funcs, sizeof(t_string_object_data)),
    {
        NULL,       // Value
        "",         // Hash value
        1,          // Needs hashing
        0,          // Internal iteration index
        NULL,       // Locale
    }
};
