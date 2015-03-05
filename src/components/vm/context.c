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
#include <string.h>
#include <libgen.h>
#include <saffire/general/output.h>
#include <saffire/vm/codeblock.h>
#include <saffire/vm/context.h>
#include <saffire/general/smm.h>
#include <saffire/debug.h>
#include <assert.h>

/**
 * Returns the context path from a specified context  (::foo::bar::baz)
 */
char *vm_context_get_path(char *class_path) {
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
char *vm_context_get_class(char *class_path) {
    char *c = strrchr(class_path, ':');
    if (c == NULL) return string_strdup0(class_path);
    if ((class_path - c) == 0) return string_strdup0(class_path);

    c++;
    return string_strdup0(c);
}


t_vm_context *vm_context_duplicate(t_vm_context *src) {
    t_vm_context *dst = smm_malloc(sizeof(t_vm_context));

    dst->module.full = string_strdup0(src->module.full);
    dst->file.path = string_strdup0(src->file.path);
    dst->file.base = string_strdup0(src->file.base);
    dst->file.full = string_strdup0(src->file.full);

    return dst;
}

t_vm_context *vm_context_new(char *module_path, char *file_path) {
    t_vm_context *context = (t_vm_context *)smm_malloc(sizeof(t_vm_context));
    bzero(context, sizeof(t_vm_context));

    if (module_path) {
        context->module.full = string_strdup0(module_path);
    }

    if (file_path) {
        char *duppath = NULL;

        context->file.full = string_strdup0(file_path);

        // dirname and/or basename can (and will)modify existing path
        duppath = string_strdup0(file_path);
        context->file.path = string_strdup0(dirname(duppath));
        smm_free(duppath);

        duppath = string_strdup0(file_path);
        context->file.base = string_strdup0(basename(duppath));
        smm_free(duppath);
    }

    return context;
}

/**
 * Frees the context for this codeblock
 */
void vm_context_free_context(t_vm_context *ctx) {
    if (! ctx) return;

    smm_free(ctx->module.full);

    smm_free(ctx->file.path);
    smm_free(ctx->file.base);
    smm_free(ctx->file.full);

    smm_free(ctx);
}




/**
 * Returns the absolute fqcp based
 * @param frame
 * @param classname
 * @return
 */
char *vm_context_fqcn(t_vm_context *ctx, char *class_name) {
    char *absolute_class_name = NULL;

    if (vm_context_is_fqcn(class_name)) {
        smm_asprintf_char(&absolute_class_name, "%s", class_name);
    } else {
        vm_context_create_fqcn(ctx->module.full, class_name, &absolute_class_name);
    }

    return absolute_class_name;
}

void vm_context_create_fqcn(char *pre, char *post, char **fqcn) {
    if (vm_context_is_fqcn(post)) {
        // ::foo  + ::bar ==> ::foo::bar
        smm_asprintf_char(fqcn, "%s%s", pre, post);
    } else {
        // ::foo  + bar ==> ::foo::bar
        smm_asprintf_char(fqcn, "%s::%s", pre, post);
    }
}


int vm_context_is_fqcn(char *class_name) {
    int i = (strstr(class_name, "::") == class_name);
    return i;
}
