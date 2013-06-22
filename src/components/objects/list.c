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
SAFFIRE_METHOD(list, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(list, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns the number of elements stored inside the list
 */
SAFFIRE_METHOD(list, length) {
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
  * Saffire method: Returns object stored at "key" inside the list (or NULL when not found)
  */
SAFFIRE_METHOD(list, get) {
    t_numerical_object *key;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &key)) {
        return NULL;
    }

    t_object *obj = ht_num_find(self->ht, key->value);
    if (obj == NULL) RETURN_NULL;
    RETURN_OBJECT(obj);
}

/**
 * Saffire method:
 */
SAFFIRE_METHOD(list, add) {
    t_object *val;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  &val)) {
        return NULL;
    }

    ht_num_add(self->ht, self->ht->element_count, val);
    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_METHOD(list, conv_boolean) {
    if (self->ht->element_count == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

/**
 *
 */
SAFFIRE_METHOD(list, conv_null) {
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(list, conv_numerical) {
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
 *
 */
SAFFIRE_METHOD(list, conv_string) {
    RETURN_STRING("list");
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
 * Initializes list methods and properties, these are used
 */
void object_list_init(void) {
    Object_List_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_List_struct, "__ctor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_ctor);
    object_add_internal_method((t_object *)&Object_List_struct, "__dtor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_dtor);

    object_add_internal_method((t_object *)&Object_List_struct, "__boolean",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_List_struct, "__null",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_null);
    object_add_internal_method((t_object *)&Object_List_struct, "__numerical",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_List_struct, "__string",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_string);

    object_add_internal_method((t_object *)&Object_List_struct, "length",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_length);
    object_add_internal_method((t_object *)&Object_List_struct, "add",            CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_add);
    object_add_internal_method((t_object *)&Object_List_struct, "get",            CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_get);

//    // list + element
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_add",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_add);
//    // list << N  shifts elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sl",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_sl);
//    // list >> N  pops elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sr",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_sr);

    vm_populate_builtins("list", (t_object *)&Object_List_struct);
}

/**
 * Frees memory for a list object
 */
void object_list_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_List_struct);
    ht_destroy(Object_List_struct.attributes);
}


static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_list_object *obj = smm_malloc(sizeof(t_list_object));
    memcpy(obj, Object_List, sizeof(t_list_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_list_object *list_obj = (t_list_object *)obj;
    // @TODO: We should duplicate the list, and add it!

    list_obj->ht = arg_list->size == 0 ? ht_create() : DLL_HEAD(arg_list)->data;
}

static void obj_free(t_object *obj) {
    t_list_object *list_obj = (t_list_object *)obj;
    if (! list_obj) return;

    if (list_obj->ht) {
        ht_destroy(list_obj->ht);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_hash_table *ht = ((t_list_object *)obj)->ht;
    sprintf(global_buf, "list[%d]", ht ? ht->element_count : 0);
    return global_buf;
}
#endif


// List object management functions
t_object_funcs list_funcs = {
        obj_new,              // Allocate a new list object
        obj_populate,         // Populate a list object
        obj_free,             // Free a list object
        obj_destroy,          // Destroy a list object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};



// Intial object
t_list_object Object_List_struct = {
    OBJECT_HEAD_INIT("list", objectTypeList, OBJECT_TYPE_CLASS, &list_funcs),
    NULL
};

