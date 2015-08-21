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
#include <saffire/memory/smm.h>
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

//t_string *object_string_cat(t_string *s1, t_string *s2) {
//    t_string *dst = string_strdup(s1);
//    string_strcat(dst, s2);
//    return dst;
//}

static int _string_compare(t_string *s1, t_string *s2) {
    create_utf8_from_string(s1);
    create_utf8_from_string(s2);

    return utf8_strcmp(s1, s2);
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
    t_string *str = NULL, *locale = NULL;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s|s", &str, &locale) != 0) {
        return NULL;
    }

    self->data.value = string_strdup(str);
    if (locale) {
        self->data.locale = STRING_CHAR0(locale);
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
    RETURN_NUMERICAL(STRING_LEN(self->data.value));
}

/**
 * Saffire method: Returns uppercased string object
 */
SAFFIRE_METHOD(string, upper) {
    create_utf8_from_string(self->data.value);

    t_string *dst = utf8_toupper(self->data.value, self->data.locale);

    // Create new object
    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}

/**
 * Saffire method: Returns ucfirst string object
 */
SAFFIRE_METHOD(string, ucfirst) {
    create_utf8_from_string(self->data.value);

    t_string *dst = utf8_ucfirst(self->data.value, self->data.locale);

    // Create new object
    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}



/**
 * Saffire method: Returns lowercased string object
 */
SAFFIRE_METHOD(string, lower) {
    create_utf8_from_string(self->data.value);

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
    utf8_free_unicode(dst);

    // Reverse all chars, except the last \0
    char *c = STRING_CHAR0(dst);
    for (int i=0; i!=STRING_LEN(dst); i++) {
        c[i] = STRING_CHAR0(self->data.value)[STRING_LEN(dst) - 1 - i];
    }

    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}


/**
 * Saffire method: Trims whitespaces left and right
 */
SAFFIRE_METHOD(string, trim) {
    if (STRING_LEN(self->data.value) == 0) {
        RETURN_STRING_FROM_CHAR("");
    }

    // Trim left
    char *str = STRING_CHAR0(self->data.value);
    while(isspace(*str)) str++;

    // Hit a \0 char? return empty string (breaks binsafe strings)
    if (*str == 0) {
        RETURN_STRING_FROM_CHAR("");
    }

    // Trim right
    char *end = STRING_CHAR0(self->data.value) + STRING_LEN(self->data.value) - 1;
    while(end >= str && isspace(*end)) end--;
    end++;

    RETURN_STRING_FROM_BINSAFE_CHAR((end-str), str);
}


/**
 * Saffire method: Trims whitespaces left
 */
SAFFIRE_METHOD(string, ltrim) {
    if (STRING_LEN(self->data.value) == 0) {
        RETURN_STRING_FROM_CHAR("");
    }

    // Trim left
    long len = STRING_LEN(self->data.value);
    char *str = STRING_CHAR0(self->data.value);
    while(isspace(*str)) {
        str++;
        len--;
    }

    // Hit a \0 char? return empty string (breaks binsafe strings)
    if (*str == 0) {
        RETURN_STRING_FROM_CHAR("");
    }

    RETURN_STRING_FROM_BINSAFE_CHAR(len, str);
}

/**
 * Saffire method: Trims whitespaces right
 */
SAFFIRE_METHOD(string, rtrim) {
    if (STRING_LEN(self->data.value) == 0) {
        RETURN_STRING_FROM_CHAR("");
    }

    // Trim right
    char *str = STRING_CHAR0(self->data.value);
    char *end = str + STRING_LEN(self->data.value) - 1;
    while(end >= str && isspace(*end)) end--;
    end++;

    if (end == str) {
        RETURN_STRING_FROM_CHAR("");
    }

    RETURN_STRING_FROM_BINSAFE_CHAR((end-str), str);
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_boolean) {
    if (STRING_LEN(self->data.value) == 0) {
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
    char *c = STROBJ2CHAR0(self);
    long value;

    if (c[0] == '0' && c[1] == 'x') {
        // Hex
        value = strtol(c, NULL, 16);
    } else if (c[0] == 'b') {
        // Binary
        value = strtol(c, NULL, 2);
    } else if (c[0] == '0') {
        // Octal
        value = strtol(c, NULL, 8);
    } else {
        // default to decimal
        value = strtol(c, NULL, 10);
    }

    RETURN_NUMERICAL(value);
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_string) {
    RETURN_SELF;
}

SAFFIRE_METHOD(string, split) {
    t_string *token;
    long max = 0;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s|n", &token, &max) != 0) {
        return NULL;
    }

    // @TODO: HIGH: Split does not work? string_split??

    RETURN_FALSE;
}

/**
 *
 */
SAFFIRE_METHOD(string, splice) {
    // @TODO: MEDIUM: We probably want to just change the length and the offset of the t_string. Not doing any real copies
    long min, max;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "n+n+", &min, &max) != 0) {
        return NULL;
    }

    // If max is 0, use the complete length of the string
    if (max == 0) {
        max = STRING_LEN(self->data.value);
    }

    if (min == 0 && max == 0) RETURN_SELF;

    // Below 0, means we have to seek from the end of the string
    if (min < 0) min = STRING_LEN(self->data.value) + min - 1;
    if (max < 0) max = STRING_LEN(self->data.value) + max - 1;

    if (min > STRING_LEN(self->data.value)) min = STRING_LEN(self->data.value);
    if (max > STRING_LEN(self->data.value) || max == 0) max = STRING_LEN(self->data.value);

    // Sanity check
    if (max < min) {
        object_raise_exception(Object_SystemException, 1, "start of a subscription must be less or equal than its end");
        return NULL;
    }

    long new_size = (max - min) + 1;
    if (new_size == 0) {
        RETURN_STRING_FROM_CHAR("");
    }


    t_string *dst = string_copy_partial(self->data.value, min, new_size);

    t_string_object *obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(obj);
}


/**
 *
 */
SAFFIRE_METHOD(string, to_locale) {
    t_string *locale;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &locale) != 0) {
        return NULL;
    }

    // Clone as new string
    t_string_object *dst = (t_string_object *)object_clone((t_object *)self);

    // Set new locale
    string_change_locale(dst, STRING_CHAR0(locale));

    RETURN_OBJECT(dst);
}


/**
 *
 */
SAFFIRE_METHOD(string, get_locale) {
    RETURN_STRING_FROM_CHAR(self->data.locale);
}


/**
 *
 */
SAFFIRE_METHOD(string, index) {
    t_string *needle;
    long offset = 0;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s|n", &needle, &offset) != 0) {
        return NULL;
    }

    create_utf8_from_string(self->data.value);

    int pos = utf8_strstr(self->data.value, needle, offset);
    if (pos == -1) {
        RETURN_FALSE;
    }

    RETURN_NUMERICAL(pos);
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(string, add) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    t_string *dst = string_strcat(self->data.value, other);

    RETURN_STRING(dst);
}

/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(string, eq) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (STRING_LEN(self->data.value) != STRING_LEN(other)) {
        RETURN_FALSE;
    }

    // @TODO: Assuming that every unique string will be at the same address, we could do a simple address check
    //        instead of a memcmp. However, it means that we MUST make sure that the value_len's are also matching,
    //        otherwise "foo" would match "foobar", as they both have the same start address
    if (_string_compare(self->data.value, other) == 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, ne) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (STRING_LEN(self->data.value) != STRING_LEN(other)) {
        RETURN_TRUE;
    }

    // @TODO: Assuming that every unique string will be at the same address, we could do a simple address check
    //        instead of a memcmp. However, it means that we MUST make sure that the value_len's are also matching,
    //        otherwise "foo" would match "foobar", as they both have the same start address
    if (_string_compare(self->data.value, other) != 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, lt) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (_string_compare(self->data.value, other) < 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, gt) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (_string_compare(self->data.value, other) > 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, le) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (_string_compare(self->data.value, other) <= 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;

}

SAFFIRE_COMPARISON_METHOD(string, ge) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    if (_string_compare(self->data.value, other) >= 0) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_COMPARISON_METHOD(string, in) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    create_utf8_from_string(self->data.value);
    create_utf8_from_string(other);

    utf8_strstr(self->data.value, other, 0) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(string, ni) {
    t_string *other;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s",  &other) != 0) {
        return NULL;
    }

    create_utf8_from_string(self->data.value);
    create_utf8_from_string(other);

    utf8_strstr(self->data.value, other, 0) ? (RETURN_FALSE) : (RETURN_TRUE);
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
    if (self->data.iter < STRING_LEN(self->data.value)) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

SAFFIRE_METHOD(string, __set) {
    RETURN_SELF;
}
SAFFIRE_METHOD(string, __remove) {
    RETURN_SELF;
}
SAFFIRE_METHOD(string, __get) {
    long idx;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &idx) != 0) {
        return NULL;
    }

    if (idx < 0 || idx > STRING_LEN(self->data.value)) {
        object_raise_exception(Object_IndexException, 1, "Index out of range");
        return NULL;
    }

    t_string *dst = string_copy_partial(self->data.value, idx, 1);
    t_string_object *dst_obj = string_create_new_object(dst, self->data.locale);
    RETURN_OBJECT(dst_obj);
}


SAFFIRE_METHOD(string, __has) {
    long idx;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &idx) != 0) {
        return NULL;
    }

    if (idx < 0 || idx > STRING_LEN(self->data.value)) {
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
    object_add_internal_method((t_object *)&Object_String_struct, "__ctor",           ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_string_method_ctor);
    object_add_internal_method((t_object *)&Object_String_struct, "__dtor",           ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_string_method_dtor);

    object_add_internal_method((t_object *)&Object_String_struct, "__boolean",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_String_struct, "__null",           ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_null);
    object_add_internal_method((t_object *)&Object_String_struct, "__numerical",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_String_struct, "__string",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_conv_string);

//    object_add_internal_method((t_object *)&Object_String_struct, "byte_length",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_byte_lengthx);
    object_add_internal_method((t_object *)&Object_String_struct, "length",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_length);
    object_add_internal_method((t_object *)&Object_String_struct, "upper",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_upper);
    object_add_internal_method((t_object *)&Object_String_struct, "ucfirst",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_ucfirst);
    object_add_internal_method((t_object *)&Object_String_struct, "lower",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_lower);
    object_add_internal_method((t_object *)&Object_String_struct, "reverse",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_reverse);
    object_add_internal_method((t_object *)&Object_String_struct, "trim",           ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_trim);
    object_add_internal_method((t_object *)&Object_String_struct, "ltrim",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_ltrim);
    object_add_internal_method((t_object *)&Object_String_struct, "rtrim",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_rtrim);

    object_add_internal_method((t_object *)&Object_String_struct, "toLocale",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_to_locale);
    object_add_internal_method((t_object *)&Object_String_struct, "getLocale",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_get_locale);

    object_add_internal_method((t_object *)&Object_String_struct, "index",          ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_index);
//    object_add_internal_method((t_object *)&Object_String_struct, "splice",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_splice);

    object_add_internal_method((t_object *)&Object_String_struct, "__opr_add",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_add);
//    object_add_internal_method((t_object *)&Object_String_struct, "__opr_sl",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sl);
//    object_add_internal_method((t_object *)&Object_String_struct, "__opr_sr",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_opr_sr);

    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_eq",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ne",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ne);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_lt",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_lt);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_gt",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_gt);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_le",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_le);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ge",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ge);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_in",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_in);
    object_add_internal_method((t_object *)&Object_String_struct, "__cmp_ni",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_string_method_cmp_ni);


    // Iterator interface
    object_add_internal_method((t_object *)&Object_String_struct, "__iterator",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___iterator);
    object_add_internal_method((t_object *)&Object_String_struct, "__key",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___key);
    object_add_internal_method((t_object *)&Object_String_struct, "__value",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___value);
    object_add_internal_method((t_object *)&Object_String_struct, "__rewind",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___rewind);
    object_add_internal_method((t_object *)&Object_String_struct, "__next",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___next);
    object_add_internal_method((t_object *)&Object_String_struct, "__hasNext",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___hasNext);
    object_add_interface((t_object *)&Object_String_struct, Object_Iterator);

    // Subscription interface
    object_add_internal_method((t_object *)&Object_String_struct, "__length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_length);
    object_add_internal_method((t_object *)&Object_String_struct, "__set",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___set);
    object_add_internal_method((t_object *)&Object_String_struct, "__remove",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___remove);
    object_add_internal_method((t_object *)&Object_String_struct, "__get",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___get);
    object_add_internal_method((t_object *)&Object_String_struct, "__has",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method___has);
    object_add_internal_method((t_object *)&Object_String_struct, "__splice",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_string_method_splice);
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
        str_obj->data.value = DLL_DATA_PTR(e);
    } else if (arg_list->size > 1) {
        // 2 (or more) elements: it's a size + char0 string

        // Get length of string
        t_dll_element *e = DLL_HEAD(arg_list);
        long value_len = DLL_DATA_LONG(e);

        // Get actual binary safe and non-encoded string
        e = DLL_NEXT(e);
        char *value = DLL_DATA_PTR(e);

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

/**
 * Clones custom string data into a the new object
 */
static void obj_clone(const t_object *original_obj, t_object *cloned_obj) {
    t_string_object *str_org_obj = (t_string_object *)original_obj;
    t_string_object *str_cloned_obj = (t_string_object *)cloned_obj;

    str_cloned_obj->data.locale = string_strdup0(str_org_obj->data.locale);
    str_cloned_obj->data.value = string_strdup(str_org_obj->data.value);
}


#ifdef __DEBUG

/**
 * Object debug doesn't output binary safe strings
 */
static char *obj_debug(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;

    if (! str_obj->data.value) {
        snprintf(str_obj->__debug_info, DEBUG_INFO_SIZE-1, "string()");
    } else {
        snprintf(str_obj->__debug_info, DEBUG_INFO_SIZE-1, "string(%zd):\"%s\"", STRING_LEN(str_obj->data.value), STRING_CHAR0(str_obj->data.value));
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
        obj_clone,            // Clone
        NULL,                 // Object cache
        obj_hash,             // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
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
    },
    OBJECT_FOOTER
};
