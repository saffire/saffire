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
SAFFIRE_METHOD(hash, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(hash, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns the number of elements stored inside the hash
 */
SAFFIRE_METHOD(hash, length) {
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
  * Saffire method: Returns object stored at "key" inside the hash (or NULL when not found)
  */
SAFFIRE_METHOD(hash, get) {
    t_object *key;
    t_object *default_value = NULL;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o|o", &key, &default_value)) {
        return NULL;
    }

    t_object *obj = ht_find_obj(self->ht, key);
    if (obj == NULL) return default_value ? default_value : Object_Null;
    RETURN_OBJECT(obj);
}

/**
 *
 */
SAFFIRE_METHOD(hash, keys) {
    t_hash_table *ht = ht_create();

    t_hash_iter iter;
    ht_iter_init(&iter, self->ht);
    while (ht_iter_valid(&iter)) {
        t_object *key = (t_object *)ht_iter_key_obj(&iter);
        ht_add_num(ht, ht->element_count, (t_object *)key);
        ht_iter_next(&iter);
    }

    RETURN_LIST(ht);
}


/**
  * Saffire method: Returns true if requested key exists and false if not
  */
SAFFIRE_METHOD(hash, has) {
    t_object *key;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &key)) {
        return NULL;
    }

    // We need to check if the address of the key(object) exists, as we only deal with object keys and values
    if (ht_exists_obj(self->ht, key)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/**
 * Saffire method:
 */
SAFFIRE_METHOD(hash, add) {
    t_object *key, *val;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "oo", &key, &val)) {
        return NULL;
    }

    ht_add_obj(self->ht, key, val);
    RETURN_SELF;
}

/**
 * Saffire method:
 */
SAFFIRE_METHOD(hash, remove) {
    t_object *key;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &key)) {
        return NULL;
    }

    ht_remove_obj(self->ht, key);
    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_METHOD(hash, conv_boolean) {
    if (self->ht->element_count == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

/**
 *
 */
SAFFIRE_METHOD(hash, conv_null) {
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(hash, conv_numerical) {
    RETURN_NUMERICAL(self->ht->element_count);
}

/**
 *
 */
SAFFIRE_METHOD(hash, conv_string) {
    char s[100];

    snprintf(s, 99, "hash[%d]", self->ht->element_count);
    RETURN_STRING(s);
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
 * Initializes hash methods and properties, these are used
 */
void object_hash_init(void) {
    Object_Hash_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_Hash_struct, "__ctor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_ctor);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__dtor",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_dtor);

    object_add_internal_method((t_object *)&Object_Hash_struct, "__boolean",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__null",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_null);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__numerical",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__string",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_string);

    object_add_internal_method((t_object *)&Object_Hash_struct, "length",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_length);
    object_add_internal_method((t_object *)&Object_Hash_struct, "add",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_add);
    object_add_internal_method((t_object *)&Object_Hash_struct, "remove",       CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_remove);
    object_add_internal_method((t_object *)&Object_Hash_struct, "get",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_get);
    object_add_internal_method((t_object *)&Object_Hash_struct, "has",          CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_has);

    object_add_internal_method((t_object *)&Object_Hash_struct, "keys",         CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_keys);


//    // hash + tuple[k,v]
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_add",     CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_add);
//    // hash << N  shifts elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sl",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_sl);
//    // hash >> N  pops elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sr",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_sr);


    vm_populate_builtins("hash", (t_object *)&Object_Hash_struct);
}

/**
 * Frees memory for a hash object
 */
void object_hash_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Hash_struct);
    ht_destroy(Object_Hash_struct.attributes);
}


static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_hash_object *obj = smm_malloc(sizeof(t_hash_object));
    memcpy(obj, Object_Hash, sizeof(t_hash_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_hash_object *hash_obj = (t_hash_object *)obj;
    // @TODO: We should duplicate the hash, and add it!

    hash_obj->ht = arg_list->size == 0 ? ht_create() : DLL_HEAD(arg_list)->data;
}

static void obj_free(t_object *obj) {
    t_hash_object *hash_obj = (t_hash_object *)obj;
    if (! hash_obj) return;

    if (hash_obj->ht) {
        ht_destroy(hash_obj->ht);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_hash_table *ht = ((t_hash_object *)obj)->ht;
    sprintf(global_buf, "hash[%d]", ht ? ht->element_count : 0);
    return global_buf;
}
#endif


// Hash object management functions
t_object_funcs hash_funcs = {
        obj_new,              // Allocate a new hash object
        obj_populate,         // Populate a hash object
        obj_free,             // Free a hash object
        obj_destroy,          // Destroy a hash object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};



// Intial object
t_hash_object Object_Hash_struct = {
    OBJECT_HEAD_INIT("hash", objectTypeHash, OBJECT_TYPE_CLASS, &hash_funcs),
    NULL
};

