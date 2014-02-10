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
#include "vm/codeframe.h"
#include "general/smm.h"
#include "debug.h"


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


t_vm_context *vm_context_new(char *class_path, char *file_path) {
    t_vm_context *context = (t_vm_context *)smm_malloc(sizeof(t_vm_context));
    bzero(context, sizeof(t_vm_context));

    if (class_path) {
        context->class.path = vm_context_strip_path(class_path);
        context->class.name = vm_context_strip_class(class_path);
        context->class.full = string_strdup0(class_path);
    }

    if (file_path) {
        char *duppath = NULL;

        context->file.full = string_strdup0(file_path);

        // dirname and/or basename can (and will)modify existing path
        duppath = string_strdup0(file_path);
        context->file.path = string_strdup0(dirname(duppath));
        smm_free(duppath);

        duppath = string_strdup0(file_path);
        context->file.name = string_strdup0(basename(duppath));
        smm_free(duppath);
    }

    return context;
}

/**
 * Frees the context for this codeframe
 */
void vm_context_free_context(t_vm_codeframe *codeframe) {
    if (! codeframe || ! codeframe->context) return;

    smm_free(codeframe->context->class.path);
    smm_free(codeframe->context->class.name);

    smm_free(codeframe->context->file.path);
    smm_free(codeframe->context->file.name);

    smm_free(codeframe->context);
}




/**
 * Returns the absolute fqcp based
 * @param frame
 * @param classname
 * @return
 */
char *vm_context_absolute_namespace(t_vm_codeframe *codeframe, char *class_name) {
    char *absolute_class_name = NULL;

    // It's already absolute
    if (strstr(class_name, "::") == class_name) {
        return string_strdup0(class_name);
    }

    // If we have a relative name, but we are actually without a codeframe. Only allowed when we import from
    // our main file.
    if (! codeframe) {
        smm_asprintf_char(&absolute_class_name, "::%s", class_name);
        return absolute_class_name;
    }

    t_vm_context *ctx = codeframe->context;
    smm_asprintf_char(&absolute_class_name, "%s::%s", ctx->class.path, class_name);
    return absolute_class_name;
}
