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
#ifndef __ATTRIB_H__
#define __ATTRIB_H__

    #include "objects/object.h"
    #include "objects/hash.h"

    // Attribute defines
    #define ATTRIB_VISIBILITY_PUBLIC           0        // Visible for all objects
    #define ATTRIB_VISIBILITY_PROTECTED        1        // Visible for current object and parents
    #define ATTRIB_VISIBILITY_PRIVATE          2        // Only visible for current object

    #define ATTRIB_ACCESS_RW                   0        // Can be redefined to another value
    #define ATTRIB_ACCESS_RO                   1        // Cannot be redefined

    #define ATTRIB_TYPE_METHOD                 0        // Method
    #define ATTRIB_TYPE_CONSTANT               1        // Constant
    #define ATTRIB_TYPE_PROPERTY               2        // Property

    // These are bitwise flags!
    #define METHOD_FLAG_STATIC                 1        // Can be called in class context
    #define METHOD_FLAG_FINAL                  2        // Cannot be extended
    #define METHOD_FLAG_ABSTRACT               4        // Must be extended by class


    #define METHOD_IS_STATIC(attrib) ((((t_attrib_object *)attrib)->method_flags & METHOD_FLAG_STATIC) == METHOD_FLAG_STATIC)
    #define METHOD_IS_FINAL(attrib) ((((t_attrib_object *)attrib)->method_flags & METHOD_FLAG_FINAL) == METHOD_FLAG_FINAL)
    #define METHOD_IS_ABSTRACT(attrib) ((((t_attrib_object *)attrib)->method_flags & METHOD_FLAG_ABSTRACT) == METHOD_FLAG_ABSTRACT)

    #define ATTRIB_IS_PUBLIC(attrib) (((t_attrib_object *)attrib)->visiblity == ATTRIB_VISIBILITY_PUBLIC)
    #define ATTRIB_IS_PROTECTED(attrib) (((t_attrib_object *)attrib)->visiblity == ATTRIB_VISIBILITY_PROTECTED)
    #define ATTRIB_IS_PRIVATE(attrib) (((t_attrib_object *)attrib)->visiblity == ATTRIB_VISIBILITY_PRIVATE)

    #define ATTRIB_IS_READWRITE(attrib) (((t_attrib_object *)attrib)->access == ATTRIB_ACCESS_RW)
    #define ATTRIB_IS_READONLY(attrib) (((t_attrib_object *)attrib)->access == ATTRIB_ACCESS_RO)

    #define ATTRIB_IS_METHOD(attrib) (((t_attrib_object *)attrib)->attrib_type == ATTRIB_TYPE_METHOD)
    #define ATTRIB_IS_CONSTANT(attrib) (((t_attrib_object *)attrib)->attrib_type == ATTRIB_TYPE_CONSTANT)
    #define ATTRIB_IS_PROPERTY(attrib) (((t_attrib_object *)attrib)->attrib_type == ATTRIB_TYPE_PROPERTY)


    typedef struct {
        SAFFIRE_OBJECT_HEADER

        char attrib_type;                   // Attribute type (constant,property,method)
        char visibility;                    // Visibility of the attribute
        char access;                        // Access of the attribute (read/write)

        t_object *attribute;                // Actual attribute

        t_object *binding;                  // Method bound to this specified class or object

        char method_flags;                  // method flags (abstract, static, final etc)
        struct _hash_object *arguments;     // Arguments, in case of a method type (key => default value (or NULL))
    } t_attrib_object;

    t_attrib_object Object_Attrib_struct;

    #define Object_Attrib   (t_object *)&Object_Attrib_struct

    void object_attrib_init(void);
    void object_attrib_fini(void);

#endif