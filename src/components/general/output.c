/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
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
#include "general/output.h"

/**
 *
 */
static void _output(FILE *f, const char *format, va_list args) {
    vfprintf(f, format, args);
}

/**
 * Outputs to specified file
 */
void foutput(FILE *fp, const char *format, ...) {
    va_list args;

    va_start(args, format);
    _output(fp, format, args);
    va_end(args);
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
 * Ouputs error (to stderr)
 */
void error(const char *format, ...) {
    va_list args;

    _output(stderr, "Error: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

}

/**
 * Ouputs error (to stderr) and exists with code.
 */
void error_and_die(int exitcode, const char *format, ...) {
    va_list args;

    _output(stderr, "Error: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

    exit(exitcode);
}


/**
 * Ouputs error (to stderr) and exists with code.
 */
void line_error_and_die(int exitcode, int lineno, const char *format, ...) {
    va_list args;

    foutput(stderr, "Error in line %d: ", lineno);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);
    _output(stderr, "\n", NULL);

    exit(exitcode);
}