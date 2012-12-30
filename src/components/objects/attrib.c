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
#include "objects/attrib.h"
#include "objects/boolean.h"
#include "objects/string.h"
#include "objects/null.h"
#include "objects/base.h"
#include "objects/numerical.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */


/* ======================================================================
 *   Object attribs
 * ======================================================================
 */

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes attribs and properties, these are used
 */
void object_attrib_init(void) {
    Object_Attrib_struct.attributes = ht_create();
}

/**
 * Frees memory for an attrib object
 */
void object_attrib_fini(void) {
    // Free attributes
    ht_destroy(Object_Attrib_struct.attributes);
}



static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_attrib_object *obj = smm_malloc(sizeof(t_attrib_object));
    memcpy(obj, Object_Attrib, sizeof(t_attrib_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, va_list arg_list) {
    t_attrib_object *attrib_obj = (t_attrib_object *)obj;

    attrib_obj->attrib_type = (char)va_arg(arg_list, int);
    attrib_obj->visibility = (char)va_arg(arg_list, int);
    attrib_obj->access = (char)va_arg(arg_list, int);
    attrib_obj->attribute = va_arg(arg_list, struct _object *);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_attrib_object *self = (t_attrib_object *)obj;
    char attrbuf[1024];
    snprintf(attrbuf, 1024, "%s", object_debug(self->attribute));
    snprintf(global_buf, 1024, "Object: %s (T/V/A)(%d/%d/%d) Value: %s", self->name, self->type, self->visibility, self->access, attrbuf);
    return global_buf;
}
#endif


// Attrib object management functions
t_object_funcs attrib_funcs = {
        obj_new,              // Allocate a new attrib object
        obj_populate,         // Populate a attrib object
        NULL,                 // Free a attrib object
        obj_destroy,          // Destroy a attrib object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};

// Intial object
t_attrib_object Object_Attrib_struct = {
    OBJECT_HEAD_INIT2("attrib", objectTypeAttribute, NULL, NULL, OBJECT_TYPE_CLASS, &attrib_funcs),
    0,
    0,
    0,

    NULL
};