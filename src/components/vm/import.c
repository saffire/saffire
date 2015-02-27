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
#include <limits.h>
#include <unistd.h>
#include <saffire/vm/vm.h>
#include <saffire/vm/stackframe.h>
#include <saffire/vm/thread.h>
#include <saffire/vm/import.h>
#include <saffire/objects/objects.h>
#include <saffire/compiler/ast_nodes.h>
#include <saffire/compiler/ast_to_asm.h>
#include <saffire/compiler/output/asm.h>
#include <saffire/general/path_handling.h>
#include <saffire/general/output.h>
#include <saffire/debug.h>


/*
 * @TODO: Modules are search in this order on these paths. They are currently hardcoded :(
 */
const char *module_search_paths[] = {
    ".",                                    // Should always be first
    "./modules",                            // Should always be second
    "/usr/share/saffire/modules",           // From here it should be customizable
    NULL
};

const char *module_searches[] = {
    "::sfl",
    NULL
};

// global FQMN : t_vm_module_mapping
t_hash_table *global_module_mapping;

// global FQCN : t_vm_class_mapping
t_hash_table *global_class_mapping;


typedef struct _vm_module_mapping {
    t_vm_stackframe *frame;             // or null when not resolved yet.
} t_vm_module_mapping;

// class mapping
typedef struct _vm_class_mapping {
    t_vm_module_mapping *module;        // Module mapping
    t_object *object;                   // Actual object from module map (or NULL when not resolved yet)
} t_vm_class_mapping;



/**
 * Initialize import cache table.
 */
void vm_namespace_cache_init(void) {
    global_module_mapping = ht_create();
    global_class_mapping = ht_create();
}

/**
 * Free import cache table.
 */
void vm_namespace_cache_fini(void) {
    t_hash_iter iter;

    // Destroy module mapping
    ht_iter_init(&iter, global_module_mapping);
    while (ht_iter_valid(&iter)) {
        t_vm_module_mapping *module_map = ht_iter_value(&iter);

        // Save codeblock reference
        t_vm_codeblock *codeblock = module_map->frame->codeblock;

        // Destroy actual frame
        vm_stackframe_destroy(module_map->frame);

        // Destroy codeblock of frame
        bytecode_free(codeblock->bytecode);
        vm_codeblock_destroy(codeblock);

        smm_free(module_map);
        ht_iter_next(&iter);
    }

    ht_destroy(global_module_mapping);


    // Destroy class mapping
    ht_iter_init(&iter, global_class_mapping);
    while (ht_iter_valid(&iter)) {
        t_vm_class_mapping *class_map = ht_iter_value(&iter);

        // Note: we don't release class_map->module, as they point to global_module_mapping entries, which we already cleaned up.

        if (class_map->object) {
            object_release(class_map->object);
        }
        smm_free(class_map);
        ht_iter_next(&iter);
    }

    ht_destroy(global_class_mapping);
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


/**
 * Creates a vm_context structure based on the *FQMN* by finding the actual module on the module search paths
 */
static t_vm_context *_vm_create_context_from_fqmn(char *fqmn) {

    // Iterate all module search paths
    char **ptr = (char **)&module_search_paths;
    while (*ptr) {
        // generate path based on the current search path
        char *absolute_import_path = _construct_import_path(NULL, *ptr, fqmn);

        if (absolute_import_path && is_file(absolute_import_path)) {

            // Do import
            t_vm_context *context = vm_context_new(fqmn, absolute_import_path);

            smm_free(absolute_import_path);
            return context;
        }

        if (absolute_import_path) {
            smm_free(absolute_import_path);
        }

        // Check next path
        ptr++;
    }

    // Nothing found
    return NULL;
}


/**
 * Create a codeblock by importing the source from the given context.
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

    // Create codeblock with duplicated context
    return vm_codeblock_new(bc, vm_context_duplicate(ctx));
}


static t_vm_stackframe *_resolve_module(t_vm_context *ctx) {
    t_vm_codeblock *codeblock = _create_import_codeblock(ctx);

    // Save original excption, if any, and clear exceptions so we can do import on the thread
    t_exception_object *exception = thread_get_exception();
    thread_clear_exception();

    t_vm_stackframe *import_stackframe = vm_execute_import(codeblock, NULL);

    // exception during import is thrown, don't continue
    if (thread_exception_thrown()) {
        vm_stackframe_destroy(import_stackframe);
        vm_codeblock_destroy(codeblock);
        return NULL;
    }

    // Restore original exception (if any)
    if (exception) {
        thread_set_exception(exception);
    }

    return import_stackframe;
}


static t_object *_resolve_class_from_module(char *fqcn, t_vm_module_mapping *module_map) {
    t_object *obj = vm_frame_identifier_exists(module_map->frame, fqcn);
    if (! obj) {
        thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find class '%s' inside imported file %s", fqcn, module_map->frame->codeblock->context->file.full);
        return NULL;
    }

    return obj;
}


t_object *_class_resolve(t_vm_stackframe *frame, char *fqcn) {
    // Find the actual class in the class_mapping

    // Check if we already imported the class
    t_vm_class_mapping *class_map = ht_find_str(global_class_mapping, fqcn);
    if (! class_map) {
        // No class map found. Add class map entry.


        // Find module mapping
        char *fqmn = vm_context_get_path(fqcn);
        t_vm_module_mapping *module_map = ht_find_str(global_module_mapping, fqmn);

        if (! module_map) {
            // no module mapping found. Create one
            t_vm_context *ctx = _vm_create_context_from_fqmn(fqmn);
            if (!ctx) {
                smm_free(fqmn);
                return NULL;
            }
            t_vm_stackframe *module_frame = _resolve_module(ctx);
            vm_context_free_context(ctx);

            if (! module_frame) {
                // Cannot resolve module from context
                //@ TODO: Throw exception here
                smm_free(fqmn);
                return NULL;
            }

            module_map = smm_malloc(sizeof(t_vm_module_mapping));
            module_map->frame = module_frame;
            ht_add_str(global_module_mapping, fqmn, module_map);
        }
        smm_free(fqmn);

        // We are sure we have a module_map now
        class_map = smm_malloc(sizeof(t_vm_class_mapping));
        class_map->module = module_map;
        class_map->object = NULL;
        ht_add_str(global_class_mapping, fqcn, class_map);
    }


    // Object not resolved yet, resolve it first
    if (! class_map->object) {
        t_object *obj = _resolve_class_from_module(fqcn, class_map->module);
        class_map->object = obj;
        object_inc_ref(obj);
    }

    return class_map->object;
}

t_object *vm_class_resolve(t_vm_stackframe *frame, char *qcn) {
    t_object *obj;

    // Check FQCN
    char *fqcn = vm_context_fqcn(frame->codeblock->context, qcn);
    obj = _class_resolve(frame, fqcn);
    smm_free(fqcn);
    if (obj) {
        return obj;
    }

    // Iterate all module search paths to see if those FQCN's fit
    char **ptr = (char **)&module_searches;
    while (*ptr) {
        char *fqcn;
        vm_context_create_fqcn(*ptr, qcn, &fqcn);

        obj = _class_resolve(frame, fqcn);
        smm_free(fqcn);
        if (obj) {
            return obj;
        }

        // Check next module
        ptr++;
    }

    // Nothing more to search
    return NULL;
}
