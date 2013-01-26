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
#ifndef __VM_H__
#define __VM_H__

    #include "compiler/bytecode.h"
    #include "objects/hash.h"
    #include "vm/frame.h"

    t_hash_table *builtin_identifiers;

    #define VM_RUNMODE_FASTCGI      0       // Virtual machine run as FastCGI
    #define VM_RUNMODE_CLI          1       // Virtual machine run as CLI
    #define VM_RUNMODE_REPL         2       // Virtual machine run as REPL

    // Actual runmode of the VM (fastcgi, cli, rep
    int vm_runmode;

    void vm_init(int mode);
    void vm_fini(void);
    int vm_execute(t_bytecode *bc);
    t_object *_vm_execute(t_vm_frame *frame);
    void vm_populate_builtins(const char *name, void *data);
    t_object *vm_object_call_args(t_object *self, t_object *callable, t_dll *arg_list);
    t_object *vm_object_call(t_object *self, t_object *method_obj, int arg_count, ...);
    t_object *object_internal_call(const char *class, const char *method, int arg_count, ...);

#endif


