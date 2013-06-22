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
#include <malloc.h>
#include <string.h>

/**
 * Parses a string into argc and argv. Note: you have to free argv when finished
 * working with it.
 *
 * @TODO  Max of 255 arguments can be parsed.
 *
 * @param buffer
 * @param argc
 * @param argv
 */
void dbgp_args_parse(char *buffer, int *argc, char **argv[]) {
    // 0 = find_arg   1 = in_arg, 1 = in_quoted_arg
    int state = 0;
    char *bufptr = buffer;

    *argc = 0;

    char **argv_tmp;
    argv_tmp = malloc(255 * sizeof(char *));
    for (int i=0; i!=255; i++) argv_tmp[i] = 0;

    while (*bufptr) {
        // Space found outside arg,
        if (*bufptr == ' ') {
            switch (state) {
                case 0 :
                    // Just skip it
                    break;
                case 1 :
                    // terminate ending arg
                    *bufptr = '\0';
                    state = 0;
                    break;
                case 2 :
                    // Space inside quoted arg, just skip it
                    break;
            }
            bufptr++;
            continue;
        }

        if (*bufptr == '"') {
            switch (state) {
                case 0 :
                    // Quote found outside arg, start quoted arg
                    state = 2;
                    argv_tmp[*argc] = bufptr + 1;
                    (*argc)++;
                    break;
                case 1 :
                    break;
                case 2 :
                    // End quote found
                    state = 0;
                    *bufptr = '\0';
                    break;
            }
            bufptr++;
            continue;
        }

        switch (state) {
            case 0 :
                // Start a unquoted arg
                state = 1;
                argv_tmp[*argc] = bufptr;
                (*argc)++;
                break;
            case 1 :
            case 2 :
                // Regular character inside a arg
                break;
        }
        bufptr++;
        continue;
    }

    *argv = argv_tmp;
}


void dbgp_args_free(char **argv) {
    free(argv);
}


/**
 * Find a match on 'name' and returns the index. Returns -1 when nothing was found
 */
int dbgp_args_find(char *name, int argc, char *argv[]) {
    for (int i=0; i!=argc; i++) {
        if (! strcmp(name, argv[i])) return i;
    }
    return -1;
}
