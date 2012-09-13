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
#include "object/base.h"
#include "object/numerical.h"
#include "object/null.h"
#include "general/smm.h"
#include <stdio.h>
#include <string.h>





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
    t_numerical_object *obj = (t_numerical_object *)object_clone((t_object *)self);
    obj->value = abs(self->value);

    RETURN_OBJECT(obj);
}


/**
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(numerical, neg) {
    t_numerical_object *obj = (t_numerical_object *)object_clone((t_object *)self);
    obj->value = 0 - self->value;

    RETURN_OBJECT(obj);
}


/**
 * Saffire method: output numerical value
 */
SAFFIRE_METHOD(numerical, print) {
    printf("%ld\n", self->value);
    RETURN_SELF;
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes numerical methods and properties
 */
void object_numerical_init(void) {
    Object_Numerical_struct.methods = ht_create();
    ht_add(Object_Numerical_struct.methods, "ctor", object_numerical_method_ctor);
    ht_add(Object_Numerical_struct.methods, "dtor", object_numerical_method_dtor);

    ht_add(Object_Numerical_struct.methods, "neg", object_numerical_method_neg);
    ht_add(Object_Numerical_struct.methods, "abs", object_numerical_method_abs);
    ht_add(Object_Numerical_struct.methods, "print", object_numerical_method_print);

    Object_Numerical_struct.properties = ht_create();
}


/**
 * Frees memory for a numerical object
 */
void object_numerical_fini(void) {
    ht_destroy(Object_Numerical_struct.methods);
    ht_destroy(Object_Numerical_struct.properties);
}


/**
 * Clones a numerical object into a new object
 */
static t_object *obj_clone(t_numerical_object *obj) {
    t_numerical_object *num_obj = (t_numerical_object *)obj;

    // Create new object and copy all info
    t_numerical_object *new_obj = smm_malloc(sizeof(t_numerical_object));
    memcpy(new_obj, num_obj, sizeof(t_numerical_object));

    // New separated object, so refcount = 1
    new_obj->ref_count = 1;

    return (t_object *)new_obj;
}


/**
 * Creates a new numerical object by "cloning" the original one
 */
static t_object *obj_new(va_list *arg_list) {
    t_numerical_object *new_obj = smm_malloc(sizeof(t_numerical_object));
    memcpy(new_obj, Object_Numerical, sizeof(t_numerical_object));

    long value = va_arg(arg_list, long);
    new_obj->value = value;

    return (t_object *)new_obj;
}



// String object management functions
t_object_funcs numerical_funcs = {
        obj_new,            // Allocate a new numerical object
        NULL,               // Free a numerical object
        obj_clone           // Clone a numerical object
};

// Intial object
t_numerical_object Object_Numerical_struct = {
    OBJECT_HEAD_INIT2("numerical", &numerical_funcs),
    0
};