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

#include <saffire/modules/module_api.h>
#include <saffire/modules/standard/cpu.h>
#include <saffire/objects/object.h>

#include <saffire/config.h>
#include "libcpuid/libcpuid/libcpuid.h"


static int cpu_data_initialized = 0;
static struct cpu_raw_data_t cpu_raw;
static struct cpu_id_t cpu_data;

/*
 * Fetches (cached) cpu id structure
 */
static struct cpu_id_t *fetch_cpu_info(void) {
    if (cpu_data_initialized) {
        return &cpu_data;
    }

    cpuid_set_warn_function(NULL);

    int res = cpuid_get_raw_data(&cpu_raw);
    if (res < 0) {
        return NULL;
    }

    if (cpu_identify(&cpu_raw, &cpu_data) < 0) {
        return NULL;
    }

    cpu_data_initialized = 1;

    return &cpu_data;
}


/**
 * Return CPU vendor string
 */
SAFFIRE_MODULE_METHOD(cpu, vendor) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_STRING("");
    }

    RETURN_STRING_FROM_CHAR(cpu_data->vendor_str);
}

SAFFIRE_MODULE_METHOD(cpu, vendor_id) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->vendor);
}


/**
 * Return CPU brand
 */
SAFFIRE_MODULE_METHOD(cpu, brand) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_STRING("");
    }

    RETURN_STRING_FROM_CHAR(cpu_data->brand_str);
}


SAFFIRE_MODULE_METHOD(cpu, family) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->family);
}

SAFFIRE_MODULE_METHOD(cpu, model) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->model);
}

SAFFIRE_MODULE_METHOD(cpu, stepping) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->stepping);
}

SAFFIRE_MODULE_METHOD(cpu, extended_family) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->ext_family);
}

SAFFIRE_MODULE_METHOD(cpu, extended_model) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->ext_model);
}

SAFFIRE_MODULE_METHOD(cpu, codename) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_STRING("");
    }

    RETURN_STRING_FROM_CHAR(cpu_data->cpu_codename);
}

SAFFIRE_MODULE_METHOD(cpu, cores) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->num_cores);
}

SAFFIRE_MODULE_METHOD(cpu, logical_cpu_count) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->num_logical_cpus);
}

SAFFIRE_MODULE_METHOD(cpu, total_logical_cpu_count) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
           RETURN_NUMERICAL(0);
    }

    RETURN_NUMERICAL(cpu_data->total_logical_cpus);
}

SAFFIRE_MODULE_METHOD(cpu, flags) {
    struct cpu_id_t *cpu_data = fetch_cpu_info();

    if (cpu_data == NULL) {
        // @TODO: MEDIUM: Throw exception ?
        RETURN_EMPTY_LIST();
    }


    t_hash_table *flags = ht_create();
    for (long i=0; i < NUM_CPU_FEATURES; i++) {
        if (cpu_data->flags[i]) {
            ht_append_num(flags, STR02OBJ(cpu_feature_str(i)));
        }
    }
    RETURN_LIST(flags);
}



t_object cpu_struct = { OBJECT_HEAD_INIT("cpu", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };


static void _init(void) {
    cpu_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&cpu_struct, "vendor",               ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_vendor);
    object_add_internal_method((t_object *)&cpu_struct, "vendorId",               ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_vendor_id);
    object_add_internal_method((t_object *)&cpu_struct, "brand",                ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_brand);
    object_add_internal_method((t_object *)&cpu_struct, "flags",                ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_flags);
    object_add_internal_method((t_object *)&cpu_struct, "family",               ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_family);
    object_add_internal_method((t_object *)&cpu_struct, "extendedFamily",       ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_extended_family);
    object_add_internal_method((t_object *)&cpu_struct, "model",                ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_model);
    object_add_internal_method((t_object *)&cpu_struct, "extendedModel",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_extended_model);
    object_add_internal_method((t_object *)&cpu_struct, "stepping",             ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_stepping);
    object_add_internal_method((t_object *)&cpu_struct, "cores",                ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_cores);
    object_add_internal_method((t_object *)&cpu_struct, "logicalCpuCount",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_logical_cpu_count);
    object_add_internal_method((t_object *)&cpu_struct, "totalLogicalCpuCount", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_total_logical_cpu_count);
    object_add_internal_method((t_object *)&cpu_struct, "codename",             ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_cpu_method_codename);
}

static void _fini(void) {
    object_free_internal_object(&cpu_struct);
}


static t_object *_objects[] = {
    &cpu_struct,
    NULL
};

t_module module_cpu = {
    "\\saffire",
    "Standard CPU module",
    _objects,
    _init,
    _fini,
};
