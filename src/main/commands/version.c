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
#include "commands/command.h"
#include "general/output.h"
#include "version.h"

static int flag_long_version = 0;


/**
 * Display version information
 */
static int do_version(void) {
    // Output long version info
    if (flag_long_version) {
        output("%s - %s\n%s\n%s\n", saffire_version, saffire_copyright, saffire_compiled, saffire_configured);
        return 0;
    }

    // Output simple parsable version
    output("%s.%s.%s\n", saffire_version_major, saffire_version_minor, saffire_version_build);
    return 0;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]   = "Displays version information about Saffire.\n"
                             "\n";

static void opt_long(void *data) {
    flag_long_version = 1;
}

static struct saffire_option version_options[] = {
    { "long", "b", no_argument, opt_long},
};

/* Config actions */
static struct command_action command_actions[] = {
    { "", "", do_version, version_options },
    { 0, 0, 0, 0 }
};

struct command_info info_version = {
    "Displays version information",
    command_actions,
    help
};
