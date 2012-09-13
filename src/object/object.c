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
#include "object/string.h"
#include "object/boolean.h"
#include "object/null.h"
#include "object/base.h"
#include "object/numerical.h"
#include "general/smm.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>

/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
void *object_call(t_object *obj, char *method) {
    printf(">>> Finding method '%s' on object %s\n", method, obj->name);

    t_hash_table_bucket *htb = ht_find(obj->methods, method);
    if (htb == NULL) {
        if (obj->parent == NULL) {
            printf(">>> Cannot call method '%s' on object %s: not found\n", method, obj->name);
            return NULL;
        }

        // Just recurse into the parent
        return object_call(obj->parent, method);
    }

    printf(">>> Calling method '%s' on object %s\n", method, obj->name);
    void *(*func)(t_object *) = htb->data;

    return func(obj);
}

/**
 * Clones an object and returns new object
 */
t_object *object_clone(t_object *obj) {
    printf("Cloning: %s\n", obj->name);

    // No clone function, so return same object
    if (! obj || ! obj->funcs || ! obj->funcs->clone) {
        return obj;
    }

    return obj->funcs->clone(obj);
}


/**
 * Increase reference to object. Returns new reference count
 */
void object_inc_ref(t_object *obj) {
    printf("Increasing reference for: %s\n", obj->name);
    if (obj) obj->ref_count++;
}

/**
 * Decrease reference from object. Returns new reference count
 */
void object_dec_ref(t_object *obj) {
    printf("Decreasing reference for: %s\n", obj->name);
    if (obj) obj->ref_count++;
}



/**
 * Creates a new object with specific values
 */
t_object *object_new(t_object *obj, ...) {
    va_list arg_list;

    printf("Instantiating a new object: %s\n", obj->name);

    // Return NULL when we cannot 'new' this object
    if (! obj || ! obj->funcs || ! obj->funcs->new) RETURN_NULL;

    va_start(arg_list, obj);
    t_object *res = obj->funcs->new(arg_list);
    va_end(arg_list);

    return res;
}


/**
 * Initialize all the (scalar) objects
 */
void object_init() {
    object_base_init();
    object_boolean_init();
    object_null_init();
    object_numerical_init();
    object_string_init();
}

/**
 * Finalize all the (scalar) objects
 */
void object_fini() {
    object_base_fini();
    object_boolean_fini();
    object_null_fini();
    object_numerical_fini();
    object_string_fini();
}



/**
 * @TODO: Remove me
 */
void test(void) {
    t_object *res;

    printf("Object init\n");
    object_init();
    setlocale(LC_ALL,"");

    // Simple numerical object called 12345
    t_object *obj_n = object_new(Object_Numerical, 12345);
    res = object_call(obj_n, "ctor");
    res = object_call(obj_n, "print");

    // Return a memory prints (not functioning yet)
    res = object_call(obj_n, "memory");
    res = object_call(res, "print");

    // Print parents
    res = object_call(obj_n, "parents");
    res = object_call(res, "print");


    res = object_call(obj_n, "immutable?");
    if (res == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }
    res = object_call(obj_n, "immutable");
    res = object_call(obj_n, "immutable?");
    if (res == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }



    // negate the numerical value
    t_object *obj_n2 = object_call(obj_n, "neg");
    res = object_call(obj_n2, "print");


    // Create a string
    t_object *obj = object_new(Object_String, (void *)L"Björk Guðmundsdóttir");
    res = object_call(obj, "ctor");
    res = object_call(obj, "print");

    res = object_call(obj, "length");
    printf("Length of the string is: ");
    res = object_call(res, "print");
    printf("\n");

    res = object_call(obj, "byte_length");
    printf("Byte Length of the string is: ");
    res = object_call(res, "print");
    printf("\n");

    // Uppercase object
    t_object *obj2 = object_call(obj, "upper");
    printf("> obj2::print\n");
    res = object_call(obj2, "print");

    // call a method that does not exist
    res = object_call(obj2, "sirdoesnotexist");

    // Create a reversed object from the uppercased one
    t_object *obj3 = object_call(obj2, "reverse");

    // Print reversed uppercase
    res = object_call(obj3, "print");

    // Print "normal" string
    res = object_call(obj, "print");

    exit(1);
}

