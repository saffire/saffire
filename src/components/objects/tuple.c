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
#include <ctype.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/hashtable.h>
#include <saffire/general/smm.h>
#include <saffire/general/md5.h>
#include <saffire/debug.h>
#include <saffire/general/output.h>

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
SAFFIRE_METHOD(tuple, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(tuple, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns the number of elements stored inside the tuple
 */
SAFFIRE_METHOD(tuple, length) {
    RETURN_NUMERICAL(self->data.ht->element_count);
}

/**
  * Saffire method: Returns object stored at index inside the tuple (or NULL when not found)
  */
SAFFIRE_METHOD(tuple, get) {
    t_numerical_object *index;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &index)) {
        return NULL;
    }

    // Check index boundaries
    long idx = OBJ2NUM(index);
    if (idx < 0 || idx >= self->data.ht->element_count) {
        object_raise_exception(Object_IndexException, 1, "Index out of range");
        return NULL;
    }

    t_object *obj = ht_find_num(self->data.ht, OBJ2NUM(index));
    if (obj == NULL) RETURN_NULL;
    RETURN_OBJECT(obj);
}

///**
// * Saffire method:
// */
//SAFFIRE_METHOD(tuple, add) {
//    t_object *val;
//
//    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &val)) {
//        return NULL;
//    }
//    ht_add_num(self->data.ht, self->data.ht->element_count, val);
//    RETURN_SELF;
//}

/**
 * Saffire method:
 */
SAFFIRE_METHOD(tuple, populate) {
    t_hash_object *ht_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  (t_object *)&ht_obj)) {
        return NULL;
    }
    if (! OBJECT_IS_HASH(ht_obj)) {
        object_raise_exception(Object_ArgumentException, 1, "populate() expects a tuple object");
        return NULL;
    }

    if (! self->data.ht) {
        self->data.ht = ht_create();
    }

    t_hash_iter iter;
    ht_iter_init(&iter, ht_obj->data.ht);
    while (ht_iter_valid(&iter)) {
        ht_add_num(self->data.ht, self->data.ht->element_count, ht_iter_value(&iter));
        ht_iter_next(&iter);
    }

    RETURN_SELF;
}

/**
 * Saffire method:
 */
/*
SAFFIRE_METHOD(tuple, remove) {
    t_string_object *key;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &key)) {
        return NULL;
    }

    ht_remove(self->data.ht, key->value);
    RETURN_SELF;
}
*/


/**
 *
 */
SAFFIRE_METHOD(tuple, conv_boolean) {
    if (self->data.ht->element_count == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

/**
 *
 */
SAFFIRE_METHOD(tuple, conv_null) {
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(tuple, conv_numerical) {
    RETURN_NUMERICAL(self->data.ht->element_count);
}

/**
 *
 */
SAFFIRE_METHOD(tuple, conv_string) {
    RETURN_STRING_FROM_CHAR("tuple");
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
 * Initializes tuple methods and properties, these are used
 */
void object_tuple_init(void) {
    Object_Tuple_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__ctor",        ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_ctor);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__dtor",        ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_dtor);

    object_add_internal_method((t_object *)&Object_Tuple_struct, "__boolean",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__null",        ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_null);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__numerical",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__string",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_string);

    // Datastructure interface
    object_add_internal_method((t_object *)&Object_Tuple_struct, "populate",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_populate);
    object_add_interface((t_object *)&Object_Tuple_struct, Object_Datastructure);

//    object_add_internal_method((t_object *)&Object_Tuple_struct, "add",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_add);
//    object_add_internal_method((t_object *)&Object_Tuple_struct, "get",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_get);
//    object_add_internal_method((t_object *)&Object_Tuple_struct, "length",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_length);

    // Subscription interface
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_length);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__set",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, NULL);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__remove",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, NULL);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__get",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_get);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__has",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, NULL);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__splice",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, NULL);
    object_add_interface((t_object *)&Object_Tuple_struct, Object_Subscription);

    vm_populate_builtins("tuple", (t_object *)&Object_Tuple_struct);
}

/**
 * Frees memory for a tuple object
 */
void object_tuple_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Tuple_struct);
}



static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_tuple_object *tuple_obj = (t_tuple_object *)obj;

    // Create new hash list
    tuple_obj->data.ht = ht_create();

    int cnt = 0;
    t_dll_element *e = DLL_HEAD(arg_list);
    if (! e) return;


    e = DLL_NEXT(e);

    t_dll *dll = (t_dll *)e->data.p;
    e = DLL_HEAD(dll);    // 2nd element of the DLL is a DLL itself.. inception!
    while (e) {
        t_object *arg_obj = (t_object *)e->data.p;

        DEBUG_PRINT_STRING_ARGS("Adding object: %s\n", object_debug(arg_obj));
        ht_add_num(tuple_obj->data.ht, cnt++, arg_obj);

        e = DLL_NEXT(e);
    }
}

static void obj_free(t_object *obj) {
    t_tuple_object *tuple_obj = (t_tuple_object *)obj;
    if (! tuple_obj) return;

    if (tuple_obj->data.ht) {
        ht_destroy(tuple_obj->data.ht);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_tuple_object *tuple_obj = (t_tuple_object *)obj;

    if (tuple_obj->data.ht) {
        snprintf(tuple_obj->__debug_info, DEBUG_INFO_SIZE-1, "tuple[%d]", tuple_obj->data.ht->element_count);
    } else {
        snprintf(tuple_obj->__debug_info, DEBUG_INFO_SIZE-1, "tuple[]");
    }
    return tuple_obj->__debug_info;

}
#endif


// Tuple object management functions
t_object_funcs tuple_funcs = {
        obj_populate,
        obj_free,             // Free a tuple object
        obj_destroy,
        NULL,                 // Clone a tuple object
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};



// Intial object
t_tuple_object Object_Tuple_struct = {
    OBJECT_HEAD_INIT("tuple", objectTypeTuple, OBJECT_TYPE_CLASS, &tuple_funcs, sizeof(t_tuple_object_data)),
    {
        NULL
    },
    OBJECT_FOOTER
};
