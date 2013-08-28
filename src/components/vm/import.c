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
#include "vm/vm.h"
#include "vm/frame.h"
#include "objects/objects.h"
#include "compiler/ast_nodes.h"
#include "compiler/ast_to_asm.h"
#include "compiler/output/asm.h"
#include "general/path_handling.h"
#include "general/output.h"
#include "debug.h"


/*
 * @TODO: Modules are search in this order on these paths. They are currently hardcoded
 */
const char *module_search_paths[] = {
    ".",
    "/usr/share/saffire/modules",
    "/usr/share/saffire/modules/sfl",
    NULL
};


/**
 * Executes a source-file inside a frame, and extracts the needed class from the frame
 *
 * @param frame
 * @param source_file
 * @param class
 * @param module_frame
 * @return
 */
static t_vm_frame *_execute_import_frame(t_vm_frame *frame, char *source_file, char *context_name, char *context_path) {

    // @TODO: Don't care about cached bytecode for now! Compile source to BC
    t_ast_element *ast = ast_generate_from_file(source_file);
    t_hash_table *asm_code = ast_to_asm(ast, 1);
    ast_free_node(ast);
    t_bytecode *bc = assembler(asm_code, source_file);
    bc->source_filename = smm_strdup(source_file);
    assembler_free(asm_code);

    // Create a new frame and run it!
    t_vm_frame *module_frame = vm_frame_new(frame, context_path, bc);
    vm_execute(module_frame);

    return module_frame;
}

/**
 * Build actual class-path from a module (Framework::Http::Request -> Framework/Http/Request)
 *
 * @param path
 * @return
 */
static char *build_class_path(char *module) {
    char *class_path = smm_strdup(module);
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
static char *construct_import_path(t_vm_frame *frame, char *root_path, char *module_path) {
    char *path = realpath(root_path, NULL);
    if (! path) return NULL;

    // Strip trailing /
    if (root_path[strlen(root_path)-1] == '/') {
        root_path[strlen(root_path)-1] = '\0';
    }

    char *class_path = build_class_path(module_path);

    char *final_path = NULL;
    if (strstr(module_path, "::") == module_path) {
        smm_asprintf(&final_path, "%s%s.sf", root_path, class_path);
    } else {
        smm_asprintf(&final_path, "%s%s%s.sf", root_path, frame->context, class_path);
    }
    smm_free(class_path);
    smm_free(path); // free realpath()
    return final_path;
}


static t_object *search_import_path(t_vm_frame *frame, char *file_path, char *module_path, char *class, t_vm_frame **import_frame) {
    if (! is_file(file_path)) return NULL;

    *import_frame = _execute_import_frame(frame, file_path, class, module_path);
    printf("IMPORT FRAME: %08lX\n", (unsigned long)*import_frame);
    return vm_frame_find_identifier(*import_frame, class);
}


/**
 * Search all import paths for class_path, and extracts+caches the needed class from that file
 *
 * @param frame
 * @param class_path
 * @param class
 * @param file_found
 * @return
 */
static t_object *search_import_paths(t_vm_frame *frame, char *module_path, char *class, t_vm_frame **import_frame) {
    t_object *obj = NULL;

    char **search_root_path = (char **)&module_search_paths;
    while (*search_root_path) {
        // Create our actual search path
        char *final_search_path = construct_import_path(frame, *search_root_path, module_path);

        DEBUG_PRINT(" * *** Searching path: %s \n", final_search_path);

        // Return object
        obj = search_import_path(frame, final_search_path, module_path, class, import_frame);

        smm_free(final_search_path);

        if (obj) break;

        // Check next root path
        search_root_path++;
    }

    return obj;
}


/**
 *
 */
t_object *vm_import(t_vm_frame *frame, char *module, char *class) {
    char module_path[2048];
    t_object *obj = NULL;

    DEBUG_PRINT("Importing class '%s' from module '%s'\n", class, module);

    // Create complete module path including the class
    if (strstr(module, "::") == module) {
        snprintf(module_path, 2048, "%s::%s", module, class);
    } else {
        snprintf(module_path, 2048, "%s::%s::%s", frame->context_path, module, class);
    }

    // Check if the object we need to import, is already imported. No exceptions if that doesn't work
    DEBUG_PRINT(" * Looking for '%s' in the current frame\n", module_path);
    obj = vm_frame_find_identifier(frame, module_path);
    if (obj != NULL) {
        return obj;
    }


    // Create complete module path, without the class so we can find the actual file
    if (strstr(module, "::") == module) {
        // This module is already absolute, don't add current frame context
        snprintf(module_path, 2048, "%s", module);
    } else {
        snprintf(module_path, 2048, "%s::%s", frame->context_path, module);
    }

    // CHeck if we already executed the frame before. If so, it's stored in cache
    t_vm_frame *cached_frame = ht_find_str(import_cache, module_path);
    DEBUG_PRINT(" * *** Looking for a frame in cache with key '%s': %s\n", module_path, (cached_frame ? "Found" : "Nothing found"));
    if (cached_frame) {
        // Fetch the object from the frame
        obj = vm_frame_find_identifier(cached_frame, class);
    } else {
        t_vm_frame *import_frame = NULL;
        obj = search_import_paths(frame, module_path, class, &import_frame);

        // Object found
        if (import_frame && obj) {
            // Add frame to cache
            DEBUG_PRINT(" * *** Adding to import cache at key '%s'\n", module_path);
            ht_add_str(import_cache, module_path, import_frame);
        }

        // No module found
        if (! import_frame) {
            object_raise_exception(Object_SystemException, 1, "Cannot find module '%s'", module);
            return NULL;
        }
    }

    // Actual object not found
    if (! obj) {
        object_raise_exception(Object_ImportException, 1, "Cannot find class '%s' in module '%s'", class, module);
    }

    // Return object (or NULL, in which case, an exception has been thrown)
    return obj;
}


void vm_free_import_cache(void) {
    printf("\n\n\n\n\nFreeing import cache\n");

    t_hash_iter iter;
    ht_iter_init(&iter, import_cache);
    while (ht_iter_valid(&iter)) {
        t_vm_frame *frame = ht_iter_value(&iter);
        printf("DESTROY FRAME: %08lX (%s)\n", (unsigned long)frame, frame->context);

        t_bytecode *bc = frame->bytecode;

        vm_frame_destroy(frame);

        if (bc) bytecode_free(bc);

        printf("\n");

        ht_iter_next(&iter);
    }

    ht_destroy(import_cache);
}
