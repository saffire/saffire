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

#include <string.h>
#include <ctype.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/memory/smm.h>


/*
 class base {
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

/**
 * @TODO: Not a pretty O(n) operation. Change so we don't have to iterate the whole methods hash to check if we already
 * added data.
 */
static t_hash_table *find_properties(t_object *self, int attrib_type, int check_parents) {
    // Store found methods
    t_hash_table *methods = ht_create();

    t_object *obj = self;
    while (obj) {
        // Iterate attributes form object
        t_hash_iter iter;
        ht_iter_init(&iter, obj->attributes);
        while (ht_iter_valid(&iter)) {
            t_attrib_object *attrib_obj = (t_attrib_object *)ht_iter_value(&iter);
            ht_iter_next(&iter);

            // Next item when this is attribute is not a method
            switch (attrib_type) {
                case ATTRIB_TYPE_METHOD:
                    if (! ATTRIB_IS_METHOD(attrib_obj)) continue;
                    break;
                case ATTRIB_TYPE_CONSTANT:
                    if (! ATTRIB_IS_CONSTANT(attrib_obj)) continue;
                    break;
                case ATTRIB_TYPE_PROPERTY:
                    if (! ATTRIB_IS_PROPERTY(attrib_obj)) continue;
                    break;
            }

            // Check if element has already been added.
            int found = 0;
            t_hash_iter iter2;
            ht_iter_init(&iter2, methods);
            while (ht_iter_valid(&iter2)) {
                t_string_object *str = ht_iter_value(&iter2);
                ht_iter_next(&iter2);
                char *s = OBJ2STR0(str);
                if (strcmp(s, attrib_obj->data.bound_name) == 0) {
                    found = 1;
                    break;
                }
            }

            if (! found) {
                t_object *str = object_alloc_instance(Object_String, 2, strlen(attrib_obj->data.bound_name), attrib_obj->data.bound_name);
               ht_add_num(methods, methods->element_count, str);
            }
        }

        // If we need parent methods as well, goto parent, otherwise we're done
        obj = check_parents ? obj->parent : NULL;
    }

    return methods;
}


static t_hash_table *find_interfaces(t_object *self, int check_parents) {
    // Store found interfaces
    t_hash_table *interfaces = ht_create();

    t_object *obj = self;
    while (obj) {
        t_dll_element *e = obj->interfaces ? DLL_HEAD(obj->interfaces) : NULL;
        while (e) {
            t_object *interface_obj = DLL_DATA_PTR(e);
            e = DLL_NEXT(e);

            // Check if element has already been added.
            int found = 0;
            t_hash_iter iter2;
            ht_iter_init(&iter2, interfaces);
            while (ht_iter_valid(&iter2)) {
                t_string_object *str = ht_iter_value(&iter2);
                ht_iter_next(&iter2);
                char *s = OBJ2STR0(str);
                if (strcmp(s, interface_obj->name) == 0) {
                    found = 1;
                    break;
                }
            }

            if (! found) {
                t_object *str = object_alloc_instance(Object_String, 2, strlen(interface_obj->name), interface_obj->name);
               ht_add_num(interfaces, interfaces->element_count, str);
            }
        }

        // If we need parent methods as well, goto parent, otherwise we're done
        obj = check_parents ? obj->parent : NULL;
    }

    return interfaces;
}


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
 * Returns a list of all properties in this class
 */
SAFFIRE_METHOD(base, constants) {
    // Check arguments
    t_boolean_object *parents_obj;
    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "|b", (t_object *)&parents_obj) != 0) {
        return NULL;
    }

    t_hash_table *methods = find_properties(self, ATTRIB_TYPE_CONSTANT, IS_BOOLEAN_TRUE(parents_obj));
    RETURN_LIST(methods);
}

/**
 * Returns a list of all properties in this class
 */
SAFFIRE_METHOD(base, properties) {
    // Check arguments
    t_boolean_object *parents_obj;
    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "|b", (t_object *)&parents_obj) != 0) {
        return NULL;
    }

    t_hash_table *methods = find_properties(self, ATTRIB_TYPE_PROPERTY, IS_BOOLEAN_TRUE(parents_obj));
    RETURN_LIST(methods);
}

/**
 * Return a list of all methods in this class
 *
 */
SAFFIRE_METHOD(base, methods) {
    // Check arguments
    t_boolean_object *parents_obj;
    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "|b", (t_object *)&parents_obj) != 0) {
        return NULL;
    }

    t_hash_table *methods = find_properties(self, ATTRIB_TYPE_METHOD, IS_BOOLEAN_TRUE(parents_obj));
    RETURN_LIST(methods);
}

/**
 * Returns a list of all the parent classes this class extends
 */
SAFFIRE_METHOD(base, parents) {
    // Store found parents
    t_hash_table *parents = ht_create();

    t_object *obj = self;
    while (obj) {
        if (obj->parent) {
            t_object *str = object_alloc_instance(Object_String, 2, strlen(obj->parent->name), obj->parent->name);
            ht_add_num(parents, parents->element_count, str);
        }

        obj = obj->parent;
    }

    RETURN_LIST(parents);
}

/**
 * Returns the name of the class
 */
SAFFIRE_METHOD(base, name) {
    RETURN_STRING_FROM_CHAR(self->name);
}

/**
 * Returns the type of the class
 */
SAFFIRE_METHOD(base, type) {
    RETURN_STRING_FROM_CHAR(objectTypeNames[self->type]);
}

/**
 * Returns a list of interfaces this object implements
 */
SAFFIRE_METHOD(base, implements) {
    // Check arguments
    t_boolean_object *parents_obj;
    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "|b", (t_object *)&parents_obj) != 0) {
        return NULL;
    }

    if (parents_obj == NULL) {
         parents_obj = (t_boolean_object *)Object_True;
    }

    t_hash_table *interfaces = find_interfaces(self, IS_BOOLEAN_TRUE(parents_obj));
    RETURN_LIST(interfaces);
}

/**
 * Returns a list of interfaces this object implements
 */
SAFFIRE_METHOD(base, instanceof) {
    t_object *obj;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &obj) != 0) {
        return NULL;
    }

    if (object_instance_of(self, obj->name)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
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
    RETURN_STRING_FROM_CHAR("annotations");
}

/**
 * Clone the object into a new object
 */
SAFFIRE_METHOD(base, clone) {
    RETURN_OBJECT(object_clone((t_object *)self));
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

SAFFIRE_OPERATOR_METHOD(base, not) {
    RETURN_FALSE;
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

    object_add_internal_method((t_object *)&Object_Base_struct, "__ctor",         ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_base_method_ctor);
    object_add_internal_method((t_object *)&Object_Base_struct, "__dtor",         ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_base_method_dtor);
    object_add_internal_method((t_object *)&Object_Base_struct, "__constants",    ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_constants);
    object_add_internal_method((t_object *)&Object_Base_struct, "__properties",   ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_properties);
    object_add_internal_method((t_object *)&Object_Base_struct, "__methods",      ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_methods);
    object_add_internal_method((t_object *)&Object_Base_struct, "__parents",      ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_parents);
    object_add_internal_method((t_object *)&Object_Base_struct, "__name",         ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_name);
    object_add_internal_method((t_object *)&Object_Base_struct, "__type",         ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_type);
    object_add_internal_method((t_object *)&Object_Base_struct, "__implements",   ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_implements);
    object_add_internal_method((t_object *)&Object_Base_struct, "__instanceOf",   ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_instanceof);
    object_add_internal_method((t_object *)&Object_Base_struct, "__memory",       ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_memory);
    object_add_internal_method((t_object *)&Object_Base_struct, "__annotations",  ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_annotations);
    object_add_internal_method((t_object *)&Object_Base_struct, "__clone",        ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_clone);
    object_add_internal_method((t_object *)&Object_Base_struct, "__immutable?",   ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_is_immutable);
    object_add_internal_method((t_object *)&Object_Base_struct, "__immutable",    ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_immutable);
    object_add_internal_method((t_object *)&Object_Base_struct, "__refcount",     ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_refcount);
    object_add_internal_method((t_object *)&Object_Base_struct, "__id",           ATTRIB_METHOD_MIXED, ATTRIB_VISIBILITY_PUBLIC, object_base_method_id);

    object_add_internal_method((t_object *)&Object_Base_struct, "__opr_not",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_base_method_opr_not);
}


/**
 * Frees memory for a base object
 */
void object_base_fini() {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Base_struct);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    snprintf(obj->__debug_info, DEBUG_INFO_SIZE-1, "%s[%s]", objectTypeNames[obj->type], obj->name);
    return obj->__debug_info;
}
#endif


// String object management functions
t_object_funcs base_funcs = {
        NULL,                 // Populate a string object
        NULL,                 // Free a string object
        obj_destroy,          // Destroy a string object
        NULL,                 // Clone
        NULL,                 // Object cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};


// Initial object
t_object Object_Base_struct = {
    OBJECT_HEAD_INIT_WITH_BASECLASS("base", objectTypeBase, OBJECT_TYPE_CLASS, &base_funcs, NULL, NULL, 0),
    OBJECT_FOOTER
};



// String object management functions
t_object_funcs user_funcs = {
        NULL,             // Populate a user object
        NULL,             // Free a user object
        obj_destroy,      // Destroy a user object
        NULL,             // Clone
        NULL,             // Object cache
        NULL,             // Hash
        NULL,             // Debug
};


// @TODO: Remove the user structure to user.c

// Initial object
t_object Object_User_struct = {
    OBJECT_HEAD_INIT_WITH_BASECLASS("user", objectTypeUser, OBJECT_TYPE_CLASS, &user_funcs, NULL, NULL, 0),
    OBJECT_FOOTER
};
