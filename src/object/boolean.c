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


t_boolean_object Object_Boolean_struct       = { OBJECT_HEAD_INIT("bool", objectTypeBoolean, 0), 0 };
t_boolean_object Object_Boolean_False_struct = { OBJECT_HEAD_INIT("bool", objectTypeBoolean, OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE), 0 };
t_boolean_object Object_Boolean_True_struct  = { OBJECT_HEAD_INIT("bool", objectTypeBoolean, OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE), 1 };
