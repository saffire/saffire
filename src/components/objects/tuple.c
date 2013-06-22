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
#include <ctype.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/hashtable.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"
#include "general/output.h"

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
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
  * Saffire method: Returns object stored at index inside the tuple (or NULL when not found)
  */
SAFFIRE_METHOD(tuple, get) {
    t_numerical_object *index;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &index)) {
        return NULL;
    }

    t_object *obj = ht_num_find(self->ht, OBJ2NUM(index));
    if (obj == NULL) RETURN_NULL;
    RETURN_OBJECT(obj);
}

/**
 * Saffire method:
 */
SAFFIRE_METHOD(tuple, add) {
    t_object *val;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &val)) {
        return NULL;
    }
    ht_num_add(self->ht, self->ht->element_count, val);
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

    ht_remove(self->ht, key->value);
    RETURN_SELF;
}
*/


/**
 *
 */
SAFFIRE_METHOD(tuple, conv_boolean) {
    if (self->ht->element_count == 0) {
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
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
 *
 */
SAFFIRE_METHOD(tuple, conv_string) {
    RETURN_STRING("tuple");
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
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__ctor",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_ctor);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__dtor",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_dtor);

    object_add_internal_method((t_object *)&Object_Tuple_struct, "__boolean",     CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__null",        CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_null);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__numerical",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "__string",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_conv_string);

    object_add_internal_method((t_object *)&Object_Tuple_struct, "add",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_add);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "get",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_get);
    object_add_internal_method((t_object *)&Object_Tuple_struct, "length",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_tuple_method_length);

    vm_populate_builtins("tuple", (t_object *)&Object_Tuple_struct);
}

/**
 * Frees memory for a tuple object
 */
void object_tuple_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Tuple_struct);
    ht_destroy(Object_Tuple_struct.attributes);
}



static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_tuple_object *obj = smm_malloc(sizeof(t_tuple_object));
    memcpy(obj, Object_Tuple, sizeof(t_tuple_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_tuple_object *tuple_obj = (t_tuple_object *)obj;

    // Create new hash list
    tuple_obj->ht = ht_create();

    int cnt = 0;
    t_dll_element *e = DLL_HEAD(arg_list);
    while (e) {
        t_object *arg_obj = (t_object *)e->data;

        DEBUG_PRINT("Adding object: %s\n", object_debug(arg_obj));
        ht_num_add(tuple_obj->ht, cnt++, arg_obj);

        e = DLL_NEXT(e);
    }
}

static void obj_free(t_object *obj) {
    t_tuple_object *tuple_obj = (t_tuple_object *)obj;
    if (! tuple_obj) return;

    if (tuple_obj->ht) {
        ht_destroy(tuple_obj->ht);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    sprintf(global_buf, "tuple[%d]", ((t_tuple_object *)obj)->ht ? ((t_tuple_object *)obj)->ht->element_count : 0);
    return global_buf;
}
#endif


// Tuple object management functions
t_object_funcs tuple_funcs = {
        obj_new,              // Allocate a new tuple object
        obj_populate,
        obj_free,             // Free a tuple object
        obj_destroy,
        NULL,                 // Clone a tuple object
#ifdef __DEBUG
        obj_debug
#endif
};



// Intial object
t_tuple_object Object_Tuple_struct = {
    OBJECT_HEAD_INIT("tuple", objectTypeTuple, OBJECT_TYPE_CLASS, &tuple_funcs),
    NULL
};
