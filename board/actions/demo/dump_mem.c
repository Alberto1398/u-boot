

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

#define PRINT   printf

/* this function is used by some external library (*.a)
 * we redirect it to the print_buffer() function */
void dump_mem(void *startaddr, unsigned int size, unsigned int showaddr, unsigned int show_bytes)
{
    if ((show_bytes != 1) && (show_bytes != 2) && (show_bytes != 4))
    {
        puts("dump_mem: not support mode\n");
        return;
    }

    if (((unsigned int)startaddr & (show_bytes -1)) ||((unsigned int)size & (show_bytes -1)))
    {
        PRINT("dump_mem: startaddr must be align by %d bytes!\n", show_bytes);
        return;
    }

    PRINT("dump_mem: startaddr=0x%x showaddr=0x%x size=%u\n",
        (unsigned int)startaddr, showaddr, size);

    print_buffer(showaddr, startaddr, show_bytes, size/show_bytes, 0);
}
