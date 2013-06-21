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
#ifndef __DEBUGGER_DBGP_H__
#define __DEBUGGER_DBGP_H__

    #include "vm/frame.h"

    typedef struct _debuginfo {
        int sock_fd;            // Network socket for read/write

        int attached;           // Debugger is attached
        int state;              // Current state of the debugger
        char *reason;           // Reason for which we are in the current state
        int step_into;          // boolean: step into the next statement
        int step_over;
        int step_out;

        struct _step_data {
            char *file;
            int lineno;
            t_vm_frame *frame;
        } step_data;

        int breakpoint_id;      // Counter for unique breakpoint ID's
        t_hash_table *breakpoints;     // Breakpoints table


        char *cur_cmd;          // Current running command
        char *cur_txid;         // Current transaction id

        t_vm_frame *frame;      // Current attached frame
    } t_debuginfo;


    typedef struct _breakpoint {
        char *id;               // Breakpoint ID
        char *type;             // BP type: DBGP_BREAKPOINT_TYPE_*
        char *filename;         // Filename (file://)
        int lineno;             // BP line number
        int state;              // BP enabled or not
        char *function;         // function for call / return BPs
        int temporary;          // deleted after first BP call
        int hit_count;          // How many times the BP has been hit
        int hit_value;          // Value for hit_condition
        char *hit_condition;    // Hit condition: ">=", "==" or "%"
        char *exception;        // Exception name for exception types BPs
        char *expression;       // Expression for conditional and watch BPs
    } t_breakpoint;

    #define DBGP_REASON_OK                      "ok"
    #define DBGP_REASON_ERROR                   "error"
    #define DBGP_REASON_ABORTED                 "aborted"
    #define DBGP_REASON_EXCEPTION               "exception"

    #define DBGP_BREAKPOINT_TYPE_LINE           "line"
    #define DBGP_BREAKPOINT_TYPE_CALL           "call"
    #define DBGP_BREAKPOINT_TYPE_RETURN         "return"
    #define DBGP_BREAKPOINT_TYPE_EXCEPTION      "exception"
    #define DBGP_BREAKPOINT_TYPE_CONDITIONAL    "conditional"
    #define DBGP_BREAKPOINT_TYPE_WATCH          "watch"

    #define DBGP_STATE_STARTING                 0
    #define DBGP_STATE_STOPPING                 1
    #define DBGP_STATE_STOPPED                  2
    #define DBGP_STATE_RUNNING                  3
    #define DBGP_STATE_BREAK                    4


    void dbgp_debug(t_debuginfo *di, t_vm_frame *frame);
    void dbgp_parse_incoming_commands(t_debuginfo *di);
    t_debuginfo *dbgp_init(t_vm_frame *frame);
    void dbgp_fini(t_debuginfo *di, t_vm_frame *frame);

#endif

