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
#ifndef __VERSION_H__
#define __VERSION_H__

    #include "gitversion.h"
    #include "config.h"

    #define saffire_version_major   "0"
    #define saffire_version_minor   "0"
    #define saffire_version_build   "1"
    #define saffire_version_binary ((1 << 24) | (2 << 12) | (3 << 0))

    #define VERSION_MAJOR(v)    ((v >> 12) & 0x0004)
    #define VERSION_MINOR(v)    ((v >>  8) & 0x0004)
    #define VERSION_BUILD(v)    ((v >>  0) & 0x0008)

    #define saffire_version       "Saffire v" saffire_version_major "." saffire_version_minor "." saffire_version_build
    #define saffire_copyright "Copyright (C) 2012-2013 The Saffire Group"
    #define saffire_compiled  "Compiled on " __DATE__ " at " __TIME__ " from git revision " git_revision
    #define saffire_configured "Configured with: ./configure " CONFIGURE_ARGS


#endif

