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
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <saffire/debug.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/memory/smm.h>

#define NUMERICAL_CACHED_MIN   -5       /* minimum numerical value to cache */
#define NUMERICAL_CACHED_MAX  256       /* maximum numerical value to cache */

// Offset calculations
#define NUMERICAL_CACHE_OFF     abs(NUMERICAL_CACHED_MIN)
// Max storage
#define NUMERICAL_CACHED_CNT    NUMERICAL_CACHED_MAX + NUMERICAL_CACHE_OFF + 1

t_numerical_object **numerical_cache;


/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(numerical, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(numerical, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(numerical, abs) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, abs(self->data.value));
    RETURN_OBJECT(obj);
}


/**
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(numerical, neg) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, 0 - self->data.value);
    RETURN_OBJECT(obj);
}


SAFFIRE_METHOD(numerical, conv_boolean) {
    if (self->data.value == 0) RETURN_FALSE;
    RETURN_TRUE;
}

SAFFIRE_METHOD(numerical, conv_null) {
    RETURN_NULL;
}

SAFFIRE_METHOD(numerical, conv_numerical) {
    RETURN_SELF;
}

SAFFIRE_METHOD(numerical, conv_string) {
    char tmp[32];       // @TODO: Should be enough??
    snprintf(tmp, 31, "%ld", self->data.value);
    RETURN_STRING_FROM_CHAR(tmp);
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(numerical, add) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, self->data.value + other->data.value);
}

SAFFIRE_OPERATOR_METHOD(numerical, sub) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, self->data.value - other->data.value);
}

SAFFIRE_OPERATOR_METHOD(numerical, mul) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, self->data.value * other->data.value);
}

SAFFIRE_OPERATOR_METHOD(numerical, div) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    if (other->data.value == 0) {
        object_raise_exception(Object_DivideByZeroException, 1, "Cannot divide by zero");
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, self->data.value / other->data.value);
}

SAFFIRE_OPERATOR_METHOD(numerical, mod) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, self->data.value % other->data.value);
}

SAFFIRE_OPERATOR_METHOD(numerical, and) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, (self->data.value & other->data.value));
}

SAFFIRE_OPERATOR_METHOD(numerical, or) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, (self->data.value | other->data.value));
}

SAFFIRE_OPERATOR_METHOD(numerical, xor) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, (self->data.value ^ other->data.value));
}

SAFFIRE_OPERATOR_METHOD(numerical, sl) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, (self->data.value << other->data.value));
}

SAFFIRE_OPERATOR_METHOD(numerical, sr) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    return object_alloc_instance(Object_Numerical, 1, (self->data.value >> other->data.value));
}

SAFFIRE_OPERATOR_METHOD(numerical, inv) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, ~self->data.value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, not) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, !self->data.value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, neg) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, -self->data.value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, pos) {
    t_object *obj = object_alloc_instance(Object_Numerical, 1, +self->data.value);
    RETURN_OBJECT(obj);
}


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(numerical, eq) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value == other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(numerical, ne) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value != other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(numerical, lt) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value < other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(numerical, gt) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value > other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(numerical, le) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value <= other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(numerical, ge) {
    t_numerical_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n",  &other)) {
        return NULL;
    }

    (self->data.value >= other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}



/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes numerical methods and properties
 */
void object_numerical_init(void) {
    Object_Numerical_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__ctor",        ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_ctor);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__dtor",        ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_dtor);

    object_add_internal_method((t_object *)&Object_Numerical_struct, "__boolean",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__null",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_conv_null);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__numerical",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__string",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_conv_string);

    object_add_internal_method((t_object *)&Object_Numerical_struct, "neg",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_neg);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "abs",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_abs);

    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_add",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_add);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_sub",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_sub);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_mul",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_mul);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_div",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_div);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_mod",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_mod);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_and",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_and);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_or",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_or);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_xor",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_xor);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_sl",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_sl);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_sr",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_sr);

    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_inv",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_inv);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_not",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_not);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_pos",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_pos);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__opr_neg",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_opr_neg);


    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_eq",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_ne",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_ne);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_lt",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_lt);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_gt",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_gt);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_le",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_le);
    object_add_internal_method((t_object *)&Object_Numerical_struct, "__cmp_ge",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_numerical_method_cmp_ge);

    vm_populate_builtins("numerical", (t_object *)&Object_Numerical_struct);


    // Create a numerical cache of numerical instances
    numerical_cache = (t_numerical_object **)smm_malloc(sizeof(t_numerical_object *) * (NUMERICAL_CACHED_CNT + 1));

    // Disable the numerical cache, as we are creating numericals for the cache
    void *temp_cache_func = Object_Numerical_struct.funcs->cache;
    Object_Numerical_struct.funcs->cache = NULL;


    long value = NUMERICAL_CACHED_MIN;
    for (int i=0; i!=NUMERICAL_CACHED_CNT; i++, value++) {
        numerical_cache[i] = (t_numerical_object *)object_alloc_instance(Object_Numerical, 1, value);
        object_inc_ref((t_object *)numerical_cache[i]);
    }

    // Restore cache function for numericals, as we can use it now.
    Object_Numerical_struct.funcs->cache = temp_cache_func;
}


/**
 * Frees memory for a numerical object
 */
void object_numerical_fini(void) {
    // Free numerical cache
    for (int i=0; i!=NUMERICAL_CACHED_CNT; i++) {
        object_release((t_object *)numerical_cache[i]);
    }
    smm_free(numerical_cache);

    // Free attributes
    object_free_internal_object((t_object *)&Object_Numerical_struct);
}



static t_object *obj_cache(t_object *obj, t_dll *arg_list) {
    t_dll_element *e = DLL_HEAD(arg_list);
    long value = DLL_DATA_LONG(e);

    // Return cached object if it's already present.
    if (value >= NUMERICAL_CACHED_MIN && value <= NUMERICAL_CACHED_MAX) {
        return (t_object *)numerical_cache[value + NUMERICAL_CACHE_OFF];
    }

    return NULL;
}

static char *obj_hash(t_object *obj) {
    char *s = (char *)smm_malloc(32);
    snprintf(s, 31, "%ld", ((t_numerical_object *)obj)->data.value);
    return s;
}



static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_numerical_object *num_obj = (t_numerical_object *)obj;

    t_dll_element *e = DLL_HEAD(arg_list);
    signed long value = (signed long)e->data.l;

    num_obj->data.value = value;
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_numerical_object *num_obj = (t_numerical_object *)obj;

    snprintf(num_obj->__debug_info, DEBUG_INFO_SIZE-1, "num:%ld", num_obj->data.value);
    return num_obj->__debug_info;
}
#endif


// String object management functions
t_object_funcs numerical_funcs = {
        obj_populate,       // Populate a numerical object
        NULL,               // Free a numerical object
        obj_destroy,        // Destroy a numerical object
        NULL,               // Clone
        obj_cache,          // cache
        obj_hash,           // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};


// Intial object
t_numerical_object Object_Numerical_struct = {
    OBJECT_HEAD_INIT("numerical", objectTypeNumerical, OBJECT_TYPE_CLASS, &numerical_funcs, sizeof(t_numerical_object_data)),
    {
        0   // value
    },
    OBJECT_FOOTER
};
