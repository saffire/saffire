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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "general/output.h"
#include "objects/object.h"
#include "objects/string.h"
#include "objects/numerical.h"

/**
 *
 */
static void _output(FILE *f, const char *format, va_list args) {
    if (args == NULL) {
        fprintf(f, format);
    } else {
        vfprintf(f, format, args);
    }
}

/**
 * Outputs to specified file
 */
void foutput(FILE *fp, const char *format, ...) {
    va_list args;

    va_start(args, format);
    _output(fp, format, args);
    va_end(args);
}

/**
 * Outputs (to stdout)
 */
void output(const char *format, ...) {
    va_list args;

    va_start(args, format);
    _output(stdout, format, args);
    va_end(args);
}


/**
 * Ouputs error (to stderr)
 */
void error(const char *format, ...) {
    va_list args;

    _output(stderr, "Error: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

}

/**
 * Ouputs error (to stderr) and exists with code.
 */
void error_and_die(int exitcode, const char *format, ...) {
    va_list args;

    _output(stderr, "Error: ", NULL);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);

    exit(exitcode);
}


/**
 * Ouputs error (to stderr) and exists with code.
 */
void line_error_and_die(int exitcode, int lineno, const char *format, ...) {
    va_list args;

    foutput(stderr, "Error in line %d: ", lineno);
    va_start(args, format);
    _output(stderr, format, args);
    va_end(args);
    _output(stderr, "\n", NULL);

    exit(exitcode);
}





// Flags user in processing format string
#define PR_LJ   0x01    // Left Justify
#define PR_CA   0x02    // Casing (A..F instead of a..f)
#define PR_SG   0x04    // Signed conversion (%d vs %u)
#define PR_32   0x08    // Long (32bit)
#define PR_16   0x10    // Short (16bit)
#define PR_WS   0x20    // PR_SG set and num < 0
#define PR_LZ   0x40    // Pad left with '0' instead of ' '
#define PR_FP   0x80    // Far pointers

#define PR_BUFLEN  16

typedef int (*fnptr)(char c, void **helper);

static long _get_long(t_dll_element **e) {
    t_object *obj = (t_object *)(*e)->data;

    if (! OBJECT_IS_NUMERICAL(obj)) {
        // Implied converesion to numerical
        t_object *obj2 = object_find_method(obj, "numerical");
        obj = object_call(obj, obj2, 0);
    }

    (*e) = DLL_NEXT((*e));

    return ((t_numerical_object *)obj)->value;
}
static unsigned char *_get_string(t_dll_element **e) {
    t_object *obj = (*e)->data;

    if (! OBJECT_IS_STRING(obj)) {
        // Implied converesion to string
        t_object *obj2 = object_find_method(obj, "string");
        obj = object_call(obj, obj2, 0);
    }

    (*e) = DLL_NEXT((*e));

    return (unsigned char *)((t_string_object *)obj)->value;
}

/**
 *
 */
static int _printf (const char *fmt, t_dll *args, fnptr fn, void *ptr) {
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
                fn(*fmt, &ptr);	/* ...just echo it */
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
                fn(*fmt, &ptr);
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
                    fn('-', &ptr);
                    count++;
                }
/* pad on left with spaces or zeroes (for right justify) */
EMIT2:				if((flags & PR_LJ) == 0)
                {
                    while(given_wd > actual_wd)
                    {
                        fn(flags & PR_LZ ?
                            '0' : ' ', &ptr);
                        count++;
                        given_wd--;
                    }
                }
/* if we pad left with SPACES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) == PR_WS)
                {
                    fn('-', &ptr);
                    count++;
                }
/* emit string/char/converted number */
                while(*where != '\0')
                {
                    fn(*where++, &ptr);
                    count++;
                }
/* pad on right with spaces (for left justify) */
                if(given_wd < actual_wd)
                    given_wd = 0;
                else given_wd -= actual_wd;
                for(; given_wd; given_wd--)
                {
                    fn(' ', &ptr);
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



/**
 *
 */
int output_helper (char c, void **ptr) {
    return fwrite(&c, 1, 1, stdout);
}


/**
 */
void output_printf(const char *format, t_dll *args) {
    _printf(format, args, output_helper, NULL);
}