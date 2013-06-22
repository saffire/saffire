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
#include "compiler/bytecode.h"
#include "vm/vm.h"
#include "vm/vm_opcodes.h"
#include "vm/block.h"
#include "vm/frame.h"
#include "vm/thread.h"
#include "general/dll.h"
#include "general/smm.h"
#include "objects/object.h"
#include "objects/objects.h"
#include "modules/module_api.h"
#include "debug.h"
#include "general/output.h"
#include "vm/import.h"
#include "gc/gc.h"
#include "debugger/dbgp/dbgp.h"

t_hash_table *builtin_identifiers;         // Builtin identifiers like first-class objects, the _sfl etc

#define REASON_NONE         0       // No return status. Just end the execution
#define REASON_RETURN       1       // Return statement given
#define REASON_CONTINUE     2
#define REASON_BREAK        3
#define REASON_BREAKELSE    4
#define REASON_EXCEPTION    5       // Exception occurred
#define REASON_RERAISE      6       // Exception not handled. Reraised in finally clause
#define REASON_FINALLY      7       // No exception raised after finally


extern char *objectOprMethods[];
extern char *objectCmpMethods[];

/**
 * Parse calling arguments. It will iterate all arguments declarations needed for the
 * callable. The arguments are placed onto the frame stack.
 *
 * Returns 1 in success, 0 on failure/exception is thrown
 */
static int _parse_calling_arguments(t_vm_frame *frame, t_callable_object *callable, t_dll *arg_list) {
    t_hash_table *ht = ((t_hash_object *)callable->arguments)->ht;
    t_dll_element *e = DLL_HEAD(arg_list);

    int need_count = ht->element_count;
    int given_count = arg_list->size;

    // When set to null, no varargs are wanted
    t_list_object *vararg_obj = NULL;

    int cur_arg = 0;

    t_hash_iter iter;
    ht_iter_init(&iter, ht);
    while (ht_iter_valid(&iter)) {
        cur_arg++;

        char *name = ht_iter_key(&iter);
        t_method_arg *arg = ht_iter_value(&iter);

        int is_vararg =0 ;
        if (arg->typehint->type != objectTypeNull && ! strcmp(OBJ2STR(arg->typehint), "...")) {
            is_vararg = 1;
        }

        // Preset object to default value if a default value was found.
        t_object *obj = (arg->value->type == objectTypeNull) ? NULL : arg->value;

        // If we have values on the calling arg list, use the next value, overriding any default values set.
        if (given_count) {
            obj = e ? e->data : NULL;
        }

        // No more arguments to pass found, so obj MUST be of a value, otherwise caller didn't specify enough arguments.
        if (obj == NULL && ! is_vararg) {
            object_raise_exception(Object_ArgumentException, "Not enough arguments passed, and no default values found");
            return 0;
        }

        if (arg->typehint->type != objectTypeNull) {
            // Check typehint / varargs

            if (is_vararg) {
                // the '...' typehint found.
                vararg_obj = (t_list_object *)object_new(Object_List, 0);

                // Add first argument
                if (obj) {
                    ht_num_add(vararg_obj->ht, vararg_obj->ht->element_count, obj);
                }

                // Make sure we add our List[] to the local_identifiers below
                obj = (t_object *)vararg_obj;
            } else if (! object_instance_of(obj, (const char *)OBJ2STR(arg->typehint))) {
                // classname does not match the typehint

                // @TODO: we need to check if object as a parent or interface that matches!
                object_raise_exception(Object_ArgumentException, "Typehinting for argument %d does not match. Wanted '%s' but found '%s'\n", cur_arg, OBJ2STR(arg->typehint), obj->name);
                return 0;
            }
        }

        // Everything is ok, add the new value onto the local identifiers
        ht_add(frame->local_identifiers->ht, name, obj);

        need_count--;
        given_count--;

        // Next needed element
        ht_iter_next(&iter);
        if (e) e = DLL_NEXT(e);
    }


    // If there are more arguments passed, check if we can feed them to the vararg, if present
    if (given_count > 0) {
        if (vararg_obj == NULL) {
            object_raise_exception(Object_ArgumentException, "No variable argument found, and too many arguments passed");
            return 0;
        }

        // Just add arguments to vararg list. No need to do any typehint checks here.
        while (e) {
            ht_num_add(vararg_obj->ht, vararg_obj->ht->element_count, e->data);
            e = DLL_NEXT(e);
        }
    }

    // All ok
    return 1;
}


/**
 * Checks visibility, returns 0 when not allowed, 1 when allowed.
 *
 *   1) if attribute == public, we always allow
 *   2) if attribute == protected, we allow from same class or when the class extends this class
 *   3) if attribute == private, we only allow from the same class
 */
int vm_check_visibility(t_object *binding, t_object *instance, t_object *attrib) {
    // Not bound, so always ok
    if (! binding) return 1;

    // Public attributes are always ok
    if (ATTRIB_IS_PUBLIC(attrib)) return 1;

    // Private visiblity is allowed when we are inside the SAME class.
    if (ATTRIB_IS_PRIVATE(attrib) && binding == instance) return 1;

    if (ATTRIB_IS_PROTECTED(attrib)) {
        // Iterate self down all its parent, to see if one matches "attrib". If so, the protected visibility is ok.
        while (binding) {
            if (binding == instance) return 1;
            binding = binding->parent;
        }
    }

    // Not
    return 0;
}

int debug = 0;
t_debuginfo *debug_info;

/**
 *
 */
t_vm_frame *vm_init(SaffireParser *sp, int runmode) {
    // Set run mode (repl, cli, fastcgi)
    vm_runmode = runmode;

    t_thread *thread = smm_malloc(sizeof(t_thread));
    bzero(thread, sizeof(t_thread));
    current_thread = thread;

    gc_init();
    builtin_identifiers = ht_create();
    object_init();
    module_init();

    // Create initial frame
    t_vm_frame *initial_frame = vm_frame_new((t_vm_frame *) NULL, NULL, NULL);

    thread_set_current_frame(initial_frame);

    // Initialize debugging if needed
    if ((runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        debug_info = dbgp_init(initial_frame);
    }


    vm_runmode &= ~VM_RUNMODE_DEBUG;

    // Implicit load the saffire module
    t_object *obj = vm_import(initial_frame, "saffire", "saffire");
    vm_frame_set_identifier(initial_frame, "saffire", obj);

    vm_runmode = runmode;

    return initial_frame;
}

void vm_fini(t_vm_frame *frame) {
    // Initialize debugging if needed
    if ((vm_runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        dbgp_fini(debug_info, frame);
    }

    vm_frame_destroy(frame);

    module_fini();
    object_fini();
    ht_destroy(builtin_identifiers);
    gc_fini();
}



int getlineno(t_vm_frame *frame) {
    if (frame->ip && frame->lineno_lowerbound <= frame->ip && frame->ip <= frame->lineno_upperbound) {
        return frame->lineno_current_line;
    }

    int delta_lino = 0;
    int delta_line = 0;

    // @TODO: Check if lino_offset doesn't go out of bounds
    if (frame->lineno_current_lino_offset >= frame->bytecode->lino_length) {
        return frame->lineno_current_line;
    }

    int i;
    do {
        i = (frame->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_line += i;
    } while (i > 127);
    do {
        i = (frame->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_lino += i;
    } while (i > 127);

    frame->lineno_lowerbound = frame->lineno_upperbound;
    frame->lineno_upperbound += delta_lino;
    frame->lineno_current_line += delta_line;

    return frame->lineno_current_line;
}


t_vm_frameblock *unwind_blocks(t_vm_frame *frame, long *reason, t_object *ret);

/**
 *
 */
t_object *_vm_execute(t_vm_frame *frame) {
    register t_object *obj1, *obj2, *obj3, *obj4;
    register t_object *left_obj, *right_obj;
    register char *name;
    register unsigned int opcode, oparg1, oparg2, oparg3;
    long reason = REASON_NONE;
    register t_object *dst;


#ifdef DEBUG
    printf(ANSI_BRIGHTRED "------------ NEW FRAME ------------\n" ANSI_RESET);
    t_vm_frame *tb_frame = frame;
    int tb_history = 0;
    while (tb_frame) {
        printf(ANSI_BRIGHTBLUE "#%d "
                ANSI_BRIGHTYELLOW "%s:%d "
                ANSI_BRIGHTGREEN "%s.%s "
                ANSI_BRIGHTGREEN "((string)foo, (string)bar, (string)baz)"
                ANSI_RESET "\n",
                tb_history,
                tb_frame->bytecode->source_filename ? tb_frame->bytecode->source_filename : "<none>",
                123,
                "class",
                "method"
                );
        tb_frame = tb_frame->parent;
        tb_history++;
    }
    printf(ANSI_BRIGHTRED "-----------------------------------\n" ANSI_RESET);
#endif

    // Set the correct current frame
    t_vm_frame *old_current_frame = thread_get_current_frame();
    thread_set_current_frame(frame);

    // Default return value;
    t_object *ret = NULL;

    for (;;) {


        // Room for some other stuff
dispatch:
        // Increase number of executions done
        frame->executions++;


        int ln = getlineno(frame);
        unsigned long cip = frame->ip;
#ifdef __DEBUG
        //vm_frame_stack_debug(frame);
#endif



        // Only do this when we are debugging and the debugger is attached
        if ((vm_runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG && debug_info->attached) {
            dbgp_debug(debug_info, frame);
        }



        // Get opcode and additional argument
        opcode = vm_frame_get_next_opcode(frame);

        // If high bit is set, get operand
        oparg1 = ((opcode & 0x80) == 0x80) ? vm_frame_get_operand(frame) : 0;
        oparg2 = ((opcode & 0xC0) == 0xC0) ? vm_frame_get_operand(frame) : 0;
        oparg3 = ((opcode & 0xE0) == 0xE0) ? vm_frame_get_operand(frame) : 0;

#ifdef __DEBUG
        if ((opcode & 0xE0) == 0xE0) {
            DEBUG_PRINT(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%s (0x%02X, 0x%02X, 0x%02X) "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1, oparg2, oparg3,
                        frame->bytecode->source_filename,
                        ln
                    );
            } else if ((opcode & 0xC0) == 0xC0) {
            DEBUG_PRINT(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%s (0x%02X, 0x%02X) "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1, oparg2,
                        frame->bytecode->source_filename,
                        ln
                    );
        } else if ((opcode & 0x80) == 0x80) {
            DEBUG_PRINT(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%s (0x%02X) "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1,
                        frame->bytecode->source_filename,
                        ln
                    );
        } else {
            DEBUG_PRINT(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%s "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        frame->bytecode->source_filename,
                        ln
                    );
        }
#endif





        if (opcode == VM_STOP) break;
        if (opcode == VM_RESERVED) {
            fatal_error(1, "VM: Reached reserved (0xFF) opcode. Halting.\n");
        }


        switch (opcode) {
            // Removes SP-0
            case VM_POP_TOP :
                obj1 = vm_frame_stack_pop(frame);
                object_dec_ref(obj1);
                goto dispatch;
                break;

            // Rotate / swap SP-0 and SP-1
            case VM_ROT_TWO :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-2
            case VM_ROT_THREE :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                obj3 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                vm_frame_stack_push(frame, obj3);
                goto dispatch;
                break;

            // Duplicate SP-0
            case VM_DUP_TOP :
                obj1 = vm_frame_stack_fetch_top(frame);
                object_inc_ref(obj1);
                vm_frame_stack_push(frame, obj1);
                goto dispatch;
                break;

            // Rotate SP-0 to SP-3
            case VM_ROT_FOUR :
                obj1 = vm_frame_stack_pop(frame);
                obj2 = vm_frame_stack_pop(frame);
                obj3 = vm_frame_stack_pop(frame);
                obj4 = vm_frame_stack_pop(frame);
                vm_frame_stack_push(frame, obj1);
                vm_frame_stack_push(frame, obj2);
                vm_frame_stack_push(frame, obj3);
                vm_frame_stack_push(frame, obj4);
                goto dispatch;
                break;

            // No operation
            case VM_NOP :
                // Do nothing..
                goto dispatch;
                break;

            // Load an attribute from an object
            case VM_LOAD_ATTRIB :
                {
                    register t_object *name = vm_frame_get_constant(frame, oparg1);
                    register t_object *search_obj = vm_frame_stack_pop(frame);
                    register t_object *bound_obj = vm_frame_find_identifier(frame, "self");

                    // Hardcoded to bind the attribute to the search object!!
                    bound_obj = search_obj;

                    // DEBUG_PRINT("Fetching %s from %s\n", OBJ2STR(name), search_obj->name);
                    // DEBUG_PRINT("Binding to: %s\n", bound_obj ? object_debug(bound_obj) : "no binding!");

                    register t_object *attrib_obj = object_find_actual_attribute(search_obj, OBJ2STR(name));
                    if (attrib_obj == NULL) {
                        reason = REASON_EXCEPTION;
                        thread_set_exception(Object_AttributeException, "Attribute not found");
                        goto block_end;
                        break;
                    }

                    // DEBUG_PRINT("     NAME: %s\n", OBJ2STR(name));
                    // DEBUG_PRINT("     SEARCH: %s\n", object_debug(search_obj));
                    // DEBUG_PRINT("     BOUND: %s\n", object_debug(bound_obj));
                    // DEBUG_PRINT("     ATTR: %s\n", object_debug(attrib_obj));

                    if (! vm_check_visibility(bound_obj, search_obj, attrib_obj)) {
                        thread_set_exception_printf(Object_VisibilityException, "Visibility does not allow to fetch attribute '%s'\n", OBJ2STR(name));
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    register t_object *value;
                    if (ATTRIB_IS_METHOD(attrib_obj)) {
                        register t_callable_object *callable_obj = (t_callable_object *)((t_attrib_object *)attrib_obj)->attribute;

                        /* Every callable method is bounded by its calling class (self). We save this info inside the callable
                         * @TODO: it would make sense to add this self as a stack parameter, but we cannot (this info is not really
                         * available in the AST.
                         *
                         * Therefore, we create new copies of callable-objects and just set a new binding-object. Since everything is
                         * linked in the callable-class, overhead should be minimal, for now... */

                        register t_callable_object *new_copy = (t_callable_object *)smm_malloc(sizeof(t_callable_object));
                        memcpy(new_copy, callable_obj, sizeof(t_callable_object));

                        new_copy->binding = bound_obj ? bound_obj : search_obj;

                        value = (t_object *)new_copy;
                    } else {
                        value = ((t_attrib_object *)attrib_obj)->attribute;
                    }

                    object_inc_ref((t_object *)value);
                    vm_frame_stack_push(frame, (t_object *)value);

                }
                goto dispatch;
                break;

            // Store an attribute into an object
            case VM_STORE_ATTRIB :
                {
                    register t_object *name = vm_frame_get_constant(frame, oparg1);
                    register t_object *search_obj = vm_frame_stack_pop(frame);
                    register t_object *bound_obj = vm_frame_find_identifier(frame, "self");

                    register t_object *attrib_obj = object_find_actual_attribute(search_obj, OBJ2STR(name));
                    if (attrib_obj && ATTRIB_IS_READONLY(attrib_obj)) {
                        thread_set_exception_printf(Object_VisibilityException, "Cannot write to readonly attribute '%s'\n", OBJ2STR(name));
                        reason = REASON_EXCEPTION;
                        goto block_end;

                    }
                    if (attrib_obj && ! vm_check_visibility(bound_obj, search_obj, attrib_obj)) {
                        thread_set_exception_printf(Object_VisibilityException, "Visibility does not allow to access attribute '%s'\n", OBJ2STR(name));
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    register t_object *value = vm_frame_stack_pop(frame);
                    object_add_property(search_obj, OBJ2STR(name), ATTRIB_TYPE_PROPERTY | ATTRIB_ACCESS_RW | ATTRIB_VISIBILITY_PUBLIC, value);

                }
                goto dispatch;
                break;

            // Load a global identifier
            case VM_LOAD_GLOBAL :
                dst = vm_frame_get_global_identifier(frame, oparg1);
                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            // store SP+0 as a global identifier
            case VM_STORE_GLOBAL :
                // Refcount stays equal. So no inc/dec ref needed
                dst = vm_frame_stack_pop(frame);
                name = vm_frame_get_name(frame, oparg1);
                vm_frame_set_global_identifier(frame, name, dst);
                goto dispatch;
                break;

            // Remove global identifier
            case VM_DELETE_GLOBAL :
                dst = vm_frame_get_global_identifier(frame, oparg1);
                object_dec_ref(dst);
                name = vm_frame_get_name(frame, oparg1);
                vm_frame_set_global_identifier(frame, name, NULL);
                goto dispatch;
                break;

            // Load and push constant onto stack
            case VM_LOAD_CONST :
                dst = vm_frame_get_constant(frame, oparg1);
                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            // Store SP+0 into identifier (either local or global)
            case VM_STORE_ID :
                // Refcount stays equal. So no inc/dec ref needed
                dst = vm_frame_stack_pop(frame);
                register char *name = vm_frame_get_name(frame, oparg1);
                DEBUG_PRINT("Storing '%s' as '%s'\n", object_debug(dst), name);
                vm_frame_set_identifier(frame, name, dst);

                goto dispatch;
                break;

                // @TODO: If string(obj1) exists in local store it there, otherwise, store in global

            // Load and push identifier onto stack (either local or global)
            case VM_LOAD_ID :
                name = vm_frame_get_name(frame, oparg1);
                dst = vm_frame_get_identifier(frame, name);
                if (dst == NULL) {
                    reason = REASON_EXCEPTION;
                    thread_set_exception(Object_AttributeException, "Attribute not found");
                    goto block_end;
                    break;
                }
                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            //
            case VM_OPERATOR :
                right_obj = vm_frame_stack_pop(frame);
                object_dec_ref(right_obj);
                left_obj = vm_frame_stack_pop(frame);
                object_dec_ref(left_obj);

                if (left_obj->type != right_obj->type) {
                    fatal_error(1, "Types are not equal. Coersing needed, but not yet implemented\n");
                }
                dst = vm_object_operator(left_obj, oparg1, right_obj);
                if (! dst) {
                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            case VM_INPLACE_OPR :
                left_obj = vm_frame_stack_pop(frame);
                object_dec_ref(left_obj);
                right_obj = vm_frame_stack_pop(frame);
                object_dec_ref(right_obj);

                if (left_obj->type != right_obj->type) {
                    fatal_error(1, "Types are not equal. Coersing needed, but not yet implemented\n");
                }
                dst = vm_object_operator(left_obj, oparg1, right_obj);
                if (! dst) {
                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            // Unconditional relative jump forward
            case VM_JUMP_FORWARD :
                frame->ip += oparg1;
                goto dispatch;
                break;

            // Conditional jump on SP-0 is true
            case VM_JUMP_IF_TRUE :
                dst = vm_frame_stack_fetch_top(frame);
                if (! OBJECT_IS_BOOLEAN(dst)) {
                    // Cast to boolean
                    register t_object *bool_method = object_find_attribute(dst, "__boolean");
                    dst = vm_object_call(dst, bool_method, 0);
                }

                if (IS_BOOLEAN_TRUE(dst)) {
                    frame->ip += oparg1;
                }

                goto dispatch;
                break;

            // Conditional jump on SP-0 is false
            case VM_JUMP_IF_FALSE :
                dst = vm_frame_stack_fetch_top(frame);
                if (! OBJECT_IS_BOOLEAN(dst)) {
                    // Cast to boolean
                    register t_object *bool_method = object_find_attribute(dst, "__boolean");
                    dst = vm_object_call(dst, bool_method, 0);
                }

                if (IS_BOOLEAN_FALSE(dst)) {
                    frame->ip += oparg1;
                }
                goto dispatch;
                break;

            case VM_JUMP_IF_FIRST_TRUE :
                dst = vm_frame_stack_fetch_top(frame);
                if (! OBJECT_IS_BOOLEAN(dst)) {
                    // Cast to boolean
                    register t_object *bool_method = object_find_attribute(dst, "__boolean");
                    dst = vm_object_call(dst, bool_method, 0);
                }

                // @TODO: We assume that this opcode has at least 1 block!
                if (IS_BOOLEAN_TRUE(dst) && frame->blocks[frame->block_cnt-1].visited == 0) {
                    frame->ip += oparg1;
                }

                // We have visited this frame
                frame->blocks[frame->block_cnt-1].visited = 1;
                goto dispatch;
                break;

            case VM_JUMP_IF_FIRST_FALSE :
                dst = vm_frame_stack_fetch_top(frame);
                if (! OBJECT_IS_BOOLEAN(dst)) {
                    // Cast to boolean
                    register t_object *bool_method = object_find_attribute(dst, "__boolean");
                    dst = vm_object_call(dst, bool_method, 0);
                }

                // @TODO: We assume that this opcode has at least 1 block!
                if (IS_BOOLEAN_FALSE(dst) && frame->blocks[frame->block_cnt-1].visited == 0) {
                    frame->ip += oparg1;
                }

                // We have visited this frame
                frame->blocks[frame->block_cnt-1].visited = 1;
                goto dispatch;
                break;

            // Unconditional absolute jump
            case VM_JUMP_ABSOLUTE :
                frame->ip = oparg1;
                goto dispatch;
                break;

            // Duplicates the SP+0 a number of times
            case VM_DUP_TOPX :
                dst = vm_frame_stack_fetch_top(frame);
                for (int i=0; i!=oparg1; i++) {
                    object_inc_ref(dst);
                    vm_frame_stack_push(frame, dst);
                }
                goto dispatch;
                break;

            // Calls a callable from SP-0 with OP+0 args starting from SP-1.
            case VM_CALL :
                {
                    // Fetch methods to call
                    register t_callable_object *callable_obj = (t_callable_object *)vm_frame_stack_pop(frame);
                    register t_object *self_obj = callable_obj->binding;

                    // Create argument list inside a DLL
                    t_dll *arg_list = dll_init();

                    // Fetch varargs object (or NULL when no varargs are needed)
                    t_list_object *varargs = (t_list_object *)vm_frame_stack_pop(frame);

                    // Add items
                    for (int i=0; i!=oparg1; i++) {
                        dll_prepend(arg_list, vm_frame_stack_pop(frame));
                    }

                    if (varargs != NULL) {
                        // iterate hash (this is the correct order), and prepend values to the arg_list DLL
                        t_hash_iter iter;
                        ht_iter_init(&iter, varargs->ht);
                        while (ht_iter_valid(&iter)) {
                            t_object *obj = ht_iter_value(&iter);
                            dll_append(arg_list, obj);
                            ht_iter_next(&iter);
                        }
                    }

                    t_object *ret_obj = vm_object_call_args(self_obj, (t_object *)callable_obj, arg_list);
                    dll_free(arg_list);

                    if (ret_obj == NULL) {
                        // NULL returned means exception occured.
                        reason = REASON_EXCEPTION;
                        goto block_end;
                        break;
                    }

                    object_inc_ref(ret_obj);
                    vm_frame_stack_push(frame, ret_obj);
                }

                goto dispatch;
                break;

            // Import X as Y from Z
            case VM_IMPORT :
                {
                    // Fetch the module to import
                    register t_object *module_obj = vm_frame_stack_pop(frame);
                    object_dec_ref(module_obj);
                    register char *module_name = OBJ2STR(module_obj);

                    // Fetch class
                    register t_object *class_obj = vm_frame_stack_pop(frame);
                    object_dec_ref(class_obj);
                    register char *class_name = OBJ2STR(class_obj);

                    dst = vm_import(frame, module_name, class_name);
                    object_inc_ref(dst);
                    vm_frame_stack_push(frame, dst);
                }

                goto dispatch;
                break;


            case VM_SETUP_LOOP :
                vm_push_block_loop(frame, BLOCK_TYPE_LOOP, frame->sp, frame->ip + oparg1, 0);
                goto dispatch;
                break;
            case VM_SETUP_ELSE_LOOP :
                vm_push_block_loop(frame, BLOCK_TYPE_LOOP, frame->sp, frame->ip + oparg1, frame->ip + oparg2);
                goto dispatch;
                break;

            case VM_POP_BLOCK :
                vm_pop_block(frame);
                goto dispatch;
                break;

            case VM_CONTINUE_LOOP :
                ret = object_new(Object_Numerical, 1, oparg1);
                reason = REASON_CONTINUE;
                goto block_end;
                break;
            case VM_BREAK_LOOP :
                reason = REASON_BREAK;
                goto block_end;
                break;
            case VM_BREAKELSE_LOOP :
                reason = REASON_BREAKELSE;
                goto block_end;
                break;

            case VM_COMPARE_OP :
                left_obj = vm_frame_stack_pop(frame);
                right_obj = vm_frame_stack_pop(frame);

                // @TODO: EQ and NE can be checked here as well. Or could we "override" them anyway? Store them inside
                // the base class!

                // Exception compare is special case. We don't let classes handle that themselves, but we
                // need to do it here.
                if (oparg1 == COMPARISON_EX) {
                    if (object_instance_of(right_obj, left_obj->name)) {
                        vm_frame_stack_push(frame, Object_True);
                    } else {
                        vm_frame_stack_push(frame, Object_False);
                    }
                    goto dispatch;
                    break;
                }

                DEBUG_PRINT("Compare '%s (%d)' against '%s (%d)'\n", left_obj->name, left_obj->type, right_obj->name, right_obj->type);

                // Compare types do not match
                if (left_obj->type != right_obj->type && !(OBJECT_IS_NULL(left_obj) || OBJECT_IS_NULL(right_obj))) {

                    // Try an implicit cast if possible
                    DEBUG_PRINT("Explicit casting '%s' to '%s'\n", right_obj->name, left_obj->name);
                    t_object *cast_method = object_find_attribute(right_obj, left_obj->name);
                    if (! cast_method) {
                        reason = REASON_EXCEPTION;
                        thread_set_exception_printf(Object_TypeException, "Cannot compare '%s' against '%s'", left_obj->name, right_obj->name);
                        goto block_end;
                    }
                    right_obj = vm_object_call(right_obj, cast_method, 0);
                    if (! right_obj) {
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }
                }

                dst = vm_object_comparison(left_obj, oparg1, right_obj);
                if (! dst) {
                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

                object_dec_ref(left_obj);
                object_dec_ref(right_obj);

                object_inc_ref(dst);
                vm_frame_stack_push(frame, dst);
                goto dispatch;
                break;

            case VM_BUILD_ATTRIB :
                {
                    // pop access object
                    register t_object *access = vm_frame_stack_pop(frame);
                    object_dec_ref(access);

                    // pop visibility object
                    register t_object *visibility = vm_frame_stack_pop(frame);
                    object_dec_ref(visibility);

                    // pop value object
                    register t_object *value_obj = vm_frame_stack_pop(frame);
                    object_dec_ref(value_obj);

                    // Deal with attribute type specific values
                    register t_object *method_flags_obj = NULL;
                    register t_object *arg_list = NULL;

                    if (oparg1 == ATTRIB_TYPE_METHOD) {
                        // We have method flags and possible arguments
                        method_flags_obj = vm_frame_stack_pop(frame);
                        object_dec_ref(method_flags_obj);

                        // Generate hash object from arguments
                        arg_list = object_new(Object_Hash, 0);
                        for (int i=0; i!=oparg2; i++) {
                            t_method_arg *arg = smm_malloc(sizeof(t_method_arg));
                            arg->value = vm_frame_stack_pop(frame);
                            register t_object *name_obj = vm_frame_stack_pop(frame);
                            arg->typehint = (t_string_object *)vm_frame_stack_pop(frame);

                            ht_add(((t_hash_object *)arg_list)->ht, OBJ2STR(name_obj), arg);
                        }

                        // Value object is already a callable, but has no arguments (or binding). Here we add the arglist
                        // @TODO: this means we cannot re-use the same codeblock with different args (which makes sense). Make sure
                        // this works.
                        ((t_callable_object *)value_obj)->arguments = (t_hash_object *)arg_list;
                    }
                    if (oparg1 == ATTRIB_TYPE_CONSTANT) {
                        // Nothing additional to do for constants
                    }
                    if (oparg1 == ATTRIB_TYPE_PROPERTY) {
                        // Nothing additional to do for regular properties
                    }

                    // Create new attribute object
                    dst = object_new(Object_Attrib, 4, oparg1, OBJ2NUM(visibility), OBJ2NUM(access), value_obj);

                    // Push method object
                    object_inc_ref(dst);
                    vm_frame_stack_push(frame, dst);
                }
                goto dispatch;
                break;


            case VM_BUILD_INTERFACE :
            case VM_BUILD_CLASS :
                {
                    // Create a userland object, and fill it
                    register t_userland_object *interface_or_class = (t_userland_object *)smm_malloc(sizeof(t_userland_object));
                    memcpy(interface_or_class, Object_Userland, sizeof(t_userland_object));

                    interface_or_class->file_identifiers = frame->local_identifiers;

                    // pop class name
                    register t_object *name_obj = vm_frame_stack_pop(frame);
                    object_dec_ref(name_obj);
                    interface_or_class->name = smm_strdup(OBJ2STR(name_obj));

                    // pop flags
                    register t_object *flags = vm_frame_stack_pop(frame);
                    object_dec_ref(flags);
                    interface_or_class->flags = OBJ2NUM(flags);

                    // depending on the opcode, we are building a class or an interface
                    if (opcode == VM_BUILD_CLASS) {
                        interface_or_class->flags |= OBJECT_TYPE_CLASS;
                    } else {
                        interface_or_class->flags |= OBJECT_TYPE_INTERFACE;
                    }

                    // Pop the number of interfaces
                    interface_or_class->interfaces = dll_init();
                    t_object *interface_cnt_obj = vm_frame_stack_pop(frame);
                    long interface_cnt = OBJ2NUM(interface_cnt_obj);
                    DEBUG_PRINT("Number of interfaces we need to implement: %ld\n", interface_cnt);

                    // Fetch all interface objects
                    for (int i=0; i!=interface_cnt; i++) {
                        register t_object *interface_name_obj = vm_frame_stack_pop(frame);
                        DEBUG_PRINT("Implementing interface: %s\n", object_debug(interface_name_obj));

                        // Check if the interface actually exists
                        t_object *interface_obj = vm_frame_find_identifier(thread_get_current_frame(), OBJ2STR(interface_name_obj));
                        if (! interface_obj) {
                            reason = REASON_EXCEPTION;
                            thread_set_exception_printf(Object_TypeException, "Interface '%s' is not found", OBJ2STR(interface_name_obj));
                            goto block_end;
                        }
                        if (! OBJECT_TYPE_IS_INTERFACE(interface_obj)) {
                            reason = REASON_EXCEPTION;
                            thread_set_exception_printf(Object_TypeException, "Object '%s' is not an interface", OBJ2STR(interface_name_obj));
                            goto block_end;
                        }

                        dll_append(interface_or_class->interfaces, interface_obj);
                    }

                    // pop parent code object (as string)
                    register t_object *parent_class_obj = vm_frame_stack_pop(frame);
                    object_dec_ref(parent_class_obj);

                    // If no parent class has been given, use the Base class as parent
                    register t_object *parent_class;
                    if (OBJECT_IS_NULL(parent_class_obj)) {
                        parent_class = Object_Base;
                    } else {
                        // Find the object of this string
                        parent_class = vm_frame_get_identifier(frame, OBJ2STR((t_string_object *)parent_class_obj));
                    }
                    object_inc_ref(parent_class);
                    interface_or_class->parent = parent_class;


                    // Fetch all attributes
                    interface_or_class->attributes = ht_create();

                    // Iterate all attributes
                    for (int i=0; i!=oparg1; i++) {
                        register t_object *name = vm_frame_stack_pop(frame);
                        register t_attrib_object *attrib_obj = (t_attrib_object *)vm_frame_stack_pop(frame);

                        if (ATTRIB_IS_METHOD(attrib_obj)) {
                            // If we are a method, we will set the name.
                            t_callable_object *callable_obj = (t_callable_object *)attrib_obj->attribute;
                            callable_obj->name = OBJ2STR(name);
                            callable_obj->binding = (t_object *)interface_or_class;
                        }

                        // Add method attribute to class
                        ht_add(interface_or_class->attributes, OBJ2STR(name), attrib_obj);

                        DEBUG_PRINT("> Added attribute '%s' to class '%s'\n", object_debug((t_object *)attrib_obj), interface_or_class->name);
                    }

                    if (opcode == VM_BUILD_CLASS && ! object_check_interface_implementations((t_object *)interface_or_class)) {
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    object_inc_ref((t_object *)interface_or_class);
                    vm_frame_stack_push(frame, (t_object *)interface_or_class);
                }

                goto dispatch;
                break;

            case VM_RETURN :
                // Pop "ret" from the stack
                ret = vm_frame_stack_pop(frame);
                object_dec_ref(ret);

                reason = REASON_RETURN;
                goto block_end;
                break;

            case VM_SETUP_EXCEPT :
                vm_push_block_exception(frame, BLOCK_TYPE_EXCEPTION, frame->sp, frame->ip + oparg1, frame->ip + oparg2, frame->ip + oparg3);
                vm_frame_stack_push(frame, object_new(Object_Numerical, 1, REASON_FINALLY));

                goto dispatch;
                break;

            case VM_END_FINALLY :
                ret = vm_frame_stack_pop(frame);
                if (OBJECT_IS_NUMERICAL(ret)) {
                    reason = OBJ2NUM(ret);

                    if (reason == REASON_RETURN || reason == REASON_CONTINUE) {
                        ret = vm_frame_stack_pop(frame);
                    }
                    goto block_end;
                    break;
                } else if (OBJECT_IS_EXCEPTION(ret)) {
                    reason = REASON_RERAISE;
                    ret = NULL;
                    goto block_end;
                    break;

                } else {
                    // This should not happen (oreally?)
                    thread_set_exception(Object_SystemException, "Unknown value on the stack during finally cleanup (probably a saffire-bug)");
                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

        } // switch(opcode) {


block_end:

        // This loop will unwind the blockstack and act accordingly on each block (if needed)
        unwind_blocks(frame, &reason, ret);

        // Still not handled, break from this frame
        if (reason != REASON_NONE) {
            break;
        }

    } // for (;;)


    // Restore current frame
    thread_set_current_frame(old_current_frame);

    return ret;
}


/**
 * Unwind blocks. There are two types of blocks: LOOP and EXCEPTION. When this function is called
 * we will by default unwind everything down EXCEPT when certain conditions are met.
 *
 * This function can change the reason and the blocks in the frame. It will not change any return values, but it can
 * change the IP pointer as well (continue, break, finally, catch etc)
 */
t_vm_frameblock *unwind_blocks(t_vm_frame *frame, long *reason, t_object *ret) {
    t_vm_frameblock *block = vm_fetch_block(frame);

    DEBUG_PRINT("init unwind_blocks: [curblocks %d] (%d)\n", frame->block_cnt, *reason);

    // Unwind block as long as there is a reason to unwind
    while (*reason != REASON_NONE && frame->block_cnt > 0) {
        DEBUG_PRINT("  CUR block: %d\n", frame->block_cnt);
        DEBUG_PRINT("  Current reason: %d\n", *reason);
        DEBUG_PRINT("  Block Type: %d\n", block->type);

        block = vm_fetch_block(frame);
        DEBUG_PRINT("Checking frameblock %d. CBT: %d\n", frame->block_cnt, block->type);

        // Case 1: Continue inside a loop block
        if (*reason == REASON_CONTINUE && block->type == BLOCK_TYPE_LOOP) {
            DEBUG_PRINT("CASE 1\n");
            DEBUG_PRINT("\n*** Continuing loop at %08lX\n\n", OBJ2NUM(ret));

            // Continue to at the start of the block
            frame->ip = OBJ2NUM(ret);
            *reason = REASON_NONE;
            break;
        }


        // Case 2: Return inside try (or catch) block, but not inside finally block
        if (*reason == REASON_RETURN && block->type == BLOCK_TYPE_EXCEPTION) {
            DEBUG_PRINT("CASE 2: RETURN IN TRY, CATCH OR FINALLY\n");
            /* We push the return value and the reason (REASON_RETURN) onto the stack, since END_FINALLY will expect
             * this. We have no real way to know if a try block has a return statement inside, so SETUP_EXCEPT will
             * push a REASON_NONE and a dummy retval onto the stack as well. END_EXCEPTION will catch this because
             * when the popped reason is REASON_NONE, no return has been called in the try block. Ultimately this
             * doesn't really matter. Because END_FINALLY will unwind the block, which means the variable-stack will
             * match prior to the exception-block, so the dummy variables will be removed as well (IF they were
             * present */

            vm_frame_stack_push(frame, ret);
            vm_frame_stack_push(frame, object_new(Object_Numerical, 1, *reason));

            /* Instead of actually returning, continue with executing the finally block. END_FINALLY will deal with
             * the delayed return. */
            *reason = REASON_NONE;

            /* If we are BELOW the finally block, we ASSUME that we have to jump to the finally block. This could
             * be triggered from both the try or a catch block (both are handled the same, so this distinction is not
             * needed. However, when ABOVE, we ASSUME to be triggered from the finally block, and this we need to skip
             * until the end of the finally */
            DEBUG_PRINT("IP F : %02X %02X\n", frame->ip, block->handlers.exception.ip_finally);
            DEBUG_PRINT("IP C : %02X %02X\n", frame->ip, block->handlers.exception.ip_catch);
            DEBUG_PRINT("IP EF: %02X %02X\n", frame->ip, block->handlers.exception.ip_end_finally);

            if (frame->ip <= block->handlers.exception.ip_finally) {
                DEBUG_PRINT("RETTING into FINALLY\n");
                frame->ip = block->handlers.exception.ip_finally;
            } else if (frame->ip <= block->handlers.exception.ip_end_finally) {
                DEBUG_PRINT("RETTING out from FINALLY\n");
                frame->ip = block->handlers.exception.ip_end_finally;

                *reason = REASON_FINALLY;
            } else {
                DEBUG_PRINT("Not retting in anything\n");
                *reason = REASON_FINALLY;
            }
            break;
        }

        // Case 4: Exception raised inside try
        if (*reason == REASON_EXCEPTION && block->type == BLOCK_TYPE_EXCEPTION) {
            DEBUG_PRINT("CASE 4: EXCEPTION TRIGGERED (IN TRY BLOCK)\n");

            // Clean up any remaining items on the variable stack, but keep the last "REASON_FINALLY"
            while (frame->sp < block->sp - 1) {
                DEBUG_PRINT("Current SP: %d -> Needed SP: %d\n", frame->sp, block->sp);
                t_object *dst = vm_frame_stack_pop(frame);
                object_dec_ref(dst);
            }

            // We throw the current exception onto the stack. The catch-blocks will expect this.
            vm_frame_stack_push(frame, thread_get_exception());

            // Continue with handling the exception in the current vm frame.
            *reason = REASON_NONE;
            frame->ip = block->handlers.exception.ip_catch;

            break;
        }

        DEBUG_PRINT("unwind!\n");

        /*
         * All cases below here are pop the block.
         */

        /* Pop the block from the frame, but we still use it. As long as we don't push another block in
         * this function, this works ok. */
        block = vm_pop_block(frame);

        // Unwind the variable stack. This will remove all variables used in the current (unwound) block.
        while (frame->sp < block->sp) {
            DEBUG_PRINT("Current SP: %d -> Needed SP: %d\n", frame->sp, block->sp);
            t_object *dst = vm_frame_stack_pop(frame);
            object_dec_ref(dst);
        }


        // Case 3: Exception raised, but we are not an exception block. Try again with the next block.
        if (*reason == REASON_EXCEPTION && block->type != BLOCK_TYPE_EXCEPTION) {
            DEBUG_PRINT("CASE 3\n");
            continue;
        }

        if (*reason == REASON_FINALLY && block->type == BLOCK_TYPE_EXCEPTION) {
            *reason = REASON_NONE;
            break;
        }

        // Case 5: BreakElse inside a loop-block
        if (*reason == REASON_BREAKELSE && block->type == BLOCK_TYPE_LOOP) {
            DEBUG_PRINT("CASE 5\n");
            DEBUG_PRINT("\nBreaking loop to %08X\n\n", block->handlers.loop.ip_else);
            frame->ip = block->handlers.loop.ip_else;
            break;
        }

        // Case 6: Break inside a loop-block
        if (*reason == REASON_BREAK && block->type == BLOCK_TYPE_LOOP) {
            DEBUG_PRINT("CASE 6\n");
            DEBUG_PRINT("\nBreaking loop to %08X\n\n", block->handlers.loop.ip);
            frame->ip = block->handlers.loop.ip;
            *reason = REASON_NONE;
            break;
        }
    }

    // It might be possible that we unwind every block and still have a a reason different than REASON_NONE. This will
    // be handled by the parent frame.

    DEBUG_PRINT("fini unwind_blocks: [block_cnt: %d] %d\n", frame->block_cnt, *reason);
    return block;
}

/**
 *
 */
int vm_execute(t_vm_frame *frame) {
    // Setup bytecode into the frame (or not?)

    // Execute the frame
    t_object *result = _vm_execute(frame);

    // @TODO: remove me
    result = NULL;

    DEBUG_PRINT("============================ VM execution done ============================\n");

//    // Check if there was an uncaught exception (when result == NULL)
//    if (result == NULL) {
//        if (thread_exception_thrown()) {
//            // handle exception
//            object_internal_call("saffire", "uncaughtExceptionHandler", 1, thread_get_exception());
//            result = object_new(Object_Numerical, 1, 1);
//        } else {
//            // result was NULL, but no exception found, just threat like regular 0
//            result = object_new(Object_Numerical, 1, 0);
//        }
//    }
//
//    // Convert returned object to numerical, so we can use it as an error code
//    if (!OBJECT_IS_NUMERICAL(result)) {
//        // Cast to numericak
//        t_object *result_numerical = object_find_attribute(result, "__numerical");
//        result = vm_object_call(result, result_numerical, 0);
//    }
//    int ret_val = ((t_numerical_object *) result)->value;

//    vm_frame_destroy(frame);
    int ret_val = 1;
    return ret_val;
}


t_object *object_internal_call(const char *class, const char *method, int arg_count, ...) {
    // @TODO: This code is pretty much duplicated from LOAD_ATTR.

    // Find attribute from class
    t_object *class_obj = vm_frame_find_identifier(thread_get_current_frame(), (char *)class);
    t_object *attrib_obj = object_find_actual_attribute(class_obj, (char *)method);

    // Check visibility
    if (! vm_check_visibility(class_obj, class_obj, attrib_obj)) {
        object_raise_exception(Object_VisibilityException, "visibility error!");
        return NULL;
    }

    // Duplicate and bind class to attribute
    t_callable_object *callable_obj = (t_callable_object *)((t_attrib_object *)attrib_obj)->attribute;
    t_callable_object *new_copy = (t_callable_object *)smm_malloc(sizeof(t_callable_object));
    memcpy(new_copy, callable_obj, sizeof(t_callable_object));
    new_copy->binding = class_obj;

    //  Create arguments DLL
    va_list args;
    va_start(args, arg_count);
    t_dll *arg_list = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(args, t_object *);
        dll_append(arg_list, obj);
    }
    va_end(args);

    // Call method
    t_object *ret = vm_object_call_args(class_obj, (t_object *)new_copy, arg_list);

    dll_free(arg_list);
    return ret;
}


/**
 *
 */
void vm_populate_builtins(const char *name, void *data) {
    DEBUG_PRINT("Added object to builtins: %s\n", name);
    ht_add(builtin_identifiers, name, data);
}


/**
* Calls a method from specified object. Returns NULL when method is not found.
*/
t_object *vm_object_call(t_object *self, t_object *method_obj, int arg_count, ...) {
    // Create DLL with all arguments
    va_list args;
    va_start(args, arg_count);
    t_dll *arg_list = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(args, t_object *);
        dll_append(arg_list, obj);
    }
    va_end(args);

    t_object *ret_obj = vm_object_call_args(self, (t_object *)method_obj, arg_list);

    // Free dll
    dll_free(arg_list);

    return ret_obj;
}


/**
 *
 */
t_object *vm_object_call_args(t_object *self, t_object *callable, t_dll *arg_list) {
    t_callable_object *callable_obj = (t_callable_object *)callable;
    t_object *self_obj = self;
    t_object *dst;

    // If the object is a class, we can call it, ie instantiating it
    if (OBJECT_TYPE_IS_CLASS(callable_obj)) {
        // Do actual instantiation (pass nothing)
        t_object *new_obj = object_find_attribute((t_object *)callable_obj, "__new");
        self_obj = vm_object_call((t_object *)callable_obj, new_obj, 0);

        // We continue the function, but using the constructor as our callable
        callable_obj = (t_callable_object *)object_find_attribute(self_obj, "__ctor");
    }

    // Check if the object is actually a callable
    if (! OBJECT_IS_CALLABLE(callable_obj)) {
        thread_set_exception(Object_CallableException, "Object is not from callable instance");
        return NULL;
    }


    // Check if we can call the method
    if (CALLABLE_IS_TYPE_METHOD(callable_obj)) {
        // Check if we are bounded to a class or instantiation
        if (!self_obj) {
            thread_set_exception_printf(Object_CallableException, "Callable '%s' is not bound to any class or instantiation\n", callable_obj->name);
            return NULL;
        }

        // Make sure we are not calling a non-static method from a static context
        if (OBJECT_TYPE_IS_CLASS(self_obj) && ! CALLABLE_IS_STATIC(callable_obj)) {
            thread_set_exception_printf(Object_CallableException, "Cannot call dynamic method '%s' from a class\n", callable_obj->name);
        }
    }

    // Check if it's a native function.
    if (CALLABLE_IS_CODE_INTERNAL(callable_obj)) {
        // Internal function call
        dst = callable_obj->code.native_func(self_obj, arg_list);
    } else {
        char context[250];
        snprintf(context, 250, "%s.%s([%ld args])", self->name, callable_obj->name, arg_list->size);

        // Create a new execution frame
        t_vm_frame *cur_frame = thread_get_current_frame();
        t_vm_frame *new_frame = vm_frame_new(cur_frame, context, callable_obj->code.bytecode);

        if (OBJECT_IS_USER(self_obj)) {
            new_frame->file_identifiers = (t_hash_object *)((t_userland_object *)self_obj)->file_identifiers;
        }

        // Add references to parent and self
        ht_replace(new_frame->local_identifiers->ht, "self", self_obj);
        ht_replace(new_frame->local_identifiers->ht, "parent", self_obj->parent);

        // Parse calling arguments to see if they match our signatures
        if (! _parse_calling_arguments(new_frame, callable_obj, arg_list)) {
            // Exception thrown in the argument parsing
            return NULL;
        }

        // Execute frame, return the last object
        dst = _vm_execute(new_frame);

        // @TODO: Destroy frame
        //vm_frame_destroy(new_frame);
    }

    if (dst == NULL) {
        // exception occurred
    }

    return dst;
}


/**
 * This method is called when we need to call an operator method. Even though eventually
 * it is a normal method call to a _opr_* method, we go a different route so we can easily
 * do custom optimizations later on.
 */
t_object *vm_object_operator(t_object *obj1, int opr, t_object *obj2) {
    char *opr_method = objectOprMethods[opr];

    t_object *found_obj = (t_object *)object_find_attribute(obj1, opr_method);
    if (! found_obj) {
        thread_set_exception_printf(Object_CallException, "Cannot find method '%s' in class '%s'", opr_method, obj1->name);
        return NULL;
    }

    DEBUG_PRINT(">>> Calling operator %s(%d) on object %s\n", opr_method, opr, obj1->name);

    // Call the actual operator and return the result
    return vm_object_call(obj1, found_obj, 1, obj2);
}

/**
 * Calls an comparison function. Returns true or false
 */
t_object *vm_object_comparison(t_object *obj1, int cmp, t_object *obj2) {
    char *cmp_method = objectCmpMethods[cmp];

    t_object *found_obj = (t_object *)object_find_attribute(obj1, cmp_method);
    if (! found_obj) {
        thread_set_exception_printf(Object_CallException, "Cannot find method '%s' in class '%s'", cmp_method, obj1->name);
        return NULL;
    }

    DEBUG_PRINT(">>> Calling comparison %s(%d) on object %s\n", cmp_method, cmp, obj1->name);

    // Call the actual operator and return the result
    t_object *ret = vm_object_call(obj1, found_obj, 1, obj2);
    if (! ret) return ret;

    // Implicit conversion to boolean if needed
    if (! OBJECT_IS_BOOLEAN(ret)) {
        t_object *bool_method = object_find_attribute(ret, "__boolean");
        ret = vm_object_call(ret, bool_method, 0);
    }

    return ret;
}



