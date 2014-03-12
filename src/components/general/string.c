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
#include "general/smm.h"


/**
 *
 */
t_string *char0_to_string(const char *s) {
    return char_to_string(s, strlen(s));
}

/**
 *
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
 *
 */
char *string_to_char(const t_string *str) {
    return string_strdup0(str->val);
}


/**
 *
 */
void string_free(t_string *s) {
    if (!s) return;

    if (s->unicode) smm_free(s->unicode);
    if (s->val) smm_free(s->val);
    smm_free(s);
}


t_string *string_new(void) {
    t_string *str = (t_string *)smm_malloc(sizeof(t_string));
    str->val = NULL;
    str->len = 0;
    str->unicode = NULL;
    return str;
}


char *string_strdup0(const char *s) {
    char *d = smm_malloc(strlen(s)+1);
    strcpy(d, s);
    return d;
}


t_string *string_strdup(t_string *s) {
    t_string *str = string_new();

    str->val = (char *)smm_malloc(s->len+1);    // 0 zerminated
    memcpy(str->val, s->val, s->len+1);
    str->len = s->len;

    return str;

}


t_string *string_strcat0(t_string *dst, const char *src) {
    t_string *s = char0_to_string(src);
    return string_strcat(dst, s);
}


t_string *string_strcat(t_string *dst, const t_string *src) {
    dst->val = (char *)smm_realloc(dst->val, dst->len + src->len + 1);
    memcpy(dst->val + dst->len, src->val, src->len);
    dst->val[dst->len + src->len] = '\0';
    dst->len += src->len;

    return dst;

}


int string_strcmp(t_string *s1, t_string *s2) {
    int res, len = s1->len;
    if (len > s2->len) len = s2->len;

    res = memcmp(s1->val, s2->val, len);
    if (! res) return res;

    return s1->len > s2->len ? 1 : -1;
}

int string_strcmp0(t_string *s1, const char *c_str) {
    t_string s2;
    s2.len = strlen(c_str);
    s2.val = (char *)c_str;
    return string_strcmp(s1, &s2);
}


t_string *string_copy_partial(t_string *src, int offset, int count) {
    t_string *dst = string_new();
    dst->len = count;
    dst->val = smm_malloc(count+1);
    memcpy(dst->val, (char *)(src->val + offset), count);
    dst->val[count] = '\0';

    return dst;
}
