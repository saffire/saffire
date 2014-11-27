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
#include "general/output.h"
#include "modules/module_api.h"
#include "general/string.h"
#include "objects/object.h"
#include "objects/objects.h"
#include "general/config.h"
#include "general/dll.h"
#include "version.h"
#include "vm/vm.h"
#include "vm/thread.h"
#include "general/smm.h"
#include "string.h"

SAFFIRE_MODULE_METHOD(saffire, get_locale) {
    t_thread *thread = thread_get_current();
    RETURN_STRING_FROM_CHAR(thread->locale);

}

SAFFIRE_MODULE_METHOD(saffire, set_locale) {
    t_string_object *locale_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &locale_obj)) {
        return NULL;
    }

    // Set locale
    t_thread *thread = thread_get_current();
    thread->locale = string_strdup0(STROBJ2CHAR0(locale_obj));

    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(saffire, version) {
    RETURN_STRING(saffire_version);
}

SAFFIRE_MODULE_METHOD(saffire, gitrev) {
    RETURN_STRING(__GIT_REVISION__);
}

SAFFIRE_MODULE_METHOD(saffire, sapi) {
    if ((vm_runmode & VM_RUNMODE_FASTCGI) == VM_RUNMODE_FASTCGI) {
        RETURN_STRING_FROM_CHAR("fastcgi");
    }
    if ((vm_runmode & VM_RUNMODE_CLI) == VM_RUNMODE_CLI) {
        RETURN_STRING_FROM_CHAR("cli");
    }
    if ((vm_runmode & VM_RUNMODE_REPL) == VM_RUNMODE_REPL) {
        RETURN_STRING_FROM_CHAR("repl");
    }

    RETURN_STRING_FROM_CHAR("unknown");
}

SAFFIRE_MODULE_METHOD(saffire, debug) {
    if ((vm_runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}


t_object saffire_struct = { OBJECT_HEAD_INIT("saffire", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0) };

static void _init(void) {
    saffire_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&saffire_struct, "version",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_version);
    object_add_internal_method((t_object *)&saffire_struct, "git_revision", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_gitrev);
    object_add_internal_method((t_object *)&saffire_struct, "sapi",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_sapi);
    object_add_internal_method((t_object *)&saffire_struct, "debug",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_debug);
    object_add_internal_method((t_object *)&saffire_struct, "set_locale",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_set_locale);
    object_add_internal_method((t_object *)&saffire_struct, "get_locale",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_get_locale);

    object_add_property((t_object *)&saffire_struct, "fastcgi",    ATTRIB_VISIBILITY_PUBLIC, object_alloc(Object_Null, 0));
    object_add_property((t_object *)&saffire_struct, "cli",        ATTRIB_VISIBILITY_PUBLIC, object_alloc(Object_Null, 0));
    object_add_property((t_object *)&saffire_struct, "repl",       ATTRIB_VISIBILITY_PUBLIC, object_alloc(Object_Null, 0));
}

static void _fini(void) {
    // Destroy methods and properties
    object_free_internal_object(&saffire_struct);
}

static t_object *_objects[] = {
    &saffire_struct,
    NULL
};

t_module module_saffire = {
    "::_sfl::saffire",
    "Saffire configuration module",
    _objects,
    _init,
    _fini
};
