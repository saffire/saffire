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

    #include <stdlib.h>
    #include <stdarg.h>
    #include "general/hashtable.h"


    // Forward define
    struct _object;
    struct _saffire_result;

    // These functions must be present to deal with object administration (cloning, allocating and free-ing info)
    typedef struct _object_funcs {
        struct _object *(*new)(va_list arg_list);       // Allocates a new object
        void (*free)(struct _object *);                  // Frees objects internal data
        struct _object *(*clone)(struct _object *);      // Clones the object
    } t_object_funcs;


    #define OBJECT_FLAG_INITIALIZED     1


    // Actual header that needs to be present in each object (as the first entry)
    #define SAFFIRE_OBJECT_HEADER \
        int ref_count;                 /* Reference count. When 0, it is targeted for garbage collection */ \
        char *name;                    /* Name of the class */ \
        \
        int immutable;                 /* 1 = immutable object.  0 = normal read/write */ \
        int flags;                     /* object flags */ \
        \
        struct _object *extends;       /* Extends object (only t_base_object is allowed to have this NULL) */ \
        \
        int implement_count;           /* Number of interfaces */ \
        struct _object **implements;   /* Actual interfaces */ \
        \
        t_hash_table *methods;         /* Object methods */ \
        t_hash_table *properties;      /* Object properties */  \
        t_hash_table *constants;       /* Object constants (needed?) */ \
        \
        t_object_funcs *funcs;         /* Functions for internal maintenance (new, free, clone etc) */


    // Actual "global" object. Every object is typed on this object.
    typedef struct _object {
        SAFFIRE_OBJECT_HEADER
    } t_object;


    extern t_object Object_Base_struct;

    #define OBJECT_HEAD_INIT3(name, funcs, base)   \
                1,              /* initial refcount */     \
                name,           /* name */                 \
                0,              /* immutable */            \
                0,              /* flags */                \
                base,           /* extends */              \
                0,              /* implement count */      \
                NULL,           /* implements */           \
                NULL,           /* methods */              \
                NULL,           /* properties */           \
                NULL,           /* constants */            \
                funcs           /* functions */


    // Object header initialization without any functions or base
    #define OBJECT_HEAD_INIT2(name, funcs) OBJECT_HEAD_INIT3(name, funcs, &Object_Base_struct)

    // Object header initialization without any functions
    #define OBJECT_HEAD_INIT(name) OBJECT_HEAD_INIT2(name, NULL)


    /*
     * Header macros
     */
    //#define SAFFIRE_NEW_OBJECT(obj) t_object *object_##obj##_new(void *args)
    #define SAFFIRE_METHOD(obj, method) t_object *object_##obj##_method_##method(t_##obj##_object *self)


    // Returns custom object 'obj'
    #define RETURN_OBJECT(obj) \
        return (t_object*)obj

    // Returns self
    #define RETURN_SELF \
        return (t_object *)self


    t_object *object_new(t_object *obj, ...);
    t_object *object_clone(t_object *obj);
    void object_inc_ref(t_object *obj);
    void object_dec_ref(t_object *obj);

    void test(void);

#endif