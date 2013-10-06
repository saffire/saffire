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
#ifndef __VM_CONTEXT_H__
#define __VM_CONTEXT_H__

    #include "vm/frame.h"

    typedef struct _vm_frame_context {
        struct {
            char *path;      // absolute namespace (::foo in case of ::foo::bar)
            char *name;      // Class name (bar in case of ::foo::bar)
        } class;

        struct {
            char *path;      // Path with loaded bytecode
            char *name;      // bytecode filename
        } file;
    } t_vm_context;

    char *vm_context_strip_path(char *full_namespace);
    char *vm_context_strip_class(char *full_namespace);

    char *vm_context_get_class_path(t_vm_context *context);
    char *vm_context_get_class_class(t_vm_context *context);

    char *vm_context_get_file_path(t_vm_context *context);
    char *vm_context_get_file_name(t_vm_context *context);

    void vm_context_set_context(t_vm_frame *frame, char *class_path, char *file_path);
    void vm_context_free_context(t_vm_frame *frame);

#endif
