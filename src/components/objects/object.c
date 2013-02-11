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
#include <string.h>
#include "objects/object.h"
#include "objects/string.h"
#include "objects/boolean.h"
#include "objects/null.h"
#include "objects/base.h"
#include "objects/numerical.h"
#include "objects/regex.h"
#include "objects/attrib.h"
#include "objects/callable.h"
#include "objects/hash.h"
#include "objects/tuple.h"
#include "objects/userland.h"
#include "general/dll.h"
#include "debug.h"
#include "gc/gc.h"
#include "general/output.h"
#include "general/smm.h"

// @TODO: in_place: is this option really needed? (inplace modifications of object, like A++; or A = A + 2;)

// Object type string constants
const char *objectTypeNames[OBJECT_TYPE_LEN] = { "object", "code", "attribute", "base", "boolean",
                                                 "null", "numerical", "regex", "string",
                                                 "hash", "tuple", "callable", "list", "exception" };


int object_is_immutable(t_object *obj) {
    return ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE);
}


/**
 * Finds the attribute inside the object, or any base objects if needed.
 */
t_object *object_find_attribute(t_object *obj, char *attr_name) {
    t_object *attr = object_find_actual_attribute(obj, attr_name);
    return attr ? ((t_attrib_object *)attr)->attribute : NULL;
}


/**
 * Finds the attribute inside the object, or any base objects if needed.
 */
t_object *object_find_actual_attribute(t_object *obj, char *attr_name) {
    t_object *attr = NULL;
    t_object *cur_obj = obj;

    while (attr == NULL) {
        // DEBUG_PRINT(">>> Finding attribute '%s' on object %s\n", attr_name, cur_obj->name);

        // Find the attribute in the current object
        attr = ht_find(cur_obj->attributes, attr_name);
        if (attr != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            DEBUG_PRINT(">>> Cannot find attribute '%s' in object %s:\n", attr_name, obj->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    // DEBUG_PRINT(">>> Found attribute '%s' in object %s (actually found in object %s)\n", attr_name, obj->name, cur_obj->name);

    return attr;
}


int object_instance_of(t_object *obj, const char *instance) {
    DEBUG_PRINT("object_instance_of(%s, %s)\n", obj->name, instance);

    t_object *cur_obj = obj;
    while (cur_obj != NULL) {
        DEBUG_PRINT("  * Checking: %s against %s\n", cur_obj->name, instance);
        // Check if name of object matches instance
        if (strcmp(cur_obj->name, instance) == 0) {
            return 1;
        }

        // @TODO: Also check interfaces

        // Trickle down to parent object and try again
        cur_obj = cur_obj->parent;
    }

    // Nothing found that matches :/
    return 0;
}


/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
t_object *object_operator(t_object *obj, int opr, int in_place, int arg_count, ...) {
    t_object *cur_obj = obj;
    va_list arg_list;
    t_object *(*func)(t_object *, t_object *, int) = NULL;

    // Try and find the correct operator (might be found of the base classes!)
    while (cur_obj && cur_obj->operators != NULL) {
        DEBUG_PRINT(">>> Finding operator '%d' on object %s\n", opr, cur_obj->name);

        switch (opr) {
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

    if (!func) {
        // No comparison found for this method
        error_and_die(1, "Cannot find operator method");
        return Object_False;
    }

    DEBUG_PRINT(">>> Calling operator %d on object %s\n", opr, obj->name);

    va_start(arg_list, arg_count);
    if (arg_count != 1) {
        error_and_die(1, "Operators must have one argument!");
        return Object_False;
    }
    t_object *obj2 = va_arg(arg_list, t_object *);

    // Call the actual operator and return the result
    t_object *ret = func(obj, obj2, in_place);

    return ret;
}

/**
 * Calls an comparison function. Returns true or false
 */
t_object *object_comparison(t_object *obj1, int cmp, t_object *obj2) {
    t_object *cur_obj = obj1;
    int (*func)(t_object *, t_object *) = NULL;

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

    if (!func) {
        // No comparison found for this method
        error_and_die(1, "Cannot find compare method");
        return Object_False;
    }


    DEBUG_PRINT(">>> Calling comparison %d on object %s\n", cmp, obj1->name);

    // Call the actual equality operator and return the result
    int ret = func(obj1, obj2);

    DEBUG_PRINT("Result from the comparison: %d\n", ret);

    return ret ? Object_True : Object_False;
}


/**
 * Increase reference to object.
 */
void object_inc_ref(t_object *obj) {
    if (! obj) return;

    obj->ref_count++;
//    DEBUG_PRINT("Increasing reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);
}


/**
 * Decrease reference from object.
 */
void object_dec_ref(t_object *obj) {
    if (! obj) return;

    obj->ref_count--;
//    DEBUG_PRINT("Decreasing reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);

    if(obj->ref_count == 0) {
        // Free object
        if ((obj->flags & OBJECT_FLAG_STATIC) != OBJECT_FLAG_STATIC) {
//            DEBUG_PRINT(" *** WOULD BE FREED, BUT WE DONT YET\n");
            // @TODO: Free objects!
            //object_free(obj);
        }
    }

}

#ifdef __DEBUG
char *object_debug(t_object *obj) {
    if (! obj) return "(no debug info)";

    if (obj && obj->funcs && obj->funcs->debug) {
        return obj->funcs->debug(obj);
    }
    return obj->name;
}
#endif

/**
 * Free an object (if needed)
 */
void object_free(t_object *obj) {
    if (! obj) return;

    // Check if we really need to free
    if (obj->ref_count > 0) return;

#ifdef __DEBUG
    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
        DEBUG_PRINT("Freeing object: %08lX (%d) %s\n", (unsigned long)obj, obj->flags, object_debug(obj));
    }
#endif

    // Need to free, check if free functions exists
    if (obj->funcs && obj->funcs->free) {
        obj->funcs->free(obj);
    }

    if (! gc_queue_add(obj) && obj->funcs && obj->funcs->destroy) {
        obj->funcs->destroy(obj);
    }
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
 *
 */
static t_object *_object_new(t_object *obj) {
    t_object *res;

    // Return NULL when we cannot 'new' this object
    if (! obj || ! obj->funcs || ! obj->funcs->new) return NULL;

    // Create or recycle an object from this type
    res = gc_queue_recycle(obj->type);
    if (! res) {
        res = obj->funcs->new(obj);
    }
    return res;
}


/**
 * Creates a new object with specific values, with a already created
 * argument DLL list.
 */
t_object *object_new_with_dll_args(t_object *obj, t_dll *arguments) {
    t_object *res = _object_new(obj);

    // Populate internal values
    if (res->funcs->populate) {
        res->funcs->populate(res, arguments);
    }

    return res;
}


/**
 * Creates a new object with specific values
 */
t_object *object_new(t_object *obj, int arg_count, ...) {
    va_list arg_list;

    // Create argument DLL
    t_dll *arguments = dll_init();
    va_start(arg_list, arg_count);
    for (int i=0; i!=arg_count; i++) {
        dll_append(arguments, va_arg(arg_list, void *));
    }
    va_end(arg_list);

    // Create new object
    t_object *new_obj = object_new_with_dll_args(obj, arguments);

    // Free argument DLL
    dll_free(arguments);

    return new_obj;
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
    object_callable_init();
    object_attrib_init();
    object_hash_init();
    object_tuple_init();
    object_userland_init();
    object_list_init();
    object_exception_init();
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
    object_callable_fini();
    object_attrib_fini();
    object_hash_fini();
    object_tuple_fini();
    object_userland_fini();
    object_list_fini();
    object_exception_fini();
}


int object_parse_arguments(t_dll *dll, const char *speclist, ...) {
    const char *ptr = speclist;
    int optional_argument = 0;
    va_list storage_list;
    t_objectype_enum type;
    int result = 0;

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
        DEBUG_PRINT("At least %d arguments are needed. Only %ld are given", cnt, dll->size);
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
                error_and_die(1, "Cannot parse argument '%c'\n", c);
                result = 0;
                goto done;
                break;
        }

        // Fetch the next object from the list. We must assume the user has added enough room
        t_object **storage_obj = va_arg(storage_list, t_object **);
        t_object *argument_obj = e->data;
        if (type != objectTypeAny && type != argument_obj->type) {
            error_and_die(1, "Wanted a %s, but got a %s\n", objectTypeNames[type], objectTypeNames[argument_obj->type]);
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


/**
 * Create method- attribute that points to an INTERNAL (C) function
 */
void object_add_internal_method(t_object *obj, char *name, int method_flags, int visibility, void *func) {
    // @TODO: Instead of NULL, we should be able to add our parameters. This way, we have a more generic way to deal
    //        with internal and external functions.
    t_callable_object *callable_obj = (t_callable_object *)object_new(Object_Callable, 4, method_flags | CALLABLE_CODE_INTERNAL | CALLABLE_TYPE_METHOD, func, NULL, NULL);
    t_attrib_object *attrib_obj = (t_attrib_object *)object_new(Object_Attrib, 4, ATTRIB_TYPE_METHOD, visibility, ATTRIB_ACCESS_RO, callable_obj);

    ht_add(obj->attributes, name, attrib_obj);
}

void object_add_property(t_object *obj, char *name, int visibility, t_object *property) {
    t_attrib_object *attrib = (t_attrib_object *)object_new(Object_Attrib, 4, ATTRIB_TYPE_PROPERTY, visibility, ATTRIB_ACCESS_RW, property);

    ht_replace(obj->attributes, name, attrib);
}

void object_add_constant(t_object *obj, char *name, int visibility, t_object *constant) {
    t_attrib_object *attrib = (t_attrib_object *)object_new(Object_Attrib, 4, ATTRIB_TYPE_CONSTANT, visibility, ATTRIB_ACCESS_RO, constant);

    if (ht_exists(obj->attributes, name)) {
        error_and_die(1, "Attribute '%s' already exists in object '%s'\n", name, obj->name);
    }
    ht_add(obj->attributes, name, attrib);
}


/**
 * Clears up all attributes found in this object
 */
void object_remove_all_internal_attributes(t_object *obj) {
    t_hash_iter iter;

    ht_iter_init(&iter, obj->attributes);
    while (ht_iter_valid(&iter)) {
        // @TODO: We must remove and free attrib-objects here..
        ht_iter_next(&iter);
    }
}