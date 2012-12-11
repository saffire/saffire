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
#include "objects/method.h"
#include "objects/code.h"
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
SAFFIRE_METHOD(method, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(method, dtor) {
    RETURN_NULL;
}


/**
 *
 */
//SAFFIRE_METHOD(method, call) {
//    // Will do a call to the code object
//    RETURN_SELF();
//}

/**
 *
 */
SAFFIRE_METHOD(method, flags) {
    RETURN_NUMERICAL(self->mflags);
}

/**
  *
  */
SAFFIRE_METHOD(method, static) {
    if (self->mflags & METHOD_FLAG_STATIC) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

/**
  *
  */
SAFFIRE_METHOD(method, abstract) {
    if (self->mflags & METHOD_FLAG_ABSTRACT) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

/**
  *
  */
SAFFIRE_METHOD(method, final) {
    if (self->mflags & METHOD_FLAG_FINAL) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


/**
 *
 */
SAFFIRE_METHOD(method, visibility) {
    RETURN_NUMERICAL(self->visibility);
}

/**
  *
  */
SAFFIRE_METHOD(method, public) {
    if (self->visibility & METHOD_VISIBILITY_PUBLIC) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

/**
  *
  */
SAFFIRE_METHOD(method, private) {
    if (self->visibility & METHOD_VISIBILITY_PRIVATE) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

/**
  *
  */
SAFFIRE_METHOD(method, protected) {
    if (self->visibility & METHOD_VISIBILITY_PROTECTED) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


/**
  *
  */
SAFFIRE_METHOD(method, class) {
    if (self->class) {
        RETURN_OBJECT(self->class);
    }
    RETURN_NULL;
}

/**
  *
  */
SAFFIRE_METHOD(method, code) {
    if (self->code) {
        RETURN_OBJECT(self->code);
    }

    RETURN_NULL;
}


/**
 *
 */
SAFFIRE_METHOD(method, conv_boolean) {
    if (self->code) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/**
 *
 */
SAFFIRE_METHOD(method, conv_null) {
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
void object_method_init(void) {
    Object_Method_struct.methods = ht_create();
    object_add_internal_method(&Object_Method_struct, "ctor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_ctor);
    object_add_internal_method(&Object_Method_struct, "dtor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_dtor);

    object_add_internal_method(&Object_Method_struct, "boolean", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_conv_boolean);
    object_add_internal_method(&Object_Method_struct, "null", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_conv_null);

    //object_add_internal_method(&Object_Method_struct, "call", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_call);
    object_add_internal_method(&Object_Method_struct, "flags", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_flags);
    object_add_internal_method(&Object_Method_struct, "static?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_static);
    object_add_internal_method(&Object_Method_struct, "abstract?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_abstract);
    object_add_internal_method(&Object_Method_struct, "final?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_final);

    object_add_internal_method(&Object_Method_struct, "visibility", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_visibility);
    object_add_internal_method(&Object_Method_struct, "public?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_public);
    object_add_internal_method(&Object_Method_struct, "private?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_private);
    object_add_internal_method(&Object_Method_struct, "protected?", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_protected);

    object_add_internal_method(&Object_Method_struct, "class", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_class);
    object_add_internal_method(&Object_Method_struct, "code", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_method_method_code);


    Object_Method_struct.properties = ht_create();
}

/**
 * Frees memory for a method object
 */
void object_method_fini(void) {
    // Free methods
    object_remove_all_internal_methods((t_object *)&Object_Method_struct);
    ht_destroy(Object_Method_struct.methods);

    // Free properties
    ht_destroy(Object_Method_struct.properties);
}



static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_method_object *obj = smm_malloc(sizeof(t_method_object));
    memcpy(obj, Object_Method, sizeof(t_method_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, va_list arg_list) {
    t_method_object *method_obj = (t_method_object *)obj;

    method_obj->mflags = va_arg(arg_list, int);
    method_obj->visibility = va_arg(arg_list, int);
    method_obj->class = va_arg(arg_list, t_object *);
    method_obj->code = va_arg(arg_list, struct _code_object *);
    method_obj->arguments = va_arg(arg_list, struct _hash_object *);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_method_object *self = (t_method_object *)obj;
    sprintf(global_buf, "method %s F: %d  V: %d Obj: %s Code: %s", self->name, self->mflags, self->visibility, self->class ? self->class->name : "no", self->code ? "yes" : "no");
    return global_buf;
}
#endif


// Method object management functions
t_object_funcs method_funcs = {
        obj_new,              // Allocate a new method object
        obj_populate,         // Populate a method object
        NULL,                 // Free a method object
        obj_destroy,          // Destroy a method object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

// Intial object
t_method_object Object_Method_struct = {
    OBJECT_HEAD_INIT2("method", objectTypeMethod, NULL, NULL, OBJECT_TYPE_CLASS, &method_funcs),
    0,
    0,
    NULL,
    NULL,
    NULL
};