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
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __LINUX__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <saffire/modules/module_api.h>
#include <saffire/modules/standard/os.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/memory/smm.h>
#include <saffire/vm/vm.h>

/**
 *
 */
SAFFIRE_MODULE_METHOD(os, stat) {
    t_string *path;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &path) != 0) {
        return NULL;
    }

    struct stat sb;

    int ret = stat(STRING_CHAR0(path), &sb);
    if (ret == -1) {
        object_raise_exception(Object_FileNotFoundException, 1, "Cannot stat file '%s'", STRING_CHAR0(path));

        RETURN_NULL;
    }

    t_stat_object *stat_obj = (t_stat_object *)object_alloc_instance(Object_Stat, 0);
    memcpy(&stat_obj->data.sb, &sb, sizeof(struct stat));

    RETURN_OBJECT(stat_obj);
}

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

SAFFIRE_MODULE_METHOD(os, usleep) {
    long delay;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &delay) != 0) {
        return NULL;
    }

    if (delay < 0) {
        delay = 0;
    }

    usleep(delay);

    RETURN_SELF;
}

SAFFIRE_MODULE_METHOD(os, realpath) {
    t_string *path;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &path) != 0) {
        return NULL;
    }

    char *rpath = smm_malloc(PATH_MAX);
    realpath(STRING_CHAR0(path), rpath);

    t_string_object *str_obj = (t_string_object *)STR02OBJ(rpath);
    smm_free(rpath);

    RETURN_OBJECT(str_obj);
}


/* ====================================================================================================== */
#define stat_func1(__method__, __property__) \
    SAFFIRE_MODULE_METHOD(stat, __method__) { \
        t_stat_object *stat_obj = (t_stat_object *)self; \
        RETURN_NUMERICAL(stat_obj->data.sb.st_##__property__); \
    }

#define stat_func2(__method__) \
    SAFFIRE_MODULE_METHOD(stat, __method__) { \
        t_stat_object *stat_obj = (t_stat_object *)self; \
        if (__method__(stat_obj->data.sb.st_mode)) { \
            RETURN_TRUE; \
        } \
        RETURN_FALSE; \
    }


stat_func1(dev, dev)
stat_func1(inode, ino)
stat_func1(links, nlink)
stat_func1(mode, mode)
stat_func1(uid, uid)
stat_func1(gid, gid)
stat_func1(rdev, rdev)
stat_func1(size, size)
stat_func1(blocksize, blksize)
stat_func1(blocks, blocks)
stat_func1(atime, atime)
stat_func1(ctime, ctime)
stat_func1(mtime, mtime)

stat_func2(S_ISREG)
stat_func2(S_ISDIR)
stat_func2(S_ISCHR)
stat_func2(S_ISBLK)
stat_func2(S_ISFIFO)
stat_func2(S_ISLNK)
stat_func2(S_ISSOCK)


static void obj_free(t_object *obj) {
    t_stat_object *stat_obj = (t_stat_object *)obj;
    if (! stat_obj) return;

    // Nothing need to be freed
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_stat_object *stat_obj = (t_stat_object *)obj;

    // Store debug info at the end of an object. There is space allocated for this purpose in debug mode
    snprintf(stat_obj->__debug_info, DEBUG_INFO_SIZE-1, "stat[%d]", stat_obj->data.sb.st_mode);
    return stat_obj->__debug_info;
}
#endif


// List object management functions
t_object_funcs stat_funcs = {
        NULL,                 // Populate a stat object
        obj_free,             // Free a stat object
        obj_destroy,          // Destroy a stat object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug
#endif
};

t_stat_object Object_Stat_struct = {
    OBJECT_HEAD_INIT("stat", objectTypeBase, OBJECT_TYPE_CLASS, &stat_funcs, sizeof(t_stat_object_data)),
    {
        { 0 }, /* sb */
    },
    OBJECT_FOOTER
};

t_object os_struct = { OBJECT_HEAD_INIT("os", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };

static void _init(void) {
    os_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&os_struct, "cwd",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_os_method_cwd);
    object_add_internal_method((t_object *)&os_struct, "usleep",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_os_method_usleep);
    object_add_internal_method((t_object *)&os_struct, "realpath",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_os_method_realpath);
    object_add_internal_method((t_object *)&os_struct, "stat",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_os_method_stat);

    Object_Stat_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_Stat_struct, "dev",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_dev);
    object_add_internal_method((t_object *)&Object_Stat_struct, "inode",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_inode);
    object_add_internal_method((t_object *)&Object_Stat_struct, "links",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_links);
    object_add_internal_method((t_object *)&Object_Stat_struct, "mode",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_mode);
    object_add_internal_method((t_object *)&Object_Stat_struct, "uid",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_uid);
    object_add_internal_method((t_object *)&Object_Stat_struct, "gid",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_gid);
    object_add_internal_method((t_object *)&Object_Stat_struct, "rdev",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_rdev);
    object_add_internal_method((t_object *)&Object_Stat_struct, "size",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_size);
    object_add_internal_method((t_object *)&Object_Stat_struct, "blocksize",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_blocksize);
    object_add_internal_method((t_object *)&Object_Stat_struct, "blocks",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_blocks);
    object_add_internal_method((t_object *)&Object_Stat_struct, "atime",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_atime);
    object_add_internal_method((t_object *)&Object_Stat_struct, "ctime",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_ctime);
    object_add_internal_method((t_object *)&Object_Stat_struct, "mtime",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_mtime);

    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISREG",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISREG);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISDIR",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISDIR);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISCHR",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISCHR);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISBLK",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISBLK);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISFIFO",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISFIFO);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISLNK",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISLNK);
    object_add_internal_method((t_object *)&Object_Stat_struct, "S_ISSOCK",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_stat_method_S_ISSOCK);

    object_add_constant((t_object *)&Object_Stat_struct, "S_IFDIR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFDIR));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFCHR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFCHR));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFBLK",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFBLK));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFREG",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFREG));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFIFO",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFIFO));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFLNK",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFLNK));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IFSOCK",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IFSOCK));

    object_add_constant((t_object *)&Object_Stat_struct, "S_ISUID",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_ISUID));
    object_add_constant((t_object *)&Object_Stat_struct, "S_ISGID",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_ISGID));
    object_add_constant((t_object *)&Object_Stat_struct, "S_ISVTX",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_ISVTX));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IRWXU",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IRWXU));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IRUSR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IRUSR));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IWUSR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IWUSR));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IXUSR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IXUSR));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IRWXG",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IRWXG));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IRGRP",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IRGRP));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IWGRP",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IWGRP));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IXGRP",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IXGRP));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IRWXO",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IRWXO));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IROTH",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IROTH));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IWOTH",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IWOTH));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IXOTH",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IXOTH));
// @TODO: HIGH: we should always define constants in Saffire. If not supported by OS, it should just be null
#ifdef S_ENFMT
    object_add_constant((t_object *)&Object_Stat_struct, "S_ENFMT",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_ENFMT));
#endif
    object_add_constant((t_object *)&Object_Stat_struct, "S_IREAD",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IREAD));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IWRITE",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IWRITE));
    object_add_constant((t_object *)&Object_Stat_struct, "S_IEXEC",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(S_IEXEC));

// @TODO: HIGH: we should always define constants in Saffire. If not supported by OS, it should just be null
#ifdef __APPLE__
    object_add_constant((t_object *)&Object_Stat_struct, "UF_NODUMP",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_NODUMP));
    object_add_constant((t_object *)&Object_Stat_struct, "UF_IMMUTABLE",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_IMMUTABLE));
    object_add_constant((t_object *)&Object_Stat_struct, "UF_APPEND",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_APPEND));
    object_add_constant((t_object *)&Object_Stat_struct, "UF_OPAQUE",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_OPAQUE));

// @TODO: HIGH: we should always define constants in Saffire. If not supported by OS, it should just be null
#ifdef UF_NOUNLINK
    object_add_constant((t_object *)&Object_Stat_struct, "UF_NOUNLINK",    ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_NOUNLINK));
#endif
    object_add_constant((t_object *)&Object_Stat_struct, "UF_COMPRESSED",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_COMPRESSED));
    object_add_constant((t_object *)&Object_Stat_struct, "UF_HIDDEN",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(UF_HIDDEN));

    object_add_constant((t_object *)&Object_Stat_struct, "SF_ARCHIVED",    ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SF_ARCHIVED));
    object_add_constant((t_object *)&Object_Stat_struct, "SF_IMMUTABLE",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SF_IMMUTABLE));
    object_add_constant((t_object *)&Object_Stat_struct, "SF_APPEND",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SF_APPEND));
#endif

// @TODO: HIGH: we should always define constants in Saffire. If not supported by OS, it should just be null
#ifdef SF_NOUNLINK
    object_add_constant((t_object *)&Object_Stat_struct, "SF_NOUNLINK",    ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SF_NOUNLINK));
#endif
// @TODO: HIGH: we should always define constants in Saffire. If not supported by OS, it should just be null
#ifdef SF_SNAPSHOT
    object_add_constant((t_object *)&Object_Stat_struct, "SF_SNAPSHOT",    ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SF_SNAPSHOT));
#endif
}

static void _fini(void) {
    object_free_internal_object((t_object *) &os_struct);
    object_free_internal_object((t_object *) &Object_Stat_struct);
}


static t_object *_objects[] = {
    (t_object *) &os_struct,
    (t_object *) &Object_Stat_struct,
    NULL
};

t_module module_os = {
    "\\saffire",
    "OS module",
    _objects,
    _init,
    _fini,
};
