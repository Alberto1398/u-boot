#include <common.h>
#include <vsprintf.h>
#include <stdarg.h>

extern long strtol(const char *nptr, char **endptr, int base);
extern unsigned long strtoul(const char *nptr, char **endptr, int base);

extern void dump_mem(void *startaddr, unsigned int size, unsigned int showaddr, unsigned int show_bytes);

