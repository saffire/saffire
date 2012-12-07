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

#include "objects/object.h"
#include "objects/boolean.h"
#include "objects/null.h"
#include "objects/numerical.h"
#include "objects/string.h"
#include "objects/method.h"
#include "general/smm.h"



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
    RETURN_NUMERICAL(self->value);
}

SAFFIRE_METHOD(boolean, conv_string) {
    if (self->value == 1) {
        RETURN_STRING("true");
    } else {
        RETURN_STRING("false");
    }
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(boolean, add) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value += self->value;
        if (self->value > 1) self->value = 1;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, sub) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value -= self->value;
        if (self->value < 0) self->value = 0;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, mul) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value *= self->value;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, div) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value /= self->value;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, mod) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value %= 1;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, and) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value &= self->value;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, or) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value |= 1;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, xor) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value ^= self->value;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, sl) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value <<= self->value;
        if (self->value > 1) self->value = 1;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(boolean, sr) {
    t_boolean_object *self = (t_boolean_object *)_self;

    if (in_place) {
        self->value >>= self->value;
        if (self->value < 0) self->value = 0;
        RETURN_SELF;
    }

    t_boolean_object *obj = (t_boolean_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(boolean, eq) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value == other->value);
}
SAFFIRE_COMPARISON_METHOD(boolean, ne) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value != other->value);
}
SAFFIRE_COMPARISON_METHOD(boolean, lt) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value < other->value);
}
SAFFIRE_COMPARISON_METHOD(boolean, gt) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value > other->value);
}
SAFFIRE_COMPARISON_METHOD(boolean, le) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value <= other->value);
}
SAFFIRE_COMPARISON_METHOD(boolean, ge) {
    t_boolean_object *self = (t_boolean_object *)_self;
    t_boolean_object *other = (t_boolean_object *)_other;

    return (self->value >= other->value);
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes string methods and properties, these are used
 */
void object_boolean_init(void) {
    Object_Boolean_struct.methods = ht_create();

    object_add_internal_method(&Object_Boolean_struct, "boolean", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_boolean_method_conv_boolean);
    object_add_internal_method(&Object_Boolean_struct, "null", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_boolean_method_conv_null);
    object_add_internal_method(&Object_Boolean_struct, "numerical", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_boolean_method_conv_numerical);
    object_add_internal_method(&Object_Boolean_struct, "string", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_boolean_method_conv_string);

    Object_Boolean_struct.properties = ht_create();

    Object_Boolean_False_struct.methods = Object_Boolean_struct.methods;
    Object_Boolean_True_struct.methods = Object_Boolean_struct.methods;

    Object_Boolean_False_struct.properties = Object_Boolean_struct.properties;
    Object_Boolean_True_struct.properties = Object_Boolean_struct.properties;
}

/**
 * Frees memory for a string object
 */
void object_boolean_fini(void) {
    // Free methods
    object_remove_all_internal_methods((t_object *)&Object_Boolean_struct);
    ht_destroy(Object_Boolean_struct.methods);

    // Free properties
    ht_destroy(Object_Boolean_struct.properties);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    if (((t_boolean_object *)obj)->value == 0) return "false";
    return "true";
}
#endif

t_object_funcs bool_funcs = {
        NULL,               // Allocate a new bool object
        NULL,               // Populate a bool object
        NULL,               // Free a bool object
        NULL,               // Destroy a bool object
        NULL,               // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

t_object_operators boolean_ops = {
    object_boolean_operator_add,          // F+F=F  F+T=T  T+F=T  T+T=T
    object_boolean_operator_sub,          // F-F=F  F-T=F  T-F=T  T-T=F
    object_boolean_operator_mul,          // F*F=F  F*T=F  T*F=F  T*T=T
    object_boolean_operator_div,          // F/F=F  F/T=F  T/F=F  T/T=T
    object_boolean_operator_mod,          // F%F=F  F%T=F  T%F=F  T%T=T
    object_boolean_operator_and,          // F&F=T  F&F=T  T&F=F  T&T=T
    object_boolean_operator_or,           // F|F=F  F|T=T  T|F=T  T|T=T
    object_boolean_operator_xor,          // F^F=F  F^T=T  T^F=T  T^T=F
    object_boolean_operator_sl,           // F<F=F  F<T=F  T<F=T  T<T=F
    object_boolean_operator_sr            // F>F=F  F>T=T  T>F=T  T>T=T
};

t_object_comparisons boolean_cmps = {
    object_boolean_comparison_eq,       // T==T, F==F
    object_boolean_comparison_ne,       // F!=T  T!=F
    object_boolean_comparison_lt,       // F < T
    object_boolean_comparison_gt,       // T > F
    object_boolean_comparison_le,       // F<=T  F<=F T<=T
    object_boolean_comparison_ge,       // F>=F  T>=F
    NULL,
    NULL
};

t_boolean_object Object_Boolean_struct       = { OBJECT_HEAD_INIT2("boolean", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_TYPE_CLASS, &bool_funcs), 0 };
t_boolean_object Object_Boolean_False_struct = { OBJECT_HEAD_INIT2("boolean", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &bool_funcs), 0 };
t_boolean_object Object_Boolean_True_struct  = { OBJECT_HEAD_INIT2("boolean", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &bool_funcs), 1 };
