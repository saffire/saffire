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
#include <string.h>
#include "general/parse_options.h"

/* Parameters known to saffire commands. Similar to argc & argv */
char **saffire_params;
int saffire_params_count;


/**
 * Converts a string value into a boolean 0 or 1. Returns -1 if cannot be converted.
 */
int to_bool(char *value) {
    if (strlen(value) == 0) return 0;
    if (! strcasecmp(value, "disabled")) return 0;
    if (! strcasecmp(value, "false")) return 0;
    if (! strcasecmp(value, "no")) return 0;
    if (! strcasecmp(value, "0")) return 0;

    if (! strcasecmp(value, "enabled")) return 1;
    if (! strcasecmp(value, "true")) return 1;
    if (! strcasecmp(value, "yes")) return 1;
    if (! strcasecmp(value, "1")) return 1;

    return -1;
}


/**
 * Processes the argument by calling the corresponding function.
 * Finds optional argument (if needed) for the current argument (like: --file <filename>), and
 * adds them as an argument.
 */
static void process_argument(int idx, int argc, char *argv[], struct saffire_option *opt) {
    int has_next_arg = 0;

    // Processing argument. Remove it from the list.
    argv[idx] = NULL;

    // No additional arguments. Process it.
    if (opt->has_arg == no_argument) {
        opt->func(NULL);
        return;
    }

    // Sanity check to see if there are additonal arguments
    if (idx == argc-1) {
        // Last argument, so we cannot have a next
        has_next_arg = 0;
    } else {
        // Otherwise, next arg is the option for this item
        has_next_arg = 1;  // (argv[idx] != '-');      // if it starts with -, then we have an argument
    }

    // Need additional argument, but found none
    if (! has_next_arg && opt->has_arg == required_argument) {
        printf("Option '%s' requires an argument\n", argv[idx]);
        exit(1);
    }

    if (has_next_arg && opt->has_arg == required_argument) {
        // Process with required argument
        opt->func(argv[idx+1]);
        argv[idx+1] = NULL;
    }
    if (has_next_arg && opt->has_arg == optional_argument) {
        // Process with optional argument
        opt->func(argv[idx+1]);
        argv[idx+1] = NULL;
    }
    if (! has_next_arg && opt->has_arg == optional_argument) {
        // Process without optional argument
        opt->func(NULL);
    }
}


/**
 * Parse commandline options. This is similar to getopt_long, but with some modifications.
 * It WILL modify the argv by cleaning the arguments. This makes it easier to process.
 */
void saffire_parse_options(int argc, char **argv, struct saffire_option *options[]) {
    int found;

    // Iterate all arguments
    for (int idx=0; idx != argc; idx++) {
        found = 0;

        struct saffire_option *opt = options[0];
        while (opt && opt->longname != NULL) {

            if (argv[idx] && argv[idx][0] == '-' && argv[idx][1] == '-') {
                // Long option found (--option)
                if (! strcasecmp(argv[idx]+2, opt->longname)) {
                    process_argument(idx, argc, argv, opt);
                    found = 1;
                }
            } else if (argv[idx] && argv[idx][0] == '-' && argv[idx][1] != '-' && argv[idx][2] == '\0') {
                // Short option found (-o)
                if (! strcasecmp(argv[idx]+1, opt->shortname)) {
                    process_argument(idx, argc, argv, opt);
                    found = 1;
                }
            } else {
                // Open argument. Just skip this argument
                found = 1;
            }

            opt++;
        }

        // Unknown option found
        if (! found) {
            printf("saffire: invalid option '%s'\n"
                   "Try 'saffire help config' for more information\n", argv[idx]);
            exit(1);
        }
    }
}


/*
 * A signature consists of a series of items:
 *  "s"  needs (any) string (including ints)
 *  "l"  needs (long) integer
 *  "b"  needs boolean (true, false, yes, no, 0, 1)
 * // Not yet implemented | : All items beghind this are optional.
 */
void saffire_parse_signature(int argc, char **argv, char *signature) {
    int correct = 0;

    // Shift all remaining settings to the front.
    int open_spot = 0;
    while (argv[open_spot]) open_spot++;

    for (int i=open_spot+1; i<argc; i++) {
        // Set initial open spot
        if (argv[i] && open_spot < i) {
            argv[open_spot] = argv[i];
            argv[i] = NULL;
            while (argv[open_spot]) open_spot++;
        }
    }


    // We can "shrink" the argument count, since there are no open spots in between.
    argc = open_spot;

    // Quick signature sanity check
    if (argc == strlen(signature)) {
        // Everything is ok. We have all our required arguments!
    } else if (argc >= strlen(signature)) {
        // Not enough arguments!
        printf("Too many arguments found. use saffire help <command> for more information\n");\
        exit(1);
    } else {
        printf("Not enough arguments found. use saffire help <command> for more information\n");
        exit(1);
    }


    // Process each character in the signature
    for (int idx=0; idx!=strlen(signature); idx++) {
        correct = 0;

        switch (signature[idx]) {
            case 's' :
                // Strings are "as-is"
                break;
            case 'b' :
                // Try and convert to boolean
                correct = to_bool(argv[idx]) != -1;

                if (! correct) {
                  printf("Found '%s', but expected a boolean value\n", argv[idx]);
                  exit(1);
                }

                break;
            case 'l' :
                // Convert to long. string("0") should be ok too!
                if (! strcasecmp(argv[idx], "0") || atol(argv[idx])) correct = 1;

                if (! correct) {
                  printf("Found '%s', but expected a numerical value\n", argv[idx]);
                  exit(1);
                }
                break;
          default :
                printf("Incorrect signature command '%c' found.\n", signature[idx]);
                exit(1);
        }

    }

    // All parameters are shifted to the front, and checked for type. From now on, we can safely use them
    saffire_params = argv;
    saffire_params_count = argc;
}



/**
 * Returns string from the argument list.
 */
char *saffire_getopt_string(int idx) {
    if (idx > saffire_params_count) return NULL;
    return saffire_params[idx];
}

/**
 * Returns boolean from the argument list.
 */
char saffire_getopt_bool(int idx) {
    if (idx > saffire_params_count) return 0;
    int ret = to_bool(saffire_params[idx]);
    if (ret == -1) {
        printf("Incorrect boolean value");
        exit(1);
    }

    return ret;
}

/**
 * Returns long from the argument list.
 */
long saffire_getopt_int(int idx) {
    if (idx > saffire_params_count) return 0;
    return atoi(saffire_params[idx]);
}

