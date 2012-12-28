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
    Object_Method_struct.attributes = ht_create();
}

/**
 * Frees memory for an method object
 */
void object_method_fini(void) {
    // Free attributes
    ht_destroy(Object_Method_struct.attributes);
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

    method_obj->name = va_arg(arg_list, char *);
    method_obj->code = (struct _code_object *)va_arg(arg_list, struct _object *);
    method_obj->binding = va_arg(arg_list, struct _object *);
    method_obj->method_flags = (char)va_arg(arg_list, int);
    method_obj->arguments = va_arg(arg_list, struct _hash_object *);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_method_object *self = (t_method_object *)obj;
    snprintf(global_buf, 1024, "Object: %s Flags: %d Binding: %s", self->name, self->method_flags, self->binding ? self->binding->name : "not bound");
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
    0,
    NULL
};