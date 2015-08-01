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
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SU^CH DAMAGE.
*/
#ifndef __UNICODE_H__
#define __UNICODE_H__

    // Make sure unicode.h uses UTF8 instead of UTF16 internally
    #define U_CHARSET_IS_UTF8 1

    #include <unicode/uchar.h>
    #include <unicode/ucnv.h>
    #include <unicode/ustring.h>

    typedef struct _string t_string;

    void create_utf8_from_string(t_string *str);
    void utf8_free_unicode(t_string *str);

    int utf8_strcmp(const t_string *s1, const t_string *s2);

//    size_t utf8_strlen(const t_string *str);
//    t_string *utf8_memcpy_offset(const t_string *src, const size_t offset, const size_t count);
//    int utf8_strcmp(const t_string *s1, const t_string *s2);
//    t_string *utf8_strdup(const t_string *src);
//    void utf8_strcpy(t_string *dst, const t_string *src);
//    void utf8_strcat(t_string *dst, const t_string *src);

    size_t utf8_strstr(const t_string *haystack, const t_string *needle, const size_t offset);

    t_string *utf8_toupper(const t_string *src, const char *locale);
    t_string *utf8_tolower(const t_string *src, const char *locale);
    t_string *utf8_ucfirst(const t_string *src, const char *locale);

#endif

