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

#include "object.h"

/*
 * The base object is the object from which all other objects will be extended. Even when an object isn't extended
 * explicitly, they still will be based on the base-object. This base class will have methods and properties about
 * dealing with objects like .methods() that returns all the methods available in the class, .immutable(), which
 * can set the object to readonly etc.
 */

t_hash_table *base_methods;
t_hash_table *base_properties;

static void object_base_alloc(t_object *obj) { }
static void object_base_free(t_object *obj) { }
static t_object *object_base_clone(t_object *obj) { }

void object_base_init() {
    base_methods = ht_create();
    base_properties = ht_create();
}
void object_base_fini() {
    ht_destroy(base_methods);
    ht_destroy(base_properties);
}

SAFFIRE_NEW_OBJECT(base) {
    t_object *obj = object_new();

    obj->header.name = "base";
    obj->header.fqn = "::base";

    obj->methods = base_methods;
    obj->properties = base_properties;

    obj->funcs.alloc = object_base_alloc;
    obj->funcs.free = object_base_free;
    obj->funcs.clone = object_base_clone;

    return obj;
}

