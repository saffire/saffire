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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"
#include "general/dll.h"


extern t_dll *dupped_attributes;

/**
 * Additional values:
 *      attribute.total_access          // How many times did we access the ->attribute value in total?
 *      attribute.access                // How many times did we access the ->attribute value (since last change?)
 *      attribute.changed               // How many times did the ->attribute value for this attribute change?
 *      attribute.type                  // method, property, constant
 *      attribute.visibility            // Public, protected, private
 *      attribute.access                // readonly, readwrite
 *      attribute.attribute             // Actual attribute
 */

/* ======================================================================
 *   Global functions
 * ======================================================================
 */

/**
 * Duplicate the attribute object and link it to the bound-obj
 */
t_attrib_object *object_attrib_duplicate(t_attrib_object *attrib, t_object *self) {
    DEBUG_PRINT("duplicating attrib '%s.%s' to '%s.%s'", attrib->bound_class->name, attrib->bound_name, self->name, attrib->bound_name);
    t_attrib_object *dup = smm_malloc(sizeof(Object_Attrib_struct));
    memcpy(dup, attrib, sizeof(Object_Attrib_struct));

    dup->ref_count = 0;     // no references yet

    // @TODO: So we keep a list of max 100 duplicated attributes. But we don't use it for caching, but just to make sure that our
    // attribute doesn't get eaten by the GC. Fix this into something a bit better...

    // Append to duplicated attributes
    object_inc_ref((t_object *)dup);    // increase reference count, so it's protected in this dll
    dll_append(dupped_attributes, dup);
    if (dupped_attributes->size > 100) {
        t_object *old_dup = (t_object *)(DLL_HEAD(dupped_attributes))->data;
        object_release(old_dup);
    }

    object_inc_ref(self);
    dup->bound_instance = self;

    return dup;
}

/**
 * find attribute inside a object. return either NULL or the actual attribute
 */
t_attrib_object *object_attrib_find(t_object *self, char *name) {
    t_attrib_object *attr = NULL;
    t_object *cur_obj = self;

    while (attr == NULL) {
        DEBUG_PRINT(">>> Finding attribute '%s' on object %s\n", name, cur_obj->name);

        // Find the attribute in the current object
        attr = ht_find_str(cur_obj->attributes, name);
        if (attr != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            DEBUG_PRINT(">>> Cannot find attribute '%s' in object %s:\n", name, self->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    // DEBUG_PRINT(">>> Found attribute '%s' in object %s (actually found in object %s)\n", attr_name, obj->name, cur_obj->name);

    return attr;
}

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

    // Dynamically allocated
    obj->flags |= OBJECT_FLAG_ALLOCATED;

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_attrib_object *attrib_obj = (t_attrib_object *)obj;

    t_dll_element *e = DLL_HEAD(arg_list);
    attrib_obj->bound_class = (t_object *)e->data;
    object_inc_ref(attrib_obj->bound_class);

    e = DLL_NEXT(e);
    attrib_obj->bound_name = smm_strdup((char *)e->data);

    e = DLL_NEXT(e);
    attrib_obj->attr_type = (long)e->data;

    e = DLL_NEXT(e);
    attrib_obj->attr_visibility = (long)e->data;

    e = DLL_NEXT(e);
    attrib_obj->attr_access = (long)e->data;

    e = DLL_NEXT(e);
    attrib_obj->attribute = (t_object *)e->data;

    e = DLL_NEXT(e);
    attrib_obj->attr_method_flags  = (long)e->data;

    // We "own" this attribute object. increase refcount
    object_inc_ref(attrib_obj->attribute);
}

static void obj_free(t_object *obj) {
    t_attrib_object *attr_obj = (t_attrib_object *)obj;

    //DEBUG_PRINT("Freeing attrib-object's attribute: %s\n", object_debug(attr_obj->attribute));

    // "free" the attribute object. decrease refcount
    object_release(attr_obj->attribute);

    if (attr_obj->bound_name) {
        smm_free(attr_obj->bound_name);
    }
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

    snprintf(global_buf, 1024, "%s [%s%s%s] Attached: %s", self->name,
        self->attr_type == 0 ? "M" : self->attr_type == 1 ? "C" : self->attr_type == 2 ? "P" : "?",
        self->attr_visibility == 0 ? "P" : self->attr_visibility == 1 ? "R" : self->attr_visibility == 2 ? "V" : "?",
        self->attr_access == 0 ? "W" : "R",
        attrbuf);

    return global_buf;
}
#endif


// Attrib object management functions
t_object_funcs attrib_funcs = {
        obj_new,              // Allocate a new attrib object
        obj_populate,         // Populate an attrib object
        obj_free,             // Free an attrib object
        obj_destroy,          // Destroy an attrib object
        NULL,                 // Clone
        NULL,                 // Cache
#ifdef __DEBUG
        obj_debug,
#endif
};

// Intial object
t_attrib_object Object_Attrib_struct = {
    OBJECT_HEAD_INIT("attrib", objectTypeAttribute, OBJECT_TYPE_CLASS, &attrib_funcs),
    0,
    0,
    0,
    0,
    NULL,
    NULL,
    NULL
};
