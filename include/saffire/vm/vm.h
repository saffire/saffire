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
#ifndef __VM_H__
#define __VM_H__

    #include <saffire/compiler/bytecode.h>
    #include <saffire/objects/hash.h>
    #include <saffire/vm/stackframe.h>
    #include <saffire/compiler/saffire_parser.h>
    #include <saffire/objects/attrib.h>

    t_hash_table *builtin_identifiers_ht;       // hash table with all builtin identifiers
    t_hash_object *builtin_identifiers;         // hash object with all builtin identifiers
    t_hash_table *class_import_table;           // hash object with all imported classes

    #define VM_RUNMODE_FASTCGI      1       // Virtual machine run as FastCGI
    #define VM_RUNMODE_CLI          2       // Virtual machine run as CLI
    #define VM_RUNMODE_REPL         4       // Virtual machine run as REPL
    #define VM_RUNMODE_DEBUG      128       // Debugging should be activated

    // Actual runmode of the VM (fastcgi, cli, repl etc)
    int vm_runmode;

    void vm_init(SaffireParser *sp, int runmode);
    void vm_fini(void);
    int vm_execute(t_vm_stackframe *stackframe);
    void vm_populate_builtins(const char *name, t_object *obj);

    t_vm_stackframe *vm_execute_import(t_vm_codeblock *codeblock, t_object **result);
    t_object *vm_object_call(t_object *self, t_attrib_object *attrib_obj, int arg_count, ...);

#endif


