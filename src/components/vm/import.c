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
#include <limits.h>
#include <unistd.h>
#include "vm/vm.h"
#include "vm/stackframe.h"
#include "vm/thread.h"
#include "vm/import.h"
#include "objects/objects.h"
#include "compiler/ast_nodes.h"
#include "compiler/ast_to_asm.h"
#include "compiler/output/asm.h"
#include "general/path_handling.h"
#include "general/output.h"
#include "debug.h"


/*
 * @TODO: Modules are search in this order on these paths. They are currently hardcoded :(
 */
const char *module_search_paths[] = {
    ".",                                    // Should always be first
    "./modules",                            // Should always be second
    "/usr/share/saffire/modules",           // From here it should be customizable
    "/usr/share/saffire/modules/sfl",
    NULL
};


void vm_import_cache_init(void) {
    frame_import_cache = ht_create();
}

void vm_import_cache_fini(void) {
    t_hash_iter iter;
    ht_iter_init(&iter, frame_import_cache);
    while (ht_iter_valid(&iter)) {
        t_vm_stackframe *stackframe = ht_iter_value(&iter);
        DEBUG_PRINT_CHAR("Freeing stackframe: %08X\n", stackframe);

        vm_stackframe_destroy(stackframe);

        ht_iter_next(&iter);
    }

    ht_destroy(frame_import_cache);
}


/**
 * Executes a source-file inside a frame, and extracts the needed class from the frame
 *
 * @param frame
 * @param source_file
 * @param class
 * @param module_frame
 * @return
 */
static t_vm_codeblock *_create_import_codeblock(t_vm_context *ctx) {
    t_ast_element *ast = ast_generate_from_file(ctx->file.full);
    t_hash_table *asm_code = ast_to_asm(ast, 1);
    ast_free_node(ast);
    t_bytecode *bc = assembler(asm_code, ctx->file.full);
    assembler_free(asm_code);

    // Create codeblock
    return vm_codeblock_new(bc, ctx);
}

/**
 * Build actual class-path from a module (Framework::Http::Request -> Framework/Http/Request)
 *
 * @param path
 * @return
 */
static char *_build_class_path(char *module) {
    char *class_path = string_strdup0(module);
    char *c;

    // We find all '::', replace the first character with '/', and move all
    // remaining chars in the string up one position
    while (c = strstr(class_path, "::"), c != NULL) {
        *c = '/';
        memmove(c+1, c+2, strlen(c)-1);
    }

    return class_path;
}

/**
 * Construct a absolute path from a root and sub path.
 *
 * @param root_path
 * @param sub_path
 * @return
 */
static char *_construct_import_path(t_vm_context *context, char *root_path, char *module_path) {
    // We must make sure that things like . are actually resolved to the path of the CURRENT frame source.
//    char *old_cwd = NULL;
    char *path = NULL;

//    if (frame && frame->context) {
//        // Change to context path
//        long size = pathconf(".", _PC_PATH_MAX);
//        old_cwd = (char *)smm_malloc((size_t)size);
//        getcwd(old_cwd, (size_t)size);
//        printf("newpath: %s\n", frame->context->file.path);
//        chdir(frame->context->file.path);
//    }
    path = realpath(root_path, NULL);
//    if (frame && frame->context) {
//        // Change back from context path
//        chdir(old_cwd);
//        smm_free(old_cwd);
//    }
    if (! path) return NULL;

    // Strip trailing /
    if (path[strlen(path)-1] == '/') {
        path[strlen(path)-1] = '\0';
    }

    // If path is just .  we need to use the current frame's filepath
    if (strcmp(path, ".") == 0) {
        path = string_strdup0(context->file.path);
    }

    char *class_path = _build_class_path(module_path);

    char *final_path = NULL;
    if (strstr(module_path, "::") == module_path) {
        smm_asprintf_char(&final_path, "%s%s.sf", path, class_path);
    } else {
        smm_asprintf_char(&final_path, "%s%s%s.sf", path, context, class_path);
    }
    smm_free(class_path);
    smm_free(path); // free realpath()


    char *real_final_path = realpath(final_path, NULL);
    smm_free(final_path);

    return real_final_path;
}



static t_vm_stackframe *_vm_import(t_vm_codeblock *codeblock, char *absolute_class_path) {

    // Iterate all module search paths
    char **ptr = (char **)&module_search_paths;
    while (*ptr) {
        // generate path based on the current search path
        char *absolute_import_path = _construct_import_path(codeblock ? codeblock->context : NULL, *ptr, absolute_class_path);

        if (absolute_import_path && is_file(absolute_import_path)) {
            // Do import
            t_vm_context *context = vm_context_new(absolute_class_path, absolute_import_path);
            t_vm_codeblock *import_codeblock = _create_import_codeblock(context);

            t_object *result = NULL;
            t_vm_stackframe *import_stackframe = vm_execute_import(import_codeblock, &result);

            // Exception is thrown, don't continue
            if (thread_exception_thrown()) {
                vm_stackframe_destroy(import_stackframe);
                smm_free(absolute_import_path);
//                smm_free(absolute_class_path);
                return NULL;
            }

            smm_free(absolute_import_path);
            return import_stackframe;
        }

        if (absolute_import_path) {
            smm_free(absolute_import_path);
        }

        // Check next path
        ptr++;
    }

    return NULL;
}



/**
 * Import a classname from the namespace into the given frame.
 *
 */
t_object *vm_import(t_vm_codeblock *codeblock, char *class_path, char *class_name) {
    // Create absolute class path for this file
    char *absolute_class_path = vm_context_absolute_namespace(codeblock, class_path);

    // Check cache for this context
    t_vm_stackframe *imported_stackframe = ht_find_str(frame_import_cache, absolute_class_path);
    if (! imported_stackframe) {
        // Not found, import it
        imported_stackframe = _vm_import(codeblock, absolute_class_path);

        // Only add to cache when something is there
        if (imported_stackframe) {
            ht_add_str(frame_import_cache, absolute_class_path, imported_stackframe);

            t_hash_iter iter;
            ht_iter_init(&iter, frame_import_cache);

#ifdef __DEBUG
            while (ht_iter_valid(&iter)) {
                t_hash_key *key = ht_iter_key(&iter);
                DEBUG_PRINT_CHAR("Frame Cache Entry: %s\n", key->val.s);
                ht_iter_next(&iter);
            }
#endif

        } else {
            // Nothing found that actually matches a file inside the searchpaths
            if (! thread_exception_thrown()) {
                thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find any file matching '%s' in searchpath", class_path);
            }
        }
    } else {
        DEBUG_PRINT_CHAR("Using cached import class: '%s'\n", absolute_class_path);
    }


    // Exception is thrown, or nothing found, don't continue
    if (thread_exception_thrown()) {
        smm_free(absolute_class_path);
        return NULL;
    }

    // Find actual object inside the
    t_object *obj = vm_frame_local_identifier_exists(imported_stackframe, class_name);
    if (! obj) {
        thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find class '%s' inside imported file ", class_name, absolute_class_path);
        smm_free(absolute_class_path);
        return NULL;
    }

    DEBUG_PRINT_CHAR("Import: found class %s::%s\n", absolute_class_path, class_name);
    smm_free(absolute_class_path);

    return obj;
}
