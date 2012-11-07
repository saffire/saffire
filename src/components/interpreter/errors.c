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
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "interpreter/errors.h"
#include "general/dll.h"

extern t_dll *lineno_stack;

#define STREAM_ERROR stderr
#define STREAM_WARNING stderr


/**
 * Print out an error and exit
 */
void saffire_error(char *str, ...) {
    t_dll_element *e = DLL_TAIL(lineno_stack);
    int lineno = (int)e->data;

    va_list args;
    va_start(args, str);
    fprintf(STREAM_ERROR, "Error in line %d: ", lineno);
    vfprintf(STREAM_ERROR, str, args);
    fprintf(STREAM_ERROR, "\n");
    va_end(args);
    exit(1);
}

/**
 * @TODO: make the output stream configurable? Either
 * Print out an error and exit
 */
void saffire_warning(char *str, ...) {
    t_dll_element *e = DLL_TAIL(lineno_stack);
    int lineno = (int)e->data;

    va_list args;
    va_start(args, str);
    fprintf(STREAM_WARNING, "Warning in line %d: ", lineno);
    vfprintf(STREAM_WARNING, str, args);
    fprintf(STREAM_WARNING, "\n");
    va_end(args);
}
