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
#ifndef __DEBUG_H__
#define __DEBUG_H__

    #include <stdio.h>
    #include <saffire/general/output.h>

    #ifdef __DEBUG
        #define DEBUG_PRINT_CHAR output_debug_char
        #define DEBUG_PRINT_STRING output_debug_string

        #define DEBUG_PRINT_STRING_ARGS(format, args...) \
            { \
                t_string *s__COUNTER__ = char0_to_string(format); \
                output_debug_string(s__COUNTER__, args); \
                string_free(s__COUNTER__); \
            }
    #else
        #define DEBUG_PRINT_CHAR(format, args...) ((void)0)
        #define DEBUG_PRINT_STRING(format, args...) ((void)0)
        #define DEBUG_PRINT_STRING_ARGS(format, args...) ((void)0)
    #endif


    #ifndef __DEBUG_STACK
        // Display stack pushes and pops
        #define __DEBUG_STACK        1
    #endif
    #ifndef __DEBUG_VM_OPCODES
        // Display VM opcodes
        #define __DEBUG_VM_OPCODES   1
    #endif
    #ifndef __DEBUG_FREE_OBJECT
        // Display when free'ing objects
        #define __DEBUG_FREE_OBJECT  0
    #endif
    #ifndef __DEBUG_STACKFRAME_DESTROY
        // Display variables when destroying stackframe
        #define __DEBUG_STACKFRAME_DESTROY  0
    #endif
    #ifndef __DEBUG_REFCOUNT
        // Display ref counts
        #define __DEBUG_REFCOUNT  0
    #endif


    // Parse flex/bison debugging
    // #define __PARSEDEBUG 1

#endif

