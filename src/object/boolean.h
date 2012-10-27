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
#ifndef __OBJECT_BOOLEAN_H__
#define __OBJECT_BOOLEAN_H__

    #include "object/object.h"

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        long    value;              // 0 = false, 1 = true
    } t_boolean_object;

    t_boolean_object Object_Boolean_struct;
    t_boolean_object Object_Boolean_True_struct;
    t_boolean_object Object_Boolean_False_struct;

    #define Object_Boolean  ((t_object *)&Object_Boolean_struct)
    #define Object_True     ((t_object *)&Object_Boolean_True_struct)
    #define Object_False    ((t_object *)&Object_Boolean_False_struct)

    // Simple macro to return either TRUE or FALSE objects from a function
    #define RETURN_TRUE   { object_inc_ref((t_object *)&Object_Boolean_True_struct); return (t_object *)(&Object_Boolean_True_struct); }
    #define RETURN_FALSE  { object_inc_ref((t_object *)&Object_Boolean_False_struct); return (t_object *)(&Object_Boolean_False_struct); }

    void object_boolean_init(void);
    void object_boolean_fini(void);

#endif