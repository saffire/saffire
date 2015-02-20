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
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"

#include "general/output.h"
#include "debug.h"


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes base methods and properties
 */
void object_user_init() {
    //Object_User_struct.attributes = ht_create();
}


/**
 * Frees memory for a base object
 */
void object_user_fini() {
    object_free_internal_object((t_object *)&Object_User_struct);
}



static void obj_populate(t_object *self, t_dll *arg_list) {
}

static void obj_free(t_object *obj) {
    t_user_object *user_obj = (t_user_object *)obj;

    DEBUG_PRINT_CHAR("Freeing user object: %s\n", user_obj->name);

    // Free attributes
    t_hash_iter iter;
    ht_iter_init(&iter, user_obj->attributes);
    while (ht_iter_valid(&iter)) {
#ifdef __DEBUG
        char *key = ht_iter_key_str(&iter);
        DEBUG_PRINT_CHAR("Releasing attribute: %s\n", key);
#endif

        t_object *attr_obj = (t_object *)ht_iter_value(&iter);
        object_release(attr_obj);

        ht_iter_next(&iter);

    }
    ht_destroy(obj->attributes);

    // Release all interface objects
    t_dll_element *e = DLL_HEAD(obj->interfaces);
    while (e) {
        object_release((t_object *)e->data);
        e = DLL_NEXT(e);
    }
    dll_free(obj->interfaces);

    // Release parent class
    object_release(obj->parent);

    // Release name
    smm_free(obj->name);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    sprintf(global_buf, "user[%s]", obj->name);
    return global_buf;
}
#endif


// object management functions
t_object_funcs user_funcs = {
        obj_populate,         // Populate a user object
        obj_free,             // Free a user object
        obj_destroy,          // Destroy a user object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug
#endif
};



// Initial object
t_user_object Object_User_struct = {
    OBJECT_HEAD_INIT("user", objectTypeBase, OBJECT_TYPE_CLASS, &user_funcs, 0),
};

