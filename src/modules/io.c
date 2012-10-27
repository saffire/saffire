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
#include <wchar.h>
#include "modules/module_api.h"
#include "object/object.h"
#include "object/string.h"
#include "general/dll.h"

/**
 *
 */
static t_object *io_print(t_object *self, t_dll *args) {
    printf("IO.self\n");
    RETURN_SELF;
}

/**
 *
 */
static t_object *io_printf(t_object *self, t_dll *args) {
    printf("IO.blaataap\n");
    RETURN_SELF;
}

/**
 *
 */
static t_object *io_sprintf(t_object *self, t_dll *args) {
    wchar_t tmp[] = L"IO.blaataap\n";
    RETURN_STRING(tmp);
}



/**
 *
 */
static t_object *console_print(t_object *self, t_dll *args) {
    printf("console.self\n");
    RETURN_SELF;
}

/**
 *
 */
static t_object *console_printf(t_object *self, t_dll *args) {
    printf("console.blaataap\n");
    RETURN_SELF;
}

/**
 *
 */
static t_object *console_sprintf(t_object *self, t_dll *args) {
    wchar_t tmp[] = L"console.blaataap\n";
    RETURN_STRING(tmp);
}



t_object io_struct       = { OBJECT_HEAD_INIT2("io", objectTypeCustom, NULL, NULL, OBJECT_NO_FLAGS, NULL) };
t_object console_struct  = { OBJECT_HEAD_INIT2("console", objectTypeCustom, NULL, NULL, OBJECT_NO_FLAGS, NULL) };


void io_init(void) {
    io_struct.methods = ht_create();
    ht_add(io_struct.methods, "print", io_print);
    ht_add(io_struct.methods, "printf", io_printf);
    ht_add(io_struct.methods, "sprintf", io_sprintf);
    io_struct.properties = ht_create();

    console_struct.methods = ht_create();
    ht_add(console_struct.methods, "print", console_print);
    ht_add(console_struct.methods, "printf", console_printf);
    ht_add(console_struct.methods, "sprintf", console_sprintf);
    console_struct.properties = ht_create();
}

void io_fini(void) {
    // Destroy methods and properties
    ht_destroy(io_struct.methods);
    ht_destroy(io_struct.properties);

    ht_destroy(console_struct.methods);
    ht_destroy(console_struct.properties);

}


t_object *io_objects[] = {
    &io_struct,
    &console_struct,
    NULL
};

t_module module_io = {
    "::_io",
    "Standard I/O module",
    io_objects,
    io_init,
    io_fini,
};
