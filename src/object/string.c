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
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
#include "object/object.h"
#include "object/boolean.h"
#include "object/string.h"
#include "object/null.h"
#include "object/base.h"
#include "object/numerical.h"
#include "general/smm.h"
#include "general/md5.h"

extern char *wctou8(const wchar_t *wstr);


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */

/**
 * Recalculate MD5 hash for current string in object.
 */
static void recalc_hash(t_string_object *obj) {
    char utf8_char_buf[MB_LEN_MAX];
    md5_state_t state;

    md5_init(&state);
    for (int i=0; i!=obj->char_length; i++) {
        // Convert wide character to UTF8 character
        int utf8_char_len = wctomb(utf8_char_buf, obj->value[i]);

        // Add character to md5
        md5_append(&state, utf8_char_buf, utf8_char_len);
    }
    md5_finish(&state, obj->hash);

    printf("Recalc_Hash:");
    for (int i=0; i!=16; i++) {
        printf("%02X", (unsigned char)obj->hash[i]);
    }
    printf("\n");
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
    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);

    for (int i=0; i!=obj->char_length; i++) {
        obj->value[i] = towupper(obj->value[i]);
    }
    recalc_hash(obj);

    RETURN_OBJECT(obj);
}

/**
 * Saffire method: Returns lowercased string object
 */
SAFFIRE_METHOD(string, lower) {
    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);

    for (int i=0; i!=obj->char_length; i++) {
        obj->value[i] = towlower(obj->value[i]);
    }
    recalc_hash(obj);

    RETURN_OBJECT(obj);
}

/**
 * Saffire method: Returns reversed string object
 */
SAFFIRE_METHOD(string, reverse) {
    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);

    wchar_t *str = obj->value;
    wchar_t *p1, *p2;

    if (! str || ! *str) {
        RETURN_OBJECT(obj);
    }

    wchar_t p3;
    for (p1 = str, p2 = str + obj->char_length - 1; p2 > p1; ++p1, --p2) {
        p3 = *p2;
        *p2 = *p1;
        *p1 = p3;
    }

    recalc_hash(obj);

    RETURN_OBJECT(obj);
}

/**
 * Saffire method: output strings
 */
SAFFIRE_METHOD(string, print) {
    char utf8_char_buf[MB_LEN_MAX];

    for (int i=0; i!=self->char_length; i++) {
        int l = wctomb(utf8_char_buf, self->value[i]);
        for (int j=0; j!=l; j++) {
            printf("%c", utf8_char_buf[j]);
        }
    }
    printf("\n");

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
    long value = wcstol(self->value, NULL, 0);

    RETURN_NUMERICAL(value);
}

/**
 *
 */
SAFFIRE_METHOD(string, conv_string) {
    RETURN_SELF;
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(string, add) {
    t_string_object *self = (t_string_object *)_self;

    if (in_place) {
        //self->value += 1;
        RETURN_SELF;
    }

    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(string, sl) {
    t_string_object *self = (t_string_object *)_self;

    // @TODO:    "foo" << 1 == "oo"
    if (in_place) {
        //self->value <<= 1;
        RETURN_SELF;
    }

    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(string, sr) {
    t_string_object *self = (t_string_object *)_self;

    // @TODO:    "foo" >> 1 == "fo"
    if (in_place) {
        //self->value >>= 1;
        RETURN_SELF;
    }

    t_string_object *obj = (t_string_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(string, eq) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) == 0);
}
SAFFIRE_COMPARISON_METHOD(string, ne) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) != 0);
}
SAFFIRE_COMPARISON_METHOD(string, lt) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) == -1);
}
SAFFIRE_COMPARISON_METHOD(string, gt) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) == 1);
}
SAFFIRE_COMPARISON_METHOD(string, le) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) <= 0);
}
SAFFIRE_COMPARISON_METHOD(string, ge) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    return (wcscmp(self->value, other->value) >= 0);
}
SAFFIRE_COMPARISON_METHOD(string, in) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    if (wcsstr(self->value, other->value) != NULL);
}
SAFFIRE_COMPARISON_METHOD(string, ni) {
    t_string_object *self = (t_string_object *)_self;
    t_string_object *other = (t_string_object *)_other;

    if (wcsstr(self->value, other->value) == NULL);
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes string methods and properties, these are used
 */
void object_string_init(void) {
    Object_String_struct.methods = ht_create();
    ht_add(Object_String_struct.methods, "ctor", object_string_method_ctor);
    ht_add(Object_String_struct.methods, "dtor", object_string_method_dtor);

    ht_add(Object_String_struct.methods, "boolean", object_string_method_conv_boolean);
    ht_add(Object_String_struct.methods, "null", object_string_method_conv_null);
    ht_add(Object_String_struct.methods, "numerical", object_string_method_conv_numerical);
    ht_add(Object_String_struct.methods, "string", object_string_method_conv_string);

    ht_add(Object_String_struct.methods, "byte_length", object_string_method_byte_length);
    ht_add(Object_String_struct.methods, "length", object_string_method_length);
    ht_add(Object_String_struct.methods, "upper", object_string_method_upper);
    ht_add(Object_String_struct.methods, "lower", object_string_method_lower);
    ht_add(Object_String_struct.methods, "reverse", object_string_method_reverse);
    ht_add(Object_String_struct.methods, "print", object_string_method_print);

    Object_String_struct.properties = ht_create();
}

/**
 * Frees memory for a string object
 */
void object_string_fini(void) {
    ht_destroy(Object_String_struct.methods);
    ht_destroy(Object_String_struct.properties);
}


/**
 * Frees memory for a string object
 */
static void obj_free(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;

    if (str_obj->value != NULL) {
        free(str_obj->value);
        str_obj->value = NULL;
    }
}

/**
 * Clones a string object into a new object
 */
static t_object *obj_clone(t_object *obj) {
    t_string_object *str_obj = (t_string_object *)obj;

    // Create new object and copy all info
    t_string_object *new_obj = smm_malloc(sizeof(t_string_object));
    memcpy(new_obj, str_obj, sizeof(t_string_object));

    // Newly separated object, so refcount is 1 again
    new_obj->ref_count = 1;

    // Copy / set internal data
    new_obj->char_length = str_obj->char_length;
    new_obj->byte_length = str_obj->byte_length;
    memcpy(new_obj->hash, str_obj->hash, 16);
    new_obj->value = wcsdup(str_obj->value);

    return (t_object *)new_obj;
}


static t_object *obj_new(va_list arg_list) {
    // Create new object and copy all info
    t_string_object *new_obj = smm_malloc(sizeof(t_string_object));
    memcpy(new_obj, Object_String, sizeof(t_string_object));

    // Set internal data
    char utf8_char_buf[MB_LEN_MAX];

    wchar_t value = va_arg(arg_list, wchar_t);

    new_obj->value = wcsdup((const wchar_t *)value);
    new_obj->char_length = wcslen(new_obj->value);

    // Calculate length for each character, and add to total
    new_obj->byte_length = 0;
    for (int i=0; i!=new_obj->char_length; i++) {
        int l = wctomb(utf8_char_buf, new_obj->value[i]);
        new_obj->byte_length += l;
    }
    recalc_hash(new_obj);

    return (t_object *)new_obj;
}

char global_buf[1024];
static char *obj_debug(struct _object *obj) {
    char *buf = wctou8(((t_string_object *)obj)->value);
    memcpy(global_buf, buf, 1024);
    global_buf[1023] = 0;
    smm_free(buf);

    return global_buf;
}


// String object management functions
t_object_funcs string_funcs = {
        obj_new,              // Allocate a new string object
        obj_free,             // Free a string object
        obj_clone,            // Clone a string object
        obj_debug
};

t_object_operators string_ops = {
    object_string_operator_add,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_string_operator_sl,
    object_string_operator_sr
};

t_object_comparisons string_cmps = {
    object_string_comparison_eq,
    object_string_comparison_ne,
    object_string_comparison_lt,
    object_string_comparison_gt,
    object_string_comparison_le,
    object_string_comparison_ge,
    object_string_comparison_in,
    object_string_comparison_ni
};

// Intial object
t_string_object Object_String_struct = {
    OBJECT_HEAD_INIT2("string", objectTypeString, &string_ops, &string_cmps, OBJECT_NO_FLAGS, &string_funcs),
    0,
    0,
    '\0',
    L'\0'
};