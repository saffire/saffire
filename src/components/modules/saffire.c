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
#include "general/output.h"
#include "modules/module_api.h"
#include "objects/object.h"
#include "objects/null.h"
#include "objects/attrib.h"
#include "objects/callable.h"
#include "objects/string.h"
#include "general/dll.h"
#include "version.h"
#include "vm/vm.h"

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, version) {
    RETURN_STRING(saffire_version);
}

SAFFIRE_MODULE_METHOD(io, gitrev) {
    RETURN_STRING(git_revision);
}

SAFFIRE_MODULE_METHOD(io, runmode) {
    if (vm_mode == VM_FASTCGI) {
        RETURN_STRING("fastcgi");
    }
    if (vm_mode == VM_CLI) {
        RETURN_STRING("cli");
    }
    if (vm_mode == VM_REPL) {
        RETURN_STRING("repl");
    }

    RETURN_STRING("unknown");
}




t_object saffire_struct = { OBJECT_HEAD_INIT2("saffire", objectTypeAny, NULL, NULL, OBJECT_TYPE_INSTANCE, NULL) };

static void _init(void) {
    saffire_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&saffire_struct, "version",      CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_version);
    object_add_internal_method((t_object *)&saffire_struct, "git_revision", CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_gitrev);
    object_add_internal_method((t_object *)&saffire_struct, "run_mode",     CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_runmode);

    object_add_property((t_object *)&saffire_struct, "fastcgi",    ATTRIB_VISIBILITY_PUBLIC, Object_Null);
    object_add_property((t_object *)&saffire_struct, "cli",        ATTRIB_VISIBILITY_PUBLIC, Object_Null);
    object_add_property((t_object *)&saffire_struct, "repl",       ATTRIB_VISIBILITY_PUBLIC, Object_Null);
}

static void _fini(void) {
    // Destroy methods and properties
    ht_destroy(saffire_struct.attributes);
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
