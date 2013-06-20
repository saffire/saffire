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
#ifndef __OBJECT_H__
#define __OBJECT_H__

    #include <stdlib.h>
    #include <stdarg.h>
    #include "general/hashtable.h"
    #include "general/dll.h"
    #include "compiler/ast_nodes.h"

    typedef struct _bytecode t_bytecode;
    typedef struct _vm_frame t_vm_frame;


    void vm_populate_builtins(const char *name, void *data);

    // Forward define
    typedef struct _object t_object;

    // These functions must be present to deal with object administration (cloning, allocating and free-ing info)
    typedef struct _object_funcs {
        t_object *(*new)(t_object *);               // Allocates a new object from base object
        void (*populate)(t_object *, t_dll *);      // Populates an object with new values
        void (*free)(t_object *);                   // Frees objects internal data and places it onto gc queue
        void (*destroy)(t_object *);                // Destroys object. Don't use object after this call!
        t_object *(*clone)(t_object *);             // Clone this object to a new object
#ifdef __DEBUG
        char *(*debug)(t_object *);                 // Return debug string (value and info)
#endif
    } t_object_funcs;

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
    #define OPERATOR_COA    10


    #define COMPARISON_EQ     0     // Equals
    #define COMPARISON_NE     1     // Not equals
    #define COMPARISON_LT     2     // Less than
    #define COMPARISON_GT     3     // Greater than
    #define COMPARISON_LE     4     // Less or equal
    #define COMPARISON_GE     5     // Greater or equal
    #define COMPARISON_IN     6     // In set
    #define COMPARISON_NI     7     // Not in set
    #define COMPARISON_EX     8     // Compare exception


    // Object flags
    #define OBJECT_TYPE_CLASS         1            /* Object is a class */
    #define OBJECT_TYPE_INTERFACE     2            /* Object is an interface */
    #define OBJECT_TYPE_ABSTRACT      4            /* Object is an abstract class */
    #define OBJECT_TYPE_INSTANCE      8            /* Object is an instance (object) */
    #define OBJECT_TYPE_MASK         15            /* Object type bitmask */

    #define OBJECT_FLAG_IMMUTABLE     16           /* Object is immutable */
    #define OBJECT_FLAG_STATIC        32           /* Do not free memory for this object */
    #define OBJECT_FLAG_FINAL         64           /* Object is finalized */
    #define OBJECT_FLAG_MASK         112           /* Object flag bitmask */



    typedef struct _attribute {
        // Meta data
        struct {
            char        visibility;
            char        access;
        } meta;

        t_object *attribute;
    } t_attribute;


    // Object type and flag checks
    #define OBJECT_TYPE_IS_CLASS(obj) ((obj->flags & OBJECT_TYPE_CLASS) == OBJECT_TYPE_CLASS)
    #define OBJECT_TYPE_IS_INTERFACE(obj) ((obj->flags & OBJECT_TYPE_INTERFACE) == OBJECT_TYPE_INTERFACE)
    #define OBJECT_TYPE_IS_ABSTRACT(obj) ((obj->flags & OBJECT_TYPE_ABSTRACT) == OBJECT_TYPE_ABSTRACT)
    #define OBJECT_TYPE_IS_INSTANCE(obj) ((obj->flags & OBJECT_TYPE_INSTANCE) == OBJECT_TYPE_INSTANCE)

    #define OBJECT_TYPE_IS_IMMUTABLE(obj) ((obj->flags & OBJECT_FLAG_IMMUTABLE) == OBJECT_FLAG_IMMUTABLE)
    #define OBJECT_TYPE_IS_STATIC(obj) ((obj->flags & OBJECT_FLAG_STATIC) == OBJECT_FLAG_STATIC)
    #define OBJECT_TYPE_IS_FINAL(obj) ((obj->flags & OBJECT_TYPE_FINAL) == OBJECT_TYPE_FINAL)


    // Simple macro's for object type checks
    #define OBJECT_IS_NULL(obj)         (obj->type == objectTypeNull)
    #define OBJECT_IS_NUMERICAL(obj)    (obj->type == objectTypeNumerical)
    #define OBJECT_IS_STRING(obj)       (obj->type == objectTypeString)
    #define OBJECT_IS_BOOLEAN(obj)      (obj->type == objectTypeBoolean)
    #define OBJECT_IS_ATTRIBUTE(obj)    (obj->type == objectTypeAttribute)
    #define OBJECT_IS_CALLABLE(obj)     (obj->type == objectTypeCallable)
    #define OBJECT_IS_USER(obj)         (obj->type == objectTypeUser)
    #define OBJECT_IS_EXCEPTION(obj)    (obj->type == objectTypeException)


    // Convert object to value
    #define OBJ2STR(_obj_) smm_strdup(((t_string_object *)_obj_)->value)
    #define OBJ2NUM(_obj_) (((t_numerical_object *)_obj_)->value)


    // Number of different object types (also needed for GC queues)
    #define OBJECT_TYPE_LEN     14

    // Object types, the objectTypeAny is a wildcard type. Matches any other type.
    const char *objectTypeNames[OBJECT_TYPE_LEN];
    typedef enum {
                   objectTypeAny, objectTypeCallable, objectTypeAttribute, objectTypeBase, objectTypeBoolean,
                   objectTypeNull, objectTypeNumerical, objectTypeRegex, objectTypeString, objectTypeHash,
                   objectTypeTuple, objectTypeUser, objectTypeList, objectTypeException
                 } t_objectype_enum;



    // Actual header that needs to be present in each object (as the first entry)
    #define SAFFIRE_OBJECT_HEADER \
        int ref_count;                  /* Reference count. When 0, it is targeted for garbage collection */ \
        \
        t_objectype_enum type;          /* Type of the (scalar) object */ \
        char *name;                     /* Name of the class */ \
        \
        int flags;                      /* object flags */ \
        \
        t_object *parent;               /* Parent object (only t_base_object is allowed to have this NULL) */ \
        \
        t_dll *interfaces;              /* Actual interfaces */ \
        \
        t_hash_table *attributes;       /* Object attributes, properties or constants */ \
        \
        t_object_funcs *funcs;          /* Functions for internal maintenance (new, free, clone etc) */


    // Actual "global" object. Every object is typed on this object.
    struct _object {
        SAFFIRE_OBJECT_HEADER
    }; // forward define has defined this as a t_object.

    extern t_object Object_Base_struct;

    #define OBJECT_HEAD_INIT_WITH_BASECLASS(name, type, flags, funcs, base) \
                0,              /* initial refcount */     \
                type,           /* scalar type */          \
                name,           /* name */                 \
                flags,          /* flags */                \
                base,           /* parent */               \
                NULL,           /* implements */           \
                NULL,           /* attribute */            \
                funcs           /* functions */

    // Object header initialization without any functions or base
    #define OBJECT_HEAD_INIT(name, type, flags, funcs) \
            OBJECT_HEAD_INIT_WITH_BASECLASS(name, type, flags, funcs, &Object_Base_struct)

    /*
     * Header macros
     */
    #define SAFFIRE_METHOD(obj, method) static t_object *object_##obj##_method_##method(t_##obj##_object *self, t_dll *arguments)
    #define SAFFIRE_OPERATOR_METHOD(obj, method) static t_object *object_##obj##_method_opr_##method(t_##obj##_object *self, t_dll *arguments)
    #define SAFFIRE_COMPARISON_METHOD(obj, method) static t_object *object_##obj##_method_cmp_##method(t_##obj##_object *self, t_dll *arguments)

    #define SAFFIRE_METHOD_ARGS arguments

    #define SAFFIRE_MODULE_METHOD(mod, method) static t_object *module_##mod##_method_##method(t_object *self, t_dll *arguments)


    // Returns custom object 'obj'
    #define RETURN_OBJECT(obj) return (t_object*)obj

    // Returns self
    #define RETURN_SELF { return (t_object *)self; }


    void object_init(void);
    void object_fini(void);

    t_object *object_find_actual_attribute(t_object *obj, char *attr_name);
    t_object *object_find_attribute(t_object *obj, char *attribute_name);

    void object_free(t_object *obj);
    char *object_debug(t_object *obj);
    int object_parse_arguments(t_dll *arguments, const char *speclist, ...);
    t_object *object_new(t_object *obj, int arg_count, ...);
    t_object *object_new_with_dll_args(t_object *obj, t_dll *arguments);
    t_object *object_clone(t_object *obj);
    void object_inc_ref(t_object *obj);
    void object_dec_ref(t_object *obj);

    void object_add_property(t_object *obj, char *name, int visibility, t_object *property);
    void object_add_internal_method(t_object *obj, char *name, int flags, int visibility, void *func);
    void object_remove_all_internal_attributes(t_object *obj);

    int object_instance_of(t_object *obj, const char *instance);
    int object_check_interface_implementations(t_object *obj);

    void object_raise_exception(t_object *exception, char *format, ...);

#endif
