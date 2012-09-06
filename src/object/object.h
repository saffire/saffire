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
#ifndef __OBJECT_H__
#define __OBJECT_H__

    #include "general/hashtable.h"

    // Define forward
    struct _object;

    //
    typedef struct _object_header {
                int ref_count;                 // Reference count
                struct _object *extends;       // Extends object (only t_base_object is allowed to have this NULL)

                int implement_count;           // Number of interfaces
                struct _object *implements;    // Actual interfaces

                char *name;                    // Name of the class
                char *fqn;                     // Fully qualified name (::<name>)
    } t_object_header;

    // These functions must be present to deal with objects (cloning, allocating and free-ing info)
    typedef struct _object_funcs {
        void (*alloc)(struct _object *);                       // Allocate objects internal data
        void (*free)(struct _object *);                        // Free objects internal data
        struct _object *(*clone)(struct _object *);            // Clone the object
    } t_object_funcs;

    // Actual "global" object. Every object is typed on this object, but with a different "data" section.
    typedef struct _object {
        t_object_header header;          // Object header

        t_hash_table *methods;           // Object methods
        t_hash_table *properties;        // Object properties
        t_hash_table *constants;         // Object constants (needed?)

        t_object_funcs funcs;           // Functions for internal maintenance (alloc, free, clone etc)

        // int type;                        // The type of the object (string, numerical, boolean etc)
        void *data;                      // Additional internal data for the object
    } t_object;


    void test(void);
    void object_init(void);
    t_object *object_new(void);
    t_object *object_clone(t_object *obj);


    typedef struct _saffire_result {
        void *result;
    } t_saffire_result;



    #define SAFFIRE_NEW_OBJECT(obj) t_object *object_##obj##_new(void)

    #define SAFFIRE_METHOD(obj, method) t_saffire_result *object_##obj##_method_##method(t_object *self)

#endif