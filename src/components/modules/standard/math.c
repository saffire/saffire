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
#include <time.h>
#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <saffire/modules/module_api.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/general/smm.h>
#include <saffire/vm/vm.h>


// Is the randomizer seeded or not
static char randomizer_initialized = 0;


// @TODO: cannot use 0 for seeding
static void _seed_randomizer(unsigned long seed) {
    if (seed == 0) {
#ifdef __APPLE__
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        seed = mts.tv_nsec;
#else
        struct timespec ct;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct);
        seed = ct.tv_nsec;
#endif
    }

    srand(seed);

    randomizer_initialized = 1;
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(math, random) {
    t_numerical_object *from_obj;
    t_numerical_object *to_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "nn", &from_obj, &to_obj)) {
        return NULL;
    }

    long from = OBJ2NUM(from_obj);
    long to = OBJ2NUM(to_obj);

    if (from >= to) {
        // Throw exception thatn to > from
    }

    if (randomizer_initialized == 0) {
        _seed_randomizer(0);
    }

    long r = rand() % to + from;

    RETURN_NUMERICAL(r);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(math, seed) {
    t_numerical_object *seed_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &seed_obj)) {
        return NULL;
    }

    long seed = OBJ2NUM(seed_obj);
    _seed_randomizer(seed);

    RETURN_SELF;
}


t_object math_struct       = { OBJECT_HEAD_INIT("math", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };

static void _init(void) {
    math_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&math_struct, "random",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_math_method_random);
    object_add_internal_method((t_object *)&math_struct, "seed",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_math_method_seed);
}

static void _fini(void) {
    object_free_internal_object(&math_struct);
}


static t_object *_objects[] = {
    &math_struct,
    NULL
};

t_module module_math = {
    "::saffire::math",
    "Math module",
    _objects,
    _init,
    _fini,
};
