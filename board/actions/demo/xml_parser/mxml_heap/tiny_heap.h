
#ifndef __BOOTLIB_TINY_HEAP_H__
#define __BOOTLIB_TINY_HEAP_H__

#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>

#define UINT32  uint32_t
#define UINT16  uint16_t
#define UINT8   uint8_t
#define INT32   int32_t
#define INT16   int16_t
#define INT8    int8_t
#define UINT    uint
#define CHAR    char


// The OO version
void * tiny_heap_init(UINT32 heap_base, UINT32 heap_size);
void * tiny_heap_malloc(void *p_obj, UINT32 rq_size);
void tiny_heap_mfree(void *p_obj, void *pbuf);
uint32_t tiny_heap_get_alloc_size(void *p_obj, void *pbuf);
void tiny_heap_dump(void *p_obj);

void __attribute__((weak)) tiny_heap_cb_assert_halt(UINT32 line_id);
void __attribute__((weak)) tiny_heap_cb_node_chk_err(UINT32 hdr, UINT32 err_type);
void __attribute__((weak)) tiny_heap_cb_dump_node(UINT32 hdr, UINT32 prev, UINT32 next, UINT32 base, UINT32 size);

#endif /* __BOOTLIB_TINY_HEAP_H__ */
