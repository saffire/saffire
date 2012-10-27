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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>
#include "object/object.h"
#include "object/string.h"
#include "object/boolean.h"
#include "object/null.h"
#include "object/base.h"
#include "object/numerical.h"
#include "object/regex.h"
#include "general/smm.h"
#include "general/dll.h"
#include "interpreter/errors.h"
#include "debug.h"


#ifdef __DEBUG
    t_dll *object_dll;
    t_hash_table *object_hash;
#endif


// Object type string constants
const char *objectTypeNames[7] = { "object", "base", "boolean", "null", "numerical", "regex", "string" };


int object_is_immutable(t_object *obj) {
    return ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE);
}

/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
t_object *object_call(t_object *obj, char *method, int arg_count, ...) {
    t_hash_table_bucket *htb = NULL;
    t_object *cur_obj = obj;
    va_list arg_list;

    // Try and find the correct method (might be found of the bases classes!)

    while (htb == NULL) {
        DEBUG_PRINT(">>> Finding method '%s' on object %s\n", method, cur_obj->name);

        // Find the method in the current object
        htb = ht_find(cur_obj->methods, method);
        if (htb != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            DEBUG_PRINT(">>> Cannot call method '%s' on object %s: not found\n", method, obj->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    DEBUG_PRINT(">>> Calling method '%s' on object %s\n", method, obj->name);
    t_object *(*func)(t_object *, t_dll *dll) = htb->data;

    // Add all arguments to a DLL
    va_start(arg_list, arg_count);
    t_dll *dll = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(arg_list, t_object *);
        dll_append(dll, obj);
    }
    va_end(arg_list);

    // Call the actual method and return the result
    t_object *ret = func(obj, dll);

    // Free dll
    dll_free(dll);
    return ret;
}


/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
t_object *object_operator(t_object *obj, int operator, int in_place, int arg_count, ...) {
    t_object *cur_obj = obj;
    va_list arg_list;
    t_object *(*func)(t_object *, t_dll *dll, int in_place);

    // Try and find the correct operator (might be found of the base classes!)
    while (cur_obj && cur_obj->operators != NULL) {
        DEBUG_PRINT(">>> Finding operator '%d' on object %s\n", operator, cur_obj->name);

        switch (operator) {
            case OPERATOR_ADD : func = cur_obj->operators->add; break;
            case OPERATOR_SUB : func = cur_obj->operators->sub; break;
            case OPERATOR_MUL : func = cur_obj->operators->mul; break;
            case OPERATOR_DIV : func = cur_obj->operators->div; break;
            case OPERATOR_MOD : func = cur_obj->operators->mod; break;
            case OPERATOR_AND : func = cur_obj->operators->and; break;
            case OPERATOR_OR  : func = cur_obj->operators->or; break;
            case OPERATOR_XOR : func = cur_obj->operators->xor; break;
            case OPERATOR_SHL : func = cur_obj->operators->shl; break;
            case OPERATOR_SHR : func = cur_obj->operators->shr; break;
        }

        // Found a function? We're done!
        if (func) break;

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }


    DEBUG_PRINT(">>> Calling operator %d on object %s\n", operator, obj->name);

    // Add all arguments to a DLL
    va_start(arg_list, arg_count);
    t_dll *dll = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(arg_list, t_object *);
        dll_append(dll, obj);
    }
    va_end(arg_list);

    // Call the actual operator and return the result
    t_object *ret = func(obj, dll, in_place);

    // Free dll
    dll_free(dll);
    return ret;
}

/**
 * Calls an comparison function. Returns true or false
 */
t_object *object_comparison(t_object *obj1, int cmp, t_object *obj2) {
    t_object *cur_obj = obj1;
    int (*func)(t_object *, t_object *);

    // Try and find the correct operator (might be found of the base classes!)
    while (cur_obj && cur_obj->comparisons != NULL) {
        DEBUG_PRINT(">>> Finding comparison '%d' on object %s\n", cmp, cur_obj->name);

        switch (cmp) {
            case COMPARISON_EQ : func = cur_obj->comparisons->eq; break;
            case COMPARISON_NE : func = cur_obj->comparisons->ne; break;
            case COMPARISON_LT : func = cur_obj->comparisons->lt; break;
            case COMPARISON_LE : func = cur_obj->comparisons->le; break;
            case COMPARISON_GT : func = cur_obj->comparisons->gt; break;
            case COMPARISON_GE : func = cur_obj->comparisons->ge; break;
            case COMPARISON_IN : func = cur_obj->comparisons->in; break;
            case COMPARISON_NI : func = cur_obj->comparisons->ni; break;
        }

        // Found a function? We're done!
        if (func) break;

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }


    DEBUG_PRINT(">>> Calling comparison %d on object %s\n", cmp, obj1->name);

    // Call the actual equality operator and return the result
    int ret = func(obj1, obj2);

    DEBUG_PRINT("Result from the comparison: %d\n", ret);

    return ret ? Object_True : Object_False;
}


/**
 * Clones an object and returns new object
 */
t_object *object_clone(t_object *obj) {
    DEBUG_PRINT("Cloning: %s\n", obj->name);

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
    DEBUG_PRINT("Increasing reference for: %s (%08X) to %d\n", obj->name, obj, obj->ref_count);
}


/**
 * Decrease reference from object.
 */
void object_dec_ref(t_object *obj) {
    obj->ref_count--;
    DEBUG_PRINT("Decreasing reference for: %s (%08X) to %d\n", obj->name, obj, obj->ref_count);
}


char *object_debug(t_object *obj) {
    if (obj && obj->funcs && obj->funcs->debug) {
        return obj->funcs->debug(obj);
    }
    return "";
}

/**
 * Free an object (if needed)
 */
void object_free(t_object *obj) {
    if (! obj) return;

    // Decrease reference count and check if we need to free
//    object_dec_ref(obj);
    if (obj->ref_count > 0) return;

    DEBUG_PRINT("Freeing object: %08X (%d) %s\n", obj, obj->flags, obj->name);


    // Don't free if it's a static object
    if ((obj->flags & OBJECT_FLAG_STATIC) == OBJECT_FLAG_STATIC) {
        saffire_warning("not allowed to free object: %s\n", obj->name);
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

    DEBUG_PRINT("Instantiating a new object: %s\n", obj->name);

    // Return NULL when we cannot 'new' this object
    if (! obj || ! obj->funcs || ! obj->funcs->new) RETURN_NULL;

    va_start(arg_list, obj);
    t_object *res = obj->funcs->new(arg_list);
    va_end(arg_list);

#ifdef __DEBUG
    char addr[10];
    sprintf(addr, "%08X", res);
    if (! ht_find(object_hash, (char *)&addr)) {
        dll_append(object_dll, res);
        ht_add(object_hash, (char *)&addr, (void *)1);
    }
#endif

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

#ifdef __DEBUG
    object_dll = dll_init();
    object_hash = ht_create();
    dll_append(object_dll, Object_True);
    dll_append(object_dll, Object_False);
    dll_append(object_dll, Object_Null);
#endif

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


int object_parse_arguments(t_dll *dll, const char *speclist, ...) {
    const char *ptr = speclist;
    int optional_argument = 0;
    va_list storage_list;
    objectTypeEnum type;
    int result = 0;
    t_object *obj;

    va_start(storage_list, speclist);

    // Point to first element
    t_dll_element *e = DLL_HEAD(dll);

    // First, check if the number of elements equals (or is more) than the number of mandatory objects in the speclist
    int cnt = 0;
    while (*ptr) {
        if (*ptr == '|') break;
        cnt++;
        ptr++;
    }
    if (cnt < dll->size) {
        DEBUG_PRINT("At least %d arguments are needed. Only %d are given", cnt, dll->size);
        result = 0;
        goto done;
    }

    // We know have have enough elements. Iterate the speclist
    ptr = speclist;
    while (*ptr) {
        char c = *ptr; // Save current spec character
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
                type = objectTypeAny;
                break;
            case '|' : /* Everything after a | is optional */
                optional_argument = 1;
                continue;
                break;
            default :
                saffire_warning("Cannot parse argument '%c'\n", c);
                result = 0;
                goto done;
                break;
        }

        // Fetch the next object from the list. We must assume the user has added enough room
        t_object **storage_obj = va_arg(storage_list, t_object **);
        t_object *argument_obj = e->data;
        if (type != objectTypeAny && type != argument_obj->type) {
            saffire_warning("Wanted a %s, but got a %s\n", objectTypeNames[type], objectTypeNames[argument_obj->type]);
            result = 0;
            goto done;
        }

        // Copy this object to here
        *storage_obj = argument_obj;

        // Goto next element
        e = e->next;
    }

    // Everything is ok
    result = 1;

    // General cleanup
done:
    va_end(storage_list);
    return result;
}

