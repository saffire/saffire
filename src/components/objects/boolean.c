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
#include <string.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>



/* ======================================================================
 *   Object methods
 * ======================================================================
 */


SAFFIRE_METHOD(boolean, conv_boolean) {
    RETURN_SELF;
}

SAFFIRE_METHOD(boolean, conv_null) {
    RETURN_NULL;
}

SAFFIRE_METHOD(boolean, conv_numerical) {
    RETURN_NUMERICAL(self->data.value);
}

SAFFIRE_METHOD(boolean, conv_string) {
    if (self->data.value == 1) {
        RETURN_STRING_FROM_CHAR("true");
    } else {
        RETURN_STRING_FROM_CHAR("false");
    }
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(boolean, add) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value + other->data.value >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, sub) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value - other->data.value >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, mul) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value * other->data.value >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, div) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    if (! other->data.value) RETURN_FALSE;

    (self->data.value / other->data.value >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, mod) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value % other->data.value >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, and) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    ((self->data.value & other->data.value) >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, or) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    ((self->data.value | other->data.value) >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, xor) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    ((self->data.value ^ other->data.value) >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, sl) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    ((self->data.value << other->data.value) >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_OPERATOR_METHOD(boolean, sr) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    ((self->data.value >> other->data.value) >= 1) ? (RETURN_TRUE) : (RETURN_FALSE);
}


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(boolean, eq) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value == other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(boolean, ne) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value != other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(boolean, lt) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value < other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(boolean, gt) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value > other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(boolean, le) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value <= other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(boolean, ge) {
    t_boolean_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "b",  &other)) {
        return NULL;
    }

    (self->data.value >= other->data.value) ? (RETURN_TRUE) : (RETURN_FALSE);
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes string methods and properties, these are used
 */
void object_boolean_init(void) {
    Object_Boolean_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&Object_Boolean_struct, "__boolean",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__null",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_conv_null);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__numerical", ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__string",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_conv_string);

    /*
        Boolean operator table:
        add:    F+F=F  F+T=T  T+F=T  T+T=T
        sub:    F-F=F  F-T=F  T-F=T  T-T=F
        mul:    F*F=F  F*T=F  T*F=F  T*T=T
        div:    F/F=F  F/T=F  T/F=F  T/T=T
        mod:    F%F=F  F%T=F  T%F=F  T%T=T
        and:    F&F=T  F&F=T  T&F=F  T&T=T
        or:     F|F=F  F|T=T  T|F=T  T|T=T
        xor:    F^F=F  F^T=T  T^F=T  T^T=F
        shl:    F<F=F  F<T=F  T<F=T  T<T=F
        shr:    F>F=F  F>T=T  T>F=T  T>T=T
    */
    // @TODO: ATTRIB_VISIBILITY_PROTECTED???
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_add",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_add);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_sub",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_sub);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_mul",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_mul);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_div",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_div);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_mod",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_mod);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_and",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_and);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_or",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_or);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_xor",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_xor);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_sl",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_sl);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__opr_sr",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_opr_sr);


    /*
        Boolean comparison table:
        eq:     T==T, F==F
        ne:     F!=T  T!=F
        lt:     F < T
        gt:     T > F
        le:     F<=T  F<=F T<=T
        ge:     F>=F  T>=F
    */
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_eq",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_ne",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_ne);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_lt",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_lt);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_gt",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_gt);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_le",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_le);
    object_add_internal_method((t_object *)&Object_Boolean_struct, "__cmp_ge",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_boolean_method_cmp_ge);

    Object_Boolean_False_struct.attributes = Object_Boolean_struct.attributes;
    Object_Boolean_True_struct.attributes = Object_Boolean_struct.attributes;

    vm_populate_builtins("false", Object_False);
    vm_populate_builtins("true", Object_True);
}

/**
 * Frees memory for a string object
 */
void object_boolean_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Boolean_struct);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    if (((t_boolean_object *)obj)->data.value == 0) return "boolean(False)";
    return "boolean(True)";
}
#endif

t_object_funcs bool_funcs = {
        NULL,               // Populate a bool object
        NULL,               // Free a bool object
        NULL,               // Destroy a bool object
        NULL,               // Clone
        NULL,               // Cache
        NULL,               // Hash
#ifdef __DEBUG
        obj_debug,
#endif
};

t_boolean_object Object_Boolean_struct       = { OBJECT_HEAD_INIT("boolean", objectTypeBoolean, OBJECT_TYPE_CLASS | OBJECT_FLAG_FINAL, &bool_funcs, sizeof(t_boolean_object_data)), { 0 } };
t_boolean_object Object_Boolean_False_struct = { OBJECT_HEAD_INIT("boolean", objectTypeBoolean, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_IMMUTABLE, &bool_funcs, sizeof(t_boolean_object_data)), { 0 } };
t_boolean_object Object_Boolean_True_struct  = { OBJECT_HEAD_INIT("boolean", objectTypeBoolean, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_IMMUTABLE, &bool_funcs, sizeof(t_boolean_object_data)), { 1 } };
