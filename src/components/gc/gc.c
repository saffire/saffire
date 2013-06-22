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
#include "objects/object.h"
#include "objects/objects.h"
#include "gc/gc.h"
#include "general/mutex.h"
#include "general/smm.h"
#include "debug.h"
#include "general/output.h"

#define GC_OBJECT_QUEUE_SIZE        100;

typedef struct _gc_queue {
    t_mutex mutex;          // Mutex to guard the queue
    t_object **queue;       // Actual queue
    int index;              // Current first free item
    int size;               // Size of the queue
} t_gc_queue;

t_gc_queue gc_queue[OBJECT_TYPE_LEN];


/**
 * Do garbage collection
 */
void gc_collect(void) {
    DEBUG_PRINT("gc_collect()");
}


/**
 * recylces an object from the specified queue
 */
t_object *gc_queue_recycle(int type) {
    // Check bounds for type
    if (type < 0 || type >= OBJECT_TYPE_LEN) {
        return NULL;
    }

    // Lock mutex for this queue
    mutex_wait(&gc_queue[type].mutex);

    t_object *obj = NULL;
    if (gc_queue[type].index > 0) {
        gc_queue[type].index--;
        obj = gc_queue[type].queue[gc_queue[type].index];
    }

    // Unlock mutex
    mutex_post(&gc_queue[type].mutex);

    return obj;
}


/**
 * Try and adds object to the end of the queue. Returns 1 on success, 0 when queue is full
 */
int gc_queue_add(t_object *obj) {
    int ret;

    // @TODO: There is no sanity check to see if obj->type is between 0 and OBJECT_TYPE_LEN!

    // Lock mutex for this queue
    mutex_wait(&gc_queue[obj->type].mutex);

    // Add to recycle queue
    if (gc_queue[obj->type].index < gc_queue[obj->type].size) {
        gc_queue[obj->type].queue[gc_queue[obj->type].index] = obj;
        gc_queue[obj->type].index++;
        ret = 1;
    } else {
        ret = 0;
    }

    // Unlock mutex
    mutex_post(&gc_queue[obj->type].mutex);

    return ret;
}


/**
 *
 */
void gc_init(void) {
    // Create all queues
    for (int i=0; i!=OBJECT_TYPE_LEN; i++) {
        mutex_create(&gc_queue[i].mutex);
        gc_queue[i].size = GC_OBJECT_QUEUE_SIZE;

        gc_queue[i].queue = smm_malloc(sizeof(t_object *) * gc_queue[i].size);
        gc_queue[i].index = 0;
    }
}


/**
 *
 */
void gc_fini(void) {
    for (int i=0; i!=OBJECT_TYPE_LEN; i++) {
        mutex_destroy(&gc_queue[i].mutex);
        smm_free(gc_queue[i].queue);
    }
}
