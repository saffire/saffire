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
#include <saffire/memory/smm.h>

/**
 * This file deals with both saffire binary-safe strings (t_string) as well as
 * "standard" zero-terminated C strings. Much functionality is present to convert
 * from either format.
 *
 * In order to make the system more robust, do not use the standard
 * string-functionality outside this file.
 */


/**
 * Converts a zero-terminated ascii string to a t_string
 */
t_string *char0_to_string(const char *s) {
    return char_to_string(s, strlen(s));
}

/**
 * Converts a binary safe string into a t_string
 */
t_string *char_to_string(const char *s, size_t len) {
    t_string *str = string_new();

    str->val = (char *)smm_malloc((len+1) * sizeof(char));
    memcpy(str->val, s, len);
    *(str->val + len) = '\0';
    str->len = len;

    return str;
}


/**
 * Converts a t_string into a zero-terminated char.
 *
 * Note that this function is not binary safe.
 */
char *string_to_char0(const t_string *str) {
    return string_strdup0(str->val);
}


/**
 * Frees a t_string
 */
void string_free(t_string *s) {
    if (!s) return;

    if (s->unicode) smm_free(s->unicode);
    if (s->val) smm_free(s->val);

    smm_free(s);
    s = NULL;
}


/**
 * Allocates an empty t_string structure
 */
t_string *string_new(void) {
    t_string *str = (t_string *)smm_malloc(sizeof(t_string));
    str->val = NULL;
    str->len = 0;
    str->unicode = NULL;
    return str;
}


/**
 * Duplicates a zero terminated string
 */
char *string_strdup0(const char *s) {
    char *d = smm_malloc(strlen(s)+1);
    strcpy(d, s);
    return d;
}


/**
 * Duplicates a t_string.
 * @TODO: MEDIUM: Does not duplicate unicode, even if it's available
 */
t_string *string_strdup(const t_string *s) {
    t_string *str = string_new();

    // @TODO: HIGH: We should not add an additional zero, as duplicating a string many times result in many
    // additional 0-bytes!

    str->val = (char *)smm_malloc(s->len+1);    // string will always be zero terminated
    memcpy(str->val, s->val, s->len+1);
    str->len = s->len;

    return str;

}

/**
 * Concatenates a zero-terminated string to a t_string.
 */
t_string *string_strcat0(t_string *dst, const char *src) {
    t_string *s = char0_to_string(src);

    return string_strcat(dst, s);
}


/**
 * Concatenates two strings together
 */
t_string *string_strcat(t_string *dst, const t_string *src) {
    // Reallocate memory and copy
    dst->val = (char *)smm_realloc(dst->val, dst->len + src->len + 1);
    memcpy(dst->val + dst->len, src->val, src->len);

    // Set terminating zero (always added!)
    dst->val[dst->len + src->len] = '\0';

    dst->len += src->len;

    if (dst->unicode) {
        // Unicode string has changed, so free if needed
        smm_free(dst->unicode);
    }

    return dst;

}

/**
 * Returns position of needle in haystack, starting the search from offset.
 *
 * This function is not unicode-safe
 */
int string_strpos(const t_string *haystack, const t_string *needle, size_t offset) {
    if (offset > haystack->len) {
        return -1;
    }

    char *str = haystack->val + offset;
    char *p = strstr(str, needle->val);

    return (str - p);
}

/**
 * Compare two strings, returns 0 when equal. 1 when s2 > s1, and -1 when s1 < s2
 *
 * This function is not unicode-safe
 */
int string_strcmp(const t_string *s1, const t_string *s2) {
    int res, len = s1->len;
    if (len > s2->len) len = s2->len;

    res = memcmp(s1->val, s2->val, len);
    if (! res) return res;

    return s1->len > s2->len ? 1 : -1;
}


/**
 * Compares a binary safe string against a zero terminated string.
 */
int string_strcmp0(const t_string *s1, const char *c_str) {
    t_string s2;
    s2.len = strlen(c_str);
    s2.val = (char *)c_str;

    return string_strcmp(s1, &s2);
}


/**
 * Copies a part of a string into a new string. Starting at offset, of "count" bytes.
 *
 * Note that this function is not unicode safe.
 */
t_string *string_copy_partial(const t_string *src, size_t offset, size_t count) {
    t_string *dst = string_new();
    dst->len = count;
    dst->val = smm_malloc(count+1);
    memcpy(dst->val, (char *)(src->val + offset), count);
    dst->val[count] = '\0';

    return dst;
}


/**
 * Copies a (partial) zero terminated string.
 */
char *string_strncpy0(const char *s, size_t len) {
    char *dst = smm_malloc(len + 1);

    strncpy(dst, s, len);
    dst[len] = '\0';

    return dst;
}
