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
#include "general/output.h"
#include "commands/command.h"
#include "fastcgi/fastcgi_srv.h"

/**
 *
 */
static int do_start(void) {
//    if (fastcgi_running()) {
//        error("FastCGI server is already running!\n");
//        return 1;
//    }

    return fastcgi_start();
}

static int do_stop(void) {
//    if (! fastcgi_running()) {
//        error("FastCGI server is not running!\n");
//        return 1;
//    }

    return fastcgi_stop();
}

static int do_status(void) {
//    if (fastcgi_running()) {
//        error("Status: running\n");
//        return 0;
//    }

    output("Status: stopped\n");
    return 1;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]   = "Controls the Saffire FastCGI Daemon\n"
                             "\n"
                             "Actions:\n"
                             "  start     Starts the daemon\n"
                             "  stop      Stops the daemon\n"
                             "  status    Display daemon status\n"
                             "  info      Returns information \n";

/* Config actions */
static struct command_action command_actions[] = {
    { "start", "", do_start, NULL },
    { "stop", "", do_stop, NULL },
    { "status", "", do_status, NULL },
//    { "info", "", do_info, NULL },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_fastcgi = {
    "FastCGI daemon",
    command_actions,
    help
};
