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
    #include "vm/vmtypes.h"

    // A method-argument hash consists of name => structure
    typedef struct _method_arg {
        t_object *value;
        t_string_object *typehint;
    } t_method_arg;

    /* Callable code types */
    #define CALLABLE_CODE_INTERNAL         1        /* This is an internal function (native_func) */
    #define CALLABLE_CODE_EXTERNAL         2        /* This is an external function (bytecode) */

    #define CALLABLE_IS_CODE_INTERNAL(callable) ((((t_callable_object *)callable)->data.routing & CALLABLE_CODE_INTERNAL) == CALLABLE_CODE_INTERNAL)
    #define CALLABLE_IS_CODE_EXTERNAL(callable) ((((t_callable_object *)callable)->data.routing & CALLABLE_CODE_EXTERNAL) == CALLABLE_CODE_EXTERNAL)

    typedef struct {
        int routing;     // CALLABLE_CODE_*

        union {
            struct {
                struct _vm_codeblock *codeblock;                    // Internal codeblock
            } external;
            struct {
                t_object *(*native_func)(t_object *, t_dll *);      // internal function
            } internal;
        } code;

        t_object *binding;                  // Bound to this attrib.  // @TODO: could be NULL when it's not bound (like a closure???)
        t_hash_table *arguments;            // Arguments (key => default value (or NULL))
    } t_callable_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_callable_object_data data;
    } t_callable_object;

    t_callable_object Object_Callable_struct;

    #define Object_Callable   (t_object *)&Object_Callable_struct

    void object_callable_init(void);
    void object_callable_fini(void);

#endif
