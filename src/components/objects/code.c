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
#include "objects/object.h"
#include "objects/boolean.h"
#include "objects/string.h"
#include "objects/null.h"
#include "objects/base.h"
#include "objects/code.h"
#include "objects/method.h"
#include "objects/numerical.h"
#include "general/smm.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */


/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(code, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(code, dtor) {
    RETURN_NULL;
}


/**
 *
 */
SAFFIRE_METHOD(code, call) {
    // @TODO: Will do a call to the code object
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(code, internal) {
    if (self->native_func) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/**
 *
 */
SAFFIRE_METHOD(code, conv_boolean) {
    if (self->bytecode || self->native_func) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/**
 *
 */
SAFFIRE_METHOD(code, conv_null) {
    RETURN_NULL;
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes methods and properties, these are used
 */
void object_code_init(void) {
    Object_Code_struct.methods = ht_create();
    object_add_internal_method(&Object_Code_struct, "ctor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_ctor);
    object_add_internal_method(&Object_Code_struct, "dtor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_dtor);

    object_add_internal_method(&Object_Code_struct, "boolean", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_conv_boolean);
    object_add_internal_method(&Object_Code_struct, "null", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_conv_null);

    object_add_internal_method(&Object_Code_struct, "call", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_call);
    object_add_internal_method(&Object_Code_struct, "internal?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_code_method_internal);

    Object_Code_struct.properties = ht_create();
}

/**
 * Frees memory for a code object
 */
void object_code_fini(void) {
    // Free methods
    object_remove_all_internal_methods((t_object *)&Object_Code_struct);
    ht_destroy(Object_Code_struct.methods);

    // Free properties
    ht_destroy(Object_Code_struct.properties);
}


static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_code_object *obj = smm_malloc(sizeof(t_code_object));
    memcpy(obj, Object_Code, sizeof(t_code_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, va_list arg_list) {
    t_code_object *code_obj = (t_code_object *)obj;

    code_obj->bytecode = va_arg(arg_list, t_bytecode *);
    code_obj->native_func = va_arg(arg_list, void *);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_code_object *self = (t_code_object *)obj;
    sprintf(global_buf, "code object. Internal: %s", self->native_func ? "yes" : "no");
    return global_buf;
}
#endif


// Code object management functions
t_object_funcs code_funcs = {
        obj_new,              // Allocate a new code object
        obj_populate,         // Populates a code object
        NULL,                 // Free a code object
        obj_destroy,          // Destroy a code object
        NULL,               // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

// Intial object
t_code_object Object_Code_struct = {
    OBJECT_HEAD_INIT2("code", objectTypeCode, NULL, NULL, OBJECT_TYPE_CLASS, &code_funcs),
    NULL,
    NULL
};