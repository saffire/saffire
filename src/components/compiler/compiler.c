/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "general/output.h"
#include "compiler/compiler.h"
#include "compiler/parser.tab.h"
#include "compiler/ast.h"
#include "general/smm.h"

extern void yyerror(const char *err);
extern int yylineno;

/**
 * Validate class modifiers
 */
void sfc_validate_class_modifiers(long modifiers) {
    // Classes do not have a visibility
    if (modifiers & MODIFIER_MASK_VISIBLITY) {
        line_error_and_die(1, yylineno, "Classes cannot have a visibility");
    }
}


/**
 * Validate abstract method body
 */
void sfc_validate_abstract_method_body(long modifiers, t_ast_element *body) {
    // If the method is not abstract, we don't need to check
    if ((modifiers & MODIFIER_ABSTRACT) == 0) {
        return;
    }

    // Right now, this is an abstract method

    // Check if we have a body
    if (body->type != typeAstNull) {
        line_error_and_die(1, yylineno, "Abstract methods cannot have a body");
    }
}


/**
 * Validate method modifiers
 */
void sfc_validate_method_modifiers(long modifiers) {
    // Make sure we have at least 1 visibility bit set
    if ((modifiers & MODIFIER_MASK_VISIBLITY) == 0) {
        line_error_and_die(1, yylineno, "Methods must define a visibility");
    }

    // Check if abstract and private are set. This is not allowed
    if ((modifiers & (MODIFIER_ABSTRACT | MODIFIER_PRIVATE)) == (MODIFIER_ABSTRACT | MODIFIER_PRIVATE)) {
        line_error_and_die(1, yylineno, "Abstract methods cannot be private");
    }
}


/**
 * Validate property modifiers
 */
void sfc_validate_property_modifiers(long modifiers) {
    if ((modifiers & MODIFIER_MASK_VISIBLITY) == 0) {
        line_error_and_die(1, yylineno, "Methods must define a visibility");
    }
}


/**
 * Checks if modifier flags are allowed (does not check the context (class, method, property etc).
 */
void sfc_validate_flags(long cur_flags, long new_flag) {
    // Only one of the visibility flags must be set
    if ((cur_flags & MODIFIER_MASK_VISIBLITY) && (new_flag & MODIFIER_MASK_VISIBLITY)) {
        line_error_and_die(1, yylineno, "Cannot have multiple visiblity masks");
    }

    // Is one of the new flags already been set?
    if (cur_flags & new_flag) {
        line_error_and_die(1, yylineno, "Modifiers can only be set once");
    }

    // Make sure abstract and final are not both set.
    if (((cur_flags | new_flag) & (MODIFIER_ABSTRACT | MODIFIER_FINAL)) == (MODIFIER_ABSTRACT | MODIFIER_FINAL)) {
       line_error_and_die(1, yylineno, "Abstract members cannot be made final");
    }
}


/**
 * Validate a constant
 */
void sfc_validate_constant(char *name) {
    // Check if the class exists in the class table
    if (global_table->active_class) {
        // inside the current class

        if (ht_exists(global_table->active_class->constants, name)) {
            line_error_and_die(1, yylineno, "Constant '%s' has already be defined in class '%s'", name, global_table->active_class->name);
        }

        // Save structure to the global class hash and set as the active class
        ht_add(global_table->active_class->constants, name, name);

    } else {
        // Global scope
        if (ht_exists(global_table->constants, name)) {
            line_error_and_die(1, yylineno, "Constant '%s' has already be defined in the global scope", name);
        }

        // Save structure to the global class hash and set as the active class
        ht_add(global_table->constants, name, name);
    }

}

/**
 * Enter a method, and validate method name
 */
void sfc_init_method(const char *name) {
    global_table->in_method = 1;
}

/**
 * Leave a method
 */
void sfc_fini_method() {
    global_table->in_method = 0;
}

/**
 * Initialize a class
 */
void sfc_init_class(int modifiers, char *name, t_ast_element *extends, t_ast_element *implements) {
    // Are we inside a class already, if so, we cannot add another class
    if (global_table->in_class == 1) {
        line_error_and_die(1, yylineno, "You cannot define a class inside another class");
    }

    // Check if the class exists in the class table
    if (ht_exists(global_table->classes, name)) {
        line_error_and_die(1, yylineno, "This class is already defined");
    }

    // Initialize and populte a new class structure
    t_class *new_class = (t_class *)smm_malloc(sizeof(t_class));
    new_class->modifiers = modifiers;
    new_class->name = strdup(name);

    // @TODO: Check if parent actually exists
    new_class->parent = NULL;

    new_class->extends = extends;
    new_class->implements = implements;

    new_class->methods = ht_create();
    new_class->constants = ht_create();
    new_class->properties = ht_create();

    new_class->interfaces = NULL;
    new_class->num_interfaces = 0;

    new_class->filename = "";
    new_class->line_start = yylineno;
    new_class->line_end = 0;

    // Save structure to the global class hash and set as the active class
    ht_add(global_table->classes, name, new_class);
    global_table->active_class = new_class;

    // We are currently inside a class.
    global_table->in_class = 1;
}


/**
 * End a class
 */
void sfc_fini_class(void) {
    // Cannot close a class when we are not inside one
    if (global_table->in_class == 0) {
        line_error_and_die(1, yylineno, "Closing a class, but we weren't inside one to begin with");
    }

    // Is there an active class?
    if (global_table->active_class == NULL) {
        line_error_and_die(1, yylineno, "Somehow we try to close the global scope");
    }

    // Set line ending
    (global_table->active_class)->line_end = yylineno;

    // Close class, move back to global scope
    global_table->active_class = NULL;

    // Not inside a class anymore
    global_table->in_class = 0;
}


/**
 * Initialize the compiler global table
 */
static void sfc_init_global_table(void) {
    // Allocate table memory
    global_table = (t_global_table *)smm_malloc(sizeof(t_global_table));

    // Initialize table
    global_table->constants = ht_create();
    global_table->classes = ht_create();
    global_table->active_class = NULL;

    global_table->in_class = 0;
    global_table->in_method = 0;
    global_table->in_loop_counter = 0;


    global_table->switches = NULL;
    global_table->current_switch = NULL;
}

static void sfc_fini_global_table(void) {
    // Iterate over classes and remove all info
    t_hash_table *ht = global_table->classes;

    t_hash_iter iter;
    ht_iter_init(&iter, ht);
    while (ht_iter_valid(&iter)) {
        t_class *class = ht_iter_value(&iter);

        smm_free(class->name);  // Strdupped

        ht_destroy(class->methods);
        ht_destroy(class->constants);
        ht_destroy(class->properties);

        smm_free(class);

        ht_iter_next(&iter);
    }

    // Destroy global constant and classes tables
    ht_destroy(global_table->constants);
    ht_destroy(global_table->classes);

    // Free actual global table
    smm_free(global_table);
}


/**
 * Enter a control-loop
 */
void sfc_loop_enter(void) {
    // Increase loop counter, since we are entering a new loop
    global_table->in_loop_counter++;
}


/**
 * Leave a loop.
 */
void sfc_loop_leave(void) {
    // Not possible to leave a loop when we aren't inside any
    if (global_table->in_loop_counter <= 0) {
        line_error_and_die(1, yylineno, "Somehow, we are trying to leave a loop from the outer scope");
    }

    // Decrease loop counter, since we are going down one loop
    global_table->in_loop_counter--;
}


/**
 * Begin switch statement
 */
void sfc_switch_begin(void) {
    // Allocate switch structure
    t_switch_struct *ss = (t_switch_struct *)smm_malloc(sizeof(t_switch_struct));

    // Set default values
    ss->has_default = 0;

    // Add the current switch as the parent
    t_switch_struct *tmp =  global_table->current_switch;
    ss->parent = tmp;

    // Set current switch to the new switch
    global_table->current_switch = ss;
}


/**
 * End switch statement
 */
void sfc_switch_end(void) {
    t_switch_struct *ss = global_table->current_switch;

    // set current to the parent
    global_table->current_switch = ss->parent;

    // Free switch structure
    smm_free(ss);
}


/**
 * Check case label
 */
void sfc_switch_case(void) {
}


/**
 * Check if a default label is valid
 */
void sfc_switch_default(void) {
    t_switch_struct *ss = global_table->current_switch;

    if (ss == NULL) {
        line_error_and_die(1, yylineno, "Default label expected inside a switch statement");
    }

    // Check if default already exists
    if (ss->has_default) {
        line_error_and_die(1, yylineno, "default label already supplied");
    }

    ss->has_default = 1;
}


/**
 * Make sure a label is not a variable
 */
void saffire_check_label(const char *name) {
}


/**
 * Make sure return happens inside a method
 *
 */
void saffire_validate_return() {
    if (global_table->in_method == 0) {
        line_error_and_die(1, yylineno, "Cannot use return outside a method");
    }
}


/**
 * Make sure break happens inside a loop
 *
 */
void saffire_validate_break() {
    if (global_table->in_loop_counter == 0) {
        line_error_and_die(1, yylineno, "We can only break inside a loop");
    }
}

void sfc_check_permitted_identifiers(char *name) {
    if (! strcasecmp(name, "null")) return;
    if (! strcasecmp(name, "false")) return;
    if (! strcasecmp(name, "true")) return;
    line_error_and_die(1, yylineno, "Incorrect identifier");
}



/**
 * Make sure continue happens inside a loop
 *
 */
void saffire_validate_continue() {
    if (global_table->in_loop_counter == 0) {
        line_error_and_die(1, yylineno, "We can only continue inside a loop");
    }
}


/**
 * Make sure breakelse happens inside a loop
 *
 */
void saffire_validate_breakelse() {
    if (global_table->in_loop_counter == 0) {
        line_error_and_die(1, yylineno, "We can only breakelse inside a loop");
    }
}


/**
 * Initialize the saffire_compiler
 */
void sfc_init(void) {
    sfc_init_global_table();
}

void sfc_fini(void) {
    sfc_fini_global_table();
}
