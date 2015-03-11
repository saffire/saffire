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
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <saffire/general/string.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/debug.h>
#include <saffire/gc/gc.h>
#include <saffire/general/output.h>
#include <saffire/general/smm.h>
#include <saffire/vm/thread.h>

// @TODO: in_place: is this option really needed? (inplace modifications of object, like A++; or A = A + 2;)

// Object type string constants
const char *objectTypeNames[OBJECT_TYPE_LEN] = { "object", "callable", "attribute", "base", "boolean",
                                                 "null", "numerical", "regex", "string",
                                                 "hash", "tuple", "list", "exception", "user" };

// Object comparison methods. These should map on the COMPARISON_* defines
const char *objectCmpMethods[9] = { "__cmp_eq", "__cmp_ne", "__cmp_lt", "__cmp_gt", "__cmp_le", "__cmp_ge",
                                    "__cmp_in", "__cmp_ni", "__cmp_ex" };

// Object operator methods. These should map on the OPERATOR_* defines
const char *objectOprMethods[10] = { "__opr_add", "__opr_sub", "__opr_mul", "__opr_div", "__opr_mod",
                                     "__opr_and", "__opr_or", "__opr_xor", "__opr_shl", "__opr_shr" };



// Include generated interfaces
#include "_generated_interfaces.inc"


int object_is_immutable(t_object *obj) {
    return ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE);
}


/**
 * Checks if an object is an instance of a class. Will check against parents too
 */
int object_instance_of(t_object *obj, const char *instance) {
    DEBUG_PRINT_CHAR("object_instance_of(%s, %s)\n", obj->name, instance);

    t_object *cur_obj = obj;
    while (cur_obj != NULL) {
        DEBUG_PRINT_CHAR("  *   Checking: %s against %s\n", cur_obj->name, instance);
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
 * Free an object (if needed)
 */
static void _object_free(t_object *obj) {
    if (! obj) return;

    // ref_count > 0, object is still in use somewhere else. Don't free it yet
    if (obj->ref_count > 0) return;

#ifdef __DEBUG
    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
        DEBUG_PRINT_CHAR("Freeing object: %p\n", obj);
    }
#endif

    // Free attributes
    t_hash_iter iter;
    ht_iter_init(&iter, obj->attributes);
    while (ht_iter_valid(&iter)) {
        // Release attribute
        t_object *attr_obj = (t_object *)ht_iter_value(&iter);
        object_release(attr_obj);

        ht_iter_next(&iter);
    }
    ht_destroy(obj->attributes);
    obj->attributes = NULL;

    // Free interfaces
    if (obj->interfaces) {
        t_dll_element *interface = DLL_HEAD(obj->interfaces);
        while (interface) {
            object_release((t_object *)interface->data.p);
            interface = DLL_NEXT(interface);
        }
        dll_free(obj->interfaces);
        obj->interfaces = NULL;
    }


    // Free values from the object
    if (obj->funcs && obj->funcs->free) {
        obj->funcs->free(obj);
    }

    if (OBJECT_IS_USERLAND(obj)) {
        // Release parent class for userland created objects
        object_release(obj->parent);
        obj->parent = NULL;
    }

    if (OBJECT_IS_ALLOCATED(obj)) {
        // Free name if the object is dynamically allocated
        smm_free(obj->name);
        obj->name = NULL;
    }

    // Free the object itself
    if (obj->funcs && obj->funcs->destroy) {
        obj->funcs->destroy(obj);
    }

    // Object is destroyed. We cannot use object anymore.
    obj = NULL;


    // @TODO: here we should actually add the object to the GC destruction queue.
    // If we need the object later, we can still catch it from this queue, otherwise it will
    // be destroyed somewhere in the future.

//    if (! gc_queue_add(obj) && obj->funcs && obj->funcs->destroy) {
//        obj->funcs->destroy(obj);
//    }
}


t_hash_table *refcount_objects = NULL;

/**
 * Increase reference to object.
 */
void object_inc_ref(t_object *obj) {
    if (! obj) return;

    obj->ref_count++;

    if (refcount_objects == NULL) {
        refcount_objects = ht_create();
    }

    ht_replace_ptr(refcount_objects, (void *)obj, (void *)obj->ref_count);


    if (OBJECT_IS_CALLABLE(obj) || OBJECT_IS_ATTRIBUTE(obj)) return;

#if __DEBUG_REFCOUNT
    DEBUG_PRINT_CHAR("Increased reference for: %s (%p) to %d\n", object_debug(obj), obj, obj->ref_count);
#endif
}


/**
 * Decrease reference from object.
 */
static long object_dec_ref(t_object *obj) {
    if (! obj) return 0;

    if (obj->ref_count == 0) {
        fprintf(stderr, "sanity check failed: ref-count of object %p\n", obj);
        abort();
    }
    obj->ref_count--;

    ht_replace_ptr(refcount_objects, obj, (void *)obj->ref_count);

#if __DEBUG_REFCOUNT
    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
        DEBUG_PRINT_CHAR("Decreased reference for: %s (%08X) to %d\n", object_debug(obj), (intptr_t)obj, obj->ref_count);
    }
#endif

    if (obj->ref_count != 0) return obj->ref_count;

#if __DEBUG_FREE_OBJECT
    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
        DEBUG_PRINT_CHAR("*** Freeing object %s (%08X)\n", object_debug(obj), (intptr_t)obj);
    }
#endif

    // Don't free static objects
    if (! OBJECT_IS_ALLOCATED(obj)) {
#if __DEBUG_FREE_OBJECT
        DEBUG_PRINT_CHAR("*** Not freeing static object %s\n", object_debug(obj));
#endif
        return 0;
    }

    // Free object
    _object_free(obj);
    return 0;
}

#ifdef __DEBUG
char *object_debug(t_object *obj) {
    if (! obj) {
        return "no object";
    }

    if (! obj->funcs || ! obj->funcs->debug) {
        snprintf(obj->__debug_info, 199, "%s[%c](%s)", objectTypeNames[obj->type], OBJECT_TYPE_IS_CLASS(obj) ? 'C' : 'I', obj->name);
        return obj->__debug_info;
    }

    return obj->funcs->debug(obj);
}
#endif


/**
 * Clones an object and returns new object
 */
t_object *object_clone(t_object *obj) {
    DEBUG_PRINT_CHAR("Cloning: %s\n", obj->name);

    // No clone function, so return same object
    if (! obj || ! obj->funcs || ! obj->funcs->clone) {
        return obj;
    }

    return obj->funcs->clone(obj);
}


/**
 *
 * @param class_obj
 * @return
 */
t_hash_table *object_duplicate_attributes(t_object *class_obj, t_object *instance_obj) {
    t_hash_table *duplicated_attributes = ht_create();

    t_hash_iter iter;
    ht_iter_init(&iter, class_obj->attributes);
    while (ht_iter_valid(&iter)) {
        char *name = ht_iter_key_str(&iter);
        t_attrib_object *attrib = ht_iter_value(&iter);

        // Duplicate attribute into new instance
        t_attrib_object *dup_attrib = object_attrib_duplicate(attrib, instance_obj);

        // We "bind" the attribute to this class
        object_attrib_bind(dup_attrib, instance_obj, name);

        // Replace the current attribute with the dupped one
        ht_add_str(duplicated_attributes, name, (void *)dup_attrib);

        // Increase reference to the duplicated attribute
        object_inc_ref((t_object *)dup_attrib);

        ht_iter_next(&iter);
    }

    return duplicated_attributes;
}


char *object_get_hash(t_object *obj) {
    // When there is no hash function, we just use the address of the object
    if (! obj->funcs->hash) {
        char *tmp = (char *)smm_malloc(32);
        snprintf(tmp, 31, "%ld", (long)obj);
        return tmp;
    }

    // Return objects hash
    return obj->funcs->hash(obj);
}



/**
 * Creates a new object with specific values, with a already created
 * argument DLL list.
 */
t_object *object_alloc_args(t_object *obj, t_dll *arguments, int *cached) {
    t_object *res = NULL;

    if (! OBJECT_TYPE_IS_CLASS(obj)) {
        fatal_error(1, "Can only object_alloc_args() from a class object.\n");
    }

    // Nothing found to new, just return NULL object
    if (! obj || ! obj->funcs) {
        RETURN_NULL;
    }

    // If we have a caching function, seek inside that cache first
    if (obj->funcs->cache) {
        res = obj->funcs->cache(obj, arguments);
        if (res) {
            // Set cached flag when the caller wants it
            if (cached) {
                *cached = 1;
            }
            return res;
        }
    }

    // Nothing found in cache, create new object

    // Create new object
    res = smm_malloc(sizeof(t_object) + obj->data_size);
    memcpy(res, obj, sizeof(t_object) + obj->data_size);

    // Since we just allocated the object, it can always be destroyed
    res->flags |= OBJECT_FLAG_ALLOCATED;

    res->ref_count = 0;
    res->class = obj;
    res->name = string_strdup0(obj->name);

    // Populate values, if needed
    if (res->funcs->populate && arguments) {
        res->funcs->populate(res, arguments);
    }

    return res;
}


/**
 * Initialize all the (scalar) objects
 */
void object_init() {
    // Attrib cannot have any callables, as callable hasn't been initialized yet
    object_attrib_init();
    // Callable can only have attribs
    object_callable_init();
    object_base_init();
    object_interfaces_init();
    object_hash_init();

    object_string_init();
    object_boolean_init();
    object_null_init();
    object_numerical_init();
    object_regex_init();
    object_tuple_init();
    object_list_init();
    object_exception_init();
}


/**
 * Finalize all the (scalar) objects
 */
void object_fini() {
    DEBUG_PRINT_CHAR("object fini\n");

    object_exception_fini();
    object_list_fini();
    object_tuple_fini();
    object_regex_fini();
    object_numerical_fini();
    object_null_fini();
    object_boolean_fini();
    object_string_fini();

    object_hash_fini();
    object_interfaces_fini();
    object_base_fini();
    object_callable_fini();
    object_attrib_fini();
}


/**
 *
 */
int object_parse_arguments(t_dll *dll, const char *spec, ...) {
    const char *ptr = spec;
    int optional_argument = 0;
    va_list storage_list;
    t_objectype_enum type;
    int result = 0;

    va_start(storage_list, spec);

    // Point to first element
    t_dll_element *e = DLL_HEAD(dll);

    // First, check if the number of elements equals (or is more) than the number of mandatory objects in the spec
    int cnt = 0;
    while (*ptr) {
        if (*ptr == '|') break;
        cnt++;
        ptr++;
    }
    if (dll->size < cnt) {
        object_raise_exception(Object_ArgumentException, 1, "Error while parsing argument list: at least %d arguments are needed. Only %ld are given", cnt, dll->size);
        result = 0;
        goto done;
    }

    // We know have have enough elements. Iterate the spec
    ptr = spec;
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
                object_raise_exception(Object_SystemException, 1, "Error while parsing argument list: cannot parse argument: '%c'", c);
                result = 0;
                goto done;
                break;
        }

        // Fetch the next object from the list. We must assume the user has added enough room
        t_object **storage_obj = va_arg(storage_list, t_object **);
        if (optional_argument == 0 && (!e || !e->data.p)) {
            object_raise_exception(Object_ArgumentException, 1, "Error while fetching mandatory argument.");
            result = 0;
            goto done;
        }

        t_object *argument_obj = e ? e->data.p : NULL;
        if (argument_obj && type != objectTypeAny && type != argument_obj->type) {
            object_raise_exception(Object_ArgumentException, 1, "Error while parsing argument list: wanted a %s, but got a %s", objectTypeNames[type], objectTypeNames[argument_obj->type]);
            result = 0;
            goto done;
        }

        // Copy this object to here
        *storage_obj = argument_obj;

        // Goto next element
        e = e ? DLL_NEXT(e) : NULL;
    }

    // Everything is ok
    result = 1;

    // General cleanup
done:
    va_end(storage_list);
    return result;
}


/**
 * Adds interface to object (class)
 */
void object_add_interface(t_object *class, t_object *interface) {
    if (! OBJECT_TYPE_IS_CLASS(class)) {
        fatal_error(1, "Interface can only be added to a class\n");     /* LCOV_EXCL_LINE */
    }

    if (! OBJECT_TYPE_IS_INTERFACE(interface)) {
        fatal_error(1, "%s is not an interface\n", interface->name);        /* LCOV_EXCL_LINE */
    }

    // Initialize interfaces DLL if not done already
    if (! class->interfaces) {
        class->interfaces = dll_init();
    }

    // Add object to interface
    dll_append(class->interfaces, interface);
    object_inc_ref(interface);
}

/**
 * Create method- attribute that points to an INTERNAL (C) function
 */
void object_add_internal_method(t_hash_table *attributes, t_object *obj, char *name, int method_flags, int visibility, void *func) {
    // @TODO: Instead of NULL, we should be able to add our parameters. This way, we have a more generic way to deal with internal and external functions.
    t_callable_object *callable_obj = (t_callable_object *)object_alloc_instance(Object_Callable, 3, CALLABLE_CODE_INTERNAL, func, /* arguments */ NULL);

    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc_instance(Object_Attrib, 7, obj, name, ATTRIB_TYPE_METHOD, visibility, ATTRIB_ACCESS_RO, callable_obj, method_flags);

    /* We don't add the attributes directly to the obj, but we store them inside attributes. Otherwise we run into trouble bootstrapping the callable and attrib objects
     * (as we need to create callables during the creation of callables in callable_init, for instance). By storing them separately inside an attribute hash, and adding the
     * hash when we are finished with the object, it works (we can't do any calls to the callables in between, but we are not allowed to anyway). */
    ht_add_str(attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}


/**
 *
 */
void object_add_property(t_hash_table *attributes, t_object *obj, char *name, int visibility, t_object *property) {
    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc_instance(Object_Attrib, 7, obj, name, ATTRIB_TYPE_PROPERTY, visibility, ATTRIB_ACCESS_RW, property, 0);

    /* We don't add the attributes directly to the obj, but we store them inside attributes. Otherwise we run into trouble bootstrapping the callable and attrib objects
     * (as we need to create callables during the creation of callables in callable_init, for instance). By storing them separately inside an attribute hash, and adding the
     * hash when we are finished with the object, it works (we can't do any calls to the callables in between, but we are not allowed to anyway). */
    ht_add_str(attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}


/**
 *
 */
void object_add_constant(t_hash_table *attributes, t_object *obj, char *name, int visibility, t_object *constant) {
    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc_instance(Object_Attrib, 7, obj, name, ATTRIB_TYPE_CONSTANT, visibility, ATTRIB_ACCESS_RO, constant, 0);

    /* We don't add the attributes directly to the obj, but we store them inside attributes. Otherwise we run into trouble bootstrapping the callable and attrib objects
     * (as we need to create callables during the creation of callables in callable_init, for instance). By storing them separately inside an attribute hash, and adding the
     * hash when we are finished with the object, it works (we can't do any calls to the callables in between, but we are not allowed to anyway). */

    if (ht_exists_str(attributes, name)) {
        object_release((t_object *)attrib_obj);
        fatal_error(1, "Attribute '%s' already exists in object '%s'\n", name, obj->name);      /* LCOV_EXCL_LINE */
    }

    ht_add_str(attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}





static void _object_remove_all_internal_interfaces(t_object *obj) {
    if (! obj->interfaces) return;

    t_dll_element *e = DLL_HEAD(obj->interfaces);
    while (e) {
        object_release((t_object *)e->data.p);
        e = DLL_NEXT(e);
    }
}


/**
 * Clears up all attributes found in this object. Note: does NOT release the object's attributes hash-table!
 */
static void _object_remove_all_internal_attributes(t_object *obj) {
    t_hash_iter iter;

    ht_iter_init(&iter, obj->attributes);
    while (ht_iter_valid(&iter)) {
        object_release((t_object *)ht_iter_value(&iter));
        ht_iter_next(&iter);
    }
}


/**
 * Frees internal object data
 */
void object_free_internal_object(t_object *obj) {
    // Free attributes
    if (obj->attributes) {
        _object_remove_all_internal_attributes(obj);
        ht_destroy(obj->attributes);
    }

    // Remove interfaces
    if (obj->interfaces) {
        _object_remove_all_internal_interfaces(obj);
        dll_free(obj->interfaces);
    }
}



/**
 *
 */
void object_raise_exception(t_object *exception, int code, char *format, ...) {
    va_list args;
    char *buf;

    va_start(args, format);
    smm_vasprintf_char(&buf, format, args);
    va_end(args);

    thread_create_exception((t_exception_object *)exception, code, buf);
    smm_free(buf);
}


static int _object_check_matching_arguments(t_callable_object *obj1, t_callable_object *obj2) {
    t_hash_table *ht1 = obj1->data.arguments;
    t_hash_table *ht2 = obj2->data.arguments;

    // Sanity check
    if (ht1->element_count != ht2->element_count) {
        return 0;
    }

    // No parameters found at all.
    if (ht1->element_count == 0) return 1;


    t_hash_iter iter1;
    t_hash_iter iter2;
    ht_iter_init(&iter1, ht1);
    ht_iter_init(&iter2, ht2);
    while (ht_iter_valid(&iter1)) {
        t_method_arg *arg1 = ht_iter_value(&iter1);
        t_method_arg *arg2 = ht_iter_value(&iter2);

        DEBUG_PRINT_STRING_ARGS("        - typehint1: '%-20s (%d)'  value1: '%-20s' \n", OBJ2STR(arg1->typehint), object_debug(arg1->value));
        DEBUG_PRINT_STRING_ARGS("        - typehint2: '%-20s (%d)'  value2: '%-20s' \n", OBJ2STR(arg2->typehint), object_debug(arg2->value));

        if (object_string_compare(arg1->typehint, arg2->typehint) != 0) {
            return 0;
        }

        ht_iter_next(&iter1);
        ht_iter_next(&iter2);
    }
    return 1;
}


static int _object_check_interface_implementations(t_object *obj, t_object *interface) {
    // ceci ne pas une interface
    if (! OBJECT_TYPE_IS_INTERFACE(interface)) {
        return 0;
    }

    // iterate all attributes from the interface
    t_hash_iter iter;
    ht_iter_init(&iter, interface->attributes);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key_str(&iter);
        t_attrib_object *attribute = (t_attrib_object *)ht_iter_value(&iter);
        DEBUG_PRINT_STRING_ARGS(ANSI_BRIGHTBLUE "    interface attribute '" ANSI_BRIGHTGREEN "%s" ANSI_BRIGHTBLUE "' : " ANSI_BRIGHTGREEN "%s" ANSI_RESET "\n", key, object_debug((t_object *)attribute));

        t_attrib_object *found_obj = (t_attrib_object *)object_attrib_find(obj, key);
        if (! found_obj) {
            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Class '%s' does not fully implement interface '%s', missing attribute '%s'", obj->name, interface->name, key);
            return 0;
        }

        DEBUG_PRINT_STRING_ARGS("     - Found object : %s\n", object_debug((t_object *)found_obj));
        DEBUG_PRINT_STRING_ARGS("     - Matching     : %s\n", object_debug((t_object *)attribute));

        if (found_obj->data.attr_type != attribute->data.attr_type ||
            found_obj->data.attr_visibility != attribute->data.attr_visibility ||
            found_obj->data.attr_access != attribute->data.attr_access) {
            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Class '%s' does not fully implement interface '%s', mismatching settings for attribute '%s'", obj->name, interface->name, key);
            return 0;
        }

        // If we are a callable, check arguments
        if (OBJECT_IS_CALLABLE(found_obj->data.attribute)) {
            DEBUG_PRINT_CHAR("     - Checking parameter signatures\n");
            if (!_object_check_matching_arguments((t_callable_object *)attribute->data.attribute, (t_callable_object *)found_obj->data.attribute)) {
                thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Class '%s' does not fully implement interface '%s', mismatching argument list for attribute '%s'", obj->name, interface->name, key);
                return 0;
            }
        }

        ht_iter_next(&iter);
    }

    return 1;
}


/**
 * Iterates all interfaces found in this object, and see if the object actually implements it fully
 */
int object_check_interface_implementations(t_object *obj) {
    t_dll_element *elem = DLL_HEAD(obj->interfaces);
    while (elem) {
        t_object *interface = (t_object *)elem->data.p;

        DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "* Checking interface: %s" ANSI_RESET "\n", interface->name);

        if (! _object_check_interface_implementations(obj, interface)) {
            return 0;
        }
        elem = DLL_NEXT(elem);
    }

    // Everything fully implemented
    return 1;
}


/**
 * Iterates all interfaces found in this object, and see if the object actually implements it fully
 */
int object_has_interface(t_object *obj, const char *interface_name) {
    DEBUG_PRINT_CHAR("object_has_interface(%s)\n", interface_name);

    t_dll_element *elem = obj->interfaces != NULL ? DLL_HEAD(obj->interfaces) : NULL;
    while (elem) {
        t_object *interface = (t_object *)elem->data.p;

        if (strcasecmp(interface->name, interface_name) == 0) {
            return 1;
        }
        elem = DLL_NEXT(elem);
    }

    // No, cannot find it
    return 0;
}




/**
 * Allocates a class from the given object
 */
t_object *object_alloc_class(t_object *obj, int arg_count, ...) {
    va_list arg_list;

    // Create argument DLL
    t_dll *arguments = dll_init();
    va_start(arg_list, arg_count);
    for (int i=0; i!=arg_count; i++) {
        dll_append(arguments, (void *)va_arg(arg_list, void *));
    }
    va_end(arg_list);

    // Create new object
    int cached = 0;
    t_object *new_obj = object_alloc_args(obj, arguments, &cached);

    // Free argument DLL
    dll_free(arguments);

    return new_obj;
}


t_dll *object_duplicate_interfaces(t_object *instance_obj) {
    if (! instance_obj->interfaces) return NULL;

    t_dll *interfaces = dll_init();
    t_dll_element *e = DLL_HEAD(instance_obj->interfaces);
    while (e) {
        dll_append(interfaces, e->data.p);
        object_inc_ref((t_object *)e->data.p);
        e = DLL_NEXT(e);
    }

    return interfaces;
}

t_object *object_alloc_instance(t_object *obj, int arg_count, ...) {
    va_list arg_list;

    // Create argument DLL
    t_dll *arguments = dll_init();
    va_start(arg_list, arg_count);
    for (int i=0; i!=arg_count; i++) {
        dll_append(arguments, (void *)va_arg(arg_list, void *));
    }
    va_end(arg_list);

    // Create new object
    int cached = 0;
    t_object *new_obj = object_alloc_args(obj, arguments, &cached);

    // Free argument DLL
    dll_free(arguments);

    // It might be possible that object_alloc_args() returned a cached instance.
    if (cached) return new_obj;

    // Instantiate the new object
    object_instantiate(new_obj, obj);

    return new_obj;
}


void object_instantiate(t_object *instance_obj, t_object *class_obj) {
    // Object is an instance, not a class
    instance_obj->flags &= ~OBJECT_TYPE_CLASS;
    instance_obj->flags |= OBJECT_TYPE_INSTANCE;

    // Bind all attributes to the instance
    if (class_obj->attributes) {
        t_hash_table *duplicated_attributes = object_duplicate_attributes(class_obj, instance_obj);
        instance_obj->attributes = duplicated_attributes;
    }

    if (class_obj->interfaces) {
        t_dll *duplicated_interfaces = object_duplicate_interfaces(instance_obj);
        instance_obj->interfaces = duplicated_interfaces;
    }
}


/**
 * Releases an object. This object cannot be used with this specific reference.
 *
 * Whenever the reference count has decreased to 0, it means that nothing holds a reference to this object.
 * It will be added to the garbage collected to either be destroyed, OR when needed, it can be revived before
 * collection whenever somebody needs this specific object again.
 */
long object_release(t_object *obj) {
    return object_dec_ref(obj);
}
