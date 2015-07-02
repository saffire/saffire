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
#include <time.h>
#include <saffire/modules/module_api.h>
#include <saffire/modules/modules.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/general/smm.h>
#include <saffire/vm/vm.h>


/**
 *
 */
SAFFIRE_MODULE_METHOD(datetime, now) {
    t_datetime_object *datetime_obj = (t_datetime_object *)object_alloc_instance(self, 0);
    datetime_obj->data.time = time(NULL);

    RETURN_OBJECT(datetime_obj);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(datetime, format) {
    size_t buf_len = 256;
    int ret_len;

    t_string_object *format_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &format_obj)) {
        return NULL;
    }


    t_datetime_object *datetime_obj = (t_datetime_object *)self;
    struct tm tm = *localtime(&datetime_obj->data.time);

    char *buf = smm_malloc(buf_len);
    while ((ret_len = strftime(buf, buf_len, STROBJ2CHAR0(format_obj), &tm)) == buf_len || ret_len == 0) {
        // Increase buffer and redo strftime again
        buf_len *= 2;
        buf = smm_realloc(buf, buf_len);
    }

    if (ret_len && ret_len != buf_len) {
        t_string_object *ret = (t_string_object *)object_alloc_instance(Object_String, 2, ret_len, buf);
        smm_free(buf);
        RETURN_OBJECT(ret);
    }

    smm_free(buf);

    RETURN_FALSE;

}

/**
 *
 */
SAFFIRE_MODULE_METHOD(datetime, epoch) {
    t_datetime_object *datetime_obj = (t_datetime_object *)self;

    double t = difftime(datetime_obj->data.time, 0);

    RETURN_NUMERICAL((long)t);
}



#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_datetime_object *datetime_obj = (t_datetime_object *)obj;

    // If we CAN print debug info, we HAVE space for debug info. At the end of an object.

    // Store debug info at the end of an object. There is space allocated for this purpose in debug mode
    snprintf(datetime_obj->__debug_info, DEBUG_INFO_SIZE-1, "datetime[%ld]", datetime_obj->data.time);
    return datetime_obj->__debug_info;
}
#endif


// List object management functions
t_object_funcs datetime_funcs = {
        NULL,       // Populate a datetime object
        NULL,       // Free a datetime object
        NULL,       // Destroy a datetime object
        NULL,       // Clone
        NULL,       // Cache
        NULL,       // Hash
#ifdef __DEBUG
        obj_debug
#endif
};

t_object datetime_struct       = { OBJECT_HEAD_INIT("datetime", objectTypeBase, OBJECT_TYPE_CLASS, &datetime_funcs, sizeof(t_datetime_data)), OBJECT_FOOTER };

static void _init(void) {
    datetime_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&datetime_struct, "now",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_datetime_method_now);

    object_add_internal_method((t_object *)&datetime_struct, "format",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_datetime_method_format);
    object_add_internal_method((t_object *)&datetime_struct, "epoch",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_datetime_method_epoch);

    object_add_constant((t_object *)&datetime_struct, "RFC1123",   ATTRIB_VISIBILITY_PUBLIC, STR02OBJ("%a, %d %b %Y %H:%M:%S GMT"));
}

static void _fini(void) {
    object_free_internal_object(&datetime_struct);
}


static t_object *_objects[] = {
    &datetime_struct,
    NULL
};

t_module module_datetime = {
    "\\saffire",
    "Datetime module",
    _objects,
    _init,
    _fini,
};
