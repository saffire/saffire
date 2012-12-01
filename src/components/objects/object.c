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
#include <string.h>
#include "objects/object.h"
#include "objects/string.h"
#include "objects/boolean.h"
#include "objects/null.h"
#include "objects/base.h"
#include "objects/numerical.h"
#include "objects/regex.h"
#include "objects/method.h"
#include "objects/code.h"
#include "objects/hash.h"
#include "objects/tuple.h"
#include "general/dll.h"
#include "debug.h"
#include "gc/gc.h"
#include "general/output.h"

// @TODO: in_place: is this option really needed? (inplace modifications of object, like A++; or A = A + 2;)

#ifdef __DEBUG
    t_hash_table *object_hash;
#endif


// Object type string constants
const char *objectTypeNames[OBJECT_TYPE_LEN] = { "object", "code", "method", "base", "boolean",
                                                 "null", "numerical", "regex", "string", "custom",
                                                 "hash", "tuple" };


int object_is_immutable(t_object *obj) {
    return ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE);
}

/**
 * Checks and returns the correct object that holds the method (if any)
 */
static t_object *_find_method(t_object *obj, char *method_name) {
    // Try and find the correct method (might be found of the bases classes!)
    t_object *method = NULL;
    t_object *cur_obj = obj;

    while (method == NULL) {
        DEBUG_PRINT(">>> Finding method '%s' on object %s\n", method_name, cur_obj->name);

        // Find the method in the current object
        method = ht_find(cur_obj->methods, method_name);
        if (method != NULL) break;

        // Not found and there is no parent, we're done!
        if (cur_obj->parent == NULL) {
            DEBUG_PRINT(">>> Cannot call method '%s' on object %s: not found\n", method_name, obj->name);
            return NULL;
        }

        // Try again in the parent object
        cur_obj = cur_obj->parent;
    }

    DEBUG_PRINT(">>> Calling method '%s' on object %s, actually: %s\n", method_name, obj->name, cur_obj->name);

    if (OBJECT_TYPE_IS_CLASS(cur_obj)) {
        DEBUG_PRINT(">>> This is a CLASS\n");
    }
    if (OBJECT_TYPE_IS_INSTANCE(cur_obj)) {
        DEBUG_PRINT(">>> This is a INSTANCE\n");
    }

    return method;
}


/**
 *
 */
t_object *object_find_method(t_object *obj, char *method_name) {
    t_object *method = _find_method(obj, method_name);
    if (! method) return NULL;
    return method;
}


/**
 * OBJECT_CALL FUNCTIONS ARE ONLY USED IN THE OBSOLETE INTERPRETER CODE! REMOVE THIS!
 *
 * Calls a method from specified object, but with a argument list. Returns NULL when method is not found.
 */
t_object *object_call_args(t_object *self, t_object *method_obj, t_dll *args) {
    t_object *ret = NULL;

    // @TODO: It should be a callable method


    // @TODO: Maybe check just for callable?
    if (! OBJECT_IS_METHOD(method_obj)) {
        error_and_die(1, "Object returned in this method is not a method, so I cannot call this!");
    }

    t_method_object *method = (t_method_object *)method_obj;

    DEBUG_PRINT("MFLAGS: %d\n", method->mflags);
    DEBUG_PRINT("OFLAGS: %d\n", method->class->flags);


    // Code object present inside method?
    t_code_object *code = (t_code_object *)method->code;
    if (! code) {
        error_and_die(1, "Code object from method is not present!");
    }

    /*
     * Everything is hunky-dory. Make the call
     */

    // @TODO: move this to the code-object
    if (code->native_func) {
        // Internal function
        ret = code->native_func(self, args);
    } else if (code->bytecode) {
        // External function found in AST
        // @TODO: How do we send our arguments?
        //ret = interpreter_leaf(code->p);
    } else {
        error_and_die(1, "Sanity error: code object has no code");
    }

    return ret;
}

/**
* Calls a method from specified object. Returns NULL when method is not found.
*/
t_object *object_call(t_object *self, t_object *method_obj, int arg_count, ...) {
    // Add all arguments to a DLL
    va_list arg_list;
    va_start(arg_list, arg_count);
    t_dll *dll = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(arg_list, t_object *);
        dll_append(dll, obj);
    }
    va_end(arg_list);

    t_object *ret = object_call_args(self, method_obj, dll);

    // Free dll
    dll_free(dll);
    return ret;
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

//    // Add all arguments to a DLL
//    va_start(arg_list, arg_count);
//    t_dll *dll = dll_init();
//    for (int i=0; i!=arg_count; i++) {
//        t_object *obj = va_arg(arg_list, t_object *);
//        dll_append(dll, obj);
//    }
//    va_end(arg_list);

    va_start(arg_list, arg_count);
    if (arg_count != 1) {
        error_and_die(1, "Operators must have one argument!");
        return Object_False;
    }
    t_object *obj2 = va_arg(arg_list, t_object *);

    // Call the actual operator and return the result
    t_object *ret = func(obj, obj2, in_place);

//    // Free dll
//    dll_free(dll);
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
    obj->ref_count++;
    DEBUG_PRINT("Increasing reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);
}


/**
 * Decrease reference from object.
 */
void object_dec_ref(t_object *obj) {
    obj->ref_count--;
    DEBUG_PRINT("Decreasing reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);

    if(obj->ref_count == 0) {
        // Free object
        if ((obj->flags & OBJECT_FLAG_STATIC) != OBJECT_FLAG_STATIC) {
            object_free(obj);
        } else {
            DEBUG_PRINT(" *** STATIC, SO NOT FREEING\n");
        }
    }

}

#ifdef __DEBUG
char *object_debug(t_object *obj) {
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
    if (! OBJECT_IS_CODE(obj) && ! OBJECT_IS_METHOD(obj)) {
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
 * Creates a new object with specific values
 */
t_object *object_new(t_object *obj, ...) {
    t_object *res;
    va_list arg_list;
    int cached;

    // Return NULL when we cannot 'new' this object
    if (! obj || ! obj->funcs || ! obj->funcs->new) return NULL;

    // Create or recycle an object from this type
    res = gc_queue_recycle(obj->type);
    if (res != NULL) {
        cached = 1;
    } else {
        res = obj->funcs->new();
        cached = 0;
    }

    // Populate internal values
    if (res->funcs->populate) {
        va_start(arg_list, obj);
        res->funcs->populate(res, arg_list);
        va_end(arg_list);
    }

    if (cached) {
        DEBUG_PRINT("Using a cached instance: %s\n", object_debug(res));
    } else {
        if (! OBJECT_IS_CODE(obj) && ! OBJECT_IS_METHOD(obj)) {
            DEBUG_PRINT("Creating a new instance: %s\n", object_debug(res));
        }
    }

#ifdef __DEBUG
//    if (! ht_num_find(object_hash, (unsigned long)res)) {
//        ht_num_add(object_hash, (unsigned long)res, res);
//    }
#endif

    return res;
}


/**
 * Initialize all the (scalar) objects
 */
void object_init() {
#ifdef __DEBUG
    object_hash = ht_create();
#endif

    object_base_init();
    object_boolean_init();
    object_null_init();
    object_numerical_init();
    object_string_init();
    object_regex_init();
    object_code_init();
    object_method_init();
    object_hash_init();
    object_tuple_init();

#ifdef __DEBUG
    ht_num_add(object_hash, (unsigned long)Object_True, Object_True);
    ht_num_add(object_hash, (unsigned long)Object_False, Object_False);
    ht_num_add(object_hash, (unsigned long)Object_Null, Object_Null);
#endif

}


/**
 * Finalize all the (scalar) objects
 */
void object_fini() {
#ifdef __DEBUG
    ht_destroy(object_hash);
#endif

    object_base_fini();
    object_boolean_fini();
    object_null_fini();
    object_numerical_fini();
    object_string_fini();
    object_regex_fini();
    object_code_fini();
    object_method_fini();
    object_hash_fini();
    object_tuple_fini();
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
 *
 */
void object_add_external_method(void *obj, char *method_name, int flags, int visibility, t_ast_element *p) {
    t_object *the_obj = (t_object *)obj;

    t_code_object *code = (t_code_object *)object_new(Object_Code, p, NULL);
    t_method_object *method = (t_method_object *)object_new(Object_Method, flags, visibility, obj, code);

    ht_add(the_obj->methods, method_name, method);
}


/**
 *
 */
void object_add_internal_method(void *obj, char *method_name, int flags, int visibility, void *func) {
    t_object *the_obj = (t_object *)obj;

    t_code_object *code = (t_code_object *)object_new(Object_Code, NULL, func);
    t_method_object *method = (t_method_object *)object_new(Object_Method, flags, visibility, obj, code);

    ht_add(the_obj->methods, method_name, method);
}

void object_remove_all_internal_methods(t_object *obj) {
    t_hash_iter iter;

    ht_iter_init(&iter, obj->methods);
    while (ht_iter_valid(&iter)) {
        t_method_object *method = ht_iter_value(&iter);

        // @TODO: We assume here that method and method->code are always used once. This does not have to be the case!
        object_free((t_object *)method->code);
        object_free((t_object *)method);

        ht_iter_next(&iter);
    }
}