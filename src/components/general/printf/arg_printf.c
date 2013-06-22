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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "general/printf.h"
#include "general/output.h"
#include "objects/object.h"
#include "objects/objects.h"
#include "vm/vm.h"

/**
 *
 */
static long _get_long(t_dll_element **e) {
    t_object *obj = (t_object *)(*e)->data;

    if (! OBJECT_IS_NUMERICAL(obj)) {
        t_object *numerical_method = object_find_attribute(obj, "__numerical");
        obj = vm_object_call(obj, numerical_method, 0);
    }

    (*e) = DLL_NEXT((*e));

    return ((t_numerical_object *)obj)->value;
}


/**
 *
 */
static unsigned char *_get_string(t_dll_element **e) {
    t_object *obj = (*e)->data;

    if (! OBJECT_IS_STRING(obj)) {
        t_object *bool_method = object_find_attribute(obj, "__string");
        obj = vm_object_call(obj, bool_method, 0);
    }

    (*e) = DLL_NEXT((*e));

    return (unsigned char *)((t_string_object *)obj)->value;
}


/**
 *
 */
int arg_printf (const char *fmt, t_dll *args, fnptr output) {
    unsigned flags, actual_wd, count, given_wd;
    unsigned char *where, buf[PR_BUFLEN];
    unsigned char state, radix;
    long num;
    t_dll_element *e = DLL_HEAD(args);

    state = flags = count = given_wd = 0;
/* begin scanning format specifier list */
    for(; *fmt; fmt++)
    {
        switch(state)
        {
/* STATE 0: AWAITING % */
        case 0:
            if(*fmt != '%')	/* not %... */
            {
                output(*fmt);	/* ...just echo it */
                count++;
                break;
            }
/* found %, get next char and advance state to check if next char is a flag */
            state++;
            fmt++;
            /* FALL THROUGH */
/* STATE 1: AWAITING FLAGS (%-0) */
        case 1:
            if(*fmt == '%')	/* %% */
            {
                output(*fmt);
                count++;
                state = flags = given_wd = 0;
                break;
            }
            if(*fmt == '-')
            {
                if(flags & PR_LJ)/* %-- is illegal */
                    state = flags = given_wd = 0;
                else
                    flags |= PR_LJ;
                break;
            }
/* not a flag char: advance state to check if it's field width */
            state++;
/* check now for '%0...' */
            if(*fmt == '0')
            {
                flags |= PR_LZ;
                fmt++;
            }
            /* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
        case 2:
            if(*fmt >= '0' && *fmt <= '9')
            {
                given_wd = 10 * given_wd +
                    (*fmt - '0');
                break;
            }
/* not field width: advance state to check if it's a modifier */
            state++;
            /* FALL THROUGH */
/* STATE 3: AWAITING MODIFIER CHARS (FNlh) */
        case 3:
            if(*fmt == 'F')
            {
                flags |= PR_FP;
                break;
            }
            if(*fmt == 'N')
                break;
            if(*fmt == 'l')
            {
                flags |= PR_32;
                break;
            }
            if(*fmt == 'h')
            {
                flags |= PR_16;
                break;
            }
/* not modifier: advance state to check if it's a conversion char */
            state++;
            /* FALL THROUGH */
/* STATE 4: AWAITING CONVERSION CHARS (Xxpndiuocs) */
        case 4:
            where = buf + PR_BUFLEN - 1;
            *where = '\0';
            switch(*fmt)
            {
            case 'X':
                flags |= PR_CA;
                /* FALL THROUGH */
/* xxx - far pointers (%Fp, %Fn) not yet supported */
            case 'x':
            case 'p':
            case 'n':
                radix = 16;
                goto DO_NUM;
            case 'd':
            case 'i':
                flags |= PR_SG;
                /* FALL THROUGH */
            case 'u':
                radix = 10;
                goto DO_NUM;
            case 'o':
                radix = 8;
/* load the value to be printed. l=long=32 bits: */
DO_NUM:
                if(flags & PR_32) {
                    num = _get_long(&e);
                }
/* h=short=16 bits (signed or unsigned) */
                else if(flags & PR_16)
                {
                    if(flags & PR_SG)
                        num = _get_long(&e);
                    else
                        num = _get_long(&e);
                }
/* no h nor l: sizeof(int) bits (signed or unsigned) */
                else
                {
                    if(flags & PR_SG)
                        num = _get_long(&e);
                    else
                        num = _get_long(&e);
                }
/* take care of sign */
                if(flags & PR_SG)
                {
                    if(num < 0)
                    {
                        flags |= PR_WS;
                        num = -num;
                    }
                }
/* convert binary to octal/decimal/hex ASCII
OK, I found my mistake. The math here is _always_ unsigned */
                do
                {
                    unsigned long temp;

                    temp = (unsigned long)num % radix;
                    where--;
                    if(temp < 10)
                        *where = (unsigned char)(temp + '0');
                    else if(flags & PR_CA)
                        *where = (unsigned char)(temp - 10 + 'A');
                    else
                        *where = (unsigned char)(temp - 10 + 'a');
                    num = (unsigned long)num / radix;
                }
                while(num != 0);
                goto EMIT;
            case 'c':
/* disallow pad-left-with-zeroes for %c */
                flags &= ~PR_LZ;
                where--;
                unsigned char *tmp = _get_string(&e);
                *where = tmp[0];
                actual_wd = 1;
                goto EMIT2;
            case 's':
/* disallow pad-left-with-zeroes for %s */
                flags &= ~PR_LZ;
                where = _get_string(&e);
EMIT:
                actual_wd = (unsigned int)strlen((const char *)where);
                if(flags & PR_WS)
                    actual_wd++;
/* if we pad left with ZEROES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) ==
                    (PR_WS | PR_LZ))
                {
                    output('-');
                    count++;
                }
/* pad on left with spaces or zeroes (for right justify) */
EMIT2:				if((flags & PR_LJ) == 0)
                {
                    while(given_wd > actual_wd)
                    {
                        output(flags & PR_LZ ? '0' : ' ');
                        count++;
                        given_wd--;
                    }
                }
/* if we pad left with SPACES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) == PR_WS)
                {
                    output('-');
                    count++;
                }
/* emit string/char/converted number */
                while(*where != '\0')
                {
                    output(*where++);
                    count++;
                }
/* pad on right with spaces (for left justify) */
                if(given_wd < actual_wd)
                    given_wd = 0;
                else given_wd -= actual_wd;
                for(; given_wd; given_wd--)
                {
                    output(' ');
                    count++;
                }
                break;
            default:
                break;
            }
        default:
            state = flags = given_wd = 0;
            break;
        }
    }
    return count;
}
