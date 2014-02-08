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
#include "vm/frame.h"
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
    ".",
    "./modules",
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
static t_vm_frame *_execute_import_frame(t_vm_frame *frame, char *class_path, char *file_path) {

    // @TODO: Don't care about cached bytecode for now! Compile source to BC
    t_ast_element *ast = ast_generate_from_file(file_path);
    t_hash_table *asm_code = ast_to_asm(ast, 1);
    ast_free_node(ast);
    t_bytecode *bc = assembler(asm_code, file_path);
    assembler_free(asm_code);

    // Create a new frame and run it!
    t_vm_frame *module_frame = vm_frame_new(frame, class_path, file_path, bc);
    t_object *result = vm_execute_import(module_frame);

    if (result == NULL && thread_exception_thrown()) return NULL;

    return module_frame;
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
static char *_construct_import_path(t_vm_frame *frame, char *root_path, char *module_path) {
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

    // If path is just .  we need to use the current filepath
    if (strcmp(path, ".") == 0) {
        path = string_strdup0(frame->context->file.path);
    }

    char *class_path = _build_class_path(module_path);

    char *final_path = NULL;
    if (strstr(module_path, "::") == module_path) {
        smm_asprintf_char(&final_path, "%s%s.sf", path, class_path);
    } else {
        smm_asprintf_char(&final_path, "%s%s%s.sf", path, frame->context, class_path);
    }
    smm_free(class_path);
    smm_free(path); // free realpath()


    char *real_final_path = realpath(final_path, NULL);
    smm_free(final_path);

    return real_final_path;
}

/**
 *
 */
static t_vm_frame *_search_import_path(t_vm_frame *frame, char *class_path, char *file_path) {
    if (! is_file(file_path)) return NULL;

    t_vm_frame *imported_frame = _execute_import_frame(frame, class_path, file_path);
    DEBUG_PRINT_CHAR("IMPORT FRAME: %08lX\n", (unsigned long)imported_frame);
    return imported_frame;
}

/**
 *
 */
static t_vm_frame *_search_import_paths(t_vm_frame *frame, char *class_path) {
    t_vm_frame *imported_frame = NULL;

    char **search_root_path = (char **)&module_search_paths;
    while (*search_root_path) {
        // Create our actual search path
        char *final_search_path = _construct_import_path(frame, *search_root_path, class_path);
        if (final_search_path) {
            DEBUG_PRINT_CHAR(" * *** Searching path: %s \n", final_search_path);

            // Return object
            imported_frame = _search_import_path(frame, class_path, final_search_path);

            smm_free(final_search_path);

            if (imported_frame) break;

        }

        // Check next root path
        search_root_path++;
    }

    return imported_frame;

}


/**
 * Find (and execute) the frame pointed by the namespace.
 *
 * @param namespace
 * @return
 */
t_vm_frame *vm_import_find_file(t_vm_frame *frame, char *class_path) {
    DEBUG_PRINT_CHAR("Importing module '%s'\n", class_path);

    // Check if we already executed the frame before. If so, it's stored in cache
    t_vm_frame *cached_frame = ht_find_str(frame_import_cache, class_path);
    DEBUG_PRINT_CHAR(" * *** Looking for a frame in cache with key '%s': %s\n", class_path, cached_frame ? "Found" : "Nothing found");
    if (cached_frame) {
        return cached_frame;
    }

    t_vm_frame *imported_frame = _search_import_paths(frame, class_path);
    if (imported_frame) {
        ht_add_str(frame_import_cache, class_path, imported_frame);
    }
    return imported_frame;
}


/**
 * Import a classname from the namespace into the given frame.
 *
 * @param frame
 * @param namespace
 * @param classname
 * @return
 */
t_object *vm_import(t_vm_frame *frame, char *class_path, char *class_name) {
    // Find and import the file
    char *abs_cp = vm_frame_absolute_namespace(frame, class_path);
    t_vm_frame *imported_frame = vm_import_find_file(frame, abs_cp);
    if (! imported_frame) {
        // If we couldn't import the frame, and no exception has been thrown in the meantime, we just throw our own exception
        if (! thread_exception_thrown()) {
            thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find module '%s'", abs_cp);
        }
        if (abs_cp) smm_free(abs_cp);
        return NULL;
    }
    if (abs_cp) smm_free(abs_cp);

    // Exception is thrown, don't continue
    if (thread_exception_thrown()) {
        return NULL;
    }

    // Find actual object inside the
    t_object *obj = vm_frame_local_identifier_exists(imported_frame, class_name);
    if (! obj) {
        thread_create_exception_printf((t_exception_object *)Object_ImportException, 1, "Cannot find class '%s'", class_name);
        return NULL;
    }
    return obj;
}


/**
 * Free all the imported files
 */
void vm_free_import_cache(void) {
    DEBUG_PRINT_CHAR("\n\n\n\n\nFreeing import cache\n");

    t_hash_iter iter;
    ht_iter_init(&iter, frame_import_cache);
    while (ht_iter_valid(&iter)) {
        t_vm_frame *frame = ht_iter_value(&iter);

        // @TODO: This should not happen. Frames inside the import-cache should stay
        // here until we actually remove it in the code below
        if (frame) {
            DEBUG_PRINT_CHAR("DESTROY FRAME: %08lX (%s)\n", (unsigned long)frame, frame->context->file.path);

            t_bytecode *bc = frame->bytecode;

            vm_frame_destroy(frame);

            if (bc) bytecode_free(bc);

            DEBUG_PRINT_CHAR("\n");
        }

        ht_iter_next(&iter);
    }

    ht_destroy(frame_import_cache);
}
