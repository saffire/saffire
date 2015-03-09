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
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>
#include <saffire/general/md5.h>
#include <saffire/vm/thread.h>
#include <saffire/debug.h>


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
SAFFIRE_METHOD(callable, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(callable, dtor) {
    RETURN_NULL;
}


/**
 *
 */
SAFFIRE_METHOD(callable, call) {
    // @TODO: Will do a call to the callable object
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(callable, internal) {
    if (CALLABLE_IS_CODE_INTERNAL(self)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

///**
//* Rebinds the callable to an object
//*/
//SAFFIRE_METHOD(callable, bind) {
//    t_object *newbound_obj;
//
//    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "u",  &newbound_obj)) {
//        return NULL;
//    }
//
//    object_release(self->attribute);
//    self->attribute = newbound_obj;
//    object_inc_ref(newbound_obj);
//    RETURN_SELF;
//}
//
///**
//* Unbinds the callable from an object
//*/
//SAFFIRE_METHOD(callable, unbind) {
//    object_release(self->attribute);
//    self->attribute =  NULL;
//    RETURN_SELF;
//}


/**
 *
 */
SAFFIRE_METHOD(callable, conv_boolean) {
    RETURN_TRUE;
}


/**
 *
 */
SAFFIRE_METHOD(callable, conv_null) {
    RETURN_NULL;
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
 * Initializes methods and properties, these are used
 */
void object_callable_init(void) {
    /* Create callable attributes away from the actual callable structure. This is because callables are actually
     * copying the callables internally. This will mean that the __ctor callable has no attributes, the __dtor callable only the __ctor callable etc
     * We create the attributes away from the callable structure itself, and add them at the end of this function instead.
     *
     * @TODO: This will probably make us run into other problems i'm not 100% forseeing right now.. :(
     */
    t_hash_table *attributes = ht_create();
    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "__ctor",         ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_ctor);

    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "__dtor",         ATTRIB_METHOD_DTOR, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_dtor);

    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "__boolean",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_conv_boolean);
    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "__null",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_conv_null);

    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "isInternal",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_internal);

//    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "bind",           ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_bind);
//    object_add_internal_method(attributes, (t_object *)&Object_Callable_struct, "unbind",         ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_unbind);

    Object_Callable_struct.attributes = attributes;
}

/**
 * Frees memory for a callable object
 */
void object_callable_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Callable_struct);
}


/**
 * Note that when we create a callable, we do not connect this to any attribute. This is done when we build attributes.
 * Thus, it IS possible that a callable is used with a NULL attribute.
 */
static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_callable_object *callable_obj = (t_callable_object *)obj;

    // The routing decides if the code is internal or external
    t_dll_element *e = DLL_HEAD(arg_list);
    callable_obj->data.routing = (int)e->data.l;
    e = DLL_NEXT(e);

    if (CALLABLE_IS_CODE_INTERNAL(callable_obj)) {
        // internal code is just a pointer to the code
        callable_obj->data.code.internal.native_func = (void *)e->data.p;
    } else {
        // external code is a bytecode structure
        callable_obj->data.code.external.codeblock = (t_vm_codeblock *)e->data.p;
    }
    e = DLL_NEXT(e);

    // Add arguments for the callable
    callable_obj->data.arguments = (t_hash_table *)e->data.p;
    e = DLL_NEXT(e);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

static void obj_free(t_object *obj) {
    t_callable_object *callable_obj = (t_callable_object *)obj;

    if (callable_obj->data.binding) {
        object_release(callable_obj->data.binding);
    }

    if (CALLABLE_IS_CODE_EXTERNAL(callable_obj)) {
        vm_codeblock_destroy(callable_obj->data.code.external.codeblock);
    }

    if (callable_obj->data.arguments) {
        t_hash_iter iter;
        ht_iter_init(&iter, callable_obj->data.arguments);
        while (ht_iter_valid(&iter)) {
            t_method_arg *arg = ht_iter_value(&iter);

            // Keys are destroyed through ht_destroy

            DEBUG_PRINT_CHAR("Freeing arg value: %s [%08X]\n", object_debug((t_object *)arg->value), (intptr_t)arg->value);
            DEBUG_PRINT_CHAR("Freeing arg hint: %s [%08X]\n", object_debug((t_object *)arg->typehint), (intptr_t)arg->typehint);

            object_release((t_object *)arg->value);
            object_release((t_object *)arg->typehint);
            smm_free(arg);

            ht_iter_next(&iter);
        }

        ht_destroy(callable_obj->data.arguments);
    }
}


#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_callable_object *self = (t_callable_object *)obj;
    snprintf(self->__debug_info, 199, "%s callable(%d parameters)",
        CALLABLE_IS_CODE_INTERNAL(self) ? "internal" : "external",
        self->data.arguments ? self->data.arguments->element_count : 0
    );

    return self->__debug_info;
}

#endif


// Callable object management functions
t_object_funcs callable_funcs = {
        obj_populate,         // Populates a callable object
        obj_free,             // Free a callable object
        obj_destroy,          // Destroy a callable object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug
#endif
};

// Initial object
t_callable_object Object_Callable_struct = {
    OBJECT_HEAD_INIT("callable", objectTypeCallable, OBJECT_TYPE_CLASS, &callable_funcs, sizeof(t_callable_object_data)),
    {
        0,
        { { NULL } },
        NULL,
        NULL
    }
};
