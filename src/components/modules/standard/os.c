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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __LINUX__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <saffire/modules/module_api.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/general/smm.h>
#include <saffire/vm/vm.h>

/**
 *
 */
SAFFIRE_MODULE_METHOD(os, cwd) {
    char *path = smm_malloc(PATH_MAX);
    getcwd(path, PATH_MAX);

    t_string_object *str_obj = (t_string_object *)STR02OBJ(path);
    smm_free(path);

    RETURN_OBJECT(str_obj);
}


t_object os_struct       = { OBJECT_HEAD_INIT("os", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };

static void _init(void) {
    os_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&os_struct, "cwd",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_os_method_cwd);
}

static void _fini(void) {
    object_free_internal_object(&os_struct);
}


static t_object *_objects[] = {
    &os_struct,
    NULL
};

t_module module_os = {
    "\\saffire",
    "OS module",
    _objects,
    _init,
    _fini,
};
