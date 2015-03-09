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
#ifndef __VM_STACKFRAME_H__
#define __VM_STACKFRAME_H__

    #include <saffire/vm/vmtypes.h>
    #include <saffire/objects/hash.h>
    #include <saffire/compiler/bytecode.h>
    #include <saffire/vm/block.h>
    #include <saffire/vm/codeblock.h>
    #include <saffire/vm/context.h>


    t_vm_stackframe *vm_stackframe_new_scoped(t_vm_stackframe *scope_frame, t_vm_stackframe *parent_frame, t_vm_context *context, t_bytecode *bytecode);
    t_vm_stackframe *vm_stackframe_new(t_vm_stackframe *parent_frame, t_vm_codeblock *codeblock);
    void vm_stackframe_destroy(t_vm_stackframe *frame);

    unsigned char vm_frame_get_next_opcode(t_vm_stackframe *frame);
    unsigned int vm_frame_get_operand(t_vm_stackframe *frame);

    t_object *vm_frame_stack_pop_attrib(t_vm_stackframe *frame);
    t_object *vm_frame_stack_pop(t_vm_stackframe *frame);
    void vm_frame_stack_push(t_vm_stackframe *frame, t_object *obj);
    void vm_frame_stack_modify(t_vm_stackframe *frame, int idx, t_object *obj);
    t_object *vm_frame_stack_fetch_top(t_vm_stackframe *frame);
    t_object *vm_frame_stack_fetch(t_vm_stackframe *frame, int idx);

    t_object *vm_frame_get_constant(t_vm_stackframe *frame, int idx);
    t_object *vm_frame_get_identifier(t_vm_stackframe *frame, char *id);
    t_object *vm_frame_find_identifier(t_vm_stackframe *frame, char *id);
    t_object *vm_frame_get_global_identifier(t_vm_stackframe *frame, char *id);
    t_object *vm_frame_identifier_exists(t_vm_stackframe *frame, char *id);

    void vm_frame_set_alias_identifier(t_vm_stackframe *frame, char *id, char *fqcn);
    void vm_frame_set_global_identifier(t_vm_stackframe *frame, char *id, t_object *obj);
    void vm_frame_set_local_identifier(t_vm_stackframe *frame, char *id, t_object *obj);
    void vm_frame_set_builtin_identifier(t_vm_stackframe *frame, char *id, t_object *obj);

    void *vm_frame_get_constant_literal(t_vm_stackframe *frame, int idx);
    char *vm_frame_get_name(t_vm_stackframe *frame, int idx);

    char *vm_frame_get_context_path(char *path);

#ifdef __DEBUG
    void vm_frame_stack_debug(t_vm_stackframe *frame);
    void print_debug_table(t_hash_table *ht, char *prefix);
#endif

#endif
