/*
 Copyright (c) 2012-2015, The Saffire Group
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
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>
#include <saffire/general/md5.h>
#include <saffire/debug.h>
#include <saffire/general/dll.h>


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
 * (Re)bind an attribute to a certain class/instance under the given name
 */
void object_attrib_bind(t_attrib_object *attrib_obj, t_object *bound_obj, char *name)
{
    // Bind the attribute to the class.
    // We don't increase bound_obj. Otherwise we get cyclic dependency, and we can't remove bound_obj without
    // removing this attribute anyway.
    attrib_obj->data.bound_class = (t_object *)bound_obj;

    // Free name if already present
    if (attrib_obj->data.bound_name) {
        smm_free(attrib_obj->data.bound_name);
    }

    // Set bounded name
    attrib_obj->data.bound_name = string_strdup0(name);
}

/**
 * Duplicate the attribute object and link it to the bound-obj
 */
t_attrib_object *object_attrib_duplicate(t_attrib_object *attrib, t_object *self) {
//    DEBUG_PRINT_CHAR("duplicating attrib %08X into %s\n", (intptr_t)attrib, self->name);

    // Create a simple duplicate object and copy everything over
    t_attrib_object *dup = smm_malloc(sizeof (Object_Attrib_struct));
    memcpy(dup, attrib, sizeof (Object_Attrib_struct));

    // Set refcount to zero, we don't care about the original reference count
    dup->ref_count = 0;

    // As there are now two attributes referencing the same attribute-value, increase the value as well.
    object_inc_ref(dup->data.attribute);

    // Since an attribute object doesn't have attributes to begin with, we just create a (dummy) hashtable. If attributes happen to get
    // attributes later on (i don't see how or why, then this should change as well)..
    dup->attributes = ht_create();

    // Self object is used in this attribute as bound instance
    dup->data.bound_instance = self;
//    object_inc_ref(self);

    // Duplicate class name
    dup->name = string_strdup0(attrib->name);

//    // Increase the original bound class
//    object_inc_ref(dup->data.bound_class);

    // bound name should have its own reference name
    if (attrib->data.bound_name) {
        dup->data.bound_name = string_strdup0(attrib->data.bound_name);
    }

    return dup;
}

/**
 * find attribute inside a object. return either NULL or the actual attribute
 */
t_attrib_object *object_attrib_find(t_object *self, char *name) {
    t_attrib_object *attr = NULL;
    t_object *cur_obj = self;

    if (!self) return NULL;

    while (attr == NULL) {
        DEBUG_PRINT_CHAR(">>> Finding attribute '%s' on object %s\n", name, cur_obj->name);

        // Find the attribute in the current object
        attr = ht_find_str(cur_obj->attributes, name);
        if (attr != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            DEBUG_PRINT_CHAR(">>> Cannot find attribute '%s' in object %s:\n", name, self->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    DEBUG_PRINT_CHAR(">>> Found attribute '%s' in object %s (actually found in object %s)\n", name, self->name, cur_obj->name);
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

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_attrib_object *attrib_obj = (t_attrib_object *) obj;

    // By default, it's not bound to any instances. This can only happen during object_attrib_duplicate, where an
    // attribute is being called inside an instance.
    attrib_obj->data.bound_instance = NULL;

    // Note that bound_class does not get a ref-increase, because of cyclic dependencies on the bound class itself
    t_dll_element *e = DLL_HEAD(arg_list);
    attrib_obj->data.bound_class = (t_object *) e->data.p;


//    // build_attrib builds attributes that aren't bound to any class (yet))
//    if (attrib_obj->data.bound_class) {
//        object_inc_ref(attrib_obj->data.bound_class);
//    }

    e = DLL_NEXT(e);
    attrib_obj->data.bound_name = string_strdup0((char *) e->data.p);

    e = DLL_NEXT(e);
    attrib_obj->data.attr_type = (long) e->data.l;

    e = DLL_NEXT(e);
    attrib_obj->data.attr_visibility = (long) e->data.l;

    e = DLL_NEXT(e);
    attrib_obj->data.attr_access = (long) e->data.l;

    e = DLL_NEXT(e);
    attrib_obj->data.attribute = (t_object *) e->data.p;
    object_inc_ref(attrib_obj->data.attribute);

    e = DLL_NEXT(e);
    attrib_obj->data.attr_method_flags = (long) e->data.l;
}

static void obj_free(t_object *obj) {
    t_attrib_object *attr_obj = (t_attrib_object *) obj;

    // Status of the attribute inside a attribute-object:
    //   data.attr_type          : long, so no need to free
    //   data.attr_visibility    : long, so no need to free
    //   data.attr_access        : long, so no need to free
    //   data.attr_method_flags  : long, so no need to free

    //   data.attribute          : object where this attribute points to
    object_release(attr_obj->data.attribute);

    //   data.bound_instance     : the instance of the class where this attribute is attached to
    if (attr_obj->data.bound_instance) {
//        object_release(attr_obj->data.bound_instance);
    }

    // @TODO: Should the bound_class not be found directly from the bound_instance??
    //   data.bound_class       : the class of the class where this attribute is attached to. Bound classes are not ref-counted!
//    if (attr_obj->data.bound_class) {
//        object_release(attr_obj->data.bound_class);
//    }

    //   data.bound_name        : the attribute name of this attribute in the class/instance (ie: method or property name)
    if (attr_obj->data.bound_name) {
        smm_free(attr_obj->data.bound_name);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_attrib_object *self = (t_attrib_object *) obj;

    char attrbuf[1024];
    //snprintf(attrbuf, 1024, "%s", self->data.attribute ? object_debug(self->data.attribute) : "unlinked");
    snprintf(attrbuf, 1024, "%s", self->data.bound_name);

    snprintf(self->__debug_info, 199, "%s [%s%s%s] Attached: %s", self->name,
        self->data.attr_type == 0 ? "M" : self->data.attr_type == 1 ? "C" : self->data.attr_type == 2 ? "P" : "?",
        self->data.attr_visibility == 0 ? "P" : self->data.attr_visibility == 1 ? "R" : self->data.attr_visibility == 2 ? "V" : "?",
        self->data.attr_access == 0 ? "W" : "R",
        attrbuf);

    return self->__debug_info;
}
#endif


// Attrib object management functions
t_object_funcs attrib_funcs = {
    obj_populate, // Populate an attrib object
    obj_free, // Free an attrib object
    obj_destroy, // Destroy an attrib object
    NULL, // Clone
    NULL, // Cache
    NULL, // Hash
#ifdef __DEBUG
    obj_debug,
#endif
};

// Initial object

t_attrib_object Object_Attrib_struct = {
    OBJECT_HEAD_INIT("attrib", objectTypeAttribute, OBJECT_TYPE_CLASS, &attrib_funcs, sizeof (t_attrib_object_data)), {
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        NULL
    }
};
