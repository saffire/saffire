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
#ifndef __VM_H__
#define __VM_H__

    #include "compiler/bytecode.h"
    #include "objects/hash.h"
    #include "vm/frame.h"
    #include "compiler/saffire_parser.h"

    t_hash_table *builtin_identifiers;

    #define VM_RUNMODE_FASTCGI      0       // Virtual machine run as FastCGI
    #define VM_RUNMODE_CLI          1       // Virtual machine run as CLI
    #define VM_RUNMODE_REPL         2       // Virtual machine run as REPL
    #define VM_RUNMODE_DEBUG      128       // Debugging should be activated

    // Actual runmode of the VM (fastcgi, cli, rep
    int vm_runmode;

    t_vm_frame *vm_init(SaffireParser *sp, int runmode);
    void vm_fini(t_vm_frame *frame);
    int vm_execute(t_vm_frame *frame);
    void vm_populate_builtins(const char *name, void *data);
    t_object *vm_object_call_args(t_object *self, t_object *callable, t_dll *arg_list);
    t_object *vm_object_call(t_object *self, t_object *method_obj, int arg_count, ...);
    t_object *vm_object_operator(t_object *obj1, int opr, t_object *obj2);
    t_object *vm_object_comparison(t_object *obj1, int cmp, t_object *obj2);
    t_object *object_internal_call(const char *class, const char *method, int arg_count, ...);

#endif


