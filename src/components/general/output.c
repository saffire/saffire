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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <saffire/general/printf.h>
#include <saffire/general/output.h>
#include <saffire/memory/smm.h>


/**
 * Generic output handlers that write to a file (which could be STDIO or a regular file)
 */
static int _stdio_output_char_helper(FILE *f, char c) {
    return fwrite(&c, 1, 1, f);
}
static int _stdio_output_string_helper(FILE *f, t_string *s) {
    return fwrite(STRING_CHAR0(s), STRING_LEN(s), 1, f);
}

// By default, the helpers will print to STDIO
int (*output_char_helper)(FILE *f, char c) = _stdio_output_char_helper;
int (*output_string_helper)(FILE *f, t_string *s) = _stdio_output_string_helper;



/**
 * Outputs a string to a specified file
 */
static void _output_string(FILE *f, t_string *format, va_list args) {
    t_string *expanded_str;

    if (args == NULL) {
        smm_asprintf_string(&expanded_str, format, NULL);
    } else {
        smm_vasprintf_string(&expanded_str, format, args);
    }
    output_string_helper(f, expanded_str);
    string_free(expanded_str);
}

/**
 * Outputs to a specified file
 */
static void _output_char0(FILE *f, char *format, va_list args) {
    char *buf;

    if (args == NULL) {
        smm_asprintf_char(&buf, format, NULL);
    } else {
        smm_vasprintf_char(&buf, format, args);
    }
    t_string *str = char0_to_string(buf);
    smm_free(buf);

    output_string_helper(f, str);
    string_free(str);
}



void output_flush(void) {
    fflush(stdout);
}

/**
 * Outputs (to stdout)
 */
void output_string(t_string *format, ...) {
    va_list args;

    va_start(args, format);
    _output_string(stdout, format, args);
    va_end(args);
}

/**
 * Outputs (to stdout)
 */
void output_char(char  *format, ...) {
    va_list args;

    va_start(args, format);
    _output_char0(stdout, format, args);
    va_end(args);
}

/**
 * Output warning (to stderr)
 */
void output_debug_string(t_string *format, ...) {
    va_list args;

    va_start(args, format);
    _output_string(stderr, format, args);
    va_end(args);
}

/**
 * Output warning (to stderr)
 */
void output_debug_char(char *format, ...) {
    va_list args;

    va_start(args, format);
    _output_char0(stderr, format, args);
    va_end(args);
}

/**
 * Output warning (to stderr)
 */
void warning(char *format, ...) {
    va_list args;

    _output_char0(stderr, "Warning: ", NULL);
    va_start(args, format);
    _output_char0(stderr, format, args);
    va_end(args);

    fflush(stderr);
}

/**
 * Output error (to stderr)
 */
void error(char *format, ...) {
    va_list args;

    _output_char0(stderr, "Error: ", NULL);
    va_start(args, format);
    _output_char0(stderr, format, args);
    va_end(args);

    fflush(stderr);
}


/**
 * Ouputs error (to stderr) and exits Saffire with code.
 */
void fatal_error(int exitcode, char *format, ...) {
    va_list args;

    _output_char0(stderr, "Fatal error: ", NULL);
    va_start(args, format);
    _output_char0(stderr, format, args);
    va_end(args);

    fflush(stderr);

    exit(exitcode);
}


/**
 * Printf's a string to stdout
 */
void output_string_printf(t_string *format, t_dll *args) {
    arg_printf_string(stdout, format, args, output_char_helper);
}


/**
 * Changes the current helpers that do the actual output.
 *
 * @param char_helper
 * @param string_helper
 */
void output_set_helpers(t_char_helper char_helper, t_string_helper string_helper) {
    output_char_helper = char_helper;
    output_string_helper = string_helper;
}

/**
 * Returns the current output helpers
 */
void output_get_helpers(t_char_helper *char_helper, t_string_helper *string_helper) {
    *char_helper = output_char_helper;
    *string_helper = output_string_helper;
}

