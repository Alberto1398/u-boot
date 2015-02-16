/* The standard console is too bug to fit in SRAM, so we have this. */

#include <common.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/afinfo.h>

#include "tiny_printf/tiny_printf.h"

DECLARE_GLOBAL_DATA_PTR;

static struct serial_device *sp_default_serial_dev;


void act_spl_serial_init(void)
{
    sp_default_serial_dev = default_serial_console();
    sp_default_serial_dev->start();
    gd->have_console = 1;
    puts("......\n\nSPL print okay\n");
}

void putc(const char c)
{
    if (!gd->have_console)
        return;
    sp_default_serial_dev->putc(c);
}

void puts(const char *s)
{
    if (!gd->have_console)
        return;
    sp_default_serial_dev->puts(s);
}

static void _tpf_put_cb(void *p_udata, uint c)
{
    sp_default_serial_dev->putc(c);
}

int printf(const char *fmt, ...)
{
    va_list args;
    if (!gd->have_console)
        return 0;
    va_start(args, fmt);
    tfp_format(0, _tpf_put_cb, fmt, args);
    va_end(args);
    return 0; /* not valid return for this lite version */
}

int vprintf(const char *fmt, va_list args)
{
    if (!gd->have_console)
        return 0;
    tfp_format(0, _tpf_put_cb, fmt, args);
    return 0;
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    return tfp_vsprintf(buf, fmt, args);
}

int ctrlc(void)
{
    return 0;
}

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#if defined(CONFIG_PANIC_HANG)
    hang();
#endif
    while(1);
}

