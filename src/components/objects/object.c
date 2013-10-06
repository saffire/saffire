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
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/dll.h"
#include "debug.h"
#include "gc/gc.h"
#include "general/output.h"
#include "general/smm.h"
#include "vm/thread.h"

t_dll *all_objects;

// @TODO: in_place: is this option really needed? (inplace modifications of object, like A++; or A = A + 2;)

// Object type string constants
const char *objectTypeNames[OBJECT_TYPE_LEN] = { "object", "callable", "attribute", "base", "boolean",
                                                 "null", "numerical", "regex", "string",
                                                 "hash", "tuple", "user", "list", "exception" };

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
 * Free an object (if needed)
 */
static void _object_free(t_object *obj) {
    if (! obj) return;

    // ref_count > 0, object is still in use somewhere else. Don't free it yet
    if (obj->ref_count > 0) return;

//#ifdef __DEBUG
//    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
//        //DEBUG_PRINT("Freeing object: %08lX (%d) %s\n", (unsigned long)obj, obj->flags, object_debug(obj));
//    }
//#endif

    // Free values from the object
    if (obj->funcs && obj->funcs->free) {
        obj->funcs->free(obj);
    }

    // Remove this object from the all_objects list
    t_dll_element *e = DLL_HEAD(all_objects);
    while (e) {
        t_object *tmp = (t_object *)e->data;

        if (tmp == obj) {
            dll_remove(all_objects, e);
            break;
        }
        e = DLL_NEXT(e);
    }


    // Free the object
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


/**
 * Increase reference to object.
 */
void object_inc_ref(t_object *obj) {
    if (! obj) return;

    obj->ref_count++;
    if (OBJECT_IS_CALLABLE(obj) || OBJECT_IS_ATTRIBUTE(obj)) return;
//    DEBUG_PRINT("Increased reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);
}


/**
 * Decrease reference from object.
 */
long object_dec_ref(t_object *obj) {
    if (! obj) return 0;

    obj->ref_count--;

//    if (! OBJECT_IS_CALLABLE(obj) && ! OBJECT_IS_ATTRIBUTE(obj)) {
//        DEBUG_PRINT("Decreased reference for: %s (%08lX) to %d\n", object_debug(obj), (unsigned long)obj, obj->ref_count);
//    }

    if (obj->ref_count != 0) return obj->ref_count;

    // Don't free static objects
    if (! OBJECT_IS_ALLOCATED(obj)) return 0;

    //DEBUG_PRINT("*** Freeing object %s (%08lX)\n", object_debug(obj), (unsigned long)obj);

    // Free object
    _object_free(obj);
    return 0;
}

#ifdef __DEBUG
char global_debug_info[256];
char *object_debug(t_object *obj) {

    if (! obj) return "(null)<0x0>";

    if (obj && obj->funcs && obj->funcs->debug) {
        char *s = obj->funcs->debug(obj);
        if (OBJECT_TYPE_IS_CLASS(obj)) {
            //s[0] = toupper(s[0]);
        }
        return s;
    }

    if (OBJECT_IS_USER(obj)) {
        snprintf(global_debug_info, 255, "user[%s]", obj->name);
        if (OBJECT_TYPE_IS_CLASS(obj)) {
            global_debug_info[0] = toupper(global_debug_info[0]);
        }
        return global_debug_info;
    }

    return "(no debug info)";
}
#endif


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
 * Actually returns a new object. Note that it must NOT populate anything through the
 * arguments. This should be done in the populate() callback. The arguments presented
 * here can be used to decide if we have a cached version of the object laying around.
 */
static t_object *_object_new(t_object *obj, t_dll *arguments) {
    t_object *res;

    // Return NULL when we cannot 'new' this object
    if (! obj->funcs->new) {
        RETURN_NULL;
    }

    // Create new object
    res = obj->funcs->new(obj);
    res->ref_count = 1;

    // We add 'res' to our list of generated objects.
    dll_append(all_objects, res);

    return res;
}


/**
 * Creates a new object with specific values, with a already created
 * argument DLL list.
 */
t_object *object_alloca(t_object *obj, t_dll *arguments) {
    t_object *res = NULL;

    // Nothing found to new, just return NULL object
    if (! obj || ! obj->funcs) {
        RETURN_NULL;
    }

    // If we have a caching function, seek inside that cache first
    if (obj->funcs->cache) {
        res = obj->funcs->cache(obj, arguments);
        if (res) {
            object_inc_ref(res);
            return res;
        }
    }

    // generate new object
    res = _object_new(obj, arguments);

    // Populate values
    if (res->funcs->populate) {
        res->funcs->populate(res, arguments);
    }

    res->ref_count = 0;
    object_inc_ref(res);

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
    t_object *new_obj = object_alloca(obj, arguments);

    // Free argument DLL
    dll_free(arguments);

    return new_obj;
}


extern t_hash_table *string_cache;
t_dll *dupped_attributes;


/**
 * Initialize all the (scalar) objects
 */
void object_init() {
    // All objects have a reference here (except dupped attributes, actually)
    all_objects = dll_init();

    // All duplicated attributes are references here, because they are short-lived, we can do some other stuff with them later.
    dupped_attributes = dll_init();

    // Create string cache.
    string_cache = ht_create();


    object_callable_init();
    object_attrib_init();
    object_base_init();
    object_string_init();

    object_boolean_init();
    object_null_init();
    object_numerical_init();
    object_regex_init();
    object_hash_init();
    object_tuple_init();
    object_userland_init();
    object_list_init();
    object_exception_init();

    object_iterator_init();
    object_datastructure_init();
}


/**
 * Finalize all the (scalar) objects
 */
void object_fini() {
    t_dll_element *e;

    DEBUG_PRINT("object fini\n");

    DEBUG_PRINT("Destroying string cache!\n");

    // Destroy string cache
    t_hash_iter iter;
    ht_iter_init(&iter, string_cache);
    while (ht_iter_valid(&iter)) {
        t_object *val = ht_iter_value(&iter);

        // Release object from this cache
        ht_iter_next(&iter);
        object_release(val);
    }
    ht_destroy(string_cache);

    // Remove all duplicated attributes
    e = DLL_HEAD(dupped_attributes);
    while (e) {
        //t_object *dup = (t_object *)(e->data);
        //object_release(dup);

        e = DLL_NEXT(e);
    }



    object_datastructure_fini();
    object_iterator_fini();

    object_exception_fini();
    object_list_fini();
    object_userland_fini();
    object_tuple_fini();
    object_regex_fini();
    object_numerical_fini();
    object_null_fini();
    object_boolean_fini();

    object_string_fini();       // has to be second-last
    object_hash_fini();
    object_base_fini();         // has to be last
    object_attrib_fini();
    object_callable_fini();


    // We really can't show anything here, since objects should have been gone now. Expect failures
    DEBUG_PRINT("At object_fini(), we still have %ld objects left on the stack\n", all_objects->size);
    e = DLL_HEAD(all_objects);
    while (e) {
        t_object *obj = (t_object *)e->data;
        //printf("%-30s %08X %d : %s\n", obj->name, (unsigned int)obj, obj->ref_count, object_debug(obj));
        DEBUG_PRINT("%-30s %08X %d\n", obj->name, (unsigned int)obj, obj->ref_count);
        e = DLL_NEXT(e);
    }
    dll_free(all_objects);
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
        if (optional_argument == 0 && (!e || !e->data)) {
            object_raise_exception(Object_ArgumentException, 1, "Error while fetching mandatory argument.");
            result = 0;
            goto done;
        }

        t_object *argument_obj = e ? e->data : NULL;
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
        fatal_error(1, "Interface can only be added to a class\n");
        return;
    }

    if (! OBJECT_TYPE_IS_INTERFACE(interface)) {
        fatal_error(1, "%s is not an interface\n", interface->name);
        return;
    }

    if (! class->interfaces) {
        class->interfaces = dll_init();
    }

    dll_append(class->interfaces, interface);
}

/**
 * Create method- attribute that points to an INTERNAL (C) function
 */
void object_add_internal_method(t_object *obj, char *name, int method_flags, int visibility, void *func) {
    // @TODO: Instead of NULL, we should be able to add our parameters. This way, we have a more generic way to deal with internal and external functions.
    t_callable_object *callable_obj = (t_callable_object *)object_alloc(Object_Callable, 3, CALLABLE_CODE_INTERNAL, func, /* arguments */ NULL);

    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc(Object_Attrib, 5, ATTRIB_TYPE_METHOD, visibility, ATTRIB_ACCESS_RO, callable_obj, method_flags);

    // Actually "bind" the attribute to this (class) object
    attrib_obj->bound_class = obj;
    attrib_obj->bound_name = smm_strdup(name);

    ht_add_str(obj->attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}


/**
 *
 */
void object_add_property(t_object *obj, char *name, int visibility, t_object *property) {
    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc(Object_Attrib, 5, ATTRIB_TYPE_PROPERTY, visibility, ATTRIB_ACCESS_RW, property, 0);

    object_inc_ref(property);

    // @TODO: why replace? why not ht_add_str()??

    ht_replace_str(obj->attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}


/**
 *
 */
void object_add_constant(t_object *obj, char *name, int visibility, t_object *constant) {
    t_attrib_object *attrib_obj = (t_attrib_object *)object_alloc(Object_Attrib, 5, ATTRIB_TYPE_CONSTANT, visibility, ATTRIB_ACCESS_RO, constant, 0);

    object_inc_ref(constant);

    if (ht_exists_str(obj->attributes, name)) {
        object_release((t_object *)attrib_obj);
        fatal_error(1, "Attribute '%s' already exists in object '%s'\n", name, obj->name);
    }

    ht_add_str(obj->attributes, name, attrib_obj);
    object_inc_ref((t_object *)attrib_obj);
}





static void _object_remove_all_internal_interfaces(t_object *obj) {
    if (! obj->interfaces) return;

    t_dll_element *e = DLL_HEAD(obj->interfaces);
    while (e) {
        object_release((t_object *)e->data);
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
        t_attrib_object *attr = (t_attrib_object *)ht_iter_value(&iter);
//        char *key = ht_iter_key_str(&iter);

        /* This is tricky. Attributes can links to other objects (callables, but also properties and constants). These
         * are created in object_add_internal_method, object_add_property, object_add_constant, or through the vm's
         * BUILD_CLASS + BUILD_ATTRIB opcodes.
         * However, allocating objects will automatically increase their refcount (normally, to 1, but if we allocate
         * an object that already exists, we could get that object, with it's refcount increased).
         * What happens, is that attributes will look something like this:
         *
         *      attrib-object (refcount = 1)
         *        -> callable-object (refcount = 2)
         *
         * The reason refcount on callable is 2, is because the attrib-object will automatically increase the refcount
         * as well. This makes sense, because the callable is "used" by the attrib, and we must make sure it can never
         * be destroyed without destroying the attrib first.
         *
         * But attrib-objects are normally added to a (user-object) class as well inside their obj->attributes hashtable.
         * This hash-table "uses" the attrib-object, which means we increase the attrib-refcount as well:
         *
         *     user-object (refcount = 1)
         *        -> attrib-object (refcount = 2)
         *             -> callable-object (refcount = 2)
         *
         * At this point, a userclass, has attrib with a refcount of AT LEAST 2, and internally callable (or property
         * or constant) objects of again AT LEAST 2. Because this is the place we are destroying the attributes, we must
         * make sure we release the attribute from the obj->attributes, and again release the attribute by itself. When
         * an attrib-objects refcount gets to 0, it will also decrease the refcount of it's used object. This means we
         * end up with a callable object with a refcount of 1: we decreased the attrib twice, but the callable only once.
         *
         * This is a tricky situation, and hopefully we can resolve this a better way than this, but for now all we do
         * is iterate our attributes, decrease attributes ONCE, decrease the callable inside the attribute ONCE, and then
         * decrease the attribute again. If everything goes according to plan, it will decrease it to 0, and
         * automatically decrease the callable as well, resulting in both objects being destroyed. If the attribute (or
         * callable) is shared and their refcount was higher than 2 to begin with, things will still work out correctly, as
         * they will be freed somewhere else.
         */

        // Release attribute from the hash
        object_release((t_object *)attr);

        // Release callable,property or constant from the attribute
        object_release(((t_attrib_object *)attr)->attribute);

        // Release attribute again, so it can be destroyed, and implicitly destroy the callable as well
        object_release((t_object *)attr);

        // This attribute hash does not know anything anymore about this attrib
        ht_iter_next(&iter);
    }

//    ht_destroy(obj->attributes);

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
 * @TODO: t_object instead of t_exception_object. We run into some forward typedef issues when we use t_exception_object
 */
void object_raise_exception(t_object *exception, int code, char *format, ...) {
    va_list args;
    char *buf;

    va_start(args, format);
    smm_vasprintf(&buf, format, args);
    va_end(args);

    thread_create_exception((t_exception_object *)exception, code, buf);
    smm_free(buf);
}


static int _object_check_matching_arguments(t_callable_object *obj1, t_callable_object *obj2) {
    t_hash_table *ht1 = obj1->arguments;
    t_hash_table *ht2 = obj2->arguments;

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

        DEBUG_PRINT("        - typehint1: '%-20s (%d)'  value1: '%-20s' \n", OBJ2STR(arg1->typehint), object_debug(arg1->value));
        DEBUG_PRINT("        - typehint2: '%-20s (%d)'  value2: '%-20s' \n", OBJ2STR(arg2->typehint), object_debug(arg2->value));

        if (strcmp(OBJ2STR(arg1->typehint), OBJ2STR(arg2->typehint)) != 0) {
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
        DEBUG_PRINT(ANSI_BRIGHTBLUE "    interface attribute '" ANSI_BRIGHTGREEN "%s" ANSI_BRIGHTBLUE "' : " ANSI_BRIGHTGREEN "%s" ANSI_RESET "\n", key, object_debug((t_object *)attribute));

        t_attrib_object *found_obj = (t_attrib_object *)object_attrib_find(obj, key);
        if (! found_obj) {
            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Class '%s' does not fully implement interface '%s', missing attribute '%s'", obj->name, interface->name, key);
            return 0;
        }

        DEBUG_PRINT("     - Found object : %s\n", object_debug((t_object *)found_obj));
        DEBUG_PRINT("     - Matching     : %s\n", object_debug((t_object *)attribute));

        if (found_obj->attr_type != attribute->attr_type ||
            found_obj->attr_visibility != attribute->attr_visibility ||
            found_obj->attr_access != attribute->attr_access) {
            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Class '%s' does not fully implement interface '%s', mismatching settings for attribute '%s'", obj->name, interface->name, key);
            return 0;
        }

        // If we are a callable, check arguments
        if (OBJECT_IS_CALLABLE(found_obj->attribute)) {
            DEBUG_PRINT("     - Checking parameter signatures\n");
            if (!_object_check_matching_arguments((t_callable_object *)attribute->attribute, (t_callable_object *)found_obj->attribute)) {
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
        t_object *interface = (t_object *)elem->data;

        DEBUG_PRINT(ANSI_BRIGHTBLUE "* Checking interface: %s" ANSI_RESET "\n", interface->name);

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
    DEBUG_PRINT("object_has_interface(%s)\n", interface_name);

    t_dll_element *elem = obj->interfaces != NULL ? DLL_HEAD(obj->interfaces) : NULL;
    while (elem) {
        t_object *interface = (t_object *)elem->data;

        if (strcasecmp(interface->name, interface_name) == 0) {
            return 1;
        }
        elem = DLL_NEXT(elem);
    }

    // No, cannot find it
    return 0;
}



/**
 * Allocate / populate a new instance from the giben object
 */
t_object *object_alloc(t_object *obj, int arg_count, ...) {
    va_list arg_list;

    // Create argument DLL
    t_dll *arguments = dll_init();
    va_start(arg_list, arg_count);
    for (int i=0; i!=arg_count; i++) {
        dll_append(arguments, va_arg(arg_list, void *));
    }
    va_end(arg_list);

    // Create new object
    t_object *new_obj = object_alloca(obj, arguments);

    // Free argument DLL
    dll_free(arguments);

    return new_obj;
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

//void object_bind_callable(t_object *callable_obj, t_object *attrib_obj, char *name) {
//    if (((t_callable_object *)callable_obj)->binding) {
//        object_release(((t_callable_object *)callable_obj)->binding);
//    }
////    if (((t_callable_object *)callable_obj)->name) {
////        smm_free(((t_callable_object *)callable_obj)->name);
////    }
//
//    ((t_callable_object *)callable_obj)->binding = (t_object *)attrib_obj;
////    ((t_callable_object *)callable_obj)->name = name ? smm_strdup(name) : smm_strdup("callable");
//
//    object_inc_ref(attrib_obj);
//}

