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
#ifndef __DEBUGGER_DBGP_COMMANDS_H__
#define __DEBUGGER_DBGP_COMMANDS_H__

    #include "debugger/dbgp/xml.h"
    #include "debugger/dbgp/dbgp.h"

    struct dbgp_command {
        char *command;
        xmlNodePtr (*func)(t_debuginfo *di, int argc, char *argv[]);
    };

    #define DBGP_CMD_DEF(name) \
        xmlNodePtr do_command_##name(t_debuginfo *di, int argc, char *argv[])

    #define DBGP_CMD_REF(name) \
        &do_command_##name


    DBGP_CMD_DEF(status);

    DBGP_CMD_DEF(feature_get);
    DBGP_CMD_DEF(feature_set);

    DBGP_CMD_DEF(run);
    DBGP_CMD_DEF(step_into);
    DBGP_CMD_DEF(step_over);
    DBGP_CMD_DEF(step_out);
    DBGP_CMD_DEF(stop);
    DBGP_CMD_DEF(detach);

    DBGP_CMD_DEF(breakpoint_get);
    DBGP_CMD_DEF(breakpoint_set);
    DBGP_CMD_DEF(breakpoint_update);
    DBGP_CMD_DEF(breakpoint_remove);
    DBGP_CMD_DEF(breakpoint_list);

    DBGP_CMD_DEF(stack_depth);
    DBGP_CMD_DEF(stack_get);

    DBGP_CMD_DEF(context_names);
    DBGP_CMD_DEF(context_get);

    DBGP_CMD_DEF(typemap_get);
    DBGP_CMD_DEF(property_get);
    DBGP_CMD_DEF(property_set);
    DBGP_CMD_DEF(property_value);

    DBGP_CMD_DEF(source);

    DBGP_CMD_DEF(set_stdout);
    DBGP_CMD_DEF(set_stderr);


#endif // __DEBUGGER_DBGP_COMMANDS_H__

