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
#include <saffire/memory/smm.h>
#include <saffire/debug.h>
#include <assert.h>


/*
 * A context is a mapping between a fully qualified class name and a file path.
 *
 */

/**
 * Converts a FQCN to a path
 */
char *vm_context_convert_fqcn_to_path(const char *fqcn) {
    char *s = string_strdup0(fqcn);

    // Convert backward slashes to forward slashes
    for (int i=0; i!=strlen(s); i++) {
        if (s[i] == '\\') {
            s[i] = '/';
        }
    }

    return s;
}

/**
 * Returns the context path from a specified context.
 */
char *vm_context_get_classname_from_fqcn(const char *fqcn) {
    char *c = strrchr(fqcn, '\\');
    if (c == NULL) return string_strdup0(fqcn);
    if ((fqcn - c) == 0) return string_strdup0(fqcn);

    c++;
    return string_strdup0(c);
}

char *vm_context_get_modulepath_from_fqcn(const char *fqcn) {
    // Special case for root, use root as module_path
    if (strcmp(fqcn, "\\") == 0) {
        return string_strdup0(fqcn);
    }

    // Find last \ and return "" when not found
    char *c = strrchr(fqcn, '\\');
    if (c == NULL) return string_strdup0("");

    // Cut off point and return initial part
    return string_strncpy0(fqcn, (c-fqcn));
}


t_vm_context *vm_context_duplicate(const t_vm_context *src) {
    t_vm_context *dst = smm_malloc(sizeof(t_vm_context));

    dst->module.full = string_strdup0(src->module.full);
    dst->file.path = string_strdup0(src->file.path);
    dst->file.base = string_strdup0(src->file.base);
    dst->file.full = string_strdup0(src->file.full);

    return dst;
}

/**
 * Creates a new context structure based on the module fqcn and given file path in which this class is found.
 */
t_vm_context *vm_context_new(const char *module_fqcn, const char *file_path) {
    t_vm_context *context = (t_vm_context *)smm_malloc(sizeof(t_vm_context));
    bzero(context, sizeof(t_vm_context));

    char *module_path = vm_context_get_modulepath_from_fqcn(module_fqcn);
    context->module.full = string_strdup0(module_path);

    char *duppath = NULL;
    context->file.full = string_strdup0(file_path);

    // dirname and/or basename can (and will)modify existing path
    duppath = string_strdup0(file_path);
    context->file.path = string_strdup0(dirname(duppath));
    smm_free(duppath);

    duppath = string_strdup0(file_path);
    context->file.base = string_strdup0(basename(duppath));
    smm_free(duppath);

    return context;
}

/**
 * Frees up the context
 */
void vm_context_free(t_vm_context *ctx) {
    if (! ctx) return;

    smm_free(ctx->module.full);

    smm_free(ctx->file.path);
    smm_free(ctx->file.base);
    smm_free(ctx->file.full);

    smm_free(ctx);
}


/**
 * Returns the fully qualified class name from a class in the given context
 *
 * @param frame
 * @param classname
 * @return
 */
char *vm_context_create_fqcn_from_context(const t_vm_context *ctx, const char *class_name) {
    if (vm_context_is_fqcn(class_name)) {
        return string_strdup0(class_name);
    }

    char *absolute_class_name = NULL;
    absolute_class_name = vm_context_concat_path(ctx->module.full, class_name);

    return absolute_class_name;
}


/**
 *  returns '\bar' from '\foo\bar' when context is '\foo'
 */
char *vm_context_strip_context(const t_vm_context *ctx, const char *class_name)
{
    // Strip the context if the class_name actually starts with the given context
    if (strstr(ctx->module.full, class_name) == 0) {
        // If context is \, then it does not have an additional \.
        int extra_separator_chars = (strcmp(ctx->module.full, "\\") == 0) ? 0 : strlen("\\");

        return string_strdup0(class_name + strlen(ctx->module.full) + extra_separator_chars);
    }

    return string_strdup0(class_name);
}

/**
 * Creates a fully qualified class name  (eg:  '/foo/bar'  'baz'  becomes '/foo/bar/baz')
 * Takes care of root context:  '/' and 'baz'  ->  '/baz'  and not '//baz'
 */
char *vm_context_concat_path(const char *prefix, const char *class) {
    char *fqcn = NULL;

    if (vm_context_is_fqcn(class)) {
        // \foo  + \bar ==> \foo\bar
        smm_asprintf_char(&fqcn, "%s%s", prefix, class);
        return fqcn;
    }

    // Special case when dealing with just the root context
    if (strcmp(prefix, "\\") == 0) {
        // \ + bar ==> \bar,  not \\bar
        smm_asprintf_char(&fqcn, "%s%s", prefix, class);
        return fqcn;
    }

    // \foo  + bar ==> \foo\bar
    smm_asprintf_char(&fqcn, "%s\\%s", prefix, class);
    return fqcn;
}

/**
 * Returns 1 when the class_name is a FQCN, 0 otherwise
 *
 * @param class_name
 * @return
 */
int vm_context_is_fqcn(const char *class_name) {
    return class_name[0] == '\\';
}
