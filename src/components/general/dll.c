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
#include "general/dll.h"
#include "general/smm.h"


/**
 * Initialize an empty DLL
 */
t_dll *dll_init(void) {
    t_dll *dll = (t_dll *)smm_malloc(sizeof(t_dll));

    dll->size = 0;
    dll->head = NULL;
    dll->tail = NULL;
    return dll;
}

/**
 * Free a DLL (assumes data in elements are already freed)
 */
void dll_free(t_dll *dll) {
    t_dll_element *cur = dll->head;

    while (cur != NULL) {
        t_dll_element *e = cur;
        cur = cur->next;
        smm_free(e);
    }
    smm_free(dll);
}


/**
 * Append an element before the head of the DLL.
 */
t_dll_element *dll_prepend(t_dll *dll, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // Correct pointer of the previous tail element
    if (dll->tail == NULL) {
        dll->tail = e;
    }

    // Set head element data
    e->prev = NULL;
    e->next = dll->head;
    e->data = data;

    
    if (dll->head != NULL) {
        dll->head->prev = e;
    }
    dll->head = e;

    // Set dll data

    dll->size++;

    return e;
}

/**
 * Append an element after the tail of the DLL.
 */
t_dll_element *dll_append(t_dll *dll, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // Correct pointer of previous tail element
    if (dll->head == NULL) {
        dll->head = e;
    }
    if (dll->tail == NULL) {
        dll->tail = e;
    } else {
        dll->tail->next = e;
    }

    // Set tail element data
    e->prev = dll->tail;
    e->next = NULL;
    e->data = data;

    // Set dll data
    dll->tail = e;
    dll->size++;

    return e;
}


/**
 * Insert an element before the specified element. There is no check if the specified
 * element is already present in the DLL.
 */
t_dll_element *dll_insert_before(t_dll *dll, t_dll_element *before_element, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // No element given, just prepend
    if (before_element == NULL) {
        return dll_prepend(dll, data);
    }

    e->next = before_element;
    e->data = data;

    before_element->prev = e;

    dll->size++;
    if (before_element == dll->head) {
        dll->head = e;
    }

    return e;
}

/**
 * Insert an element after the specified element. There is no check if the specified
 * element is already present in the DLL.
 */
t_dll_element *dll_insert_after(t_dll *dll, t_dll_element *after_element, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // No element given, just prepend
    if (after_element == NULL) {
        return dll_append(dll, data);
    }

    e->prev = after_element;
    e->data = data;

    after_element->next = e;

    dll->size++;
    if (after_element == dll->tail) {
        dll->tail = e;
    }

    return e;
}


/**
 * Remove an element from the linked list
 */
int dll_remove(t_dll *dll, t_dll_element *element) {
    t_dll_element *tmp;

    if (element == dll->head) {
        dll->head = dll->head->next;
    }
    if (element == dll->tail) {
        dll->tail = dll->tail->prev;
    }

    dll->size--;

    tmp = element->next;
    element->next = element->prev;
    element->prev = tmp;

    return 1;
}

/**
 * Pushes data at the tail of the DLL
 */
void dll_push(t_dll *dll, void *data) {
    dll_insert_after(dll, DLL_TAIL(dll), data);
}

/**
 * Pops data at the tail of the DLL
 */
void *dll_pop(t_dll *dll) {
    t_dll_element *e = DLL_TAIL(dll);
     if (!e) return NULL;

     void *ret = e->data;
     dll_remove(dll, e);
     return ret;
}

/*
 * Peeks at data at the tail of the DLL
 */
void *dll_top(t_dll *dll) {
    t_dll_element *e = DLL_TAIL(dll);
     if (!e) return NULL;

     return e->data;
}
