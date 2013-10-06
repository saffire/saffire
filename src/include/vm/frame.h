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
#ifndef __VM_FRAME_H__
#define __VM_FRAME_H__

    #include "objects/hash.h"
    #include "compiler/bytecode.h"
    #include "block.h"
    #include "vm/context.h"



    struct _vm_frame {
        t_vm_frame *parent;                         // Parent frame, or NULL when we reached the initial / global frame.

        t_vm_context *context;                      // Actual context

        t_bytecode *bytecode;                       // Global bytecode array
        int ip;                                     // Instruction pointer

        int lineno_lowerbound;                      // Lower bytecode offset for this line
        int lineno_upperbound;                      // Upper bytecode offset for this line
        int lineno_current_line;                    // Current line pointer
        int lineno_current_lino_offset;             // Current offset in the bytecode lineno table

        t_object **stack;                           // Local variable stack
        unsigned int sp;                            // Stack pointer

        t_hash_object *local_identifiers;           // Local identifiers
        t_hash_object *global_identifiers;          // Global identifiers
        t_hash_object *builtin_identifiers;         // Builtin identifiers

        t_object **constants_objects;               // Constants converted to objects
        t_dll *created_objects;                     // Created objects by the bytecode

        int block_cnt;                              // Last used block number (0 = no blocks on the stack)
        t_vm_frameblock blocks[BLOCK_MAX_DEPTH];    // Frame blocks

        char *class;                                // Class (or NULL in global?)
        char *method;                               // Method (or NULL in global?)
        int param_count;                            // Number of arguments
        t_object **params;                          // The arguments list (start offset on stack)

        //unsigned int time;                        // Total time spend in this bytecode block
        unsigned int executions;                    // Number of total executions (opcodes processed)
    };

    char *vm_frame_absolute_namespace(t_vm_frame *frame, char *namespace);


    t_vm_frame *vm_frame_new_scoped(t_vm_frame *scope_frame, t_vm_frame *parent_frame, t_vm_context *context, t_bytecode *bytecode);
    t_vm_frame *vm_frame_new(t_vm_frame *parent_frame, char *class_path, char *file_path, t_bytecode *bytecode);
    void vm_frame_destroy(t_vm_frame *frame);
    void vm_attach_bytecode(t_vm_frame *frame, char *class_path, char *file_path, t_bytecode *bytecode);
    void vm_detach_bytecode(t_vm_frame *frame);

    unsigned char vm_frame_get_next_opcode(t_vm_frame *frame);
    unsigned int vm_frame_get_operand(t_vm_frame *frame);

    t_object *vm_frame_stack_pop(t_vm_frame *frame);
    void vm_frame_stack_push(t_vm_frame *frame, t_object *obj);
    void vm_frame_stack_modify(t_vm_frame *frame, int idx, t_object *obj);
    t_object *vm_frame_stack_fetch_top(t_vm_frame *frame);
    t_object *vm_frame_stack_fetch(t_vm_frame *frame, int idx);

    t_object *vm_frame_get_constant(t_vm_frame *frame, int idx);
    t_object *vm_frame_get_identifier(t_vm_frame *frame, char *id);
    t_object *vm_frame_find_identifier(t_vm_frame *frame, char *id);
    t_object *vm_frame_get_global_identifier(t_vm_frame *frame, char *id);
    t_object *vm_frame_local_identifier_exists(t_vm_frame *frame, char *id);
    t_object *vm_frame_resolve_identifier(t_vm_frame *frame, char *id);

    void vm_frame_set_global_identifier(t_vm_frame *frame, char *id, t_object *obj);
    void vm_frame_set_identifier(t_vm_frame *frame, char *id, t_object *obj);

    void vm_frame_set_builtin_identifier(t_vm_frame *frame, char *id, t_object *obj);

    void *vm_frame_get_constant_literal(t_vm_frame *frame, int idx);
    char *vm_frame_get_name(t_vm_frame *frame, int idx);

    char *vm_frame_get_context_path(char *path);

    void vm_frame_register_userobject(t_vm_frame *frame, t_object *obj);

#ifdef __DEBUG
    void vm_frame_stack_debug(t_vm_frame *frame);
    void print_debug_table(t_hash_table *ht, char *prefix);
#endif

#endif
