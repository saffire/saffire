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
#ifndef __VM_TYPES_H__
#define __VM_TYPES_H__

    #include <saffire/compiler/bytecode.h>
    #include <saffire/objects/objects.h>


    #define BLOCK_MAX_DEPTH             20          // Maximum depth of the number of blocks we can have (nested if's, for instance)
                                                    // This is the same depth as defined in python


    /**
     * A structure that keeps information about FQCN <-> filename mappings.
     */
    typedef struct _vm_frame_context {
        struct {
            char *full;      // full namespace (::foo::bar)
        } module;

        struct {
            char *path;      // Path with loaded bytecode
            char *base;      // bytecode filename
            char *full;      // Full path
        } file;
    } t_vm_context;


    typedef struct _vm_codeblock {
        t_vm_context *context;          // Context of this codeblock

        t_bytecode *bytecode;           // Frame's bytecode
        long constants_objects_len;     // Length of the constants
        t_object **constants_objects;   // Constants taken from bytecode, converted to actual objects
    } t_vm_codeblock;


    typedef struct _vm_frameblock {
        int type;       // Type (any of the BLOCK_TYPE_*)
        union {
            struct {
                int ip;         // Saved instruction pointer
                int ip_else;    // Saved instruction pointer to ELSE part
            } loop;
            struct {
                int ip_catch;           // Saved instruction pointer to CATCH blocks
                int ip_finally;         // Saved instruction pointer to FINALLY block
                int ip_end_finally;     // Saved instruction pointer to the end of FINALLY block

                int in_finally;         // 1: We are currently handling the finally block
            } exception;
        } handlers;
        int sp;         // Saved stack pointer
        int visited;    // When !=0, this frame is already visited by a JUMP_IF_*_AND_FIRST
    } t_vm_frameblock;


    struct _vm_stackframe {
        t_vm_stackframe *parent;                    // Parent frame, or NULL when we reached the initial / global frame.

        t_vm_codeblock *codeblock;                  // Actual codeblock

        int ip;                                     // Instruction pointer

        int lineno_lowerbound;                      // Lower bytecode offset for this line
        int lineno_upperbound;                      // Upper bytecode offset for this line
        int lineno_current_line;                    // Current line pointer
        int lineno_current_lino_offset;             // Current offset in the bytecode lineno table

        t_object **stack;                           // Local variable stack
        int sp;                                     // Stack pointer (signed so we can detect -1 for overflow)

        t_hash_object *local_identifiers;           // Local identifiers (local variables, method arguments etc)
        t_hash_object *global_identifiers;          // Global identifiers
        t_hash_object *builtin_identifiers;         // Builtin identifiers (String, Numerical, modules etc)

        t_hash_table *object_aliases;               // Object aliases from imports

        int block_cnt;                              // Last used block number (0 = no blocks on the stack)
        t_vm_frameblock blocks[BLOCK_MAX_DEPTH];    // Frame blocks

        char *trace_class;                          // Class that is currently executed
        char *trace_method;                         // Method that is currently executed
        int param_count;                            // Number of arguments
        t_object **params;                          // The arguments list (start offset on stack)

        //unsigned int time;                        // Total time spend in this bytecode block
        unsigned int executions;                    // Number of total executions (opcodes processed)
    };

#endif
