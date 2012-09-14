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
#include "object/regex.h"
#include "general/smm.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>


/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
t_object *object_call(t_object *obj, char *method, int arg_count, ...) {
    t_hash_table_bucket *htb = NULL;
    t_object *cur_obj = obj;
    va_list arg_list;

    // Try and find the correct method (might be found of the bases classes!)

    while (htb == NULL) {
        printf(">>> Finding method '%s' on object %s\n", method, cur_obj->name);

        // Find the method in the current object
        htb = ht_find(cur_obj->methods, method);
        if (htb != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            printf(">>> Cannot call method '%s' on object %s: not found\n", method, obj->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    printf(">>> Calling method '%s' on object %s\n", method, obj->name);
    t_object *(*func)(t_object *) = htb->data;

    va_start(arg_list, arg_count);
    t_object *ret = func(obj, arg_count, arg_list);
    va_end(arg_list);
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
 * Increase reference to object.
 */
void object_inc_ref(t_object *obj) {
    obj->ref_count++;
    printf("Increasing reference for: %s (%08X) to %d\n", obj->name, obj, obj->ref_count);
}


/**
 * Decrease reference from object.
 */
void object_dec_ref(t_object *obj) {
    obj->ref_count--;
    printf("Decreasing reference for: %s (%08X) to %d\n", obj->name, obj, obj->ref_count);
}


/**
 * Free an object (if needed)
 */
void object_free(t_object *obj) {
    if (! obj) return;

    // Decrease reference count and check if we need to free
    object_dec_ref(obj);
    if (obj->ref_count > 0) return;

    printf("Freeing object: %08X (%d) %s\n", obj, obj->flags, obj->name);


    // Don't free if it's a static object
    if ((obj->flags & OBJECT_FLAG_STATIC) == OBJECT_FLAG_STATIC) {
        printf("not allowed to free object: %s\n", obj->name);
        return;
    }

    // Need to free, check if free functions exists
    if (obj->funcs && obj->funcs->free) {
        obj->funcs->free(obj);
    }

    // Free actual object
    smm_free(obj);
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
    object_regex_init();
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
    object_regex_fini();
}


static int _object_parse_arguments(const char *speclist, int arg_count, va_list arg_list) {
    const char *ptr = arguments;
    t_object *obj;
    objectTypeEnum type;
    int optional = 0;

    while (ptr != '\0') {
        char c = *ptr;
        ptr++;
        switch (c) {
            case 'n' : /* numerical */
                type = objectTypeNumerical;
                break;
            case 'N' : /* null */
                type = objectTypeNull;
                break;
            case 's' : /* string */
                type = objectTypeString;
                break;
            case 'r' : /* regex */
                type = objectTypeRegex;
                break;
            case 'b' : /* boolean */
                type = objectTypeBoolean;
                break;
            case 'o' : /* any object */
                type = NULL;
                break;
            case '|' : /* Everything after a | is optional */
                optional = 1;
                continue;
                break;
            default :
                printf("Cannot parse argument '%c'\n", c);
                return -1;
                break;
        }

        // Fetch the next object from the list
        obj = va_arg(arg_list, t_object *);
        if (type != NULL && obj->type != type) {
            printf("Wanted a %d, but got a %s\n", type, obj->type);
            return -1;
        }

    }
}


int object_parse_arguments(int arg_count, va_list args, const char speclist, ...) {
    va_list arg_list;

    // Empty argument list
    if (arg_count == 0) return 0;

    va_start(arg_list, speclist);
    int result = _object_parse_arguments(arg_count, arg_list, speclist);
    va_end(arg_list);

    return result;
}


/**
 * @TODO: Remove me
 */
void test(void) {
    t_object *obj[100];

    // Initialize objects placeholders ("variables") to NULL
    for (int i=0; i!=100; i++) obj[i] = NULL;

    printf("Object init\n");
    object_init();
    setlocale(LC_ALL,"");

    printf("=================\n");
    obj[71] = object_new(Object_Regex, L"(o+)", 0);
    obj[72] = object_new(Object_String, L"foofoobarbaz"));
    obj[73] = object_call(obj[71], "match", 1, obj[72]);
    printf("=================\n");

    printf("=================\n");
    obj[51] = object_new(Object_Regex, L"(o+)", 0);
    obj[70] = object_new(Object_String, L"foofoobarbaz"));
    obj[52] = object_call(obj[51], "match", 1, obj[70]);
    obj[53] = object_call(obj[52], "print", 0);
    printf("=================\n");
    obj[54] = object_new(Object_Regex, L"(ba+)", 0);
    obj[55] = object_call(obj[54], "match", 1, obj[70]);
    obj[56] = object_call(obj[55], "print", 0);
    printf("=================\n");
    obj[58] = object_new(Object_Regex, L"(BA+)", 0);
    obj[59] = object_call(obj[58], "match", 1, obj[70]);
    obj[60] = object_call(obj[59], "print", 0);
    printf("=================\n");
    obj[62] = object_new(Object_Regex, L"(BA+)", PCRE_CASELESS);
    obj[63] = object_call(obj[62], "match", 1, obj[70]);
    obj[64] = object_call(obj[63], "print", 0);
    printf("=================\n");

    printf("=================\n");
    obj[0] = object_new(Object_Numerical, 123);
    obj[99] = object_call(obj[0], "print", 0);
    obj[1] = object_call(obj[0], "boolean", 0);
    obj[2] = object_call(obj[1], "string", 0);
    obj[92] = object_call(obj[2], "print", 0);

    obj[3] = object_new(Object_Numerical, 0);
    obj[98] = object_call(obj[3], "print", 0);
    obj[4] = object_call(obj[3], "boolean", 0);
    obj[5] = object_call(obj[4], "string", 0);
    obj[93] = object_call(obj[5], "print", 0);

    obj[6] = object_new(Object_Numerical, 1234);
    obj[97] = object_call(obj[6], "print", 0);
    obj[7] = object_call(obj[6], "string", 0);
    obj[96] = object_call(obj[7], "print", 0);

    printf("=================\n");
    obj[8] = object_new(Object_String, (void *)L"");
    obj[9] = object_call(obj[8], "print", 0);
    obj[10] = object_call(obj[8], "boolean", 0);
    obj[11] = object_call(obj[10], "string", 0);
    obj[95] = object_call(obj[11], "print", 0);

    obj[12] = object_new(Object_String, (void *)L"false");
    obj[13] = object_call(obj[12], "print", 0);
    obj[14] = object_call(obj[12], "boolean", 0);
    obj[15] = object_call(obj[14], "string", 0);
    obj[94] = object_call(obj[15], "print", 0);

    obj[16] = object_new(Object_String, (void *)L"true");
    obj[17] = object_call(obj[16], "print", 0);
    obj[18] = object_call(obj[16], "boolean", 0);
    obj[19] = object_call(obj[18], "string", 0);
    obj[90] = object_call(obj[19], "print", 0);

    obj[20] = object_new(Object_String, (void *)L"anythingelse");
    obj[21] = object_call(obj[20], "print", 0);
    obj[22] = object_call(obj[20], "boolean", 0);
    obj[23] = object_call(obj[22], "string", 0);
    obj[89] = object_call(obj[23], "print", 0);

    printf("=================\n");
    // Simple numerical object called 12345
    obj[24] = object_new(Object_Numerical, 12345);
    obj[25] = object_call(obj[24], "ctor", 0);
    obj[26] = object_call(obj[24], "print", 0);

    // Return a memory prints (not functioning yet)
    obj[27] = object_call(obj[24], "memory", 0);
    obj[28] = object_call(obj[27], "print", 0);

    // Print parents
    obj[29] = object_call(obj[24], "parents", 0);
    obj[30] = object_call(obj[29], "print", 0);


    obj[31] = object_call(obj[24], "immutable?", 0);
    if (obj[31] == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }
    obj[32] = object_call(obj[24], "immutable", 0);
    obj[33] = object_call(obj[24], "immutable?", 0);
    if (obj[33] == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }


    // negate the numerical value
    obj[34] = object_call(obj[24], "neg", 0);
    obj[36] = object_call(obj[34], "print", 0);


    // Create a string
    obj[35] = object_new(Object_String, (void *)L"Björk Guðmundsdóttir");
    obj[37] = object_call(obj[35], "ctor", 0);
    obj[38] = object_call(obj[35], "print", 0);

    obj[39] = object_call(obj[35], "length", 0);
    printf("Length of the string is: ");
    obj[40] = object_call(obj[39], "print", 0);
    printf("\n");

    obj[41] = object_call(obj[35], "byte_length", 0);
    printf("Byte Length of the string is: ");
    obj[48] = object_call(obj[41], "print", 0);
    printf("\n");

    // Uppercase object
    obj[42] = object_call(obj[35], "upper", 0);
    printf("> obj2::print\n");
    obj[44] = object_call(obj[42], "print", 0);

    // call a method that does not exist
    obj[45] = object_call(obj[42], "sirdoesnotexist", 0);

    // Create a reversed object from the uppercased one
    obj[43] = object_call(obj[42], "reverse", 0);

    // Print reversed uppercase
    obj[46] = object_call(obj[43], "print", 0);

    // Print "normal" string
    obj[47] = object_call(obj[35], "print", 0);


    // Free allocated objects
    for (int i=0; i!=100; i++) {
        if (! obj[i]) continue;
        object_free(obj[i]);
    }

    object_fini();

    exit(1);
}

