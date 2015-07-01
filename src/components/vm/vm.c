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
#include <saffire/compiler/bytecode.h>
#include <saffire/vm/vm.h>
#include <saffire/vm/stackframe.h>
#include <saffire/vm/context.h>
#include <saffire/vm/vm_opcodes.h>
#include <saffire/vm/block.h>
#include <saffire/vm/thread.h>
#include <saffire/vm/import.h>
#include <saffire/general/dll.h>
#include <saffire/general/smm.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/modules/module_api.h>
#include <saffire/debug.h>
#include <saffire/general/output.h>
#include <saffire/vm/import.h>
#include <saffire/gc/gc.h>
#include <saffire/debugger/dbgp/dbgp.h>

t_hash_table *builtin_identifiers_ht;       // Builtin identifiers - actual hash table
t_hash_object *builtin_identifiers;         // Builtin identifiers - hash object

// Flow termination reasons
#define REASON_NONE         0       // No return status. Just end the execution
#define REASON_RETURN       1       // Return statement given
#define REASON_CONTINUE     2       // Continue the current loop
#define REASON_BREAK        3       // Break the current loop
#define REASON_BREAKELSE    4       // Break the current loop, and jump to the else statement of the given loop
#define REASON_EXCEPTION    5       // Exception occurred
#define REASON_RERAISE      6       // Exception not handled. Reraised in finally clause
#define REASON_FINALLY      7       // No exception raised after finally


t_object *_vm_execute(t_vm_stackframe *frame);

// Defines the object operator and comparison methods.
extern char *objectOprMethods[];
extern char *objectCmpMethods[];

/**
 * This method is called when we need to call an operator method. Even though eventually
 * it is a normal method call to a _opr_* method, we go a different route so we can easily
 * do custom optimizations later on.
 */
static t_object *vm_object_operator(t_object *obj1, int opr, t_object *obj2) {
    char *opr_method = objectOprMethods[opr];

    t_attrib_object *found_obj = object_attrib_find(obj1, opr_method);
    if (! found_obj) {
        thread_create_exception_printf((t_exception_object *)Object_CallException, 1, "Cannot find method '%s' in class '%s'", opr_method, obj1->name);
        return NULL;
    }

    DEBUG_PRINT_CHAR(">>> Calling operator %s(%d) on object %s\n", opr_method, opr, obj1->name);

    // Call the actual operator and return the result
    return vm_object_call(obj1, found_obj, 1, obj2);
}


/**
 * Calls an comparison function. Returns true or false objects
 */
static t_object *vm_object_comparison(t_object *obj1, int cmp, t_object *obj2) {
    char *cmp_method = objectCmpMethods[cmp];

    t_attrib_object *found_obj = object_attrib_find(obj1, cmp_method);
    if (! found_obj) {
        thread_create_exception_printf((t_exception_object *)Object_CallException, 1, "Cannot find method '%s' in class '%s'", cmp_method, obj1->name);
        return NULL;
    }

    DEBUG_PRINT_CHAR(">>> Calling comparison %s(%d) on object %s\n", cmp_method, cmp, obj1->name);

    // Call the actual operator and return the result
    t_object *ret = vm_object_call(obj1, found_obj, 1, obj2);
    if (! ret) return ret;

    // Implicit conversion to boolean if needed
    if (! OBJECT_IS_BOOLEAN(ret)) {
        t_attrib_object *bool_method = object_attrib_find(ret, "__boolean");
        ret = vm_object_call(ret, bool_method, 0);
    }

    return ret;
}


/**
 * Parse calling arguments. It will iterate all arguments declarations needed for the
 * callable. The arguments are placed onto the frame stack.
 *
 * Returns 1 in success, 0 on failure/exception is thrown
 */
static int _parse_calling_arguments(t_vm_stackframe *frame, t_callable_object *callable, t_dll *arg_list) {
    t_hash_table *ht = callable->data.arguments;
    t_dll_element *e = DLL_HEAD(arg_list);

#ifdef __DEBUG
    ht_debug_keys(ht);
#endif

    int need_count = ht->element_count;
    int given_count = arg_list->size;

    // When set to null, no varargs are wanted
    t_list_object *vararg_obj = NULL;

    int cur_arg = 0;

    t_hash_iter iter;
    ht_iter_init(&iter, ht);
    while (ht_iter_valid(&iter)) {
        cur_arg++;

        char *name = ht_iter_key_str(&iter);
        t_method_arg *arg = ht_iter_value(&iter);

        int is_vararg =0 ;
        if (arg->typehint->type != objectTypeNull && ! string_strcmp0(arg->typehint->data.value, "...")) {
            is_vararg = 1;
        }

        // Preset object to default value if a default value was found.
        t_object *obj = (arg->value->type == objectTypeNull) ? NULL : arg->value;

        // If we have values on the calling arg list, use the next value, overriding any default values set.
        if (given_count) {
            obj = e ? e->data.p : NULL;
            given_count--;
        }

        // No more arguments to pass found, so obj MUST be of a value, otherwise caller didn't specify enough arguments.
        if (obj == NULL && ! is_vararg) {
            object_raise_exception(Object_ArgumentException, 1, "Not enough arguments passed, and no default values found");
            return 0;
        }

        if (arg->typehint->type != objectTypeNull) {
            // Check typehint / varargs


            if (is_vararg) {
                // the '...' typehint found.
                vararg_obj = (t_list_object *)object_alloc_instance(Object_List, 0);

                // Add first argument
                if (obj) {
                    ht_add_num(vararg_obj->data.ht, vararg_obj->data.ht->element_count, obj);
                }

                // Make sure we add our List[] to the local_identifiers below
                obj = (t_object *)vararg_obj;
            } else {
                char *instance = string_to_char(OBJ2STR(arg->typehint));
                if (! object_instance_of(obj, instance)) {
                    smm_free(instance);

                    // classname does not match the typehint

                    // @TODO: we need to check if object as a parent or interface that matches!
                    object_raise_exception(Object_ArgumentException, 1, "Typehinting for argument %d does not match. Wanted '%s' but found '%s'\n", cur_arg, OBJ2STR(arg->typehint), obj->name);
                    return 0;
                }

                smm_free(instance);
            }
        }

        // Everything is ok, add the new value onto the local identifiers
        ht_add_str(frame->local_identifiers->data.ht, name, obj);
        object_inc_ref(obj);

        need_count--;

        // Next needed element
        ht_iter_next(&iter);
        if (e) e = DLL_NEXT(e);
    }


    // If there are more arguments passed, check if we can feed them to the vararg, if present
    if (given_count > 0) {
        if (vararg_obj == NULL) {
            object_raise_exception(Object_ArgumentException, 1, "No variable argument found, and too many arguments passed");
            return 0;
        }

        // Just add arguments to vararg list. No need to do any typehint checks here.
        while (e) {
            ht_add_num(vararg_obj->data.ht, vararg_obj->data.ht->element_count, e->data.p);
            e = DLL_NEXT(e);
        }
    }

    // All ok
    return 1;
}

/**
 * Creates a userland object, based on a parent class, with interfaces, attributes etc.
 */
static t_object *vm_create_userland_object(t_vm_stackframe *frame, char *name, int flags, t_dll *interfaces, t_object *parent_class, t_hash_table *attributes) {
    // Allocate a new user object (doesn't contain any attributes)
    t_object *user_obj = object_alloc_class(parent_class, 0);

    DEBUG_PRINT_CHAR("User object created %08X based on %s\n", user_obj, parent_class->name);

    // Set the frame in which this frame is created
    user_obj->frame = frame;

    // Set name
    smm_free(user_obj->name);
    user_obj->name = string_strdup0(name);

    // Set to user object
    user_obj->type = objectTypeUser;

    // Set flags
    user_obj->flags |= flags;

    // Set interfaces
    user_obj->interfaces = interfaces;
    t_dll_element *interface = DLL_HEAD(user_obj->interfaces);
    while (interface) {
        object_inc_ref((t_object *)interface->data.p);
        interface = DLL_NEXT(interface);
    }

    // Set parent class
    user_obj->parent = parent_class;

    // Set attributes. Since we have allocated a parent CLASS, the attributes in the user_obj are not duplicated.
    // Therefor we can safely overwrite them with our own attributes.
    user_obj->attributes = attributes;

    return user_obj;
}

/**
 * Call a callable with arguments
 */
static t_object *_object_call_callable_with_args(t_object *self_obj, t_vm_stackframe *scope_frame, char *name, t_callable_object *callable_obj, t_dll *arg_list) {
    t_object *ret;

    // Check if the object is actually a callable
    if (! OBJECT_IS_CALLABLE(callable_obj)) {
        thread_create_exception((t_exception_object *)Object_CallableException, 1, "Object is not from callable instance");
        return NULL;
    }


    // Call native code
    if (CALLABLE_IS_CODE_INTERNAL(callable_obj)) {
        // @TODO: should internal code not have a frame as well?
        // Internal function call
        ret = callable_obj->data.code.internal.native_func(self_obj, arg_list);
        object_inc_ref(ret);
        return ret;
    }


    // External code

    // Create context name
    char context[1250];
    char args[1000];
    strcpy(args, "");
    t_dll_element *e = DLL_HEAD(arg_list);
    while (e) {
#ifdef __DEBUG
        strcat(args, object_debug(e->data.p));
#else
        strcat(args, "an object");
#endif
        e = DLL_NEXT(e);
        if (e) strcat(args, ", ");
    }
    snprintf(context, 1249, "%s.%s([%ld args: %s])", self_obj ? self_obj->name : "<anonymous>", callable_obj->name, arg_list->size, args);

    // Create a new execution frame
    t_vm_stackframe *child_frame = vm_stackframe_new(scope_frame, callable_obj->data.code.external.codeblock);
    child_frame->trace_class = self_obj ? string_strdup0(self_obj->name) : string_strdup0("<anonymous>");
    child_frame->trace_method = string_strdup0(name);

    // Create self inside the new frame
    t_object *old_self_obj = ht_replace_str(child_frame->local_identifiers->data.ht, "self", self_obj);
    if (old_self_obj) object_release(old_self_obj);
    object_inc_ref(self_obj);

    // Parse calling arguments to see if they match our signatures
    if (! _parse_calling_arguments(child_frame, callable_obj, arg_list)) {
        vm_stackframe_destroy(child_frame);

        // Exception thrown in the argument parsing
        return NULL;
    }

    // Execute frame, return the last object
    ret = _vm_execute(child_frame);

    // Destroy frame
    vm_stackframe_destroy(child_frame);

    if (ret == NULL) {
        // exception occurred (but, do we care?)
    }

    return ret;
}

/**
 * Returns a few values:
 *   - 1 called statically from an instance (could trigger error later on)
 *   - 1 called statically from a class (ok)
 *   - 0 called dynamically from an instance (not ok)
 *   - 1 called statically from a class (not ok)
 *
 */
static int _check_attribute_for_static_call(t_object *self, t_attrib_object *attrib_obj) {
    // Calls to constants are ok from both instance and class
    if (ATTRIB_IS_CONSTANT(attrib_obj)) {
        return 1;
    }

    if (! OBJECT_TYPE_IS_CLASS(self)) {
        // calls from an instance are ok (@TODO: Really, should static calls from dynamic be not ok)
        return 1;
    }

    // class, only ok when method is static
    return ATTRIB_METHOD_IS_STATIC(attrib_obj) ? 1 : 0;
}

/**
 * Checks visibility, returns 0 when not allowed, 1 when allowed.
 *
 * Binding = the object that actually defined the attribute.  (class foo { public property a = 1; })
 * Instance = the instance from the bounded object. (c = foo();  io.print(c.a);
 * Attribute = the attribute we are checking
 *
 * class foo {
 *    public property a = 1;
 * }
 * bar = foo();
 * io.print(bar.a);  // binding = foo,  instance = bar,  attribute = a
 * io.print(foo.a);  // binding = foo,  instance = foo,  attribute = a
 *
 *   1) if attribute == public, we always allow
 *   2) if attribute == protected, we allow from same class or when the class extends this class
 *   3) if attribute == private, we only allow from the same class
 */
static int _check_attrib_visibility(t_object *self, t_attrib_object *attrib) {
//    if (attrib->bound_class == NULL) {
//        int i = 0;
//    }

    // Not bound, so always ok
    if (! attrib->data.bound_instance) return 1;

    // Public attributes are always ok
    if (ATTRIB_IS_PUBLIC(attrib)) return 1;

    // Private visibility is allowed when we are inside the SAME class.
    if (ATTRIB_IS_PRIVATE(attrib) && attrib->data.bound_instance->class == self) return 1;

    if (ATTRIB_IS_PROTECTED(attrib)) {
        // Iterate self down all its parent, to see if one matches "attrib". If so, the protected visibility is ok.
        t_object *parent_binding = self;
        while (parent_binding) {
            if (parent_binding->class == attrib->data.bound_class || parent_binding == attrib->data.bound_class) return 1;
            parent_binding = parent_binding->parent;
        }
    }

    // Attribute is not found (@TODO: is this even possible?)
    return 0;
}


/**
 * Check an attribute and if ok, chck
 */
static t_object *_object_call_attrib_with_args(t_object *self, t_attrib_object *attrib_obj, t_dll *arg_list) {
    // Call on the frame stored in the object. If none is found, use the current frame as the base frame
    t_vm_stackframe *frame = attrib_obj->frame ? attrib_obj->frame : thread_get_current_frame();

    return _object_call_callable_with_args(self, frame, attrib_obj->data.bound_name, (t_callable_object *)attrib_obj->data.attribute, arg_list);
}


#define MAX_VEC 30

static t_object *_do_regex_match(t_regex_object *regex_obj, t_string_object *str_obj) {
    int subStrVec[MAX_VEC];
    int ret;


    ret = pcre_exec(regex_obj->data.regex, NULL /* no study yet */,
        STROBJ2CHAR0(str_obj), STROBJ2CHAR0LEN(str_obj),
        0,  /* start */
        0,  /* options */
        subStrVec, MAX_VEC);

    if (ret < -1) {
        // Error occurred
        thread_create_exception_printf((t_exception_object *)Object_CallException, 1, "Error during regex: %d", ret);
        return NULL;
    }

    if (ret == -1) {
        RETURN_FALSE;
    }

    // 0 or higher are ok
    RETURN_TRUE;
}


int debug = 0;
t_debuginfo *debug_info;


/**
 *
 */
void vm_init(SaffireParser *sp, int runmode) {
    // Set run mode (repl, cli, fastcgi)
    vm_runmode = runmode;

    // Setup the initial thread
    t_thread *thread = thread_new();
    current_thread = thread;

    gc_init();

    // Initialize hash where everybody can add their builtins to. Since object_hash does not exist yet,
    // we must use a generic hash for this. We will "convert" this to an hash-hobject later
    builtin_identifiers_ht = ht_create();

    object_init();
    module_init();

    vm_namespace_cache_init();

    // Convert our builtin identifiers to an actual hash object
    builtin_identifiers = (t_hash_object *)object_alloc_instance(Object_Hash, 1, builtin_identifiers_ht);
    object_inc_ref((t_object *)builtin_identifiers);

    // Initialize debugging if needed
    if ((runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        debug_info = dbgp_init();
    }
}


extern t_hash_table *refcount_objects;

void vm_fini(void) {
    // Initialize debugging if needed
    if ((vm_runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        dbgp_fini(debug_info);
    }

    // Free all namespaces
    vm_namespace_cache_fini();


    // Decrease builtin reference count. Should be 0 now, and will cleanup the hash used inside
#ifdef __DEBUG
#if __DEBUG_FREE_OBJECT
    DEBUG_PRINT_CHAR("\n\n\n--- CURRENT builtins\n");
    ht_debug(builtin_identifiers->data.ht);
#endif
#endif

    thread_free(current_thread);

    // We must release our builtins before we release our objects.
    t_hash_iter iter;
    ht_iter_init(&iter, builtin_identifiers->data.ht);
    while (ht_iter_valid(&iter)) {
        t_object *val = ht_iter_value(&iter);
#if __DEBUG_FREE_OBJECT
        DEBUG_PRINT_CHAR("\n\n\nBUILTIN DECREASE reference: %08X from %d %s\n", (intptr_t)val, val->ref_count, object_debug(val));
#endif
        object_release(val);
        ht_iter_next(&iter);
    }
    object_release((t_object *)builtin_identifiers);

    module_fini();
    object_fini();

#ifdef __DEBUG
#if __DEBUG_FREE_OBJECT
    // We really can't show anything here, since objects should have been gone now. Expect failures
    DEBUG_PRINT_CHAR("At object_fini(), we still have %ld objects left on the stack\n", refcount_objects->element_count);
    ht_iter_init(&iter, refcount_objects);
    while (ht_iter_valid(&iter)) {
        t_hash_key *key = ht_iter_key(&iter);
        t_object *addr = (t_object *)key->val.p;
        long refcount = (long) ht_iter_value(&iter);
        if (refcount != 0) {
#if __DEBUG_REFCOUNT
            DEBUG_PRINT_CHAR("REFCOUNT %08X => %ld %s\n", (intptr_t)addr, refcount, object_debug((t_object *)addr));
#endif
        }
        ht_iter_next(&iter);
    }
    ht_destroy(refcount_objects);
#endif
#endif


    gc_fini();
}

int getlineno(t_vm_stackframe *frame) {
    if (frame->lineno_upperbound != 0 && frame->lineno_lowerbound <= frame->ip && frame->ip < frame->lineno_upperbound) {
        return frame->lineno_current_line;
    }

    int delta_lino = 0;
    int delta_line = 0;

    // @TODO: Check if lino_offset doesn't go out of bounds
    if (frame->lineno_current_lino_offset >= frame->codeblock->bytecode->lino_length) {
        return frame->lineno_current_line;
    }

    int i;
    do {
        i = (frame->codeblock->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_line += i;
    } while (i > 127);
    do {
        i = (frame->codeblock->bytecode->lino[frame->lineno_current_lino_offset++] & 127);
        delta_lino += i;
    } while (i > 127);

    frame->lineno_lowerbound = frame->lineno_upperbound;
    frame->lineno_upperbound += delta_lino;
    frame->lineno_current_line += delta_line;

    return frame->lineno_current_line;
}


t_vm_frameblock *unwind_blocks(t_vm_stackframe *frame, long *reason, t_object *ret);

/**
 *
 */
t_object *_vm_execute(t_vm_stackframe *frame) {
    t_object *obj1, *obj2, *obj3, *obj4;
    t_object *left_obj, *right_obj;
    t_attrib_object *attr_obj;
    unsigned int opcode, oparg1, oparg2, oparg3;
    long reason = REASON_NONE;
    t_object *dst;
    char *s;


#ifdef __DEBUG
    if (frame->local_identifiers) print_debug_table(frame->local_identifiers->data.ht, "Locals");
    if (frame->global_identifiers) print_debug_table(frame->global_identifiers->data.ht, "Globals");
#endif


#ifdef __DEBUG
    DEBUG_PRINT_CHAR(ANSI_BRIGHTRED "------------ NEW FRAME ------------\n" ANSI_RESET);
    t_vm_stackframe *tb_frame = frame;
    int tb_depth = 0;
    while (tb_frame) {
        DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "#%d "
                ANSI_BRIGHTYELLOW "%s:%d "
                ANSI_BRIGHTGREEN "%s.%s"
                ANSI_BRIGHTGREEN "(<args>)"
                ANSI_RESET "\n",
                tb_depth,
                tb_frame->codeblock->context->file.full ? tb_frame->codeblock->context->file.full : "<none>",
                getlineno(tb_frame),
                tb_frame->codeblock->context->module.full ? tb_frame->codeblock->context->module.full : "",
                tb_frame->trace_method ? tb_frame->trace_method : ""
            );
        tb_frame = tb_frame->parent;
        tb_depth++;
    }
    DEBUG_PRINT_CHAR(ANSI_BRIGHTRED "-----------------------------------\n" ANSI_RESET);
#endif

    // Set the correct current frame
    t_vm_stackframe *parent_frame = thread_get_current_frame();
    thread_set_current_frame(frame);

    // Default return value;
    t_object *ret = NULL;

    for (;;) {


        // Room for some other stuff
dispatch:
        // Increase number of executions done
        frame->executions++;


#ifdef __DEBUG
    #if __DEBUG_VM_OPCODES
        int ln = getlineno(frame);
        unsigned long cip = frame->ip;
        //vm_frame_stack_debug(frame);
    #endif
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
    #if __DEBUG_VM_OPCODES
        if ((opcode & 0xE0) == 0xE0) {
            DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%-20s (0x%02X, 0x%02X, 0x%02X)     "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1, oparg2, oparg3,
                        frame->codeblock->context->file.full,
                        ln
                    );
            } else if ((opcode & 0xC0) == 0xC0) {
            DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%-20s (0x%02X, 0x%02X)           "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1, oparg2,
                        frame->codeblock->context->file.full,
                        ln
                    );
        } else if ((opcode & 0x80) == 0x80) {
            DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%-20s (0x%02X)                 "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        oparg1,
                        frame->codeblock->context->file.full,
                        ln
                    );
        } else {
            DEBUG_PRINT_CHAR(ANSI_BRIGHTBLUE "%08lX "
                        ANSI_BRIGHTGREEN "%-20s                        "
                        ANSI_BRIGHTYELLOW "[%s:%d] "
                        "\n" ANSI_RESET,
                        cip,
                        vm_code_names[vm_codes_offset[opcode]],
                        frame->codeblock->context->file.full,
                        ln
                    );
        }
    #endif
#endif





        if (opcode == VM_STOP) break;
        if (opcode == VM_RESERVED) {
            fatal_error(1, "VM: Reached reserved (0xFF) opcode. Halting.\n");       /* LCOV_EXCL_LINE */
        }


        switch (opcode) {
            // Removes SP-0
            case VM_POP_TOP :
                obj1 = vm_frame_stack_pop(frame);
                object_release(obj1);
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
                vm_frame_stack_push(frame, obj1);
                object_inc_ref(obj1);
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
                    // The object to load the attribute from.
                    t_object *self_obj = vm_frame_stack_pop(frame);

                    // Name of attribute to load
                    t_object *name_obj = vm_frame_get_constant(frame, oparg1);
                    char *name = string_to_char(OBJ2STR(name_obj));

                    // Scope of the loading (start from self. or parent.)
                    int scope = oparg2;

                    DEBUG_PRINT_CHAR("Loading attribute: '%s' from '%s' (scope: %s)\n", name, self_obj->name, scope == OBJECT_SCOPE_SELF ? "self" : "parent");

                    // The object where we start looking for attributes (either self or parent)
                    t_object *offset_obj = self_obj;

                    // If we need the parent scope (parent.whatever), just move directly to the parent class before looking
                    if (scope == OBJECT_SCOPE_PARENT) {
                        if (self_obj->parent == NULL) {
                            object_release(self_obj);

                            // @TODO: We should throw an exception, as we don't have a parent class. Can only happen
                            // when we are inside the base-class, and we do: parent.whatever
                            return NULL;
                        }
                        // We should start in parent object
                        offset_obj = self_obj->parent;
                    }

                    t_attrib_object *attrib_obj = object_attrib_find(offset_obj, name);
                    if (attrib_obj == NULL) {
                        object_release(self_obj);

                        reason = REASON_EXCEPTION;
                        thread_create_exception_printf((t_exception_object *)Object_AttributeException, 1, "Attribute '%s' in class '%s' not found", name, self_obj->name);
                        smm_free(name);
                        goto block_end;
                        break;
                    }

                    // Make sure we are not loading a non-static attribute from a static context
                    if (! _check_attribute_for_static_call(self_obj, attrib_obj)) {
                        object_release(self_obj);

                        thread_create_exception_printf((t_exception_object *)Object_CallableException, 1, "Cannot call dynamic method '%s' from class '%s'\n", attrib_obj->data.bound_name, self_obj->name);
                        smm_free(name);
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Check visibility of attribute
                    if (! _check_attrib_visibility(self_obj, attrib_obj)) {
                        object_release(self_obj);

                        thread_create_exception_printf((t_exception_object *)Object_VisibilityException, 1, "Visibility does not allow to fetch attribute '%s'\n", OBJ2STR(name));
                        smm_free(name);
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // We don't actually use the original attribute, but a duplicated one. Here we add our reference to the
                    // current object so we can do correct calls to the attributes method found in parent classes with the correct "self".
                    attrib_obj = object_attrib_duplicate(attrib_obj, self_obj);
                    DEBUG_PRINT_CHAR("Loaded attribute: %s.%s\n", self_obj->name, attrib_obj->data.bound_name);

                    /* increase ref count to the bound_instance, and make sure we release it on attribute destroy. This is needed to make sure
                     * that f.foo().bar().baz() works correctly as it might release objects prematurely. We don't ALWAYS increase refcounts because
                     * it would mean that every instance have like 30-40 refcounts by just their attributes */
                    attrib_obj->data.bound_instance_decref = 1;
                    object_inc_ref(attrib_obj->data.bound_instance);

                    vm_frame_stack_push(frame, (t_object *)attrib_obj);
                    object_inc_ref((t_object *)attrib_obj);

                    object_release(self_obj);
                    smm_free(name);
                }
                goto dispatch;
                break;

            // Store an attribute into an object
            case VM_STORE_ATTRIB :
                {
                    t_object *name_obj = vm_frame_get_constant(frame, oparg1);
                    t_object *search_obj = vm_frame_stack_pop(frame);

#ifdef __DEBUG
                    ht_debug(search_obj->attributes);
#endif

                    s = string_to_char(OBJ2STR(name_obj));
                    t_attrib_object *attrib_obj = object_attrib_find(search_obj, s);
                    smm_free(s);

                    if (attrib_obj && ATTRIB_IS_READONLY(attrib_obj)) {
                        object_release(search_obj);

                        thread_create_exception_printf((t_exception_object *)Object_VisibilityException, 1, "Cannot write to readonly attribute '%s'\n", OBJ2STR(name_obj));
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }
                    if (attrib_obj && ! _check_attrib_visibility(search_obj, attrib_obj)) {
                        object_release(search_obj);

                        thread_create_exception_printf((t_exception_object *)Object_VisibilityException, 1, "Visibility does not allow to access attribute '%s'\n", string_to_char(OBJ2STR(name_obj)));
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // @TODO: Not everything is a property by default. Check value to make sure it's a property or a method

                    t_object *value = vm_frame_stack_pop(frame);
                    if (attrib_obj) {
                        // Decrease reference of original value
                        if (attrib_obj->data.attribute) {
                            object_release(attrib_obj->data.attribute);
                        }

                        // Set to new value
                        attrib_obj->data.attribute = value;
                        object_inc_ref(value);

                    } else {
                        // if we don't have a attrib_obj, we just add a new attribute to the object (RW/PUBLIC)
                        s = string_to_char(OBJ2STR(name_obj));
                        object_add_property(search_obj, s, ATTRIB_TYPE_PROPERTY | ATTRIB_ACCESS_RW | ATTRIB_VISIBILITY_PUBLIC, value);
                        smm_free(s);
                    }

                    object_release(value);
                    object_release(search_obj);
                }
                goto dispatch;
                break;


//            // Load a global identifier
//            case VM_LOAD_GLOBAL :
//                dst = vm_frame_get_global_identifier(frame, oparg1);
//                vm_frame_stack_push(frame, dst);
//                goto dispatch;
//                break;
//
//            // store SP+0 as a global identifier
//            case VM_STORE_GLOBAL :
//                // Refcount stays equal. So no inc/dec ref needed
//                dst = vm_frame_stack_pop(frame);
//                name = vm_frame_get_name(frame, oparg1);
//                vm_frame_set_global_identifier(frame, name, dst);
//                goto dispatch;
//                break;
//
//            // Remove global identifier
//            case VM_DELETE_GLOBAL :
//                dst = vm_frame_get_global_identifier(frame, oparg1);
//                name = vm_frame_get_name(frame, oparg1);
//                vm_frame_set_global_identifier(frame, name, NULL);
//                goto dispatch;
//                break;

            // Load and push constant onto stack
            case VM_LOAD_CONST :
                dst = vm_frame_get_constant(frame, oparg1);
                vm_frame_stack_push(frame, dst);
                object_inc_ref(dst);
                goto dispatch;
                break;

            // Store SP+0 into identifier
            case VM_STORE_ID :
                dst = vm_frame_stack_pop(frame);
                s = vm_frame_get_name(frame, oparg1);
                vm_frame_set_local_identifier(frame, s, dst);

                object_release(dst);
                goto dispatch;
                break;

            // Load and push identifier onto stack (either local or global)
            case VM_LOAD_ID :
                s = vm_frame_get_name(frame, oparg1);

                dst = vm_frame_find_identifier(frame, s);
                if (dst == NULL) {
                    reason = REASON_EXCEPTION;
                    thread_create_exception_printf((t_exception_object *)Object_AttributeException, 1, "Identifier '%s' is not found", s, dst);
                    goto block_end;
                    break;
                }

                vm_frame_stack_push(frame, dst);
                object_inc_ref(dst);
                goto dispatch;
                break;

            //
            case VM_OPERATOR :
                right_obj = vm_frame_stack_pop(frame);
                left_obj = vm_frame_stack_pop(frame);

                if (left_obj->type != right_obj->type) {
                    fatal_error(1, "Types are not equal. Coersing needed, but not yet implemented\n");      /* LCOV_EXCL_LINE */
                }
                dst = vm_object_operator(left_obj, oparg1, right_obj);
                if (! dst) {
                    object_release(right_obj);
                    object_release(left_obj);

                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

                object_release(right_obj);
                object_release(left_obj);

                vm_frame_stack_push(frame, dst);
                object_inc_ref(dst);
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
                    t_attrib_object *bool_method = object_attrib_find(dst, "__boolean");
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
                    t_attrib_object *bool_method = object_attrib_find(dst, "__boolean");
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
                    t_attrib_object *bool_method = object_attrib_find(dst, "__boolean");
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
                    t_attrib_object *bool_method = object_attrib_find(dst, "__boolean");
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
                    vm_frame_stack_push(frame, dst);
                    object_inc_ref(dst);
                }
                goto dispatch;
                break;

            // Calls an callable attribute from SP-0 with OP+0 args starting from SP-1
            case VM_CALL :
                {
                    // Fetch methods to call
                    obj1 = vm_frame_stack_pop_attrib(frame);
                    if (
                         ! (OBJECT_IS_ATTRIBUTE(obj1) && ATTRIB_IS_METHOD(obj1)) &&
                         ! (OBJECT_TYPE_IS_CLASS(obj1))
                       ) {
                        object_release(obj1);

                        reason = REASON_EXCEPTION;
                        thread_create_exception_printf((t_exception_object *)Object_AttributeException, 1, "'%s' is must be a class or callable", OBJ2STR(obj1));
                        goto block_end;
                        break;
                    }

                    t_object *self;

                    // Check if we are a calling a class, if so, we are actually instantiating it
                    if (OBJECT_TYPE_IS_CLASS(obj1)) {
                        self = object_alloc_instance(obj1, 0);
                        object_inc_ref(self);

                        object_release(obj1);

                        // We continue the function, but using the constructor as our attribute
                        obj1 = (t_object *)object_attrib_find(self, "__ctor");

                        // we have to duplicate the object, as we will free this later on
                        obj1 = (t_object *)object_attrib_duplicate((t_attrib_object *)obj1, self);
                        object_inc_ref(obj1);
                    } else {
                        // Otherwise, we are just calling an attribute from an instance / class
                        self = ((t_attrib_object *)obj1)->data.bound_instance;
                    }

                    // Create argument list inside a DLL
                    t_dll *arg_list = dll_init();

                    // Fetch varargs object (or null_object when no varargs are needed)
                    t_list_object *varargs = (t_list_object *)vm_frame_stack_pop(frame);

                    // Add items
                    for (int i=0; i!=oparg1; i++) {
                        // We pop arguments, but we add it to a dll, don't decrease refcount, but we must do so
                        // when we finish with our dll
                        dll_prepend(arg_list, vm_frame_stack_pop(frame));
                    }

                    if (! OBJECT_IS_NULL(varargs)) {
                        // iterate hash (this is the correct order), and prepend values to the arg_list DLL
                        t_hash_iter iter;
                        ht_iter_init(&iter, varargs->data.ht);
                        while (ht_iter_valid(&iter)) {
                            t_object *obj = ht_iter_value(&iter);
                            dll_append(arg_list, obj);
                            object_inc_ref(obj);
                            ht_iter_next(&iter);
                        }
                    }

                    t_object *ret_obj = _object_call_attrib_with_args(self, (t_attrib_object *)obj1, arg_list);

                    // Release (duplicated) attribute
                    object_release(obj1);

                    // Decrefs our arguments here
                    t_dll_element *e = DLL_HEAD(arg_list);
                    while (e) {
                        object_release(e->data.p);
                        e = DLL_NEXT(e);
                    }
                    dll_free(arg_list);

                    object_release((t_object *)varargs);


                    if (ret_obj == NULL) {
                        // NULL returned means exception occurred.
                        reason = REASON_EXCEPTION;
                        goto block_end;
                        break;
                    }

                    vm_frame_stack_push(frame, ret_obj);
                    object_inc_ref(ret_obj);

                    // We're done with our ret_obj here..
                    object_release(ret_obj);
                }

                goto dispatch;
                break;

            // Import X as Y from Z
            case VM_IMPORT :
                {
                    // Fetch alias
                    t_object *alias_obj = vm_frame_stack_pop(frame);
                    char *alias_name = string_to_char(OBJ2STR(alias_obj));
                    printf("Import alias: %s\n", alias_name);

                    // Fetch the module to import
                    t_object *module_obj = vm_frame_stack_pop(frame);
                    char *module_name = string_to_char(OBJ2STR(module_obj));
                    printf("Import module: %s\n", module_name);

                    // Fetch class
                    t_object *class_obj = vm_frame_stack_pop(frame);
                    char *class_name = string_to_char(OBJ2STR(class_obj));
//                    char *class_name = vm_context_get_classname_from_fqcn(tmp);
                    printf("Import class: %s\n", class_name);
                    //smm_free(tmp);

                    char *target_name = vm_context_create_fqcn_from_name(module_name, class_name);

                    smm_free(module_name);
                    smm_free(class_name);

                    printf("Total: Import alias: %s\n", alias_name);
                    printf("Total: Import module: %s\n", target_name);
                    printf("\n");

                    object_release(module_obj);
                    object_release(class_obj);
                    object_release(alias_obj);

                    vm_frame_set_alias_identifier(frame, alias_name, target_name);

                    smm_free(target_name);
                    smm_free(alias_name);
                }
                goto dispatch;
                break;

            // Sets up loop block
            case VM_SETUP_LOOP :
                vm_push_block_loop(frame, BLOCK_TYPE_LOOP, frame->sp, frame->ip + oparg1, 0);
                goto dispatch;
                break;

            // Sets up loop block with else clause
            case VM_SETUP_ELSE_LOOP :
                vm_push_block_loop(frame, BLOCK_TYPE_LOOP, frame->sp, frame->ip + oparg1, frame->ip + oparg2);
                goto dispatch;
                break;

            // Pops the most inner loop-block
            case VM_POP_BLOCK :
                vm_pop_block(frame);
                goto dispatch;
                break;

            // Continue the most inner loop-block
            case VM_CONTINUE_LOOP :
                ret = object_alloc_instance(Object_Numerical, 1, oparg1);
                reason = REASON_CONTINUE;
                goto block_end;
                break;

            // Breaks out a loop-block
            case VM_BREAK_LOOP :
                reason = REASON_BREAK;
                goto block_end;
                break;

            // Breaks out a loop-block, and continue with the else clause
            case VM_BREAKELSE_LOOP :
                reason = REASON_BREAKELSE;
                goto block_end;
                break;

            // Compare 2 objects and push a boolean(true) or boolean(false) object back onto the stack
            case VM_COMPARE_OP :
                left_obj = vm_frame_stack_pop(frame);
                right_obj = vm_frame_stack_pop(frame);

                // @TODO: EQ and NE can be checked here as well. Or could we "override" them anyway? Store them inside
                // the base class!

                if (oparg1 == COMPARISON_RE || oparg1 == COMPARISON_NRE) {

                    if (! OBJECT_IS_REGEX(right_obj)) {

                        object_release(left_obj);
                        object_release(right_obj);

                        reason = REASON_EXCEPTION;
                        thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Can only regmatch against a regular expression");
                        goto block_end;
                    }

                    if (! OBJECT_IS_STRING(left_obj)) {

                        object_release(left_obj);
                        object_release(right_obj);

                        reason = REASON_EXCEPTION;
                        thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Can only regmatch a string");
                        goto block_end;
                    }


                    t_object *ret_obj = _do_regex_match((t_regex_object *)right_obj, (t_string_object *)left_obj);
                    if (! ret_obj) {
                        object_release(left_obj);
                        object_release(right_obj);

                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    if (oparg1 == COMPARISON_NRE) {
                        // Inverse the result, as we are NOT matching.
                        ret_obj = (ret_obj == Object_True) ? Object_False : Object_True;
                    }

                    object_release(left_obj);
                    object_release(right_obj);

                    vm_frame_stack_push(frame, ret_obj);
                    object_inc_ref(ret_obj);
                    goto dispatch;
                }


                // Exception compare is special case. We don't let classes handle that themselves, but we
                // need to do it here.
                if (oparg1 == COMPARISON_EX) {
                    if (object_instance_of(right_obj, left_obj->name)) {
                        vm_frame_stack_push(frame, Object_True);
                        object_inc_ref(Object_True);
                    } else {
                        vm_frame_stack_push(frame, Object_False);
                        object_inc_ref(Object_False);
                    }

                    object_release(right_obj);
                    object_release(left_obj);

                    goto dispatch;
                    break;
                }

                DEBUG_PRINT_CHAR("Compare '%s (%d)' against '%s (%d)'\n", left_obj->name, left_obj->type, right_obj->name, right_obj->type);

                // Compare types do not match
                if (left_obj->type != right_obj->type && !(OBJECT_IS_NULL(left_obj) || OBJECT_IS_NULL(right_obj))) {

                    // Try an implicit cast if possible
                    DEBUG_PRINT_CHAR("Explicit casting '%s' to '%s'\n", right_obj->name, left_obj->name);
                    t_attrib_object *cast_method = object_attrib_find(right_obj, left_obj->name);
                    if (! cast_method) {
                        object_release(right_obj);
                        object_release(left_obj);

                        reason = REASON_EXCEPTION;
                        thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Cannot compare '%s' against '%s'", left_obj->name, right_obj->name);
                        goto block_end;
                    }
                    obj1 = vm_object_call(right_obj, cast_method, 0);
                    if (! obj1) {
                        object_release(right_obj);
                        object_release(left_obj);

                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Release original right obj, and change to obj1, the casted object
                    object_release(right_obj);
                    right_obj = obj1;


                }

                dst = vm_object_comparison(left_obj, oparg1, right_obj);
                if (! dst) {
                    object_release(left_obj);
                    object_release(right_obj);

                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

                object_release(left_obj);
                object_release(right_obj);

                vm_frame_stack_push(frame, dst);
                object_inc_ref(dst);
                goto dispatch;
                break;

            // Build an attribute object from the values of the stack, and push attribute object back onto the stack
            case VM_BUILD_ATTRIB :
                {
                    // pop access object
                    t_object *access = vm_frame_stack_pop(frame);

                    // pop visibility object
                    t_object *visibility = vm_frame_stack_pop(frame);

                    // pop value object
                    t_object *value_obj = vm_frame_stack_pop(frame);

                    int method_flags = 0;

                    if (oparg1 == ATTRIB_TYPE_METHOD) {
                        // Pop method flags (not used yet)
                        t_object *method_flag_obj = vm_frame_stack_pop(frame);
                        method_flags = OBJ2NUM(method_flag_obj);
                        object_release(method_flag_obj);

                        // Generate hash object from arguments
                        t_hash_table *arg_list = ht_create();
                        for (int i=0; i!=oparg2; i++) {
                            t_method_arg *arg = smm_malloc(sizeof(t_method_arg));
                            arg->value = vm_frame_stack_pop(frame);

                            t_object *name_obj = vm_frame_stack_pop(frame);
                            s = string_to_char(OBJ2STR(name_obj));
                            object_release(name_obj);

                            arg->typehint = (t_string_object *)vm_frame_stack_pop(frame);

                            ht_add_str(arg_list, s, arg);
                            smm_free(s);
                        }

                        // Value object is already a callable, but has no arguments (or binding). Here we add the arglist
                        // @TODO: this means we cannot re-use the same codeblock with different args (which makes sense). Make sure
                        // this works.
                        ((t_callable_object *)value_obj)->data.arguments = arg_list;

#ifdef __DEBUG
                        ht_debug_keys(arg_list);
#endif
                    }
                    if (oparg1 == ATTRIB_TYPE_CONSTANT) {
                        // Nothing additional to do for constants
                    }
                    if (oparg1 == ATTRIB_TYPE_PROPERTY) {
                        // Nothing additional to do for regular properties
                    }

                    // Create new attribute object
                    dst = object_alloc_instance(Object_Attrib, 7, NULL, "", oparg1, OBJ2NUM(visibility), OBJ2NUM(access), value_obj, method_flags);

                    // @TODO: Do we still need this?
                    // Set the created frame for this attribute-object
                    dst->frame = thread_get_current_frame();

                    // Push method object
                    vm_frame_stack_push(frame, dst);
                    object_inc_ref(dst);

                    object_release(access);
                    object_release(visibility);
                    object_release(value_obj);
                }
                goto dispatch;
                break;

            // Build interface or class from the values on the stack and push the object back onto the stack
            case VM_BUILD_INTERFACE :
            case VM_BUILD_CLASS :
                {
                    // pop class flags (abstract,
                    // @TODO: Do we need to mask certain flags, as they should not be set directly through opcodes?
                    obj1 = vm_frame_stack_pop(frame);
                    int flags = OBJ2NUM(obj1);
                    object_release(obj1);

                    // Depending on the opcode, we are building a class or an interface
                    if (opcode == VM_BUILD_CLASS) {
                        flags |= OBJECT_TYPE_CLASS;
                    } else {
                        flags |= OBJECT_TYPE_INTERFACE;
                    }

                    // As we are generated from opcodes, it's a userland object
                    flags |= OBJECT_TYPE_USERLAND;


                    // Pop the number of interfaces
                    t_dll *interfaces = dll_init();
                    t_object *interface_cnt_obj = vm_frame_stack_pop(frame);
                    long interface_cnt = OBJ2NUM(interface_cnt_obj);
                    object_release(interface_cnt_obj);
                    DEBUG_PRINT_CHAR("Number of interfaces we need to implement: %ld\n", interface_cnt);

                    // Fetch all interface objects
                    for (int i=0; i!=interface_cnt; i++) {
                        t_object *interface_name_obj = vm_frame_stack_pop(frame);
                        DEBUG_PRINT_STRING_ARGS("Implementing interface: %s\n", object_debug(interface_name_obj));

                        // Check if the interface actually exists
                        s = string_to_char(OBJ2STR(interface_name_obj));
                        t_object *interface_obj = vm_frame_find_identifier(thread_get_current_frame(), s);
                        smm_free(s);

                        if (! interface_obj) {
                            dll_free(interfaces);

                            reason = REASON_EXCEPTION;
                            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "Interface '%s' is not found", OBJ2STR(interface_name_obj));
                            goto block_end;
                        }
                        if (! OBJECT_TYPE_IS_INTERFACE(interface_obj)) {
                            dll_free(interfaces);

                            reason = REASON_EXCEPTION;
                            thread_create_exception_printf((t_exception_object *)Object_TypeException, 1, "'%s' is not an interface", OBJ2STR(interface_name_obj));
                            goto block_end;
                        }

                        dll_append(interfaces, interface_obj);
                    }


                    // pop parent code object (as string)
                    t_object *parent_class = vm_frame_stack_pop(frame);
                    if (OBJECT_IS_NULL(parent_class)) {
                        object_release(parent_class);

                        parent_class = Object_Base;
                    } else {
                        // Find the object of this string
                        s = string_to_char(OBJ2STR(parent_class));
                        object_release(parent_class);
                        parent_class = vm_frame_find_identifier(frame, s);
                        if (parent_class == NULL) {
                            reason = REASON_EXCEPTION;
                            thread_create_exception_printf((t_exception_object *)Object_AttributeException, 1, "Class '%s' not found", s);
                            goto block_end;
                            break;
                        }
                        smm_free(s);
                    }
                    object_inc_ref(parent_class);


                    // pop class name
                    t_object *name_obj = vm_frame_stack_pop(frame);
                    char *name = string_to_char(OBJ2STR(name_obj));
                    object_release(name_obj);

                    // Fetch all attributes
                    t_hash_table *attributes = ht_create();
                    for (int i=0; i!=oparg1; i++) {
                        t_object *attr_name = vm_frame_stack_pop(frame);
                        t_attrib_object *attrib_obj = (t_attrib_object *)vm_frame_stack_pop_attrib(frame);

                        // Add method attribute to class
                        s = string_to_char(OBJ2STR(attr_name));
                        object_release(attr_name);
                        ht_add_str(attributes, s, attrib_obj);

                        // Increase because we are using it inside our hash table
                        object_inc_ref((t_object *)attrib_obj);
                        // Decrease because we have popped it from the stack
                        object_release((t_object *)attrib_obj);

                        smm_free(s);
                    }

                    // Actually create the object
                    t_object *new_obj = vm_create_userland_object(frame, name, flags, interfaces, parent_class, attributes);

                    // Check if the build class actually got all interfaces implemented
                    if (opcode == VM_BUILD_CLASS && ! object_check_interface_implementations((t_object *)new_obj)) {

                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // All done
                    vm_frame_stack_push(frame, (t_object *)new_obj);
                    object_inc_ref((t_object *)new_obj);

                    smm_free(name);
                }

                goto dispatch;
                break;

            case VM_STORE_FRAME_ID :
                obj1 = vm_frame_stack_pop(frame);

                s = vm_frame_get_name(frame, oparg1);

                // Make the class known in the local identifiers
                char *fqcn = vm_context_create_fqcn_from_context(frame->codeblock->context, s);
                vm_frame_set_local_identifier(frame, fqcn, (t_object *)obj1);
                smm_free(fqcn);


                // Register in the frame for cleanup and gc protection
                // vm_frame_register_userobject(frame, (t_object *)new_obj);

                object_release(obj1);

                goto dispatch;
                break;


            // Return / end the current frame
            case VM_RETURN :
                // Pop "ret" from the stack
                ret = vm_frame_stack_pop(frame);

                // We will be using the object in 'block_end'
                object_inc_ref(ret);

                // Decrease object count, as we popped it from the stack
                object_release(ret);

                reason = REASON_RETURN;
                goto block_end;
                break;

            // Setup an exception try/catch block
            case VM_SETUP_EXCEPT :
                vm_push_block_exception(frame, BLOCK_TYPE_EXCEPTION, frame->sp, frame->ip + oparg1, frame->ip + oparg2, frame->ip + oparg3);

                t_object *obj = object_alloc_instance(Object_Numerical, 1, REASON_FINALLY);
                vm_frame_stack_push(frame, obj);
                object_inc_ref(obj);

                goto dispatch;
                break;

            // Setup an exception try/catch block with finally clause
            case VM_END_FINALLY :
                ret = vm_frame_stack_pop(frame);

                if (OBJECT_IS_NUMERICAL(ret)) {
                    reason = OBJ2NUM(ret);
                    object_release(ret);

                    if (reason == REASON_RETURN || reason == REASON_CONTINUE) {
                        ret = vm_frame_stack_pop(frame);
                    }
                    goto block_end;
                    break;
                } else if (OBJECT_IS_EXCEPTION(ret)) {
                    object_release(ret);

                    reason = REASON_RERAISE;
                    ret = NULL;
                    goto block_end;
                    break;

                } else {
                    object_release(ret);

                    // This should not happen (orly?)
                    thread_create_exception((t_exception_object *)Object_SystemException, 1, "Unknown value on the stack during finally cleanup (probably a saffire-bug)");
                    reason = REASON_EXCEPTION;
                    goto block_end;
                    break;
                }

            // Throw an exception
            case VM_THROW :
                {
                    // Fetch exception object
                    t_object *obj = (t_object *)vm_frame_stack_pop(frame);

                    // Check if object extends exception
                    if (! object_instance_of(obj, "exception")) {
                        object_release(obj);

                        thread_create_exception((t_exception_object *)Object_ExtendException, 1, "Object must extend the 'exception' class");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Set exception
                    thread_set_exception((t_exception_object *)obj);

                    object_release(obj);

                    reason = REASON_EXCEPTION;
                    goto block_end;

                }
                break;

            // Pack a tuple object with values from the stack
            case VM_PACK_TUPLE :
                {
                    // Create an empty tuple
                    t_tuple_object *obj = (t_tuple_object *)object_alloc_instance(Object_Tuple, 0);

                    // Add elements from the stack into the tuple, sort in reverse order!
                    for (int i=0; i!=oparg1; i++) {
                        t_object *val = vm_frame_stack_pop(frame);

                        ht_add_num(obj->data.ht, oparg1 - i - 1, val);
                    }

                    // Push tuple on the stack
                    vm_frame_stack_push(frame, (t_object *)obj);
                    object_inc_ref((t_object *)obj);
                }

                goto dispatch;
                break;

            // Unpack a tuple object
            case VM_UNPACK_TUPLE :
                {
                    // Check if we are are unpacking a tuple
                    t_tuple_object *obj = (t_tuple_object *)vm_frame_stack_pop(frame);

                    if (! OBJECT_IS_TUPLE(obj)) {
                        object_release((t_object *)obj);

                        thread_create_exception((t_exception_object *)Object_TypeException, 1, "Argument is not a tuple");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Push the tuple vars. Make sure we start from the correct position
                    int offset = oparg1 < obj->data.ht->element_count ? oparg1 : obj->data.ht->element_count;
                    for (int i=0; i < offset; i++) {
                        t_object *val = ht_find_num(obj->data.ht, i);
                        vm_frame_stack_push(frame, val);
                        object_inc_ref(val);
                    }

                    // If we haven't got enough elements in our tuple, pad the result with NULLs first
                    while (oparg1-- > obj->data.ht->element_count) {
                        vm_frame_stack_push(frame, Object_Null);
                        object_inc_ref(Object_Null);
                    }

                    object_release((t_object *)obj);
                }

                goto dispatch;
                break;

            // Reset an iteration
            case VM_ITER_RESET :
                {
                    obj1 = vm_frame_stack_pop(frame);

                    // check if we have the iterator interface implemented
                    if (! object_has_interface(obj1, "iterator")) {
                        object_release(obj1);

                        thread_create_exception((t_exception_object *)Object_InterfaceException, 1, "Object must inherit the 'iterator' interface");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Fetch the actual iterator and push it to the stack
                    attr_obj = object_attrib_find(obj1, "__iterator");
                    obj3 = vm_object_call(obj1, attr_obj, 0);
                    vm_frame_stack_push(frame, obj3);
                    object_inc_ref(obj3);

                    // Call rewind
                    attr_obj = object_attrib_find(obj3, "__rewind");
                    vm_object_call(obj3, attr_obj, 0);

                    object_release(obj1);
                }
                goto dispatch;
                break;

            // Fetch iteration values (key, val, meta)
            case VM_ITER_FETCH :
                {
                    obj1 = vm_frame_stack_pop(frame);

                    // If we need 3 values, create and push metadata
                    if (oparg1 == 3) {
                        vm_frame_stack_push(frame, Object_Null);
                        object_inc_ref(Object_Null);
                    }
                    // Always push value
                    attr_obj = object_attrib_find(obj1, "__value");
                    obj3 = vm_object_call(obj1, attr_obj, 0);
                    vm_frame_stack_push(frame, obj3);
                    object_inc_ref(obj3);

                    if (oparg1 >= 2) {
                        // Push value of key
                        attr_obj = object_attrib_find(obj1, "__key");
                        obj3 = vm_object_call(obj1, attr_obj, 0);
                        vm_frame_stack_push(frame, obj3);
                        object_inc_ref(obj3);
                    }

                    // Push value of hasNext
                    attr_obj = object_attrib_find(obj1, "__hasNext");
                    obj3 = vm_object_call(obj1, attr_obj, 0);
                    vm_frame_stack_push(frame, obj3);
                    object_inc_ref(obj3);

                    if (IS_BOOLEAN_TRUE(obj3)) {
                        attr_obj = object_attrib_find(obj1, "__next");
                        obj3 = vm_object_call(obj1, attr_obj, 0);
                    }

                    object_release(obj1);
                }
                goto dispatch;
                break;


            // Build a datastructure from the values on the stack and place the datastructure object back onto the stack
            case VM_BUILD_DATASTRUCT   :
                {
                    // Fetch methods to call
                    t_object *obj = (t_object *)vm_frame_stack_pop(frame);

                    // We can only call a class, as we are instantiating a data structure
                    if (! OBJECT_TYPE_IS_CLASS(obj)) {
                        object_release(obj);

                        // We can only instantiate here through a class!
                        thread_create_exception((t_exception_object *)Object_CallException, 1, "Datastructure must be a class, not an instance");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Check if object has interface datastructure
                    if (! object_has_interface(obj, "datastructure")) {
                        object_release(obj);

                        thread_create_exception((t_exception_object *)Object_InterfaceException, 1, "Class must inherit the 'datastructure' interface");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    // Create argument list.
                    t_dll *dll = dll_init();
                    for (int i=0; i!=oparg1; i++) {
                        dll_append(dll, vm_frame_stack_pop(frame));
                    }

                    // Create new object, because we know it's a data-structure, just add them to the list
                    t_object *ret_obj = (t_object *)object_alloc_instance(obj, 2, NULL, dll);  // arg 1 is hashtable, arg2 is dll

                    vm_frame_stack_push(frame, ret_obj);
                    object_inc_ref(ret_obj);

                    object_release(obj);
                }
                goto dispatch;
                break;

            // Load a subscription [] value out of a datastructure onto the stack
            case VM_LOAD_SUBSCRIPT :
                {
                    // Fetch actual data structure
                    obj1 = vm_frame_stack_pop(frame);
                    if (! object_has_interface(obj1, "iterator")) {
                        object_release(obj1);

                        thread_create_exception((t_exception_object *)Object_InterfaceException, 1, "Class must inherit the 'iterator' interface");
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    t_object *ret_obj = NULL;

                    switch (oparg1) {
                        case 0 :
                            object_release(obj1);
                            // foo[]
                            thread_create_exception((t_exception_object *)Object_InterfaceException, 1, "not supporting [] yet");
                            reason = REASON_EXCEPTION;
                            goto block_end;

                            break;
                        case 1 :
                            // foo[n]
                            obj2 = vm_frame_stack_pop(frame);       // first key

                            attr_obj = object_attrib_find(obj1, "__get");
                            ret_obj = vm_object_call(obj1, attr_obj, 1, obj2);

                            object_release(obj2);
                            break;
                        case 2 :
                            // foo[n..m]
                            obj3 = vm_frame_stack_pop(frame);       // max
                            obj2 = vm_frame_stack_pop(frame);       // min

                            attr_obj = object_attrib_find(obj1, "__splice");
                            if (! attr_obj) {
                                object_release(obj1);

                                object_release(obj2);
                                object_release(obj3);

                                thread_create_exception((t_exception_object *)Object_AttributeException, 1, "__splice() not found");
                                reason = REASON_EXCEPTION;
                                goto block_end;
                            }
                            ret_obj = vm_object_call(obj1, attr_obj, 2, obj2, obj3);

                            object_release(obj2);
                            object_release(obj3);
                            break;
                    }

                    object_release(obj1);

                    if (! ret_obj) {
                        reason = REASON_EXCEPTION;
                        goto block_end;
                    }

                    vm_frame_stack_push(frame, ret_obj);
                    object_inc_ref(ret_obj);
                }
                goto dispatch;
                break;

            // Store a value into a datastructure
            case VM_STORE_SUBSCRIPT :
                obj1 = vm_frame_stack_pop(frame);       // datastructure
                obj2 = vm_frame_stack_pop(frame);       // key

                attr_obj = object_attrib_find(obj1, "__set");
                t_object *ret_obj = vm_object_call(obj1, attr_obj, 1, obj2);
                vm_frame_stack_push(frame, ret_obj);
                object_inc_ref(ret_obj);

                object_release(obj2);
                object_release(obj1);

                goto dispatch;
                break;


            default:
                fatal_error(1, "unknown vm opcode!");
                break;


        } // switch(opcode) {


        /*
         * Block_end is only reached when we need to change something in our block flow. Normally, it means that we
         * returned from our method, but it could also be a that we have an exception, or simply that we break, or
         * continue from a (while,for,etc) loop. The unwind_blocks() function will make sure that we jump to the correct
         * block and that everything according to the stack will be handled. The reason variable holds the current
         * reason of ending the block, and it will return what has happened.
         */
block_end:
        // This loop will unwind the blockstack and act accordingly on each block (if needed)
        unwind_blocks(frame, &reason, ret);

        // Still not handled, break from this frame
        if (reason != REASON_NONE) {
            //object_release(ret);
            break;
        }

    } // for (;;)


    // Restore current frame
    thread_set_current_frame(parent_frame);

    // We don't increase the refcount on the returned object. The caller must deal with this.
    return ret;
}

/**
 * Unwind blocks. There are two types of blocks: LOOP and EXCEPTION. When this function is called
 * we will by default unwind everything down EXCEPT when certain conditions are met.
 *
 * This function can change the reason and the blocks in the frame. It will not change any return values, but it can
 * change the IP pointer as well (continue, break, finally, catch etc)
 */
t_vm_frameblock *unwind_blocks(t_vm_stackframe *frame, long *reason, t_object *ret) {
    // Peek block to see if there is any
    t_vm_frameblock *block = vm_peek_block(frame);
//    DEBUG_PRINT_CHAR("init unwind_blocks: [curblocks %d] (%d)\n", frame->block_cnt, *reason);

    // Unwind block as long as there is a reason to unwind
    while (*reason != REASON_NONE && frame->block_cnt > 0) {
//        DEBUG_PRINT_CHAR("  CUR block: %d\n", frame->block_cnt);
//        DEBUG_PRINT_CHAR("  Current reason: %d\n", *reason);
//        DEBUG_PRINT_CHAR("  Block Type: %d\n", block->type);

        // The next scenario's will change the instruction pointer (and maybe some other values), but they don't
        // change anything with the block-stack. For instance, when we continue a while-loop. This means we will only
        // peek at the current block, but not pop it.
        block = vm_peek_block(frame);
        //DEBUG_PRINT_CHAR("Checking frameblock %d. CBT: %d\n", frame->block_cnt, block->type);

        // Case 1: Continue called inside a loop block
        if (*reason == REASON_CONTINUE && block->type == BLOCK_TYPE_LOOP) {
//            DEBUG_PRINT_CHAR("CASE 1\n");
//            DEBUG_PRINT_CHAR("\n*** Continuing loop at %08lX\n\n", OBJ2NUM(ret));

            // Continue to at the start of the block
            frame->ip = OBJ2NUM(ret);
            *reason = REASON_NONE;
            break;
        }


        // Case 2: Return called inside try (or catch) block, but not inside finally block
        if (*reason == REASON_RETURN && block->type == BLOCK_TYPE_EXCEPTION) {
//            DEBUG_PRINT_CHAR("CASE 2: RETURN IN TRY, CATCH OR FINALLY\n");
            /* We push the return value and the reason (REASON_RETURN) onto the stack, since END_FINALLY will expect
             * this. We have no real way to know if a try block has a return statement inside, so SETUP_EXCEPT will
             * push a REASON_NONE and a dummy retval onto the stack as well. END_EXCEPTION will catch this because
             * when the popped reason is REASON_NONE, no return has been called in the try block. Ultimately this
             * doesn't really matter. Because END_FINALLY will unwind the block, which means the variable-stack will
             * match prior to the exception-block, so the dummy variables will be removed as well (IF they were
             * present) */

            vm_frame_stack_push(frame, ret);
            object_inc_ref(ret);

            t_object *obj = object_alloc_instance(Object_Numerical, 1, *reason);
            vm_frame_stack_push(frame, obj);
            object_inc_ref(obj);

            /* Instead of actually returning, continue with executing the finally block. END_FINALLY will deal with
             * the delayed return. */
            *reason = REASON_NONE;

            /* If we are BELOW the finally block, we ASSUME that we have to jump to the finally block. This could
             * be triggered from both the try or a catch block (both are handled the same, so this distinction is not
             * needed. However, when ABOVE, we ASSUME to be triggered from the finally block, and this we need to skip
             * until the end of the finally */
            DEBUG_PRINT_CHAR("IP F : %02X %02X\n", frame->ip, block->handlers.exception.ip_finally);
            DEBUG_PRINT_CHAR("IP C : %02X %02X\n", frame->ip, block->handlers.exception.ip_catch);
            DEBUG_PRINT_CHAR("IP EF: %02X %02X\n", frame->ip, block->handlers.exception.ip_end_finally);

            if (frame->ip <= block->handlers.exception.ip_finally) {
                DEBUG_PRINT_CHAR("RETTING into FINALLY\n");
                frame->ip = block->handlers.exception.ip_finally;
            } else if (frame->ip <= block->handlers.exception.ip_end_finally) {
                DEBUG_PRINT_CHAR("RETTING out from FINALLY\n");
                frame->ip = block->handlers.exception.ip_end_finally;

                *reason = REASON_FINALLY;
            } else {
                DEBUG_PRINT_CHAR("Not retting in anything\n");
                *reason = REASON_FINALLY;
            }
            break;
        }

        // Case 3: Exception raised inside a try block (normal behaviour)
        if (*reason == REASON_EXCEPTION && block->type == BLOCK_TYPE_EXCEPTION) {
            DEBUG_PRINT_CHAR("CASE 4: EXCEPTION TRIGGERED (IN TRY BLOCK)\n");

            // Clean up any remaining items on the variable stack, but keep the last "REASON_FINALLY"
            while (frame->sp < block->sp - 1) {
                DEBUG_PRINT_CHAR("Current SP: %d -> Needed SP: %d\n", frame->sp, block->sp);
                t_object *obj = vm_frame_stack_pop(frame);
                object_release(obj);
            }

            // We throw the current exception onto the stack. The catch-blocks will expect this.
            vm_frame_stack_push(frame, (t_object *)thread_get_exception());
            object_inc_ref((t_object *)thread_get_exception());

            // Continue with handling the exception in the current vm frame.
            *reason = REASON_NONE;
            frame->ip = block->handlers.exception.ip_catch;

            break;
        }

        DEBUG_PRINT_CHAR("unwind!\n");

        /*
         * All cases below here will change the block-stack, so we can pop the block from this point onwards.
         */

        /* Pop the block from the frame, but we still use it. As long as we don't push another block in
         * this function, this works ok. */
        block = vm_peek_block(frame);

        // Unwind the variable stack. This will remove all variables used in the current (unwound) block.
        while (frame->sp < block->sp) {
            t_object *obj = vm_frame_stack_pop(frame);
            object_release(obj);
        }


        // Case 4: Exception raised, but we are not an exception block. Try again with the next block.
        if (*reason == REASON_EXCEPTION && block->type != BLOCK_TYPE_EXCEPTION) {
            vm_pop_block(frame);
            // Ignore this block. Keep on iterating blocks until we find an exception-block (or we run out of blocks)
            continue;
        }

        // Case 5: retting out of an exception or finally block
        if (*reason == REASON_FINALLY && block->type == BLOCK_TYPE_EXCEPTION) {
            vm_pop_block(frame);

            // Don't need to do anything.
            *reason = REASON_NONE;
            break;
        }

        // Case 6: Breakelse inside a loop-block
        if (*reason == REASON_BREAKELSE && block->type == BLOCK_TYPE_LOOP) {
            // Jump to our else-clause
            frame->ip = block->handlers.loop.ip_else;
            *reason = REASON_NONE;
            break;
        }

        // Case 7: Break inside a loop-block
        if (*reason == REASON_BREAK && block->type == BLOCK_TYPE_LOOP) {
            // Simply jump to the end of the loop-block
            frame->ip = block->handlers.loop.ip;
            *reason = REASON_NONE;
            break;
        }
    }

    // It might be possible that we unwind every block and still have a a reason different than REASON_NONE. This will
    // be handled by the parent frame.

    DEBUG_PRINT_CHAR("fini unwind_blocks: [block_cnt: %d] %d\n", frame->block_cnt, *reason);
    return block;
}

/**
* The same as a normal execute, but don't handle exceptions
* @param frame
* @return
*/
t_vm_stackframe *vm_execute_import(t_vm_codeblock *codeblock, t_object **result) {
    // Execute the frame
    DEBUG_PRINT_CHAR("\n       ============================ VM import execution start (%s)============================\n", codeblock->context->module.full);

    //t_vm_stackframe *current_frame = thread_get_current_frame();
    t_vm_stackframe *import_frame = vm_stackframe_new(NULL, codeblock);
//    import_frame->trace_class = string_strdup0(current_frame->trace_class);
//    import_frame->trace_method = string_strdup0("#import");
    import_frame->trace_class = string_strdup0("##");
    import_frame->trace_method = string_strdup0("#import");

    if (result) {
        *result = _vm_execute(import_frame);
    } else {
        // Store in temporary object
        t_object *ret = _vm_execute(import_frame);
        // Decrease reference, as we don't use this object
        object_release(ret);
    }

    DEBUG_PRINT_CHAR("\n       ============================ VM import execution fini (%s) ============================\n", codeblock->context->module.full);

    return import_frame;
}


/**
 *
 */
void _vm_load_implicit_builtins(t_vm_stackframe *frame) {
    // Implicit load the saffire module, without any debugging
    int runmode = vm_runmode;
    vm_runmode &= ~VM_RUNMODE_DEBUG;

    //  Import mandatory saffire object (make '\saffire\saffire' known as "saffire" in the current namespace)
    vm_frame_set_alias_identifier(frame, "saffire", "\\saffire");

    // Back to normal runmode again
    vm_runmode = runmode;
}

/**
 *
 */
int vm_execute(t_vm_stackframe *frame) {
    int ret_val = 0;

    thread_set_current_frame(frame);

    _vm_load_implicit_builtins(frame);

    // Execute the frame
    t_object *result = _vm_execute(frame);


    DEBUG_PRINT_CHAR("\n\n\n============================ TOTAL VM execution done ============================\n\n\n");
#ifdef __DEBUG
    if (thread_exception_thrown()) {
        DEBUG_PRINT_CHAR(ANSI_BRIGHTRED "============================ EXCEPTION OCCURED. ============================\n\n\n" ANSI_RESET);
    }
    #if __DEBUG_STACKFRAME_DESTROY
        DEBUG_PRINT_CHAR("----- [END FRAME: %s (%08X)] ----\n", frame->codeblock->context->module.full, (uintptr_t)frame);
        if (frame->local_identifiers) print_debug_table(frame->local_identifiers->data.ht, "Locals");
        if (frame->global_identifiers) print_debug_table(frame->global_identifiers->data.ht, "Globals");
        if (frame->builtin_identifiers) print_debug_table(frame->builtin_identifiers->data.ht, "Builtins");
    #endif
#endif


    // Check if there was an uncaught exception (happened when result == NULL)
    if (result == NULL) {
        if (thread_exception_thrown()) {

#ifdef __DEBUG
            // Display stacktrace from the exception
#endif

            // handle exceptions
            t_object *saffire_obj = vm_frame_find_identifier(frame, "saffire");  // This is an alias to \saffire\saffire
            if (! saffire_obj) {
                fatal_error(1, "The \\saffire module was not found");
            }

            t_attrib_object *exceptionhandler_obj = object_attrib_find(saffire_obj, "uncaughtExceptionHandler");
            if (!exceptionhandler_obj) {
                // Uh-oh. uncaughtExceptionHandler method not found :(
                fatal_error(1, "The saffire.uncaughtExceptionHandler() method was not found");
            }
            // We assume that finding our exception handler always work
            result = vm_object_call(saffire_obj, exceptionhandler_obj, 1, thread_get_exception());
            if (result == NULL) {
                result = object_alloc_instance(Object_Numerical, 1, 0);
            }
        } else {
            // result was NULL, but no exception found, just threat like regular 0
            result = object_alloc_instance(Object_Numerical, 1, 0);
        }
    }

    if (OBJECT_IS_NUMERICAL(result)) {
        // Correct numerical returned, use as return code
        ret_val = ((t_numerical_object *) result)->data.value;
        object_release(result);
    } else {
        // Convert returned object to numerical, so we can use it as an error code
        t_attrib_object *result_numerical = object_attrib_find(result, "__numerical");
        if (result_numerical) {
            t_object *result2 = vm_object_call(result, result_numerical, 0);
            object_release(result);
            ret_val = ((t_numerical_object *) result2)->data.value;
            object_release(result2);
        } else {
            // Not a numerical result returned, and we cannot cast it to numerical neither :/
            object_release(result);
            ret_val = 1;
        }
    }


    // If there is an exception thrown, remove the refcount of the exception
    if (thread_exception_thrown()) {
        thread_clear_exception();
    }

//    vm_frame_destroy(frame);
    return ret_val;
}

/**
 * Called to add a global class. Things like numeric, boolean, strings, saffire, etc etc.
 */
void vm_populate_builtins(const char *name, t_object *obj) {
    object_inc_ref(obj);
    ht_add_str(builtin_identifiers_ht, (char *)name, (void *)obj);
}

/**
 * Calls a method from specified object. Returns NULL when method is not found.
 */
t_object *vm_object_call(t_object *self, t_attrib_object *attrib_obj, int arg_count, ...) {
    if (! self || ! attrib_obj) return NULL;

    // Create DLL with all arguments
    va_list args;
    va_start(args, arg_count);
    t_dll *arg_list = dll_init();
    for (int i=0; i!=arg_count; i++) {
        t_object *obj = va_arg(args, t_object *);
        dll_append(arg_list, obj);
    }
    va_end(args);

    t_object *ret_obj = _object_call_attrib_with_args(self, attrib_obj, arg_list);

    // Free dll
    dll_free(arg_list);

    return ret_obj;
}
