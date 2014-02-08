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
#include <string.h>
#include <libgen.h>
#include "general/output.h"
#include "vm/frame.h"
#include "general/smm.h"


/**
 * Returns the context path from a specified context
 */
char *vm_context_strip_path(char *class_path) {
    char *s = string_strdup0(class_path);

    // Seek last ':'
    char *c = strrchr(s, ':');
    if (c == NULL) {
        fatal_error(1, "context does not have any ::");     /* LCOV_EXCL_LINE */
    }

    c--;
    if (c == s) {
        // if we stripped everything, we should just return the :: part. this is an edge-case for top-level class paths
        c+=2;
    }
    *c = '\0';
    return s;
}


/**
 * Returns the context path from a specified context.
 */
char *vm_context_strip_class(char *class_path) {
    char *c = strrchr(class_path, ':');
    if (c == NULL) return string_strdup0(class_path);
    if ((class_path - c) == 0) return string_strdup0(class_path);

    c++;
    return string_strdup0(c);
}


/**
 * Sets the context for this frame and plits the nessesary class and path.
 */
void vm_context_set_context(t_vm_frame *frame, char *class_path, char *file_path) {
    frame->context = (t_vm_context *)smm_malloc(sizeof(t_vm_context));
    bzero(frame->context, sizeof(t_vm_context));

    if (class_path) {
        frame->context->class.path = vm_context_strip_path(class_path);
        frame->context->class.name = vm_context_strip_class(class_path);
    }

    if (file_path) {
        char *duppath = string_strdup0(file_path);
        frame->context->file.path = string_strdup0(dirname(duppath));
        frame->context->file.name = string_strdup0(basename(duppath));
        smm_free(duppath);
    }
}

/**
 * Frees the context for this frame
 */
void vm_context_free_context(t_vm_frame *frame) {
    smm_free(frame->context->class.path);
    smm_free(frame->context->class.name);

    smm_free(frame->context->file.path);
    smm_free(frame->context->file.name);

    smm_free(frame->context);
    frame->context = NULL;
}

