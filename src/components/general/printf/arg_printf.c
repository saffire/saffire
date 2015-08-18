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
#include <stdarg.h>
#include <string.h>
#include <saffire/general/printf.h>
#include <saffire/general/output.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/vm/vm.h>

/**
 *
 */
static long _get_long(t_dll_element **e) {
    t_object *obj = (t_object *)DLL_DATA_PTR((*e));

    if (! OBJECT_IS_NUMERICAL(obj)) {
        t_attrib_object *numerical_method = object_attrib_find(obj, "__numerical");
        obj = call_saffire_method(obj, numerical_method, 0);
    }

    (*e) = DLL_NEXT((*e));

    return ((t_numerical_object *)obj)->data.value;
}


/**
 *
 */
static t_string *_get_string(t_dll_element **e) {
    t_object *obj = DLL_DATA_PTR((*e));

    if (! OBJECT_IS_STRING(obj)) {
        t_attrib_object *string_method = object_attrib_find(obj, "__string");
        obj = call_saffire_method(obj, string_method, 0);
        if (! obj) return char0_to_string("");
    }

    (*e) = DLL_NEXT((*e));

    return ((t_string_object *)obj)->data.value;
}


/**
 *
 */
int arg_printf_string(FILE *f, t_string *fmt, t_dll *args, fnptr output) {
    unsigned flags, actual_wd, count, given_wd;
    unsigned char *where_char, buf[PR_BUFLEN];
    unsigned char state, radix;
    t_string *where = NULL;
    long num;
    t_dll_element *e = DLL_HEAD(args);
    char *fmt_char = STRING_CHAR0(fmt);

    state = flags = count = given_wd = 0;

/* begin scanning format specifier list */
    for (; fmt_char <= STRING_CHAR0(fmt) + STRING_LEN(fmt) - 1; fmt_char++)
    {
        switch(state)
        {
/* STATE 0: AWAITING % */
        case 0:
            if (*fmt_char != '%') {  /* not %... */
                output(f, *fmt_char);   /* ...just echo it */
                count++;
                break;
            }
/* found %, get next char and advance state to check if next char is a flag */
            state++;
            fmt_char++;
            /* FALL THROUGH */
/* STATE 1: AWAITING FLAGS (%-0) */
        case 1:
            if (*fmt_char == '%') {  /* %% */
                output(f, *fmt_char);
                count++;
                state = flags = given_wd = 0;
                break;
            }
            if (*fmt_char == '-') {
                if (flags & PR_LJ) { /* %-- is illegal */
                    state = flags = given_wd = 0;
                } else {
                    flags |= PR_LJ;
                }
                break;
            }
/* not a flag char: advance state to check if it's field width */
            state++;
/* check now for '%0...' */
            if (*fmt_char == '0') {
                flags |= PR_LZ;
                fmt_char++;
            }
            /* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
        case 2:
            if (*fmt_char >= '0' && *fmt_char <= '9') {
                given_wd = 10 * given_wd + (*fmt_char - '0');
                break;
            }
/* not field width: advance state to check if it's a modifier */
            state++;
            /* FALL THROUGH */
/* STATE 3: AWAITING MODIFIER CHARS (FNlh) */
        case 3:
            if (*fmt_char == 'F') {
                flags |= PR_FP;
                break;
            }
            if (*fmt_char == 'N') break;
            if (*fmt_char == 'l') {
                flags |= PR_32;
                break;
            }
            if (*fmt_char == 'h') {
                flags |= PR_16;
                break;
            }
/* not modifier: advance state to check if it's a conversion char */
            state++;
            /* FALL THROUGH */
/* STATE 4: AWAITING CONVERSION CHARS (Xxpndiuocs) */
        case 4:
            where_char = buf + PR_BUFLEN - 1;
            *where_char = '\0';
            switch (*fmt_char) {
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
                if (flags & PR_32) {
                    num = _get_long(&e);
                }
/* h=short=16 bits (signed or unsigned) */
                else if (flags & PR_16) {
                    if (flags & PR_SG) {
                        num = _get_long(&e);
                    } else {
                        num = _get_long(&e);
                    }
                } else {
/* no h nor l: sizeof(int) bits (signed or unsigned) */
                    if (flags & PR_SG) {
                        num = _get_long(&e);
                    } else {
                        num = _get_long(&e);
                    }
                }
/* take care of sign */
                if (flags & PR_SG) {
                    if (num < 0) {
                        flags |= PR_WS;
                        num = -num;
                    }
                }
/* convert binary to octal/decimal/hex ASCII
OK, I found my mistake. The math here is _always_ unsigned */
                do {
                    unsigned long temp;

                    temp = (unsigned long)num % radix;
                    where_char--;
                    if (temp < 10) {
                        *where_char = (unsigned char)(temp + '0');
                    } else if(flags & PR_CA) {
                        *where_char = (unsigned char)(temp - 10 + 'A');
                    } else {
                        *where_char = (unsigned char)(temp - 10 + 'a');
                    }
                    num = (unsigned long)num / radix;
                } while(num != 0);
                goto EMIT;
            case 'c':
/* disallow pad-left-with-zeroes for %c */
                flags &= ~PR_LZ;
                where_char--;
                t_string *tmp = _get_string(&e);
                *where_char = STRING_CHAR0(tmp)[0];
                actual_wd = 1;
                goto EMIT2;
            case 's':
/* disallow pad-left-with-zeroes for %s */
                flags &= ~PR_LZ;
                where = _get_string(&e);
                where_char = (unsigned char *)STRING_CHAR0(where);
EMIT:
                actual_wd = STRING_LEN(where);
                if (flags & PR_WS) actual_wd++;
/* if we pad left with ZEROES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) == (PR_WS | PR_LZ)) {
                    output(f, '-');
                    count++;
                }
/* pad on left with spaces or zeroes (for right justify) */
EMIT2:
                if ((flags & PR_LJ) == 0) {
                    while(given_wd > actual_wd) {
                        output(f, flags & PR_LZ ? '0' : ' ');
                        count++;
                        given_wd--;
                    }
                }
/* if we pad left with SPACES, do the sign now */
                if ((flags & (PR_WS | PR_LZ)) == PR_WS) {
                    output(f, '-');
                    count++;
                }
/* emit string/char/converted number */
                while (*where_char != '\0') {
                    output(f, *where_char++);
                    count++;
                }
/* pad on right with spaces (for left justify) */
                if (given_wd < actual_wd) {
                    given_wd = 0;
                } else {
                    given_wd -= actual_wd;
                }
                for (; given_wd; given_wd--) {
                    output(f, ' ');
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
