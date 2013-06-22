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
#ifndef __DEBUG_H__
#define __DEBUG_H__

    #include <stdio.h>

    #ifdef __DEBUG
        #define DEBUG_PRINT output
    #else
        #define DEBUG_PRINT(format, args...) ((void)0)
    #endif


    #ifdef __DEBUG
        #define ANSI_BRIGHTRED    "\33[31;1m"
        #define ANSI_BRIGHTGREEN  "\33[32;1m"
        #define ANSI_BRIGHTYELLOW "\33[33;1m"
        #define ANSI_BRIGHTBLUE   "\33[34;1m"
        #define ANSI_RESET        "\33[0m"
    #else
        #define ANSI_BRIGHTRED
        #define ANSI_BRIGHTGREEN
        #define ANSI_BRIGHTYELLOW
        #define ANSI_BRIGHTBLUE
        #define ANSI_RESET
    #endif

#endif

