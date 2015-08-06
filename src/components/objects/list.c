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
#include <saffire/memory/smm.h>
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
    RETURN_NUMERICAL(self->data.ht->element_count);
}

SAFFIRE_METHOD(list, __iterator) {
    RETURN_SELF;
}
SAFFIRE_METHOD(list, __key) {
    RETURN_NUMERICAL(self->data.iter.idx);
}
SAFFIRE_METHOD(list, __value) {
    t_object *obj = ht_find_num(self->data.ht, self->data.iter.idx);
    if (obj == NULL) RETURN_NULL;
    RETURN_OBJECT(obj);
}
SAFFIRE_METHOD(list, __next) {
    self->data.iter.idx++;
    RETURN_SELF;
}
SAFFIRE_METHOD(list, __rewind) {
    self->data.iter.idx = 0;
    RETURN_SELF;
}
SAFFIRE_METHOD(list, __hasNext) {
    if (self->data.iter.idx < self->data.ht->element_count) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


/**
  * Saffire method: Returns object stored at "key" inside the list (or NULL when not found)
  */
SAFFIRE_METHOD(list, get) {
    t_numerical_object *key;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &key)) {
        return NULL;
    }

    t_object *obj = ht_find_num(self->data.ht, key->data.value);
    if (obj == NULL) RETURN_NULL;
    RETURN_OBJECT(obj);
}

volatile int rdtscll() {
    int a,d;
    __asm__ __volatile__
        (".byte 0x0f, 0x31 #rdtsc\n" // edx:eax
         :"=a"(a), "=d"(d));
    return a;
}


int randomized = 0;


/**
  * Shuffle all elements in the linked list
  *
  * This uses Fisher-Yates. For new allocated shuffles (for instance: shuffle!(), use inside-out)
  */
SAFFIRE_METHOD(list, shuffle) {
    if (! randomized) {
        randomized = 1;
        srand(rdtscll());
    }


    // The hashtable has a linked list, we shuffle this one as it is used during iteration
    for (int i = self->data.ht->element_count-1; i >= 1; i--) {
        /* because we are exchanging values, the keys can stay the same. We basically fetch the
         * objects and swap them. */

        int j = (rand () % i);
        t_object *obj1 = ht_find_num(self->data.ht, i);
        t_object *obj2 = ht_find_num(self->data.ht, j);
        ht_replace_num(self->data.ht, i, obj2);
        ht_replace_num(self->data.ht, j, obj1);
    }
    RETURN_SELF;
}

/**
  * Pick random element
  */
SAFFIRE_METHOD(list, random) {
    if (! randomized) {
        randomized = 1;
        srand(rdtscll());
    }

    t_object *obj = ht_find_num(self->data.ht, (rand () % self->data.ht->element_count));
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

    ht_add_num(self->data.ht, self->data.ht->element_count, val);
    RETURN_SELF;
}



/**
 * Saffire method:
 */
SAFFIRE_METHOD(list, populate) {
    t_hash_object *ht_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  (t_object *)&ht_obj)) {
        return NULL;
    }
    if (! OBJECT_IS_HASH(ht_obj)) {
        object_raise_exception(Object_ArgumentException, 1, "populate() expects a list object");
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
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(list, sequence) {
    t_object *from;
    t_object *to;
    t_object *skip;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "oo|o",  &from, &to, &skip)) {
        return NULL;
    }

    // Default skip value
    if (skip == NULL) {
        skip = object_alloc_instance(Object_Numerical, 1, 1);
    }

    if (! OBJECT_IS_NUMERICAL(from) || ! OBJECT_IS_NUMERICAL(to) || ! OBJECT_IS_NUMERICAL(to)) {
        object_raise_exception(Object_ArgumentException, 1, "sequence() only works with numerical values");
        return NULL;
    }

    if (((t_numerical_object *)from)->data.value > ((t_numerical_object *)to)->data.value) {
        object_raise_exception(Object_ArgumentException, 1, "'from' value must be lower than the 'to' value");
        return NULL;
    }

    if (((t_numerical_object *)skip)->data.value <= 0) {
        object_raise_exception(Object_ArgumentException, 1, "'skip' must be 1 or higher");
        return NULL;
    }

    t_list_object *list_obj = (t_list_object *)object_alloc_instance(Object_List, 0);
    for (int i=((t_numerical_object *)from)->data.value; i<=((t_numerical_object *)to)->data.value; i+=((t_numerical_object *)skip)->data.value) {
        ht_add_num(list_obj->data.ht, list_obj->data.ht->element_count, object_alloc_instance(Object_Numerical, 1, i));
    }

    RETURN_OBJECT(list_obj);
}



/**
 *
 */
SAFFIRE_METHOD(list, conv_boolean) {
    if (self->data.ht->element_count == 0) {
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
    RETURN_NUMERICAL(self->data.ht->element_count);
}

/**
 *
 */
SAFFIRE_METHOD(list, conv_string) {
    char s[100];

    snprintf(s, 99, "list[%d]", self->data.ht->element_count);
    RETURN_STRING_FROM_CHAR(s);
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

    object_add_internal_method((t_object *)&Object_List_struct, "__ctor",         ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_list_method_ctor);
    object_add_internal_method((t_object *)&Object_List_struct, "__dtor",         ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_list_method_dtor);

    object_add_internal_method((t_object *)&Object_List_struct, "__boolean",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_List_struct, "__null",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_null);
    object_add_internal_method((t_object *)&Object_List_struct, "__numerical",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_List_struct, "__string",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_conv_string);

    // Datastructure interface
    object_add_internal_method((t_object *)&Object_List_struct, "populate",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_populate);

    // Iterator interface
    object_add_internal_method((t_object *)&Object_List_struct, "__iterator",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___iterator);
    object_add_internal_method((t_object *)&Object_List_struct, "__key",          ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___key);
    object_add_internal_method((t_object *)&Object_List_struct, "__value",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___value);
    object_add_internal_method((t_object *)&Object_List_struct, "__rewind",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___rewind);
    object_add_internal_method((t_object *)&Object_List_struct, "__next",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___next);
    object_add_internal_method((t_object *)&Object_List_struct, "__hasNext",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method___hasNext);
    object_add_internal_method((t_object *)&Object_List_struct, "__length",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_length);

    object_add_internal_method((t_object *)&Object_List_struct, "length",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_length);
    object_add_internal_method((t_object *)&Object_List_struct, "add",            ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_add);
    object_add_internal_method((t_object *)&Object_List_struct, "get",            ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_get);
    object_add_internal_method((t_object *)&Object_List_struct, "shuffle",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_shuffle);
    object_add_internal_method((t_object *)&Object_List_struct, "random",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_random);

    object_add_internal_method((t_object *)&Object_List_struct, "sequence",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_sequence);

//    // list + element
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_add",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_add);
//    // list << N  shifts elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sl",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_sl);
//    // list >> N  pops elements from the list
//    object_add_internal_method((t_object *)&Object_List_struct, "__opr_sr",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, object_list_method_opr_sr);


    object_add_interface((t_object *)&Object_List_struct, Object_Iterator);
    object_add_interface((t_object *)&Object_List_struct, Object_Datastructure);

    vm_populate_builtins("list", (t_object *)&Object_List_struct);
}

/**
 * Frees memory for a list object
 */
void object_list_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_List_struct);
}



static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_list_object *list_obj = (t_list_object *)obj;

    // No arguments
    if (arg_list->size == 0) {
        list_obj->data.ht = ht_create();
        return;
    }

    if (arg_list->size == 1) {
        // Simple hash table. Direct copy
        t_dll_element *e = DLL_HEAD(arg_list);
        list_obj->data.ht = DLL_DATA_PTR(e);
        return;
    }

    // 2 (or higher). Use the DLL in arg2
    list_obj->data.ht = ht_create();
    t_dll_element *e = DLL_HEAD(arg_list);
    e = DLL_NEXT(e);
    t_dll *dll = DLL_DATA_PTR(e);
    e = DLL_HEAD(dll);    // 2nd elementof the DLL is a DLL itself.. inception!
    while (e) {
        t_object *val = DLL_DATA_PTR(e);
        ht_add_num(list_obj->data.ht, list_obj->data.ht->element_count, val);
        e = DLL_NEXT(e);
    }
}

static void obj_free(t_object *obj) {
    t_list_object *list_obj = (t_list_object *)obj;
    if (! list_obj) return;

    if (list_obj->data.ht) {
        ht_destroy(list_obj->data.ht);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_list_object *list_obj = (t_list_object *)obj;

    if (list_obj->data.ht) {
        snprintf(list_obj->__debug_info, DEBUG_INFO_SIZE-1, "list[%d]", list_obj->data.ht->element_count);
    } else {
        snprintf(list_obj->__debug_info, DEBUG_INFO_SIZE-1, "list[]");
    }
    return list_obj->__debug_info;
}
#endif


// List object management functions
t_object_funcs list_funcs = {
        obj_populate,         // Populate a list object
        obj_free,             // Free a list object
        obj_destroy,          // Destroy a list object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};



// Intial object
t_list_object Object_List_struct = {
    OBJECT_HEAD_INIT("list", objectTypeList, OBJECT_TYPE_CLASS, &list_funcs, sizeof(t_list_object_data)),
    {
        NULL,
        {
            0,
        }
    },
    OBJECT_FOOTER
};

