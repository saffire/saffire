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
#include "general/string.h"
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/output.h"
#include "vm/thread.h"

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

    self->data.message = string_strdup(msg_obj->data.value);
    if (code_obj) {
        self->data.code = code_obj->data.value;
    }

    RETURN_SELF;
}

SAFFIRE_METHOD(exception, conv_boolean) {
    RETURN_SELF;
}

SAFFIRE_METHOD(exception, conv_null) {
    RETURN_NULL;
}

SAFFIRE_METHOD(exception, conv_numerical) {
    RETURN_NUMERICAL(self->data.code);
}

SAFFIRE_METHOD(exception, conv_string) {
    RETURN_STRING(self->data.message);
}

SAFFIRE_METHOD(exception, getmessage) {
    RETURN_STRING(self->data.message);
}

SAFFIRE_METHOD(exception, setmessage) {
    t_string_object *message;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &message)) {
        object_raise_exception(Object_ArgumentException, 1, "error while parsing argument list");
        return NULL;
    }

    self->data.message = message->data.value;
    RETURN_SELF;
}

SAFFIRE_METHOD(exception, getcode) {
    RETURN_NUMERICAL(self->data.code);
}

SAFFIRE_METHOD(exception, setcode) {
    t_numerical_object *code;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &code)) {
        return NULL;
    }

    self->data.code = code->data.value;
    RETURN_SELF;
}

SAFFIRE_METHOD(exception, getline) {
    t_vm_stackframe *stackframe = thread_get_exception_frame();

    RETURN_NUMERICAL(stackframe->lineno_current_line);
}

SAFFIRE_METHOD(exception, getfile) {
    t_vm_stackframe *stackframe = thread_get_exception_frame();

    RETURN_STRING_FROM_CHAR(stackframe->codeblock->context->file.full);
}

SAFFIRE_METHOD(exception, getclass) {
    t_vm_stackframe *stackframe = thread_get_exception_frame();

    RETURN_STRING_FROM_CHAR(stackframe->codeblock->context->class.full);
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

    object_add_internal_method((t_object *)&Object_Exception_struct, "__ctor",   ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_ctor);

    object_add_internal_method((t_object *)&Object_Exception_struct, "__boolean",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__null",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_null);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__numerical", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__string",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_conv_string);

    object_add_internal_method((t_object *)&Object_Exception_struct, "getMessage", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setMessage", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setmessage);
    object_add_internal_method((t_object *)&Object_Exception_struct, "getCode",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getcode);
    object_add_internal_method((t_object *)&Object_Exception_struct, "setCode",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_setcode);

    object_add_internal_method((t_object *)&Object_Exception_struct, "getFile",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getfile);
    object_add_internal_method((t_object *)&Object_Exception_struct, "getClass",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getclass);
    object_add_internal_method((t_object *)&Object_Exception_struct, "getLine",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_getline);

    object_add_internal_method((t_object *)&Object_Exception_struct, "__cmp_eq", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_cmp_eq);
    object_add_internal_method((t_object *)&Object_Exception_struct, "__cmp_ne", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_exception_method_cmp_ne);

    vm_populate_builtins("exception", Object_Exception);

    object_exception_add_generated_exceptions();
}

/**
 * Frees memory for a string object
 */
void object_exception_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Exception_struct);
}


static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_exception_object *exception_obj = (t_exception_object *)obj;

    t_dll_element *e = DLL_HEAD(arg_list);
    // Optional (numerical) code
    if (e != NULL) {
        exception_obj->data.code = (int)e->data;
        e = DLL_NEXT(e);
    }

    if (e != NULL) {
        exception_obj->data.message = (t_string *)e->data;
        e = DLL_NEXT(e);
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
    if (OBJECT_TYPE_IS_CLASS(obj)) {
        snprintf(global_buf, 1023, "%s", obj->name);
    } else {
        t_exception_object *exception = (t_exception_object *)obj;
        snprintf(global_buf, 1023, "%s(%ld)[%s]", exception->name, exception->data.code, exception->data.message ? exception->data.message->val : "");
    }
    return global_buf;
}
#endif

t_object_funcs exception_funcs = {
        obj_populate,       // Populate a exception object
        obj_free,           // Free a exception object
        obj_destroy,        // Destroy a exception object
        NULL,               // Clone
        NULL,               // Cache
        NULL,               // Hash
#ifdef __DEBUG
        obj_debug
#endif
};


t_exception_object Object_Exception_struct = {
    OBJECT_HEAD_INIT("exception", objectTypeException, OBJECT_TYPE_CLASS, &exception_funcs, sizeof(t_exception_object_data)),
    {
        NULL, 0
    }
};

// Include generated exceptions
#include "_generated_exceptions.inc"
