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


/*
 class base {
	public method new();
    public method ctor();
    public method dtor();
    public method properties();
    public method methods();
    public method parents();
    public method name();
    public method implements();
    public method memory();
    public method annotations();
    public method clone();
    public method immutable();
    public method is_immutable();
    public method destroy();
    public method refcount();
    public method id();
 }

 */


/* ======================================================================
 *   Object methods
 * ======================================================================
 */

/**
 * Instantiation
 */
SAFFIRE_METHOD(base, new) {
    t_object *obj = object_new_with_dll_args((t_object *)self, arguments);
    RETURN_OBJECT(obj);
}

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
    RETURN_STRING("methods");
}

/**
 * Returns a list of all the parent classes this class extends
 */
SAFFIRE_METHOD(base, parents) {
    // @TODO: return list of parents
    RETURN_STRING("parents");
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
    RETURN_STRING("implementations");
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
    RETURN_STRING("annotations");
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
    self->flags |= OBJECT_FLAG_IMMUTABLE;
    RETURN_SELF;
}

/**
 * Returns TRUE when immutable, FALSE otherwise
 */
SAFFIRE_METHOD(base, is_immutable) {
    if ((self->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE) {
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

/**
 * Returns id for this object
 */
SAFFIRE_METHOD(base, id) {
    RETURN_NUMERICAL((long)self);
}

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes base methods and properties
 */
void object_base_init() {
    Object_Base_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&Object_Base_struct, "__new",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_new);
    object_add_internal_method((t_object *)&Object_Base_struct, "__ctor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_ctor);
    object_add_internal_method((t_object *)&Object_Base_struct, "__dtor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_dtor);
    object_add_internal_method((t_object *)&Object_Base_struct, "__properties",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_properties);
    object_add_internal_method((t_object *)&Object_Base_struct, "__methods",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_methods);
    object_add_internal_method((t_object *)&Object_Base_struct, "__parents",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_parents);
    object_add_internal_method((t_object *)&Object_Base_struct, "__name",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_name);
    object_add_internal_method((t_object *)&Object_Base_struct, "__implements",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_implements);
    object_add_internal_method((t_object *)&Object_Base_struct, "__memory",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_memory);
    object_add_internal_method((t_object *)&Object_Base_struct, "__annotations",  CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_annotations);
    object_add_internal_method((t_object *)&Object_Base_struct, "__clone",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_clone);
    object_add_internal_method((t_object *)&Object_Base_struct, "__immutable?",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_is_immutable);
    object_add_internal_method((t_object *)&Object_Base_struct, "__immutable",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_immutable);
    object_add_internal_method((t_object *)&Object_Base_struct, "__destroy",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_destroy);
    object_add_internal_method((t_object *)&Object_Base_struct, "__refcount",     CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_refcount);
    object_add_internal_method((t_object *)&Object_Base_struct, "__id",           CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_base_method_id);
}


/**
 * Frees memory for a base object
 */
void object_base_fini() {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Base_struct);
    ht_destroy(Object_Base_struct.attributes);
}



// Initial object
t_object Object_Base_struct = {
    OBJECT_HEAD_INIT_WITH_BASECLASS("base", objectTypeBase, OBJECT_TYPE_CLASS, NULL, NULL)
};

