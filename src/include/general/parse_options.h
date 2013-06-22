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
#ifndef __PARSE_OPTIONS_H__
#define __PARSE_OPTIONS_H__

    // Might be already defined if getopt is included
    #ifndef no_argument
        #define     no_argument         0
    #endif
    #ifndef required_argument
        #define     required_argument   1
    #endif
    #ifndef optional_argument
        #define     optional_argument   2
    #endif

    char **saffire_params;
    int saffire_params_count;

    struct saffire_option {
        const char *longname;
        const char *shortname;
        int has_arg;                // no_argument,
        void(*func)(void *);        // Function to call when this setting is found
    };

    void saffire_parse_options(int, char **, struct saffire_option *options[]);
    int saffire_parse_signature(int argc, char **argv, char *signature, char **error);
    char *saffire_getopt_string(int idx);
    char saffire_getopt_bool(int idx);
    long saffire_getopt_int(int idx);

    int to_bool(char *value);

#endif

