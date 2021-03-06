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
#ifndef __STRING_H__
#define __STRING_H__

    #include <saffire/general/unicode.h>
    #include <saffire/general/md5.h>


    // Forward defined in general/unicode.h

    // t_string are compatible with 0-terminated char strings.
    struct _string {
        char            *val;           // Pointer to char data
        size_t          len;            // Length of the string
        UChar           *unicode;       // Unicode string. May or may not be filled.
    };

    #define STRING_CHAR0(str)       str->val
    #define STRING_LEN(str)         str->len
    #define STRING_UNICODE(str)     str->unicode

    t_string *char0_to_string(const char *s);
    t_string *char_to_string(const char *s, size_t len);
    char *string_to_char0(const t_string *s);

    t_string *string_new(void);

    int string_strcmp(const t_string *s1, const t_string *s2);
    int string_strcmp0(const t_string *s1, const char *c_str);

    char *string_strncpy0(const char *s, size_t len);

    char *string_strdup0(const char *s);
    t_string *string_strdup(const t_string *s);

    t_string *string_strcat0(const t_string *pre, const char *post);
    t_string *string_strcat(const t_string *pre, const t_string *post);

    t_string *string_copy_partial(const t_string *src, size_t offset, size_t count);

    int string_strpos(const t_string *haystack, const t_string *needle, size_t offset);

    void string_free(t_string *str);

#endif

