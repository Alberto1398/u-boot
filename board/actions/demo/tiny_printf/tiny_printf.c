/*
 * Copyright (c) 2004,2012 Kustaa Nyholm / SpareTimeLabs
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * Neither the name of the Kustaa Nyholm or SpareTimeLabs nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */


/*tiny printf from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
the original version is for 8/16bit machine, here we've modified it to adapt 32bit machine
cls, 2013-5-15 */



#include <linux/types.h>
#include <stdarg.h>
#include "tiny_printf.h"

static PRINTF_PUTC_FUNC_t *s_stdout_putc_func;
static void* s_stdout_usr_data;

static void _ui2a(UINT num, UINT base, UINT is_signed, CHAR *pbuf)
{
    UINT div;

    if(is_signed != 0 && (SINT)num < 0)
    {
        num = -((SINT)num);
        *pbuf++ = '-';
    }

    div = 1;
    while (num / div >= base)
        div *= base;
    while (div != 0)
    {
        UINT digit = num / div;
        num %= div;
        div /= base;
        *pbuf++ = digit + (digit < 10U ? '0' : 'a'-10U);
    }
    *pbuf = 0; // zero ending.
}

#if 0
static SINT _a2d(UINT ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}
static CHAR _a2ui(CHAR ch, const CHAR **src, UINT base, UINT *nump)
{
    const CHAR *p = *src;
    UINT digit, num = 0;
    SINT ret;
    while ((ret = _a2d(ch)) >= 0)
    {
        digit = ret;
        if (digit > base)
            break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}
#endif

static inline CHAR _a2ui_10base(CHAR ch, const CHAR **src, UINT *nump)
{
    const CHAR *p = *src;
    UINT num = 0;
    while(ch >= '0' && ch <= '9')
    {
        num = num * 10U + (ch - '0');
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

static void _putchw(void *p_udata, PRINTF_PUTC_FUNC_t *pf_putc, UINT digits, UINT leading_fill_char, CHAR *pbuf)
{
    UINT ch;
    char *p = pbuf;
    while (*p++ && digits > 0)
        digits--;
    while (digits-- > 0)
        pf_putc(p_udata, leading_fill_char);
    while ((ch = *pbuf++))
        pf_putc(p_udata, ch);
}

void tfp_format(void *p_udata, PRINTF_PUTC_FUNC_t *pf_putc, const CHAR *fmt, va_list va)
{
    CHAR digit_buf[12]; /* max 11 chars for 2^32-1 */
    UINT ch;

    while ((ch = *(fmt++)))
    {
        if (ch != '%')
        {
            pf_putc(p_udata, ch);
        }
        else
        {
            UINT leading_fill_char = ' ';
            UINT digit_width = 0;

            ch = *(fmt++);
            if (ch == '0')
            {
                ch = *(fmt++);
                leading_fill_char = '0';
            }
            if (ch >= '0' && ch <= '9')
            {
                ch = _a2ui_10base(ch, &fmt, &digit_width);
            }
            if (ch=='l')
            {
                ch=*(fmt++); /* skip the 'long' flag */
            }

            switch (ch)
            {
            case 0:
                goto abort;
            case 'u':
                _ui2a(va_arg(va, UINT32), 10, FALSE, digit_buf);
                _putchw(p_udata, pf_putc, digit_width, leading_fill_char, digit_buf);
                break;
            case 'd':
                _ui2a(va_arg(va, INT32), 10, TRUE, digit_buf);
                _putchw(p_udata, pf_putc, digit_width, leading_fill_char, digit_buf);
                break;
            case 'x':
            case 'X':
                _ui2a(va_arg(va, UINT32), 16, FALSE, digit_buf);
                _putchw(p_udata, pf_putc, digit_width, leading_fill_char, digit_buf);
                break;
            case 'c':
                pf_putc(p_udata, va_arg(va, UINT32));
                break;
            case 's':
                _putchw(p_udata, pf_putc, digit_width, leading_fill_char, va_arg(va, CHAR*));
                break;
            case '%':
                pf_putc(p_udata, ch); // print the '%'
                break;
            default:
                break;
            }
        }
    }
    abort: ;
}

void tfp_printf_init(void *p_udata, PRINTF_PUTC_FUNC_t *pf_putc)
{
    s_stdout_putc_func = pf_putc;
    s_stdout_usr_data = p_udata;
}

void tfp_vprintf(const CHAR *fmt, va_list va)
{
    tfp_format(s_stdout_usr_data, s_stdout_putc_func, fmt, va);
}

void tfp_printf(const CHAR *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    tfp_format(s_stdout_usr_data, s_stdout_putc_func, fmt, va);
    va_end(va);
}

static void _putc_buf(void *p, UINT c)
{
    *(*((CHAR**)p))++ = c;
}

UINT tfp_vsprintf(CHAR *pbuf, const CHAR *fmt, va_list va)
{
    CHAR *pbuf_init = pbuf;

    tfp_format(&pbuf, _putc_buf, fmt, va);
    _putc_buf(&pbuf, 0);

    return (size_t)pbuf - (size_t)pbuf_init;
}

UINT tfp_sprintf(CHAR *pbuf, const CHAR *fmt, ...)
{
    CHAR *pbuf_init = pbuf;
    va_list va;

    va_start(va, fmt);
    tfp_format(&pbuf, _putc_buf, fmt, va);
    _putc_buf(&pbuf, 0);
    va_end(va);

    return (size_t)pbuf - (size_t)pbuf_init;
}
