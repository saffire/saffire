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
#include <saffire/modules/module_api.h>
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


// Modules are normally FQCN. When not found, we have a prefix backup. This allows to do  "import io" which
// equals "import \saffire\io"
const char *module_searches[] = {
    "\\saffire",
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


static char *_construct_import_path_with_extension(char *root_path, char *module_path, char *extension) {
    char *path = NULL;

    path = realpath(root_path, NULL);
    if (! path) return NULL;

    // Strip trailing /
    if (path[strlen(path)-1] == '/') {
        path[strlen(path)-1] = '\0';
    }

    char *final_path = NULL;
    smm_asprintf_char(&final_path, "%s%s.%s", path, module_path, extension);
    smm_free(path); // free realpath()

    char *real_final_path = realpath(final_path, NULL);

    smm_free(final_path);

    return real_final_path;
}

/**
 * Construct a absolute path from a root and sub path.
 *
 * @param root_path
 * @param sub_path
 * @return
 */
static char *_construct_import_path(char *root_path, char *module_path, int *extension_type) {
    char *real_final_path;

    // Find source file
    real_final_path = _construct_import_path_with_extension(root_path, module_path, "sf");
    if (real_final_path) {
        *extension_type = module_type_source;
        return real_final_path;
    }

    // Find bytecode file
    real_final_path = _construct_import_path_with_extension(root_path, module_path, "sfc");
    if (real_final_path) {
        *extension_type = module_type_bytecode;
        return real_final_path;
    }

    // Find extension shared object
    real_final_path = _construct_import_path_with_extension(root_path, module_path, "so");
    if (real_final_path) {
        *extension_type = module_type_shared_object;
        return real_final_path;
    }

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
    if (! ast) return NULL;

    t_hash_table *asm_code = ast_to_asm(ast, 1);
    ast_free_node(ast);
    t_bytecode *bc = assembler(asm_code, ctx->file.full);
    assembler_free(asm_code);

    // Create codeblock with duplicated context
    return vm_codeblock_new(bc, vm_context_duplicate(ctx));
}


static t_vm_stackframe *_load_module_from_context(t_vm_context *ctx) {
    t_vm_codeblock *codeblock = _create_import_codeblock(ctx);

    if (! codeblock) {
        thread_create_exception_printf((t_exception_object *)Object_CompileException, 1, "Could not compile %s", ctx->file.full);
        return NULL;
    }


    t_exception_object *original_exception = thread_save_exception();

    t_vm_stackframe *import_stackframe = vm_execute_import(codeblock, NULL);

    // exception during import is thrown, don't continue
    if (thread_exception_thrown()) {
        thread_dump_exception(original_exception);

        return NULL;
    }

    // Restore original exception (if any)
    thread_restore_exception(original_exception);

    return import_stackframe;
}


t_vm_stackframe *_load_module_from_sharedobject(char *fqmn, char *path) {
    t_vm_stackframe *frame = vm_create_empty_stackframe();

    // Load the objects from this module inside this frame
    register_external_module(path, frame);

    return frame;
}


t_vm_stackframe *_load_module_from_bytecode(char *fqmn, char *path) {
    fatal_error(1, "Cannot load saffire bytecode yet as module");
    return NULL;
}

t_vm_stackframe *_load_module_from_source(char *fqmn, char *path) {
    // Create a context in which the module will be run
    t_vm_context *ctx = vm_context_new(fqmn, path);
    if (!ctx) {
        smm_free(fqmn);
        return NULL;
    }

    // Resolve the module
    t_vm_stackframe *module_frame = _load_module_from_context(ctx);
    vm_context_free(ctx);

    return module_frame;
}


static t_vm_stackframe *_create_module_frame_from_path(char *root_path, char *module_fqcn) {
    t_vm_stackframe *module_frame = NULL;
    int detected_extension_type = 0;

    // Construct actual path by
    char *module_path = vm_context_convert_fqcn_to_path(module_fqcn);
    char *absolute_import_path = _construct_import_path(root_path, module_path, &detected_extension_type);

    // Found a real file?
    if (absolute_import_path && is_file(absolute_import_path)) {
        //printf("* Loading absolute path: '%s'\n", absolute_import_path);
        switch (detected_extension_type) {
            case module_type_source:
                // Saffire source
                module_frame = _load_module_from_source(module_fqcn, absolute_import_path);
                break;
            case module_type_bytecode :
                // Saffire bytecode
                module_frame = _load_module_from_bytecode(root_path, absolute_import_path);
                break;
            case module_type_shared_object :
                // Saffire shared object
                module_frame = _load_module_from_sharedobject(root_path, absolute_import_path);
                break;
        }

        // Free file path
        if (absolute_import_path) {
            smm_free(absolute_import_path);
        }

        return module_frame;
    }

    // Free file path
    if (absolute_import_path) {
        smm_free(absolute_import_path);
    }
    return NULL;
}

static t_vm_stackframe *_create_module_frame_from_module_path(char *module_path) {
    t_vm_stackframe *module_frame;

    // Iterate all module search paths
    char **root_path = (char **)&module_search_paths;
    while (*root_path) {
        // Check path based on the available search paths
        module_frame = _create_module_frame_from_path(*root_path, module_path);
        if (module_frame) return module_frame;

        // Exception was thrown during import of a file
        if (thread_exception_thrown()) {
            return NULL;
        }

        // Check next path
        root_path++;
    }

    // Nothing found
    return NULL;
}


static t_vm_module_mapping *_resolve_module_map(char *fqcn)
{
    t_vm_stackframe *module_frame = _create_module_frame_from_module_path(fqcn);

    if (! module_frame) {
        // Cannot resolve module from context.
        return NULL;
    }

    t_vm_module_mapping *module_map = smm_malloc(sizeof(t_vm_module_mapping));
    module_map->frame = module_frame;

    return module_map;
}


static t_vm_class_mapping *_resolve_class_map(char *fqcn)
{
    // Find module mapping
    char *module_path = vm_context_convert_fqcn_to_path(fqcn);
    t_vm_module_mapping *module_map = ht_find_str(global_module_mapping, module_path);

    if (! module_map) {
        module_map = _resolve_module_map(fqcn);
        if (! module_map) {
            smm_free(module_path);
            return NULL;
        }

        ht_add_str(global_module_mapping, module_path, module_map);
    }
    smm_free(module_path);

    // We are sure we have a module_map now
    t_vm_class_mapping *class_map = smm_malloc(sizeof(t_vm_class_mapping));
    class_map->module = module_map;
    class_map->object = NULL;

    return class_map;
}


static t_object *_resolve_class_map_object(t_vm_class_mapping *class_map, char *fqcn)
{

    // Check fully qualified class name first
    t_object *obj = vm_frame_identifier_exists(class_map->module->frame, fqcn);
//    if (! obj) {
//        char *class_name = NULL;
//        // check direct class name (@TODO: Why is this happening? In order to make sure they get loaded directly from new frames, so it seems)
//        class_name = vm_context_get_classname_from_fqcn(fqcn);
//        obj = vm_frame_identifier_exists(class_map->module->frame, class_name);
//        smm_free(class_name);
//    }

    if (! obj) {
        return NULL;
    }

    class_map->object = obj;
    object_inc_ref(obj);

    return obj;
}

/**
 * Do actual resolving of a FQCN
 */
t_object *_class_resolve(t_vm_stackframe *frame, char *fqcn)
{
    // Check if the FQCN is registered in the built-ins first.
    t_object *builtin_obj = ht_find_str(frame->builtin_identifiers->data.ht, fqcn);
    if (builtin_obj) {
        if (builtin_obj == OBJECT_NEEDS_RESOLVING) {
            fatal_error(1, "Cannot resolve built-in object");
        }
        return builtin_obj;
    }


    // Check if we already imported the module which contains this class. If not, import the module
    t_vm_class_mapping *class_map = ht_find_str(global_class_mapping, fqcn);
    if (! class_map) {
        class_map = _resolve_class_map(fqcn);
        if (! class_map) {
            return NULL;
        }
        ht_add_str(global_class_mapping, fqcn, class_map);
    }

    // Object not resolved yet, resolve it first
    if (! class_map->object) {
        _resolve_class_map_object(class_map, fqcn);
    }

    // Module resolved, but could not load class (does not exist in the class)
    if (! class_map->object) {
        t_vm_context *ctx = vm_frame_get_context(class_map->module->frame);
        thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find class '%s' inside imported file %s", fqcn, ctx->file.full);

        return NULL;
    }

    return class_map->object;
}


/**
 * Resolved a class based on the QCN. Throws an exception when it cannot resolve.
 */
t_object *vm_class_resolve(t_vm_stackframe *frame, char *qcn) {
    t_object *obj;

    // Convert QCN to FQCN if needed
    char *fqcn = vm_context_create_fqcn_from_context(vm_frame_get_context(frame), qcn);

    obj = _class_resolve(frame, fqcn);
    smm_free(fqcn);
    if (obj) {
        return obj;
    }

    // Exception is thrown when trying to resolve the module
    if (thread_exception_thrown()) {
        return NULL;
    }

    // Iterate all module search paths to see if those FQCN's fit
    char **ptr = (char **)&module_searches;
    while (*ptr) {
        char *stripped_qcn = vm_context_strip_context(vm_frame_get_context(frame), qcn);
        char *fqcn = vm_context_concat_path(*ptr, stripped_qcn);
        smm_free(stripped_qcn);

        obj = _class_resolve(frame, fqcn);
        smm_free(fqcn);
        if (obj) {
            return obj;
        }

        // Check next module
        ptr++;
    }

    // Throw exception as we could not resolve
    object_raise_exception(Object_ImportException, 1, "Cannot resolve module '%s'", qcn);
    return NULL;
}
