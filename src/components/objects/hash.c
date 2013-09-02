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


SAFFIRE_METHOD(hash, __iterator) {
    RETURN_SELF;
}
SAFFIRE_METHOD(hash, __key) {
    RETURN_OBJECT(ht_iter_key_obj(&self->iter));
}
SAFFIRE_METHOD(hash, __value) {
    RETURN_OBJECT(ht_iter_value(&self->iter));
}
SAFFIRE_METHOD(hash, __next) {
    ht_iter_next(&self->iter);
    RETURN_SELF;
}
SAFFIRE_METHOD(hash, __rewind) {
    ht_iter_init(&self->iter, self->ht);
    RETURN_SELF;
}
SAFFIRE_METHOD(hash, __hasNext) {
    if (self->iter.bucket->next_element != NULL) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}



SAFFIRE_METHOD(hash, populate) {
    t_hash_object *ht_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  (t_object *)&ht_obj)) {
        return NULL;
    }
    if (! OBJECT_IS_HASH(ht_obj)) {
        object_raise_exception(Object_ArgumentException, 1, "populate() expects a list object");
        return NULL;
    }

    if (! self->ht) {
        self->ht = ht_create();
    }

    t_hash_iter iter;
    ht_iter_init(&iter, ht_obj->ht);
    while (ht_iter_valid(&iter)) {
        t_object *key = ht_iter_value(&iter);
        ht_iter_next(&iter);
        t_object *val = ht_iter_value(&iter);
        ht_iter_next(&iter);

    printf("KEY Hash increasing reference: %08X from %d to %d\n", (unsigned int)key, key->ref_count, key->ref_count+1);
    printf("VAL Hash increasing reference: %08X from %d to %d\n", (unsigned int)val, val->ref_count, val->ref_count+1);
        object_inc_ref(key);
        object_inc_ref(val);
        ht_add_obj(self->ht, key, val);
    }

    RETURN_SELF;
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
    if (obj == NULL) return default_value ? default_value : object_alloc(Object_Null, 0);
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

    printf("KEY Hash increasing reference: %08X from %d to %d\n", (unsigned int)key, key->ref_count, key->ref_count+1);
    printf("VAL Hash increasing reference: %08X from %d to %d\n", (unsigned int)val, val->ref_count, val->ref_count+1);

    object_inc_ref(key);
    object_inc_ref(val);
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

    t_object *obj = ht_remove_obj(self->ht, key);
    object_release(key);
    if (obj) object_release(obj);

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
    object_add_internal_method((t_object *)&Object_Hash_struct, "__ctor",         ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_ctor);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__dtor",         ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_dtor);

    object_add_internal_method((t_object *)&Object_Hash_struct, "__boolean",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__null",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_null);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__numerical",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__string",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_conv_string);

    // Datastructure interface
    object_add_internal_method((t_object *)&Object_Hash_struct, "populate",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_populate);

    // Iterator interface
    object_add_internal_method((t_object *)&Object_Hash_struct, "__iterator",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___iterator);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__key",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___key);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__value",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___value);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__rewind",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___rewind);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__next",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___next);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__hasNext",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method___hasNext);


    object_add_internal_method((t_object *)&Object_Hash_struct, "__length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_length);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__add",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_add);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__remove",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_remove);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__get",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_get);
    object_add_internal_method((t_object *)&Object_Hash_struct, "__has",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_has);

    object_add_internal_method((t_object *)&Object_Hash_struct, "length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_length);
    object_add_internal_method((t_object *)&Object_Hash_struct, "add",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_add);
    object_add_internal_method((t_object *)&Object_Hash_struct, "remove",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_remove);
    object_add_internal_method((t_object *)&Object_Hash_struct, "get",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_get);
    object_add_internal_method((t_object *)&Object_Hash_struct, "has",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_has);

    object_add_internal_method((t_object *)&Object_Hash_struct, "keys",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_keys);


//    // hash + tuple[k,v]
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_add",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_add);
//    // hash << N  shifts elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sl",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_sl);
//    // hash >> N  pops elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sr",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_hash_method_opr_sr);


    object_add_interface((t_object *)&Object_Hash_struct, Object_Iterator);
    object_add_interface((t_object *)&Object_Hash_struct, Object_Datastructure);

    vm_populate_builtins("hash", (t_object *)&Object_Hash_struct);
}

/**
 * Frees memory for a hash object
 */
void object_hash_fini(void) {
    object_free_internal_object((t_object *)&Object_Hash_struct);
}


static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_hash_object *obj = smm_malloc(sizeof(t_hash_object));
    memcpy(obj, Object_Hash, sizeof(t_hash_object));

    // Dynamically allocated
    obj->flags |= OBJECT_FLAG_ALLOCATED;

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

/**
 * Datastructures populate can be done in 2 ways:   1 argument: HASHTABLE.   2nd argument: DLL (first arg is ignored)
 */
static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_hash_object *hash_obj = (t_hash_object *)obj;

    // No arguments
    if (arg_list->size == 0) {
        hash_obj->ht = ht_create();
        return;
    }

    if (arg_list->size == 1) {
        // Simple hash table. Direct copy
        hash_obj->ht = DLL_HEAD(arg_list)->data;
        return;
    }

    // 2 (or higher). Use the DLL in arg2
    hash_obj->ht = ht_create();
    t_dll_element *e = DLL_HEAD(arg_list);
    e = DLL_NEXT(e);
    t_dll *dll = (t_dll *)e->data;
    e = DLL_HEAD(dll);    // 2nd element of the DLL is a DLL itself.. inception!
    while (e) {
        t_object *key = (t_object *)e->data;
        e = DLL_NEXT(e);
        if (! e) break;
        t_object *val = (t_object *)e->data;

    printf("KEY Hash increasing reference: %08X from %d to %d\n", (unsigned int)key, key->ref_count, key->ref_count+1);
    printf("VAL Hash increasing reference: %08X from %d to %d\n", (unsigned int)val, val->ref_count, val->ref_count+1);

        object_inc_ref(key);
        object_inc_ref(val);

        ht_add_obj(hash_obj->ht, key, val);
        e = DLL_NEXT(e);
    }
}

static void obj_free(t_object *obj) {
    t_hash_object *hash_obj = (t_hash_object *)obj;
    if (! hash_obj) return;

    // Because this is a hash-object, we can only add objects to the hash itself,
    // We must decrease their references as well

    t_hash_iter iter;
    ht_iter_init(&iter, hash_obj->ht);
    while (ht_iter_valid(&iter)) {
        t_object *key = ht_iter_key_obj(&iter);
        t_object *val = ht_iter_value(&iter);
        printf("KEY Hash decreasing reference: %08X from %d to %d\n", (unsigned int)key, key->ref_count, key->ref_count-1);
        printf("VAL Hash decreasing reference: %08X from %d to %d\n", (unsigned int)val, val->ref_count, val->ref_count-1);
        object_release(key);
        object_release(val);
        ht_iter_next(&iter);
    }

    ht_destroy(hash_obj->ht);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    if (OBJECT_TYPE_IS_CLASS(obj)) {
        sprintf(global_buf, "Hash");
    } else {
        t_hash_table *ht = ((t_hash_object *)obj)->ht;
        sprintf(global_buf, "hash[%d]", ht ? ht->element_count : 0);
    }
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
        NULL,                 // Cach
#ifdef __DEBUG
        obj_debug
#endif
};



// Intial object
t_hash_object Object_Hash_struct = {
    OBJECT_HEAD_INIT("hash", objectTypeHash, OBJECT_TYPE_CLASS, &hash_funcs),
    NULL,
    /* t_hash_iter */
};

