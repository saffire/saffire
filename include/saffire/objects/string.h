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
#ifndef __OBJECT_STRING_H__
#define __OBJECT_STRING_H__

    #include <saffire/objects/object.h>
    #include <saffire/general/string.h>

    // Allocate string objects
    #define STR02OBJ(mystr)          object_alloc_instance(Object_String, 2, strlen(mystr), mystr)
    #define STR2OBJ(mystr)           object_alloc_instance(Object_String, 1, mystr)

    // Return a zero-terminated string
    #define RETURN_STRING_FROM_CHAR(s)                  RETURN_OBJECT(STR02OBJ(s))

    // Return binary safe string
    #define RETURN_STRING_FROM_BINSAFE_CHAR(l, s)       RETURN_OBJECT(object_alloc_instance(Object_String, 2, l, s))

    #define RETURN_STRING(s)                            RETURN_OBJECT(STR2OBJ(s))

    // Returns value inside the string object's t_string
    #define STROBJ2CHAR0(obj)                           ((((t_string_object *)obj)->data.value)->val)
    // Returns length inside the string object's t_string
    #define STROBJ2CHAR0LEN(obj)                        ((((t_string_object *)obj)->data.value)->len)


    typedef struct {
        t_string *value;            // string value
        md5_byte_t hash[16];        // (MD5) hash of the actual string
        int needs_hashing;          // 1 : string needs hashing, 0 : hash done

        int iter;                   // Simple iteration index on the characters
        char *locale;               // Locale
    } t_string_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_string_object_data data;
    } t_string_object;

    t_string_object Object_String_struct;

    #define Object_String   (t_object *)&Object_String_struct

    void object_string_init(void);
    void object_string_fini(void);


    int object_string_hash_compare(t_string_object *s1, t_string_object *s2);
    int object_string_compare(t_string_object *s1, t_string_object *s2);

#endif
