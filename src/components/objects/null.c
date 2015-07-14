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

#include <string.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>

/* ======================================================================
 *   Object methods
 * ======================================================================
 */


SAFFIRE_METHOD(null, conv_boolean) {
    RETURN_FALSE;
}

SAFFIRE_METHOD(null, conv_null) {
    RETURN_SELF;
}

SAFFIRE_METHOD(null, conv_numerical) {
    RETURN_NUMERICAL(0);
}

SAFFIRE_METHOD(null, conv_string) {
    RETURN_STRING_FROM_CHAR("null");
}

SAFFIRE_COMPARISON_METHOD(null, ne) {
    t_object *obj = DLL_HEAD(arguments)->data.p;

    if(OBJECT_IS_NULL(obj)) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

SAFFIRE_OPERATOR_METHOD(null, not) {
    RETURN_TRUE;
}


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes string methods and properties, these are used
 */
void object_null_init(void) {
    Object_Null_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&Object_Null_struct, "__boolean",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Null_struct, "__null",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_conv_null);
    object_add_internal_method((t_object *)&Object_Null_struct, "__numerical", ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_conv_numerical);
    object_add_internal_method((t_object *)&Object_Null_struct, "__string",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_conv_string);

    object_add_internal_method((t_object *)&Object_Null_struct, "__cmp_ne",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_cmp_ne);

    object_add_internal_method((t_object *)&Object_Null_struct, "__opr_not",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, object_null_method_opr_not);

    vm_populate_builtins("null", Object_Null);
}

/**
 * Frees memory for a string object
 */
void object_null_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Null_struct);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    return "Null";
}
#endif


static t_object *obj_cache(t_object *self, t_dll *arg_list) {
    return Object_Null;
}


t_object_funcs null_funcs = {
        NULL,               // Populate
        NULL,               // Free
        NULL,               // Destroy
        NULL,               // Clone
        obj_cache,          // Cache
        NULL,               // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};


t_null_object Object_Null_struct = { OBJECT_HEAD_INIT("null", objectTypeNull, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_FINAL | OBJECT_FLAG_IMMUTABLE, &null_funcs, 0), OBJECT_FOOTER };


