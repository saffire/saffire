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
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SU^CH DAMAGE.
*/
#ifndef __UNICODE_H__
#define __UNICODE_H__

    #define U_CHARSET_IS_UTF8 1

    #include "unicode/uchar.h"
    #include "unicode/ucnv.h"
    #include "unicode/ustring.h"

    typedef struct _string t_string;

//    typedef struct {
//        UChar   *val;       // Binary safe UTF8 string
//    } t_unicode_string;

    void utf8_free_unicode(t_string *str);

    int utf8_strcmp(t_string *s1, t_string *s2);

//    #define UTF8_C2U(c) utf8_char_to_string(c, strlen(c))

//    t_string *utf8_string_new(int len);
//    void utf8_string_free(t_string *s);

//    t_string *utf8_char_to_string(char *value, size_t value_len);
//    char *utf8_string_to_char(t_string *str, int *bytes_len);

//    size_t utf8_strlen(t_string *str);

//    t_string *utf8_memcpy_offset(t_string *src, int offset, int count);

//    int utf8_strcmp(t_string *s1, t_string *s2);
//    t_string *utf8_strdup(t_string *src);

//    void utf8_strcpy(t_string *dst, t_string *src);
//    void utf8_strcat(t_string *dst, t_string *src);

    int utf8_strstr(t_string *haystack, t_string *needle);

    t_string *utf8_toupper(t_string *src, char *locale);
    t_string *utf8_tolower(t_string *src, char *locale);

//    int utf8_strstr(t_string *haystack, t_string *needle);

#endif

