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

#include "object/object.h"
#include "object/boolean.h"
#include "object/null.h"
#include "object/numerical.h"
#include "object/string.h"
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
        RETURN_STRING(L"true");
    } else {
        RETURN_STRING(L"false");
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
    return 0;
}
SAFFIRE_COMPARISON_METHOD(boolean, ne) {
    return 0;
}
SAFFIRE_COMPARISON_METHOD(boolean, lt) {
    return 0;
}
SAFFIRE_COMPARISON_METHOD(boolean, gt) {
    return 0;
}
SAFFIRE_COMPARISON_METHOD(boolean, le) {
    return 0;
}
SAFFIRE_COMPARISON_METHOD(boolean, ge) {
    return 0;
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

    ht_add(Object_Boolean_struct.methods, "boolean", object_boolean_method_conv_boolean);
    ht_add(Object_Boolean_struct.methods, "null", object_boolean_method_conv_null);
    ht_add(Object_Boolean_struct.methods, "numerical", object_boolean_method_conv_numerical);
    ht_add(Object_Boolean_struct.methods, "string", object_boolean_method_conv_string);

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
    ht_destroy(Object_Boolean_struct.methods);
    ht_destroy(Object_Boolean_struct.properties);
}


static char *obj_debug(struct _object *obj) {
    if (((t_boolean_object *)obj)->value == 0) return "false";
    return "true";
}

t_object_funcs bool_funcs = {
        NULL,               // Allocate a new bool object
        NULL,               // Free a bool object
        NULL,               // Clone a bool object
        obj_debug
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

t_boolean_object Object_Boolean_struct       = { OBJECT_HEAD_INIT2("bool", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_NO_FLAGS, &bool_funcs), 0 };
t_boolean_object Object_Boolean_False_struct = { OBJECT_HEAD_INIT2("bool", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &bool_funcs), 0 };
t_boolean_object Object_Boolean_True_struct  = { OBJECT_HEAD_INIT2("bool", objectTypeBoolean, &boolean_ops, &boolean_cmps, OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &bool_funcs), 1 };
