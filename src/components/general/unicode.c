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
#include <stdio.h>
#include <string.h>

#include <saffire/general/string.h>
#include <saffire/objects/objects.h>
#include <saffire/general/smm.h>
#include <saffire/general/output.h>

// Our default converter (@TODO: what about reading from other converters like utf16 etc??)
UConverter *converter;


/*
 * Unicode strings (UCode) are stored within a t_string. In order to speed up processing, all
 * strings are stored by default as bytestreams without encoding. As soon as any textual processing
 * needs to take place (like comparing, slicing etc), it will need to "convert" this bytestream
 * to a Ucode unicode string. It will store this inside the t_string structure, which means that it
 * only has to do this once, and only when usage is really needed. It also means that these utf* functions
 * will change the incoming t_string objects, as unicode representations can be stored on them.
 */


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
void create_utf8_from_string(t_string *str) {
    // unicode string already created
    if (str->unicode) return;

    // Create unicode from string, and store this in string
    str->unicode = (UChar *)smm_malloc(sizeof(UChar) * (str->len + 1));
    u_uastrncpy(str->unicode, str->val, str->len);
}


/**
 * Convert utf8 unicode string into a t_string
 */
static t_string *utf8_to_string(const UChar *str, size_t len) {
    // Convert unicode back to string
    char *c_str = (char *)smm_malloc(len);
    u_austrncpy(c_str, str, len);

    // Create string from char, and we can also cache the unicode
    t_string *dst = char_to_string(c_str, len);

    dst->unicode = (UChar *)smm_malloc(sizeof(UChar) * (len + 1));
    u_memcpy(dst->unicode, str, len);

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

/**
 * Find a substring withing a string. When offset > 0, it will start at that offset in the string.
 *
 * Note that offset is in chars, not in bytes
 */
size_t utf8_strstr(const t_string *haystack, const t_string *needle, size_t offset) {
    UChar *pos = u_strstr(haystack->unicode + offset, needle->unicode);

    if (pos == NULL) return -1;

    return (pos - (haystack->unicode + offset));
}

/**
 * Compares s1 against s2. Returns 0 when equal, -1 when s1 if "larger" and 1 when s2 is "larger".
 */
int utf8_strcmp(const t_string *s1, const t_string *s2) {
    int res, len = s1->len;
    if (len > s2->len) len = s2->len;

    res = u_memcmp(s1->unicode, s2->unicode, len);
    if (! res) return res;
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
 * Returns an upper cased string
 */
t_string *utf8_toupper(const t_string *src, const char *locale) {
    UErrorCode status = U_ZERO_ERROR;

    if (src->unicode == NULL) {
        return NULL;
    }

    // Convert the unicode string
    UChar *u_str = (UChar *)smm_malloc(sizeof(UChar) * (src->len + 1));
    u_strToUpper(u_str, src->len+1, src->unicode, src->len, locale, &status);

    t_string *dst = utf8_to_string(u_str, src->len);
    return dst;
}


/**
 * Returns a lower cased string
 */
t_string *utf8_tolower(const t_string *src, const char *locale) {
    UErrorCode status = U_ZERO_ERROR;

    if (src->unicode == NULL) {
        return NULL;
    }

    // Convert the unicode string
    UChar *u_str = (UChar *)smm_malloc(sizeof(UChar) * (src->len + 1));
    u_strToLower(u_str, src->len, src->unicode, src->len, locale, &status);

    t_string *dst = utf8_to_string(u_str, src->len);
    return dst;
}

/**
 * Returns a lower cased string with the first char upper cased
 */
t_string *utf8_ucfirst(const t_string *src, const char *locale) {
    UErrorCode status = U_ZERO_ERROR;

    if (src->unicode == NULL) {
        return NULL;
    }

    // Convert the unicode string
    UChar *u_str = (UChar *)smm_malloc(sizeof(UChar) * (src->len + 1));
    u_strToLower(u_str, src->len, src->unicode, src->len, locale, &status);

    u_strToUpper(u_str, 1, src->unicode, src->len, locale, &status);

    t_string *dst = utf8_to_string(u_str, src->len);
    return dst;
}



