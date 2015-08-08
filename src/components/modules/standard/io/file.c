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
#include <saffire/memory/smm.h>
#include <saffire/vm/vm.h>
#include <saffire/vm/thread.h>


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */

/* ======================================================================
 *   Object methods
 * ======================================================================
 */

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, open) {
    t_string_object *name_obj;
    t_string_object *mode_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "ss", &name_obj, &mode_obj)) {
        return NULL;
    }

    // Instantiate new file object

    t_file_object *file_obj = (t_file_object *)object_alloc_instance(self, 0);
    file_obj->data.fp = fopen(OBJ2STR0(name_obj), OBJ2STR0(mode_obj));

    if (file_obj->data.fp == NULL) {
        object_raise_exception(Object_FileNotFoundException, 1, "Cannot open file '%s'", OBJ2STR0(name_obj));
        return NULL;
    }

    file_obj->data.bytes_in = 0;
    file_obj->data.bytes_out = 0;
    file_obj->data.path = string_strdup0(OBJ2STR0(name_obj));
    file_obj->data.stat = NULL;

    RETURN_OBJECT(file_obj);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, fileno) {
    t_file_object *file_obj = (t_file_object *)self;
    RETURN_NUMERICAL(fileno(file_obj->data.fp));
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, close) {
    t_file_object *file_obj = (t_file_object *)self;

    if (file_obj->data.fp) {
        fclose(file_obj->data.fp);
        file_obj->data.fp = NULL;
    }

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, bytes_in) {
    t_file_object *file_obj = (t_file_object *)self;

    RETURN_NUMERICAL(file_obj->data.bytes_in);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, bytes_out) {
    t_file_object *file_obj = (t_file_object *)self;

    RETURN_NUMERICAL(file_obj->data.bytes_out);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, path) {
    t_file_object *file_obj = (t_file_object *)self;

    RETURN_STRING_FROM_CHAR(file_obj->data.path);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, stat) {
    t_file_object *file_obj = (t_file_object *)self;

    if (! file_obj->data.stat) {
        struct stat filestat;
        int ret = fstat(fileno(file_obj->data.fp), &filestat);

        if (ret < 0) {
            object_raise_exception(Object_FileNotFoundException, 1, "Cannot stat file '%s'", file_obj->data.path);
            return NULL;
        }

        // Create hash table
        file_obj->data.stat = ht_create();

        ht_add_obj(file_obj->data.stat, STR02OBJ("st_mode"), NUM2OBJ(filestat.st_mode));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_ino"), NUM2OBJ(filestat.st_ino));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_dev"), NUM2OBJ(filestat.st_dev));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_uid"), NUM2OBJ(filestat.st_uid));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_gid"), NUM2OBJ(filestat.st_gid));

        ht_add_obj(file_obj->data.stat, STR02OBJ("st_atime"), NUM2OBJ(filestat.st_atime));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_ctime"), NUM2OBJ(filestat.st_ctime));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_mtime"), NUM2OBJ(filestat.st_mtime));

        ht_add_obj(file_obj->data.stat, STR02OBJ("st_nlink"), NUM2OBJ(filestat.st_nlink));
        ht_add_obj(file_obj->data.stat, STR02OBJ("st_size"), NUM2OBJ(filestat.st_size));
    }

    RETURN_HASH(file_obj->data.stat);
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, tell) {
    t_file_object *file_obj = (t_file_object *)self;

    RETURN_NUMERICAL(ftell(file_obj->data.fp));
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, seek) {
    t_file_object *file_obj = (t_file_object *)self;

    t_numerical_object *offset_obj;
    t_numerical_object *origin_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "nn", &offset_obj, &origin_obj)) {
        return NULL;
    }

    long ret = fseek(file_obj->data.fp, OBJ2NUM(offset_obj), OBJ2NUM(origin_obj));

    RETURN_NUMERICAL(ret);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, read) {
    t_file_object *file_obj = (t_file_object *)self;

    t_numerical_object *size_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &size_obj)) {
        return NULL;
    }

    // Allocate buffer to hold maximum number of bytes
    long buflen = OBJ2NUM(size_obj);
    char *buf = (char *)smm_malloc(buflen);

    // Read bytes
    long bytes_read = fread(buf, 1, buflen, file_obj->data.fp);
    file_obj->data.bytes_in += bytes_read;

    // Resize buffer back when we have read less bytes than requested
    if (bytes_read < buflen) {
        buf = smm_realloc(buf, bytes_read);
    }

    RETURN_STRING_FROM_BINSAFE_CHAR(bytes_read, buf);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, write) {
    t_file_object *file_obj = (t_file_object *)self;

    t_string_object *str_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &str_obj)) {
        return NULL;
    }

    long bytes_written = fwrite(str_obj->data.value->val, 1, str_obj->data.value->len, file_obj->data.fp);
    file_obj->data.bytes_out += bytes_written;

    RETURN_NUMERICAL(bytes_written);
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(io_file, lines) {
    t_file_object *file_obj = (t_file_object *)self;

    t_hash_table *ht = ht_create();

    char *line;
    size_t len;
    ssize_t bytes_read = 1;
    while (bytes_read > 0) {
        line = NULL;
        len = 0;
        bytes_read = getline(&line, &len, file_obj->data.fp);
        file_obj->data.bytes_out += bytes_read;

        if (bytes_read > 0) {
            ht_add_num(ht, ht->element_count, STR02OBJ(line));
        }
        free(line);
    }

    RETURN_LIST(ht);
}

SAFFIRE_MODULE_METHOD(io_file, contents) {
    t_file_object *file_obj = (t_file_object *)self;

    struct stat filestat;
    fstat(fileno(file_obj->data.fp), &filestat);
    int size = filestat.st_size;

    long old_pos = fseek(file_obj->data.fp, 0L, SEEK_CUR);

    char *buf = smm_malloc(size);
    rewind(file_obj->data.fp);
    fread(buf, size, 1, file_obj->data.fp);

    fseek(file_obj->data.fp, old_pos, SEEK_SET);

    // TODO: Duplicates buffer. Use string_create_new_object() instead!
    t_string_object *s = (t_string_object *)object_alloc_instance(Object_String, 2, size, buf);
    smm_free(buf);
    RETURN_OBJECT(s);
}

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

static void _init(void) {
    Object_File_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_File_struct, "open",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_open);
    object_add_internal_method((t_object *)&Object_File_struct, "fileNo",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_fileno);
    object_add_internal_method((t_object *)&Object_File_struct, "close",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_close);

    object_add_internal_method((t_object *)&Object_File_struct, "tell",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_tell);
    object_add_internal_method((t_object *)&Object_File_struct, "seek",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_seek);
    object_add_internal_method((t_object *)&Object_File_struct, "read",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_read);
    object_add_internal_method((t_object *)&Object_File_struct, "write",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_write);

    object_add_internal_method((t_object *)&Object_File_struct, "lines",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_lines);
    object_add_internal_method((t_object *)&Object_File_struct, "contents",  ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_contents);

    object_add_internal_method((t_object *)&Object_File_struct, "path",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_path);
    object_add_internal_method((t_object *)&Object_File_struct, "stat",      ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_stat);

    object_add_internal_method((t_object *)&Object_File_struct, "bytesIn",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_bytes_in);
    object_add_internal_method((t_object *)&Object_File_struct, "bytesOut",  ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_file_method_bytes_out);

    object_add_constant((t_object *)&Object_File_struct, "SEEK_SET",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SEEK_SET));
    object_add_constant((t_object *)&Object_File_struct, "SEEK_CUR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SEEK_CUR));
    object_add_constant((t_object *)&Object_File_struct, "SEEK_END",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SEEK_END));
}

static void _fini(void) {
    object_free_internal_object((t_object *)&Object_File_struct);
}


static void obj_free(t_object *obj) {
    t_file_object *file_obj = (t_file_object *)obj;
    if (! file_obj) return;

    if (file_obj->data.fp) {
        fclose(file_obj->data.fp);
        file_obj->data.fp = NULL;
    }

    if (file_obj->data.path) {
        smm_free(file_obj->data.path);
    }

    if (file_obj->data.stat) {
        ht_destroy(file_obj->data.stat);
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_file_object *file_obj = (t_file_object *)obj;

    int fn = file_obj->data.fp ? fileno(file_obj->data.fp) : -1;

    // If we CAN print debug info, we HAVE space for debug info. At the end of an object.

    // Store debug info at the end of an object. There is space allocated for this purpose in debug mode
    snprintf(file_obj->__debug_info, DEBUG_INFO_SIZE-1, "file[%d]", fn);
    return file_obj->__debug_info;
}
#endif


// List object management functions
t_object_funcs file_funcs = {
        NULL,                 // Populate a file object
        obj_free,             // Free a file object
        obj_destroy,          // Destroy a file object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug
#endif
};


// Initial object
t_file_object Object_File_struct = {
    OBJECT_HEAD_INIT("file", objectTypeUser, OBJECT_TYPE_CLASS, &file_funcs, sizeof(t_file_object_data)),
    {
        NULL,   /* file resource handle */
        NULL,   /* path */
        0,      /* bytes in */
        0,      /* bytes out */
        NULL,   /* stat info */
    },
    OBJECT_FOOTER
};

/* ======================================================================
 *   Module definition
 * ======================================================================
 */

static t_object *_objects[] = {
    (t_object *)&Object_File_struct,
    NULL
};

t_module module_io_file = {
    "\\saffire\\io",
    "File module",
    _objects,
    _init,
    _fini,
};

