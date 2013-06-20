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
#ifndef __CALLABLE_H__
#define __CALLABLE_H__

    #include "objects/object.h"
    #include "objects/objects.h"
    #include "compiler/bytecode.h"

    // A method-argument hash consists of name => structure
    typedef struct _method_arg {
        t_object *value;
        t_string_object *typehint;
    } t_method_arg;


    /* Callable flags, mostly method flags */
    #define CALLABLE_FLAG_STATIC              1      /* Static callable */
    #define CALLABLE_FLAG_ABSTRACT            2      /* Abstract callable */
    #define CALLABLE_FLAG_FINAL               4      /* Final callable */
    #define CALLABLE_FLAG_CONSTRUCTOR         8      /* Constructor */
    #define CALLABLE_FLAG_DESTRUCTOR         16      /* Destructor */
    #define CALLABLE_FLAG_MASK               31

    /* Callable types. */
    #define CALLABLE_TYPE_METHOD            256
    #define CALLABLE_TYPE_MASK             1023 - CALLABLE_FLAG_MASK

    /* Callable code types */
    #define CALLABLE_CODE_INTERNAL         16384        /* This is an internal function (native_func) */
    #define CALLABLE_CODE_EXTERNAL         32768        /* This is an external function (bytecode) */
    #define CALLABLE_CODE_MASK             65535 - CALLABLE_TYPE_MASK

    #define CALLABLE_IS_CODE_INTERNAL(callable) ((((t_callable_object *)callable)->callable_flags & CALLABLE_CODE_INTERNAL) == CALLABLE_CODE_INTERNAL)
    #define CALLABLE_IS_CODE_EXTERNAL(callable) ((((t_callable_object *)callable)->callable_flags & CALLABLE_CODE_EXTERNAL) == CALLABLE_CODE_EXTERNAL)
    #define CALLABLE_IS_STATIC(callable)        ((((t_callable_object *)callable)->callable_flags & CALLABLE_FLAG_STATIC) == CALLABLE_FLAG_STATIC)
    #define CALLABLE_IS_ABSTRACT(callable)      ((((t_callable_object *)callable)->callable_flags & CALLABLE_FLAG_ABSTRACT) == CALLABLE_FLAG_ABSTRACT)
    #define CALLABLE_IS_FINAL(callable)         ((((t_callable_object *)callable)->callable_flags & CALLABLE_FLAG_FINAL) == CALLABLE_FLAG_FINAL)
    #define CALLABLE_IS_CONSTRUCTOR(callable)   ((((t_callable_object *)callable)->callable_flags & CALLABLE_FLAG_CONSTRUCTOR) == CALLABLE_FLAG_CONSTRUCTOR)
    #define CALLABLE_IS_DESTRUCTOR(callable)    ((((t_callable_object *)callable)->callable_flags & CALLABLE_FLAG_DESTRUCTOR) == CALLABLE_FLAG_DESTRUCTOR)

    #define CALLABLE_IS_TYPE_METHOD(callable)   ((((t_callable_object *)callable)->callable_flags & CALLABLE_TYPE_METHOD) == CALLABLE_TYPE_METHOD)

    typedef struct {
        SAFFIRE_OBJECT_HEADER

        int callable_flags;                   // Callable flags

        union {
            t_bytecode *bytecode;                              // External bytecode
            t_object *(*native_func)(t_object *, t_dll *);     // internal function
        } code;

        t_object *binding;                  // Bound to a class (or NULL)
        t_hash_object *arguments;           // Arguments (key => default value (or NULL))

        // @TODO: Additional information for callable?
//        int calls;                  // Number of calls made to this code
//        int time_spent;             // Time spend in this code
    } t_callable_object;

    t_callable_object Object_Callable_struct;

    #define Object_Callable   (t_object *)&Object_Callable_struct

    void object_callable_init(void);
    void object_callable_fini(void);

#endif
