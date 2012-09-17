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
    #include "general/dll.h"


    // Forward define
    struct _object;
    struct _saffire_result;

    // These functions must be present to deal with object administration (cloning, allocating and free-ing info)
    typedef struct _object_funcs {
        struct _object *(*new)(va_list arg_list);       // Allocates a new object
        void (*free)(struct _object *);                  // Frees objects internal data
        struct _object *(*clone)(struct _object *);      // Clones the object
    } t_object_funcs;

    // Operator defines
    #define OPERATOR_ADD    1
    #define OPERATOR_SUB    2
    #define OPERATOR_MUL    3
    #define OPERATOR_DIV    4
    #define OPERATOR_MOD    5
    #define OPERATOR_AND    6
    #define OPERATOR_OR     7
    #define OPERATOR_XOR    8
    #define OPERATOR_SL     9
    #define OPERATOR_SR    10

    // Standard operators for
    typedef struct _object_operators {
        struct _object *(*add)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*sub)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*mul)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*div)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*mod)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*and)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*or)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*xor)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*sl)(struct _object *, t_dll *dll, int in_place);
        struct _object *(*sr)(struct _object *, t_dll *dll, int in_place);
    } t_object_operators;


    // Object flags
    #define OBJECT_NO_FLAGS           0            /* No flags */
    #define OBJECT_FLAG_IMMUTABLE     1            /* Object is immutable */
    #define OBJECT_FLAG_STATIC        2            /* Do not free memory for this object */

    // Object types, the objectTypeAny is a wildcard type. Matches any other type.
    const char *objectTypeNames[7];
    typedef enum { objectTypeAny, objectTypeBase, objectTypeBoolean, objectTypeNull, objectTypeNumerical, objectTypeRegex, objectTypeString } objectTypeEnum;

    // Actual header that needs to be present in each object (as the first entry)
    #define SAFFIRE_OBJECT_HEADER \
        int ref_count;                 /* Reference count. When 0, it is targeted for garbage collection */ \
        \
        objectTypeEnum type;           /* Type of the (scalar) object */ \
        char *name;                    /* Name of the class */ \
        \
        int flags;                     /* object flags */ \
        \
        struct _object *parent;        /* Parent object (only t_base_object is allowed to have this NULL) */ \
        \
        int implement_count;           /* Number of interfaces */ \
        struct _object **implements;   /* Actual interfaces */ \
        \
        t_hash_table *methods;         /* Object methods */ \
        t_hash_table *properties;      /* Object properties */  \
        t_hash_table *constants;       /* Object constants (needed?) */ \
        t_object_operators *operators; /* Object operators */ \
        \
        t_object_funcs *funcs;         /* Functions for internal maintenance (new, free, clone etc) */


    // Actual "global" object. Every object is typed on this object.
    typedef struct _object {
        SAFFIRE_OBJECT_HEADER
    } t_object;

    extern t_object Object_Base_struct;

    #define OBJECT_HEAD_INIT3(name, type, operators, flags, funcs, base) \
                1,              /* initial refcount */     \
                type,           /* scalar type */          \
                name,           /* name */                 \
                flags,          /* flags */                \
                base,           /* parent */               \
                0,              /* implement count */      \
                NULL,           /* implements */           \
                NULL,           /* methods */              \
                NULL,           /* properties */           \
                NULL,           /* constants */            \
                operators,      /* operators */            \
                funcs           /* functions */

    // Object header initialization without any functions or base
    #define OBJECT_HEAD_INIT2(name, type, operators, flags, funcs) OBJECT_HEAD_INIT3(name, type, operators, flags, funcs, &Object_Base_struct)

    // Object header initialization without any functions
    #define OBJECT_HEAD_INIT(name, type, operators, flags) OBJECT_HEAD_INIT2(name, type, operators, flags, NULL)


    /*
     * Header macros
     */
    #define SAFFIRE_METHOD(obj, method) static t_object *object_##obj##_method_##method(t_##obj##_object *self, t_dll *dll)

    #define SAFFIRE_OPERATOR_METHOD(obj, opr) static t_object *object_##obj##_operator_##opr(t_object *_self, t_dll *dll, int in_place)


    #define SAFFIRE_METHOD_ARGS dll



    // Returns custom object 'obj'
    #define RETURN_OBJECT(obj) \
        return (t_object*)obj

    // Returns self
    #define RETURN_SELF \
        { object_inc_ref((t_object *)self); return (t_object *)self; }



    void object_init(void);
    void object_fini(void);
    t_object *object_call(t_object *obj, char *method, int arg_count, ...);
    t_object *object_operator(t_object *obj, int operator, int in_place, int arg_count, ...);
    void object_free(t_object *obj);
    int object_parse_arguments(t_dll *dll, const char *speclist, ...);
    t_object *object_new(t_object *obj, ...);
    t_object *object_clone(t_object *obj);
    void object_inc_ref(t_object *obj);
    void object_dec_ref(t_object *obj);

    void test(void);

#endif