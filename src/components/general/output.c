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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "general/printf.h"
#include "general/output.h"


/**
 * Generic output handlers that write to stdout
 */
static int _stdio_output_char_helper(char c) {
    return fwrite(&c, 1, 1, stdout);
}
static int _stdio_output_string_helper(char *s) {
    return fwrite(s, strlen(s), 1, stdout);
}

int (*output_char_helper)(char c) = _stdio_output_char_helper;
int (*output_string_helper)(char *s) = _stdio_output_string_helper;




/**
 * Outputs to a specified file
 */
static void _output(FILE *f, const char *format, va_list args) {
    char *buf;

    if (args == NULL) {
        asprintf(&buf, format, NULL);
    } else {
        vasprintf(&buf, format, args);
    }
    output_string_helper(buf);
    free(buf);
}



/**
 * Outputs (to stdout)
 */
void output(const char *format, ...) {
    va_list args;

    va_start(args, format);
    _output(stdout, format, args);
    va_end(args);
}


/**
 * Output warning (to stderr)
 */
void warning(const char *format, ...) {
    va_list args;

    _output(stderr, "\033[43;30m", NULL);

    _output(stderr, "Warning: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

    _output(stderr, "\033[0m", NULL);

    fflush(stdout);
}


/**
 * Ouputs error (to stderr) and exists with code.
 */
void fatal_error(int exitcode, const char *format, ...) {
    va_list args;

    _output(stderr, "\033[41;33;1m", NULL);

    _output(stderr, "Fatal error: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

    _output(stderr, "\033[0m", NULL);

    exit(exitcode);
}


/**
 */
void output_printf(const char *format, t_dll *args) {
    arg_printf(format, args, output_char_helper);
}



