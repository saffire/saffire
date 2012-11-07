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
#ifndef __METHOD_H__
#define __METHOD_H__

    #include "object/object.h"

    #define METHOD_NO_FLAGS                 0      /* No flags */
    #define METHOD_FLAG_STATIC              1      /* Static method */
    #define METHOD_FLAG_ABSTRACT            2      /* Abstract method */
    #define METHOD_FLAG_FINAL               4      /* Final method */
    #define METHOD_FLAG_CONSTRUCTOR         8      /* Constructor */
    #define METHOD_FLAG_DESTRUCTOR         16      /* Destructor */
    #define METHOD_FLAG_MASK               31

    #define METHOD_VISIBILITY_PUBLIC        1      /* Public visibility */
    #define METHOD_VISIBILITY_PROTECTED     2      /* Protected visibility */
    #define METHOD_VISIBILITY_PRIVATE       3      /* Private visibility */

    #define RETURN_METHOD(f, v, cl, co)   RETURN_OBJECT(object_new(Object_Method, f, v, cl, co));

    #define METHOD_IS_STATIC(method) ((method->mflags & METHOD_FLAG_MASK) == METHOD_FLAG_STATIC)
    #define METHOD_IS_ABSTRACT(method) ((method->mflags & METHOD_FLAG_MASK) == METHOD_FLAG_ABSTRACT)
    #define METHOD_IS_FINAL(method) ((method->mflags & METHOD_FLAG_MASK) == METHOD_FLAG_FINAL)
    #define METHOD_IS_CONSTRUCTOR(method) ((method->mflags & METHOD_FLAG_MASK) == METHOD_FLAG_CONSTRUCTOR)
    #define METHOD_IS_DESTRUCTOR(method) ((method->mflags & METHOD_FLAG_MASK) == METHOD_FLAG_DESTRUCTOR)

    #define METHOD_IS_PUBLIC(method) ((method->visibility == METHOD_VISIBILITY_PUBLIC)
    #define METHOD_IS_PROTECTED(method) ((method->visibility == METHOD_VISIBILITY_PROTECTED)
    #define METHOD_IS_PRIVATE(method) ((method->visibility == METHOD_VISIBILITY_PRIVATE)


    typedef struct {
        SAFFIRE_OBJECT_HEADER

        int mflags;                 // Method flags
        int visibility;             // Method visibility

        t_object *class;            // Bound to a class (or NULL)
        struct _code_object *code;        // Code for this method

//        // Additional information for methods
//        int calls;                  // Number of calls made to this method
//        int time_spent;             // Time spend in this method
    } t_method_object;

    t_method_object Object_Method_struct;

    #define Object_Method   (t_object *)&Object_Method_struct

    void object_method_init(void);
    void object_method_fini(void);

#endif