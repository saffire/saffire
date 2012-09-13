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
#include "object/base.h"
#include "object/string.h"
#include "object/numerical.h"
#include "object/boolean.h"
#include "object/null.h"
#include "general/smm.h"
#include <string.h>


/* ======================================================================
 *   Object methods
 * ======================================================================
 */

/**
 * Constructor
 */
SAFFIRE_METHOD(base, ctor) {
    RETURN_SELF;
}

/**
 * Destructor
 */
SAFFIRE_METHOD(base, dtor) {
    RETURN_SELF;
}

/**
 * Returns a list of all properties in this class (but not the parents)
 */
SAFFIRE_METHOD(base, properties) {
    RETURN_SELF;
}

/**
 * Return a list of all methods in this class (but not from the parents)
 */
SAFFIRE_METHOD(base, methods) {
    // @TODO: return list of methods
    RETURN_STRING(L"methods");
}

/**
 * Returns a list of all the parent classes this class extends
 */
SAFFIRE_METHOD(base, parents) {
    // @TODO: return list of parents
    RETURN_STRING(L"parents");
}

/**
 * Returns the name of the class
 */
SAFFIRE_METHOD(base, name) {
    RETURN_STRING(self->name);
}

/**
 * Returns a list of interfaces this object implements
 */
SAFFIRE_METHOD(base, implements) {
    // @TODO: return list of implementations
    RETURN_STRING(L"implementations");
}

/**
 * Returns the number of memory in KB
 */
SAFFIRE_METHOD(base, memory) {
    // @TODO: return memory usage of this object
    RETURN_NUMERICAL(rand() % 100000)
}

/**
 * Returns a list of annotations on the methods
 */
SAFFIRE_METHOD(base, annotations) {
    // @TODO: return method annotations
    RETURN_STRING(L"annotations");
}

/**
 * Clone the object into a new object
 */
SAFFIRE_METHOD(base, clone) {
    t_object *obj = (t_object *)object_clone((t_object *)self);
    RETURN_OBJECT(obj);
}

/**
 * Sets object to immutable. Cannot undo.
 */
SAFFIRE_METHOD(base, immutable) {
    self->immutable = 1;
    RETURN_SELF;
}

/**
 * Returns TRUE when immutable, FALSE otherwise
 */
SAFFIRE_METHOD(base, is_immutable) {
    if (self->immutable == 1) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

/**
 * Destroys the object.
 */
SAFFIRE_METHOD(base, destroy) {
    // @TODO: destroy the object
    RETURN_NULL;
}

/**
 * Returns reference count for this object
 */
SAFFIRE_METHOD(base, refcount) {
    RETURN_NUMERICAL(self->ref_count);
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes base methods and properties
 */
void object_base_init() {
    Object_Base_struct.methods = ht_create();

    ht_add(Object_Base_struct.methods, "ctor", object_base_method_ctor);
    ht_add(Object_Base_struct.methods, "dtor", object_base_method_ctor);
    ht_add(Object_Base_struct.methods, "properties", object_base_method_properties);
    ht_add(Object_Base_struct.methods, "methods", object_base_method_methods);
    ht_add(Object_Base_struct.methods, "parents", object_base_method_parents);
    ht_add(Object_Base_struct.methods, "name", object_base_method_name);
    ht_add(Object_Base_struct.methods, "implements", object_base_method_implements);
    ht_add(Object_Base_struct.methods, "memory", object_base_method_memory);
    ht_add(Object_Base_struct.methods, "annotations", object_base_method_annotations);
    ht_add(Object_Base_struct.methods, "clone", object_base_method_clone);
    ht_add(Object_Base_struct.methods, "immutable", object_base_method_immutable);
    ht_add(Object_Base_struct.methods, "immutable?", object_base_method_is_immutable);
    ht_add(Object_Base_struct.methods, "destroy", object_base_method_destroy);
    ht_add(Object_Base_struct.methods, "refcount", object_base_method_refcount);

    Object_Base_struct.properties = ht_create();
}


/**
 * Frees memory for a base object
 */
void object_base_fini() {
    ht_destroy(Object_Base_struct.methods);
    ht_destroy(Object_Base_struct.properties);
}


/**
 * Clones a base object into a new object
 */
static t_object *obj_clone(t_object *obj) {
    // Create new object and copy all info
    t_base_object *new_obj = (t_base_object *)smm_malloc(sizeof(t_base_object));
    memcpy(new_obj, obj, sizeof(t_base_object));

    // New separated object, so refcount = 1
    new_obj->ref_count = 1;

    return (t_object *)new_obj;
}

// Base object management functions
t_object_funcs base_funcs = {
        NULL,               // Allocate a new string object
        NULL,               // Free a string object
        obj_clone           // Clone a string object
};

// Initial object
t_object Object_Base_struct = {
    OBJECT_HEAD_INIT3("base", 0, &base_funcs, NULL)
};

