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
#include <stdarg.h>
#include <general/string.h>
#include <general/smm.h>


/**
 * Our own custom ASPRINTF functionality. This is basically sprintf(), but
 * it will automatically allocate the memory for the string.
 *
 * Both t_string* and char* functions are present
 */



int smm_vasprintf_string(t_string **ret, t_string *format, va_list args) {
    char *tmp;

    va_list copy;
    va_copy(copy, args);

    int len = smm_vasprintf_char(&tmp, format->val, copy);

    va_end(copy);

    *ret = string_new();
    (*ret)->val = tmp;
    (*ret)->len = len;
    return (*ret)->len;
}


/**
 * Our own implementation. This ensures that *ret will always be NULL on an error.
 */
int smm_vasprintf_char(char **ret, const char *format, va_list args) {
    va_list copy;
    va_copy(copy, args);

    *ret = NULL;

    int len = vsnprintf(NULL, 0, format, args);

    if (len >= 0) {
        char *buffer = smm_malloc(len+1);
        if (! buffer) return -1;

        len = vsnprintf(buffer, len+1, format, copy);

        if (len < 0) {
            smm_free(buffer);
            return len;
        }
        *ret = buffer;
    }
    va_end(copy);

    return len;
}

int smm_asprintf_string(t_string **ret, t_string *format, ...) {
    va_list args;

    va_start(args, format);
    int len = smm_vasprintf_string(ret, format, args);
    va_end(args);

    return(len);
}

int smm_asprintf_char(char **ret, const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = smm_vasprintf_char(ret, format, args);
    va_end(args);

    return(len);
}
