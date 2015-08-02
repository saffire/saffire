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
#ifndef __OBJECT_H__
#define __OBJECT_H__

    #include <stdlib.h>
    #include <stdarg.h>
    #include <saffire/general/hashtable.h>
    #include <saffire/general/dll.h>
    #include <saffire/compiler/ast_nodes.h>

    void vm_populate_builtins(const char *name, t_object *obj);


#ifdef __DEBUG
    #define DEBUG_INFO_SIZE     200
#endif



    // These functions must be present to deal with object administration (cloning, allocating and free-ing info)
    typedef struct _object_funcs {
        void (*populate)(t_object *, t_dll *);      // Populates an object with new values
        void (*free)(t_object *);                   // Frees objects internal data and places it onto gc queue
        void (*destroy)(t_object *);                // Destroys object. Don't use object after this call!
        t_object *(*clone)(t_object *);             // Clone this object to a new object
        t_object *(*cache)(t_object *, t_dll *);    // Returns a cached object or NULL when no cached object is found
        char *(*hash)(t_object *);                  // Returns a string hash (prob md5) of the object
        char *(*debug)(t_object *);                 // Return debug string (value and info)
    } t_object_funcs;


    #define OBJECT_SCOPE_SELF       1       // LOAD_ATTRIB scope is SELF
    #define OBJECT_SCOPE_PARENT     2       // LOAD_ATTRIB scope is PARENT (start looking for attribs in the first parent)

    // Operator defines
    #define OPERATOR_ADD    0       // Start at 0, to make sure function lookups work
    #define OPERATOR_SUB    1
    #define OPERATOR_MUL    2
    #define OPERATOR_DIV    3
    #define OPERATOR_MOD    4
    #define OPERATOR_AND    5
    #define OPERATOR_OR     6
    #define OPERATOR_XOR    7
    #define OPERATOR_SHL    8
    #define OPERATOR_SHR    9

    // Unary operators
    #define OPERATOR_UNARY_INV    10
    #define OPERATOR_UNARY_NOT    11
    #define OPERATOR_UNARY_POS    12
    #define OPERATOR_UNARY_NEG    13




    // Comparisson defines
    #define COMPARISON_EQ     0     // Equals
    #define COMPARISON_NE     1     // Not equals
    #define COMPARISON_LT     2     // Less than
    #define COMPARISON_GT     3     // Greater than
    #define COMPARISON_LE     4     // Less or equal
    #define COMPARISON_GE     5     // Greater or equal
    #define COMPARISON_IN     6     // In set
    #define COMPARISON_NI     7     // Not in set
    #define COMPARISON_EX     8     // Compare exception, not a "real" comparison, but performs an "instanceof" check
    #define COMPARISON_RE     9     // Regex match
    #define COMPARISON_NRE   10     // Regex inverse match


    // Object flags
    #define OBJECT_TYPE_CLASS         1            /* Object is a class */
    #define OBJECT_TYPE_INTERFACE     2            /* Object is an interface */
    #define OBJECT_TYPE_ABSTRACT      4            /* Object is an abstract class */
    #define OBJECT_TYPE_INSTANCE      8            /* Object is an instance */
    #define OBJECT_TYPE_MASK         15            /* Object type bitmask */

    #define OBJECT_TYPE_USERLAND   4096            /* Object is a userland generated class */

    #define OBJECT_FLAG_IMMUTABLE     16           /* Object is immutable */
    #define OBJECT_FLAG_ALLOCATED     32           /* Object can be freed, as it is allocated through alloc() */
    #define OBJECT_FLAG_FINAL         64           /* Object is finalized */
    #define OBJECT_FLAG_MASK         112           /* Object flag bitmask */



    // Object type and flag checks
    #define OBJECT_TYPE_IS_CLASS(obj)       ((obj->flags & OBJECT_TYPE_CLASS) == OBJECT_TYPE_CLASS)
    #define OBJECT_TYPE_IS_INTERFACE(obj)   ((obj->flags & OBJECT_TYPE_INTERFACE) == OBJECT_TYPE_INTERFACE)
    #define OBJECT_TYPE_IS_ABSTRACT(obj)    ((obj->flags & OBJECT_TYPE_ABSTRACT) == OBJECT_TYPE_ABSTRACT)
    #define OBJECT_TYPE_IS_INSTANCE(obj)    ((obj->flags & OBJECT_TYPE_INSTANCE) == OBJECT_TYPE_INSTANCE)

    #define OBJECT_TYPE_IS_IMMUTABLE(obj)   ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE)
    #define OBJECT_TYPE_IS_FINAL(obj)       ((obj->flags & OBJECT_TYPE_FINAL) == OBJECT_TYPE_FINAL)
    #define OBJECT_IS_ALLOCATED(obj)        ((obj->flags & OBJECT_FLAG_ALLOCATED) == OBJECT_FLAG_ALLOCATED)
    #define OBJECT_IS_USERLAND(obj)         ((obj->flags & OBJECT_TYPE_USERLAND) == OBJECT_TYPE_USERLAND)


    // Simple macro's for object type checks
    #define OBJECT_IS_NULL(obj)         (obj->type == objectTypeNull)
    #define OBJECT_IS_NUMERICAL(obj)    (obj->type == objectTypeNumerical)
    #define OBJECT_IS_STRING(obj)       (obj->type == objectTypeString)
    #define OBJECT_IS_REGEX(obj)        (obj->type == objectTypeRegex)
    #define OBJECT_IS_BOOLEAN(obj)      (obj->type == objectTypeBoolean)
    #define OBJECT_IS_ATTRIBUTE(obj)    (obj->type == objectTypeAttribute)
    #define OBJECT_IS_CALLABLE(obj)     (obj->type == objectTypeCallable)
    #define OBJECT_IS_EXCEPTION(obj)    (obj->type == objectTypeException)
    #define OBJECT_IS_TUPLE(obj)        (obj->type == objectTypeTuple)
    #define OBJECT_IS_LIST(obj)         (obj->type == objectTypeList)
    #define OBJECT_IS_HASH(obj)         (obj->type == objectTypeHash)
    #define OBJECT_IS_BASE(obj)         (obj->type == objectTypeBase)


    // Fetches t_string value from a string object
    #define OBJ2STR(_obj_)       (((t_string_object *)_obj_)->data.value)

    // Fetches value from a string object (assumes zero terminated string)
    #define OBJ2STR0(_obj_)      (((t_string_object *)_obj_)->data.value->val)

    // Duplicates zero terminated string from t_string object
    #define DUP_OBJ2STR0(_obj_)  string_to_char0(((t_string_object *)_obj_)->data.value)

    // Fetches (long) value from a numerical object
    #define OBJ2NUM(_obj_) (((t_numerical_object *)_obj_)->data.value)


    // Number of different object types (also needed for GC queues)
    #define OBJECT_TYPE_LEN     14

    // Object types, the objectTypeAny is a wildcard type. Matches any other type.
    const char *objectTypeNames[OBJECT_TYPE_LEN];
    typedef enum {
                   objectTypeAny, objectTypeCallable, objectTypeAttribute, objectTypeBase, objectTypeBoolean,
                   objectTypeNull, objectTypeNumerical, objectTypeRegex, objectTypeString, objectTypeHash,
                   objectTypeTuple, objectTypeList, objectTypeException, objectTypeUser
                 } t_objectype_enum;



    // Actual header that needs to be present in each object (as the first entry)
    #define SAFFIRE_OBJECT_HEADER \
        long ref_count;                  /* Reference count. When 0, it is targeted for garbage collection */ \
        \
        t_objectype_enum type;          /* Type of the (scalar) object */ \
        char *name;                     /* Name of the class */ \
        \
        int flags;                      /* object flags */ \
        \
        t_object *class;                /* Points to the class that has been instantiated from this, or points to NULL if it's a class */ \
        t_object *parent;               /* Parent object (only t_base_object is allowed to have this NULL) */ \
        \
        t_dll *interfaces;              /* Actual interfaces */ \
        \
        t_hash_table *attributes;       /* Object attributes, properties or constants */ \
        \
        t_object_funcs *funcs;          /* Functions for internal maintenance (new, free, clone etc) */ \
        \
        int data_size;                  /* Additional data size. If 0, no additional data is used in this object */ \
        \
        struct _vm_stackframe *frame;   /* The frame in which this object is born */


#ifdef __DEBUG
    #define SAFFIRE_OBJECT_FOOTER \
        char __debug_info_available; \
        char __debug_info[DEBUG_INFO_SIZE];

    #define OBJECT_FOOTER \
        1, \
        "" \

#else

    #define SAFFIRE_OBJECT_FOOTER \
        char __debug_info_available;

    #define OBJECT_FOOTER \
        0

#endif



    // Actual "global" object. Every object is typed on this object.
    struct _object {
        SAFFIRE_OBJECT_HEADER
        SAFFIRE_OBJECT_FOOTER
    }; // forward define has defined this as a t_object.

    extern t_object Object_Base_struct;
    extern t_object Object_User_struct;

    #define OBJECT_HEAD_INIT_WITH_BASECLASS(name, type, flags, funcs, base, interfaces, data_size) \
                0,              /* initial refcount */     \
                type,           /* base object type */     \
                name,           /* name */                 \
                flags,          /* flags */                \
                NULL,           /* class */                \
                base,           /* parent */               \
                interfaces,     /* implements */           \
                NULL,           /* attribute */            \
                funcs,          /* functions */            \
                data_size,      /* data length */          \
                NULL            /* frame */


    // Object header initialization without any functions or base
    #define OBJECT_HEAD_INIT(name, type, flags, funcs, data_size) \
            OBJECT_HEAD_INIT_WITH_BASECLASS(name, type, flags, funcs, &Object_Base_struct, NULL, data_size)

    /*
     * Header macros
     */
    #define SAFFIRE_METHOD(obj, method) static t_object *object_##obj##_method_##method(t_##obj##_object *self, t_dll *arguments)
    #define SAFFIRE_OPERATOR_METHOD(obj, method) static t_object *object_##obj##_method_opr_##method(t_##obj##_object *self, t_dll *arguments)
    #define SAFFIRE_COMPARISON_METHOD(obj, method) static t_object *object_##obj##_method_cmp_##method(t_##obj##_object *self, t_dll *arguments)

    #define SAFFIRE_METHOD_ARGS arguments

    #define SAFFIRE_MODULE_METHOD(mod, method) static t_object *module_##mod##_method_##method(t_object *self, t_dll *arguments)


    // Returns custom object 'obj'
    #define RETURN_OBJECT(obj)   { return (t_object*)obj; }

    // Returns self
    #define RETURN_SELF { return (t_object *)self; }


    void object_init(void);
    void object_fini(void);

    char *object_debug(t_object *obj);
    int object_parse_arguments(t_dll *arguments, const char *speclist, ...);
    t_object *object_clone(t_object *obj);
    char *object_get_hash(t_object *obj);
    t_object *object_alloc_instance(t_object *obj, int arg_count, ...);
    t_object *object_alloc_class(t_object *obj, int arg_count, ...);
    t_object *object_alloc_args(t_object *obj, t_dll *arguments, int *cached);
    void object_instantiate(t_object *instance_obj, t_object *class_obj);
    void object_inc_ref(t_object *obj);
    long object_release(t_object *obj);


    t_hash_table *object_duplicate_attributes(t_object *class_obj, t_object *instance_obj);

    void object_add_interface(t_object *class, t_object *interface);
    void object_add_property(t_object *obj, char *name, int visibility, t_object *property);
    void object_add_constant(t_object *obj, char *name, int visibility, t_object *constant);
    void object_add_internal_method(t_object *obj, char *name, int flags, int visibility, void *func);
    void object_add_internal_method_attributes(t_hash_table *attributes, t_object *obj, char *name, int flags, int visibility, void *func);
    void object_free_internal_object(t_object *obj);

    int object_instance_of(t_object *obj, const char *instance);
    int object_check_interface_implementations(t_object *obj);
    int object_has_interface(t_object *obj, const char *interface);

    void object_raise_exception(t_object *exception, int code, char *format, ...);

#endif
