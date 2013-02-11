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
#include "objects/objects.h"
#include "general/smm.h"
#include "general/output.h"

/* ======================================================================
 *   Object methods
 * ======================================================================
 */

SAFFIRE_METHOD(exception, conv_boolean) {
    RETURN_SELF;
}

SAFFIRE_METHOD(exception, conv_null) {
    RETURN_NULL;
}

SAFFIRE_METHOD(exception, conv_numerical) {
    RETURN_NUMERICAL(self->code);
}

SAFFIRE_METHOD(exception, conv_string) {
    RETURN_STRING(self->message);
}

SAFFIRE_METHOD(exception, getmessage) {
    RETURN_STRING(self->message);
}

SAFFIRE_METHOD(exception, setmessage) {
    t_string_object *message;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &message)) {
        error_and_die(1, "Error while parsing argument list\n");
    }

    self->message = message->value;
    RETURN_SELF;
}

SAFFIRE_METHOD(exception, getcode) {
    RETURN_NUMERICAL(self->code);
}

SAFFIRE_METHOD(exception, setcode) {
    t_numerical_object *code;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &code)) {
        error_and_die(1, "Error while parsing argument list\n");
    }

    self->code = code->value;
    RETURN_SELF;
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */

/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(exception, eq) {
    return (_self == _other);
}
SAFFIRE_COMPARISON_METHOD(exception, ne) {
    return (_self != _other);
}

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes string methods and properties, these are used
 */
void object_exception_init(void) {
    Object_Exception_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&Object_Exception_struct, "boolean",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Exception_struct, "null",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_null);
    object_add_internal_method((t_object *)&Object_Exception_struct, "numerical", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Exception_struct, "string",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_string);

    object_add_internal_method((t_object *)&Object_Exception_struct, "getMessage", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setMessage", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "getCode", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getcode);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setCode", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setcode);

    Object_Exception_AttributeException_struct.attributes = Object_Exception_struct.attributes;
    Object_Exception_IndexException_struct.attributes = Object_Exception_struct.attributes;

    vm_populate_builtins("attributeException", Object_AttributeException);
    vm_populate_builtins("indexException", Object_IndexException);
    vm_populate_builtins("exception", Object_Exception);
}

/**
 * Frees memory for a string object
 */
void object_exception_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Exception_struct);
    ht_destroy(Object_Exception_struct.attributes);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_exception_object *exception = (t_exception_object *)obj;
    snprintf(global_buf, 1023, "%s(%ld)[%s]", exception->name, exception->code, exception->message);
    return global_buf;
}
#endif

t_object_funcs exception_funcs = {
        NULL,               // Allocate a new exception object
        NULL,               // Populate a exception object
        NULL,               // Free a exception object
        NULL,               // Destroy a exception object
        NULL,               // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

t_object_comparisons exception_cmps = {
    object_exception_comparison_eq,
    object_exception_comparison_ne,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

t_exception_object Object_Exception_struct       = { OBJECT_HEAD_INIT2("exception", objectTypeException, NULL, &exception_cmps, OBJECT_TYPE_CLASS, &exception_funcs), "Generic exception" };

t_exception_object Object_Exception_AttributeException_struct = { OBJECT_HEAD_INIT3("attributeException", objectTypeException, NULL, &exception_cmps, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_struct), "Attribute is not found"};
t_exception_object Object_Exception_IndexException_struct  = { OBJECT_HEAD_INIT3("indexException", objectTypeException, NULL, &exception_cmps, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_struct), "Index out of range" };


// Define