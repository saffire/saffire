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
#include <string.h>
#include "vm/frame.h"
#include "general/smm.h"

char *vm_context_get_path(char *context) {
    char *s = smm_strdup(context);

    char *c = strrchr(s, ':');
    if (c == NULL) return smm_strdup("");
    if ((s - c) == 0) return smm_strdup("");

    c--;
    if (*c == ':') *c = '\0';
    return s;
}


char *vm_context_get_class(char *context) {
    char *s = smm_strdup(context);

    char *c = strrchr(s, ':');
    if (c == NULL) return s;
    if ((s - c) == 0) return s;

    c++;
    return c;
}


void vm_context_set_context(t_vm_frame *frame, char *context) {
    if (frame->context) smm_free(frame->context);
    frame->context = smm_strdup(context);

    if (frame->context_path) smm_free(frame->context_path);
    frame->context_path = vm_context_get_path(context);

    if (frame->context_class) smm_free(frame->context_class);
    frame->context_class = vm_context_get_class(context);
}
