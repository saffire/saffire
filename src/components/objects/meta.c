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
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>

#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/memory/smm.h>
#include <saffire/general/md5.h>
#include <saffire/general/output.h>
#include <saffire/debug.h>

/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */

/* ======================================================================
 *   Object methods
 * ======================================================================
 */

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes meta methods and properties, these are used
 */
void object_meta_init(void) {
    Object_Meta_struct.attributes = ht_create();
}

/**
 * Frees memory for a meta object
 */
void object_meta_fini(void) {
    // Free attributes
    object_free_internal_object((t_object *)&Object_Meta_struct);
}


static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG

/**
 * Object debug doesn't output binary safe strings
 */
static char *obj_debug(t_object *obj) {
    t_meta_object *str_obj = (t_meta_object *)obj;

    snprintf(str_obj->__debug_info, DEBUG_INFO_SIZE-1, "meta()");
    return str_obj->__debug_info;
}
#endif



// meta object management functions
t_object_funcs meta_funcs = {
        NULL,                 // Populate a meta object
        NULL,                 // Free a meta object
        obj_destroy,          // Destroy a meta object
        NULL,                 // Clone
        NULL,                 // Object cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug,
#else
        NULL,
#endif
};


// Intial object
t_meta_object Object_Meta_struct = {
    OBJECT_HEAD_INIT("meta", objectTypeUser, OBJECT_TYPE_CLASS, &meta_funcs, 0),
    OBJECT_FOOTER
};
