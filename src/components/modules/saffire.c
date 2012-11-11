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
#include "modules/module_api.h"
#include "objects/object.h"
#include "objects/method.h"
#include "objects/string.h"
#include "general/dll.h"
#include "version.h"

/**
 *
 */
static t_object *saffire_return_version(t_object *self, t_dll *args) {
    RETURN_STRING(saffire_version_wide);
}


t_object saffire_struct       = { OBJECT_HEAD_INIT2("saffire", objectTypeCustom, NULL, NULL, OBJECT_TYPE_INSTANCE, NULL) };

static void _init(void) {
    saffire_struct.methods = ht_create();
    ht_add(saffire_struct.methods, "version", saffire_return_version);
    saffire_struct.properties = ht_create();
}
static void _fini(void) {
    // Destroy methods and properties
    ht_destroy(saffire_struct.methods);
    ht_destroy(saffire_struct.properties);
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
