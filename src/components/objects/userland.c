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

#include <string.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes base methods and properties
 */
void object_userland_init() {
    Object_Userland_struct.attributes = ht_create();
}


/**
 * Frees memory for a base object
 */
void object_userland_fini() {
    ht_destroy(Object_Userland_struct.attributes);
}


static t_object *obj_new(t_object *self) {
    t_userland_object *obj = smm_malloc(sizeof(t_userland_object));
    memcpy(obj, self, sizeof(t_userland_object));

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    char *s = obj->name;
    snprintf(global_buf, 1023, "User[%s]", s);
    return global_buf;
}
#endif


// object management functions
t_object_funcs userland_funcs = {
        obj_new,              // Allocate a new string object
        NULL,                 // Populate a string object
        NULL,                 // Free a string object
        obj_destroy,          // Destroy a string object
        NULL,                 // Clone
#ifdef __DEBUG
        obj_debug
#endif
};



// Initial object
t_userland_object Object_Userland_struct = {
    OBJECT_HEAD_INIT("user", objectTypeUser, OBJECT_TYPE_CLASS, &userland_funcs)
};

