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
#ifndef __ATTRIB_H__
#define __ATTRIB_H__

    #include <saffire/objects/object.h>
    #include <saffire/objects/objects.h>


    // Attribute visibility
    #define ATTRIB_VISIBILITY_PUBLIC           0        // Visible for all objects
    #define ATTRIB_VISIBILITY_PROTECTED        1        // Visible for current object and parents
    #define ATTRIB_VISIBILITY_PRIVATE          2        // Only visible for current object

    #define ATTRIB_IS_PUBLIC(attrib)    (((t_attrib_object *)attrib)->data.attr_visibility == ATTRIB_VISIBILITY_PUBLIC)
    #define ATTRIB_IS_PROTECTED(attrib) (((t_attrib_object *)attrib)->data.attr_visibility == ATTRIB_VISIBILITY_PROTECTED)
    #define ATTRIB_IS_PRIVATE(attrib)   (((t_attrib_object *)attrib)->data.attr_visibility == ATTRIB_VISIBILITY_PRIVATE)


    // Attribute access
    #define ATTRIB_ACCESS_RW                   0        // Can be redefined to another value
    #define ATTRIB_ACCESS_RO                   1        // Cannot be redefined.

    #define ATTRIB_IS_READWRITE(attrib) (((t_attrib_object *)attrib)->data.attr_access == ATTRIB_ACCESS_RW)
    #define ATTRIB_IS_READONLY(attrib)  (((t_attrib_object *)attrib)->data.attr_access == ATTRIB_ACCESS_RO)


    // Attribute types
    #define ATTRIB_TYPE_METHOD                 0        // Method
    #define ATTRIB_TYPE_CONSTANT               1        // Constant
    #define ATTRIB_TYPE_PROPERTY               2        // Property

    #define ATTRIB_IS_METHOD(attrib)    (((t_attrib_object *)attrib)->data.attr_type == ATTRIB_TYPE_METHOD)
    #define ATTRIB_IS_CONSTANT(attrib)  (((t_attrib_object *)attrib)->data.attr_type == ATTRIB_TYPE_CONSTANT)
    #define ATTRIB_IS_PROPERTY(attrib)  (((t_attrib_object *)attrib)->data.attr_type == ATTRIB_TYPE_PROPERTY)


    // Method types
    #define ATTRIB_METHOD_NONE                 0        // No arguments for this method
    #define ATTRIB_METHOD_STATIC               1        // Static method
    #define ATTRIB_METHOD_ABSTRACT             2        // Abstract method
    #define ATTRIB_METHOD_FINAL                4        // Finalized method
    #define ATTRIB_METHOD_CTOR                 8        // Constructor method
    #define ATTRIB_METHOD_DTOR                16        // Descructor method

    #define ATTRIB_METHOD_IS_STATIC(attrib)     ((((t_attrib_object *)attrib)->data.attr_method_flags & ATTRIB_METHOD_STATIC) == ATTRIB_METHOD_STATIC)
    #define ATTRIB_METHOD_IS_ABSTRACT(attrib)   ((((t_attrib_object *)attrib)->data.attr_method_flags & ATTRIB_METHOD_ABSTRACT) == ATTRIB_METHOD_ABSTRACT)
    #define ATTRIB_METHOD_IS_FINAL(attrib)      ((((t_attrib_object *)attrib)->data.attr_method_flags & ATTRIB_METHOD_FINAL) == ATTRIB_METHOD_FINAL)
    #define ATTRIB_METHOD_IS_CTOR(attrib)       ((((t_attrib_object *)attrib)->data.attr_method_flags & ATTRIB_METHOD_CTOR) == ATTRIB_METHOD_CTOR)
    #define ATTRIB_METHOD_IS_DTOR(attrib)       ((((t_attrib_object *)attrib)->data.attr_method_flags & ATTRIB_METHOD_DTOR) == ATTRIB_METHOD_DTOR)


    typedef struct {
        long attr_type;                     // ATTRIB_TYPE_* constants
        long attr_visibility;               // ATTRIB_VISIBILITY_* constants
        long attr_access;                   // ATTRIB_ACCESS_* constants
        long attr_method_flags;             // ATTRIB_METHOD_* constants

        t_object *attribute;                // Actual attribute (callback, or data value)

        t_object *bound_instance;           // Instance to which the attribute is bound. NULL when not bound (ie; when defined in a class)
        int bound_instance_decref;          // This tells if the bound_instance should be released or not.
        t_object *bound_class;              // Class to which the attribute is bound. This is always a class, NULL when not yet bound into a class.
        char *bound_name;                   // Name under which the attribute is known in the class. (ie: "bar" in "foo.bar")

    } t_attrib_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_attrib_object_data data;
        SAFFIRE_OBJECT_FOOTER
    } t_attrib_object;

    t_attrib_object Object_Attrib_struct;

    #define Object_Attrib   (t_object *)&Object_Attrib_struct

    void object_attrib_init(void);
    void object_attrib_fini(void);

    void object_attrib_bind(t_attrib_object *attrib_obj, t_object *bound_obj, char *name);
    t_attrib_object *object_attrib_duplicate(t_attrib_object *attrib, t_object *bound_obj);
    t_attrib_object *object_attrib_find(t_object *self, char *name);

#endif
