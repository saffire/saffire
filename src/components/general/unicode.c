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
#include <stdio.h>
#include <string.h>

#include "general/string.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/output.h"

// Our default converter (@TODO: what about reading from other converters like utf16 etc??)
UConverter *converter;


/**
 * Free unicode string if available
 */
void utf8_free_unicode(t_string *str) {
    if (! str->unicode) return;

    smm_free(str->unicode);
    str->unicode = NULL;
}


/**
 * convert t_string into a utf8 unicode string which gets stored into the t_string
 */
UChar *utf8_from_string(t_string *str) {
    // unicode string already created
    if (str->unicode) return str->unicode;

    // Create unicode from string, and store this in string
    str->unicode = (UChar *)smm_malloc(str->len);
    u_uastrncpy(str->unicode, str->val, str->len);

    return str->unicode;
}


/**
 * convert utf8 unicode string into a t_string
 */
t_string *utf8_to_string(UChar *str, size_t len) {
    // Convert unicode back to string
    char *c_str = (char *)smm_malloc(len);
    u_austrncpy(c_str, str, len);

    // Create string from char, and we can also cache the unicode
    t_string *dst = char_to_string(c_str, len);
    dst->unicode = str;

    return dst;
}

//
///**
// *
// */
//t_string *utf8_char_to_string(char *value, size_t value_len) {
//    UErrorCode status = U_ZERO_ERROR;
//
//    if (converter == NULL) {
//        // Create new converted if needed
//        converter = ucnv_open("UTF-8", &status);
//        if (U_FAILURE(status)) {
//            fatal_error(1, "Cannot create converter: %d\n", status);
//        }
//    }
//
//    // Allocate room and convert
//    t_string *utf8 = utf8_string_new(value_len);
//    return utf8;
//}
//
//
///**
// *
// */
//char *utf8_string_to_char(t_string *s, int *bytes_len) {
//    char *bytes = (char *)smm_malloc(s->len);
//    u_austrncpy(bytes, s->val, s->len);
//
//    if (bytes_len != NULL) {
//        *bytes_len = s->len;
//    }
//
//    return bytes;
//}

int utf8_strstr(t_string *haystack, t_string *needle) {
    utf8_from_string(haystack);
    utf8_from_string(needle);

    return (u_strstr(haystack->unicode, needle->unicode) != NULL);
}

/**
 *
 */
int utf8_strcmp(t_string *s1, t_string *s2) {
    int res, len = s1->len;
    if (len > s2->len) len = s2->len;

    utf8_from_string(s1);
    utf8_from_string(s2);

    if ((res = u_memcmp(s1->unicode, s2->unicode, len))) return res;
    return s1->len > s2->len ? 1 : -1;
}

//t_string *utf8_strdup(t_string *src) {
//    t_string *s = utf8_string_new(src->len);
//    u_memcpy(s->val, src->val, src->len);
//    return s;
//}
//
//t_string *utf8_string_new(int len) {
//    t_string *s = (t_string *)smm_malloc(sizeof(t_string));
//    s->val = (UChar *)smm_malloc(len * sizeof(UChar));
//    s->len = len;
//    return s;
//}

//void utf8_string_free(t_string *s) {
//    if (!s) return;
//
//    if (s->val) smm_free(s->val);
//    smm_free(s);
//}

//void utf8_strcpy(t_string *dst, t_string *src) {
//    u_strncpy(dst->val, src->val, src->len);
//}
//
//void utf8_strcat(t_string *dst, t_string *src) {
//    u_strncat(dst->val + dst->len, src->val, src->len);
//}

//t_string *utf8_memcpy_offset(t_string *src, int offset, int count) {
//    t_string *dst = utf8_string_new(count);
//
//    u_memcpy(dst->val, src->val + offset, count);
//    return dst;
//}


/**
 *
 */
t_string *utf8_toupper(t_string *src, char *locale) {
    UErrorCode status = U_ZERO_ERROR;

    utf8_from_string(src);

    // Convert the unicode string
    UChar *u_str = (UChar *)smm_malloc(src->len);
    u_strToUpper(u_str, src->len, src->unicode, src->len, locale, &status);

    t_string *dst = utf8_to_string(u_str, src->len);
    return dst;
}


/**
 *
 */
t_string *utf8_tolower(t_string *src, char *locale) {
    UErrorCode status = U_ZERO_ERROR;

    utf8_from_string(src);

    // Convert the unicode string
    UChar *u_str = (UChar *)smm_malloc(src->len);
    u_strToLower(u_str, src->len, src->unicode, src->len, locale, &status);

    t_string *dst = utf8_to_string(u_str, src->len);
    return dst;
}



