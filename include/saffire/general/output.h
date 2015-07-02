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
#ifndef __OUTPUT_H__
#define __OUTPUT_H__

    #include <stdio.h>
    #include <stdarg.h>
    #include <saffire/general/dll.h>
    #include <saffire/general/string.h>

    #define ANSI_BRIGHTRED    "\33[31;1m"
    #define ANSI_BRIGHTGREEN  "\33[32;1m"
    #define ANSI_BRIGHTYELLOW "\33[33;1m"
    #define ANSI_BRIGHTBLUE   "\33[34;1m"
    #define ANSI_RESET        "\33[0m"

    void output_set_helpers(int (*char_helper)(FILE *f, char c), int (*string_helper)(FILE *f, t_string *s));
    void output_get_helpers(int (**char_helper)(FILE *f, char c), int (**string_helper)(FILE *f, t_string *s));

    void output_flush(void);

    // Normal output
    void output_char(char *format, ...);
    void output_string(t_string *format, ...);

    void output_debug_char(char *format, ...);
    void output_debug_string(t_string *format, ...);

//    void output_char_printf(char *format, t_dll *args);
    void output_string_printf(t_string *format, t_dll *args);

    void output_debug_char_printf(char *format, t_dll *args);
    void output_debug_string_printf(t_string *format, t_dll *args);


    // Errors and warnings are only for chars. We probably don't need string/unicode variants for this
    void error(char *format, ...);
    void warning(char *format, ...);
    void fatal_error(int exitcode, char *format, ...);

#endif
