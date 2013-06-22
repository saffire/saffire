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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "general/output.h"
#include "compiler/parser_helpers.h"
#include "compiler/saffire_parser.h"
#include "compiler/parser.tab.h"
#include "objects/callable.h"

#include "compiler/ast_nodes.h"
#include "general/smm.h"
#include "objects/attrib.h"


/**
 * Convert modifier list to visibility flags for astAttribute nodes.
 */
char parser_mod_to_visibility(SaffireParser *sp, int lineno, long modifiers) {
    modifiers &= MODIFIER_MASK_VISIBILITY;

    if ((modifiers & MODIFIER_MASK_VISIBILITY) == MODIFIER_PUBLIC) return ATTRIB_VISIBILITY_PUBLIC;
    if ((modifiers & MODIFIER_MASK_VISIBILITY) == MODIFIER_PROTECTED) return ATTRIB_VISIBILITY_PROTECTED;
    if ((modifiers & MODIFIER_MASK_VISIBILITY) == MODIFIER_PRIVATE) return ATTRIB_VISIBILITY_PRIVATE;

    return 0;
}

/**
 * Convert modifier list to method flags for astAttribute nodes.
 */
char parser_mod_to_methodflags(SaffireParser *sp, int lineno, long modifiers) {
    char ret = 0;

    if ((modifiers & MODIFIER_STATIC) == MODIFIER_STATIC) ret |= CALLABLE_FLAG_STATIC;
    if ((modifiers & MODIFIER_ABSTRACT) == MODIFIER_ABSTRACT) ret |= CALLABLE_FLAG_ABSTRACT;
    if ((modifiers & MODIFIER_FINAL) == MODIFIER_FINAL) ret |= CALLABLE_FLAG_FINAL;

    return ret;
}


/**
 * Validate class modifiers
 */
void parser_validate_class_modifiers(SaffireParser *sp, int lineno, long modifiers) {
    // Classes do not have a visibility
    if (modifiers & MODIFIER_MASK_VISIBILITY) {
        parser_error(sp, lineno, "Classes cannot have a visibility");
        return;
    }
}


/**
 * Validate abstract method body
 */
void parser_validate_abstract_method_body(SaffireParser *sp, int lineno, long modifiers, t_ast_element *body) {
    // If the method is not abstract, we don't need to check
    if ((modifiers & MODIFIER_ABSTRACT) == 0) {
        return;
    }

    // Right now, this is an abstract method

    // Check if we have a body
    if (body->type != typeAstNull) {
        parser_error(sp, lineno, "Abstract methods cannot have a body");
        return;
    }
}


/**
 * Validate method modifiers
 */
void parser_validate_method_modifiers(SaffireParser *sp, int lineno, long modifiers) {
    // Make sure we have at least 1 visibility bit set
    if ((modifiers & MODIFIER_MASK_VISIBILITY) == 0) {
        parser_error(sp, lineno, "Methods must define a visibility");
        return;
    }

    // Check if abstract and private are set. This is not allowed
    if ((modifiers & (MODIFIER_ABSTRACT | MODIFIER_PRIVATE)) == (MODIFIER_ABSTRACT | MODIFIER_PRIVATE)) {
        parser_error(sp, lineno, "Abstract methods cannot be private");
        return;
    }
}


/**
 * Validate property modifiers
 */
void parser_validate_property_modifiers(SaffireParser *sp, int lineno, long modifiers) {
    if ((modifiers & MODIFIER_MASK_VISIBILITY) == 0) {
        parser_error(sp, lineno, "Methods must define a visibility");
        return;
    }
}


/**
 * Checks if modifier flags are allowed (does not check the context (class, method, property etc).
 */
void parser_validate_flags(SaffireParser *sp, int lineno, long cur_flags, long new_flag) {
    // Only one of the visibility flags must be set
    if ((cur_flags & MODIFIER_MASK_VISIBILITY) && (new_flag & MODIFIER_MASK_VISIBILITY)) {
        parser_error(sp, lineno, "Cannot have multiple visiblity masks");
        return;
    }

    // Is one of the new flags already been set?
    if (cur_flags & new_flag) {
        parser_error(sp, lineno, "Modifiers can only be set once");
        return;
    }

    // Make sure abstract and final are not both set.
    if (((cur_flags | new_flag) & (MODIFIER_ABSTRACT | MODIFIER_FINAL)) == (MODIFIER_ABSTRACT | MODIFIER_FINAL)) {
       parser_error(sp, lineno, "Abstract members cannot be made final");
       return;
    }
}


/**
 * Validate a constant
 */
void parser_validate_constant(SaffireParser *sp, int lineno, char *name) {
    // Check if the class exists in the class table
    if (sp->parserinfo->active_class) {
        // inside the current class

        if (ht_exists(sp->parserinfo->active_class->constants, name)) {
            parser_error(sp, lineno, "Constant '%s' has already be defined in class '%s'", name, sp->parserinfo->active_class->name);
            return;
        }

        // Save structure to the global class hash and set as the active class
        ht_add(sp->parserinfo->active_class->constants, name, name);

    } else {
        // Global scope
        if (ht_exists(sp->parserinfo->constants, name)) {
            parser_error(sp, lineno, "Constant '%s' has already be defined in the global scope", name);
            return;
        }

        // Save structure to the global class hash and set as the active class
        ht_add(sp->parserinfo->constants, name, name);
    }

}

/**
 * Enter a method, and validate method name
 */
void parser_init_method(SaffireParser *sp, int lineno, const char *name) {
    sp->parserinfo->in_method = 1;
}

/**
 * Leave a method
 */
void parser_fini_method(SaffireParser *sp, int lineno) {
    sp->parserinfo->in_method = 0;
}

/**
 * Initialize a class
 */
void parser_init_class(SaffireParser *sp, int lineno, int modifiers, char *name, t_ast_element *extends, t_ast_element *implements) {
    // Are we inside a class already, if so, we cannot add another class
    if (sp->parserinfo->in_class == 1) {
        parser_error(sp, lineno, "You cannot define a class inside another class");
        return;
    }

    // Check if the class exists in the class table
    if (ht_exists(sp->parserinfo->classes, name)) {
        parser_error(sp, lineno, "This class is already defined");
        return;
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
    new_class->line_start = lineno;
    new_class->line_end = 0;

    // Save structure to the global class hash and set as the active class
    ht_add(sp->parserinfo->classes, name, new_class);
    sp->parserinfo->active_class = new_class;

    // We are currently inside a class.
    sp->parserinfo->in_class = 1;
}


/**
 * End a class
 */
void parser_fini_class(SaffireParser *sp, int lineno) {
    // Cannot close a class when we are not inside one
    if (sp->parserinfo->in_class == 0) {
        parser_error(sp, lineno, "Closing a class, but we weren't inside one to begin with");
        return;
    }

    // Is there an active class?
    if (sp->parserinfo->active_class == NULL) {
        parser_error(sp, lineno, "Somehow we try to close the global scope");
        return;
    }

    // Set line ending
    (sp->parserinfo->active_class)->line_end = lineno;

    // Close class, move back to global scope
    sp->parserinfo->active_class = NULL;

    // Not inside a class anymore
    sp->parserinfo->in_class = 0;
}


/**
 * Enter a control-loop
 */
void parser_loop_enter(SaffireParser *sp, int lineno) {
    // Increase loop counter, since we are entering a new loop
    sp->parserinfo->in_loop_counter++;
}


/**
 * Leave a loop.
 */
void parser_loop_leave(SaffireParser *sp, int lineno) {
    // Not possible to leave a loop when we aren't inside any
    if (sp->parserinfo->in_loop_counter <= 0) {
        parser_error(sp, lineno, "Somehow, we are trying to leave a loop from the outer scope");
        return;
    }

    // Decrease loop counter, since we are going down one loop
    sp->parserinfo->in_loop_counter--;
}


/**
 * Begin switch statement
 */
void parser_switch_begin(SaffireParser *sp, int lineno) {
    // Allocate switch structure
    struct _pi_switch *ss = (struct _pi_switch *)smm_malloc(sizeof(struct _pi_switch));

    // Set default values
    ss->has_default = 0;

    // Add the current switch as the parent
    ss->parent = sp->parserinfo->current_switch;

    // Set current switch to the new switch
    sp->parserinfo->current_switch = ss;
}


/**
 * End switch statement
 */
void parser_switch_end(SaffireParser *sp, int lineno) {
    struct _pi_switch *ss = sp->parserinfo->current_switch;

    // set current to the parent
    sp->parserinfo->current_switch = ss->parent;

    // Free switch structure
    smm_free(ss);
}


/**
 * Check case label
 */
void parser_switch_case(SaffireParser *sp, int lineno) {
}


/**
 * Check if a default label is valid
 */
void parser_switch_default(SaffireParser *sp, int lineno) {
    struct _pi_switch *ss = sp->parserinfo->current_switch;

    if (ss == NULL) {
        parser_error(sp, lineno, "Default label expected inside a switch statement");
        return;
    }

    // Check if default already exists
    if (ss->has_default) {
        parser_error(sp, lineno, "default label already supplied");
        return;
    }

    ss->has_default = 1;
}


/**
 * Make sure a label is not a variable
 */
void parser_check_label(SaffireParser *sp, int lineno, const char *name) {
    //struct yyguts_t *yyg = (struct yyguts_t *)scanner;
    // @TODO: We need to do a check on labels, to see if the current block has already defined the label
}


/**
 * Make sure return happens inside a method
 *
 */
void parser_validate_return(SaffireParser *sp, int lineno) {
    if (sp->parserinfo->in_method == 0) {
        parser_error(sp, lineno, "Cannot use return outside a method");
        return;
    }
}


/**
 * Make sure break happens inside a loop
 *
 */
void parser_validate_break(SaffireParser *sp, int lineno) {
    if (sp->parserinfo->in_loop_counter == 0) {
        parser_error(sp, lineno, "We can only break inside a loop");
        return;
    }
}

void parser_check_permitted_identifiers(SaffireParser *sp, int lineno, const char *name) {
    if (! strcmp(name, "null")) return;
    if (! strcmp(name, "false")) return;
    if (! strcmp(name, "true")) return;

    parser_error(sp, lineno, "Incorrect identifier: '%s'", name);
    return;
}



/**
 * Make sure continue happens inside a loop
 *
 */
void parser_validate_continue(SaffireParser *sp, int lineno) {
    if (sp->parserinfo->in_loop_counter == 0) {
        parser_error(sp, lineno, "We can only continue inside a loop");
        return;
    }
}


/**
 * Make sure breakelse happens inside a loop
 *
 */
void parser_validate_breakelse(SaffireParser *sp, int lineno) {
    if (sp->parserinfo->in_loop_counter == 0) {
        parser_error(sp, lineno, "We can only breakelse inside a loop");
        return;
    }
}



/**
 * Initialize the parser table with parser state information
 */
t_parserinfo *alloc_parserinfo() {
    // Allocate table memory
    t_parserinfo *pi = (t_parserinfo *)smm_malloc(sizeof(t_parserinfo));

    // Initialize table
    pi->constants = ht_create();
    pi->classes = ht_create();
    pi->active_class = NULL;

    pi->in_class = 0;
    pi->in_method = 0;
    pi->in_loop_counter = 0;

    pi->switches = NULL;
    pi->current_switch = NULL;

    return pi;
}

void free_parserinfo(t_parserinfo *pi) {
    // Iterate over classes and remove all info
    t_hash_table *ht = pi->classes;

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
    ht_destroy(pi->constants);
    ht_destroy(pi->classes);

    // Free actual global table
    smm_free(pi);
}


/**
 * Ouputs error (to stderr) and exists with code.
 */
void parser_error(SaffireParser *sp, int lineno, const char *format, ...) {
    va_list args;

    char errorbuf[2048];
    va_start(args, format);
    snprintf(errorbuf, 2047, format, args);
    va_end(args);

    warning("%s, found in %s on line %d", errorbuf, sp->filename, lineno);
    //warning("Parser error: %s\n", errorbuf);

    // Flush current buffer, and return when we are in interactive/REPL mode.
    if (sp->mode == SAFFIRE_EXECMODE_REPL) {
        //flush_buffer(scanner);
        return;
    }

    // Otherwise, exit.
    exit(1);
}
