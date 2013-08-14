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
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/output.h"

/* ======================================================================
 *   Object methods
 * ======================================================================
 */

SAFFIRE_METHOD(exception, ctor) {
    t_string_object *msg_obj;
    t_numerical_object *code_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s|n",  (t_object *)&msg_obj, (t_object *)&code_obj)) {
        return NULL;
    }

    self->message = smm_strdup(msg_obj->value);
    self->code = code_obj->value;

    RETURN_SELF;
}

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
        object_raise_exception(Object_ArgumentException, 1, "error while parsing argument list");
        return NULL;
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
        return NULL;
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
    t_exception_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  &other)) {
        return NULL;
    }

    (self == other) ? (RETURN_TRUE) : (RETURN_FALSE);
}

SAFFIRE_COMPARISON_METHOD(exception, ne) {
    t_exception_object *other;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  &other)) {
        return NULL;
    }

    (self != other) ? (RETURN_TRUE) : (RETURN_FALSE);
}

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


void object_exception_add_generated_exceptions(void);

/**
 * Initializes string methods and properties, these are used
 */
void object_exception_init(void) {
    Object_Exception_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&Object_Exception_struct, "__ctor",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_ctor);

    object_add_internal_method((t_object *)&Object_Exception_struct, "__boolean",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__null",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_null);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__numerical", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__string",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_string);

    object_add_internal_method((t_object *)&Object_Exception_struct, "getMessage", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setMessage", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "getCode",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getcode);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setCode",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setcode);

    object_add_internal_method((t_object *)&Object_Exception_struct, "__cmp_eq", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__cmp_ne", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_cmp_ne);

    vm_populate_builtins("exception", Object_Exception);
    object_exception_add_generated_exceptions();
}

/**
 * Frees memory for a string object
 */
void object_exception_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Exception_struct);
    ht_destroy(Object_Exception_struct.attributes);
}

/**
 * obj_new is called dynamically. The "self" class points to the actual object
 * we want to "new". This is always based on exception, so we are ok by actually
 * copying self and using the size of the base exception object (so code and
 * message gets copied too)
 *
 * @param self
 * @return
 */
static t_object *obj_new(t_object *self) {
    t_exception_object *obj = smm_malloc(sizeof(t_exception_object));
    memcpy(obj, self, sizeof(t_exception_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_exception_object *exception_obj = (t_exception_object *)obj;


    t_dll_element *e = DLL_HEAD(arg_list);
    // Optional (string) message
    if (e != NULL) {
        exception_obj->code = (int)e->data;
        e = DLL_NEXT(e);
    }

    // Optional (numerical) code
    if (e != NULL) {
        exception_obj->message = smm_strdup((char *)e->data);
    }
}

static void obj_free(t_object *obj) {
    // TODO: We have static and dynamic allocation of message. Make this more generic.
//   t_string_object *str_obj = (t_string_object *)obj;
//
//   if (str_obj->value) {
//       smm_free(str_obj->value);
//   }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
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
        obj_new,            // Allocate a new exception object
        obj_populate,       // Populate a exception object
        obj_free,           // Free a exception object
        obj_destroy,        // Destroy a exception object
        NULL,               // Clone
#ifdef __DEBUG
        obj_debug
#endif
};


t_exception_object Object_Exception_struct = { OBJECT_HEAD_INIT("exception", objectTypeException, OBJECT_TYPE_CLASS, &exception_funcs), "", 0};

// Include generated exceptions
#include "_generated_exceptions.inc"
