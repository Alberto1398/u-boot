/*******************************************************************************
 *                              NOYA1100
 *                            Module: boot
 *                 Copyright(c) 2003-2008 Actions Semiconductor,
 *                            All Rights Reserved.
 * History:
 *      <author>           <time>           <version >             <desc>
*******************************************************************************/

#ifndef __BOOTLIB_MALLOC_H__
#define __BOOTLIB_MALLOC_H__

#include <common.h>
#include "tiny_heap.h"

// The static version
uint mxml_init_heap(uint32_t heap_base, uint32_t heap_size);
void * mxml_malloc(uint32_t rq_size);
void * mxml_calloc(uint32_t n_elements, uint32_t elem_size);
void * mxml_realloc(void *p_old, uint32_t new_size);
void mxml_mfree(void *pbuf);


#endif /* __BOOTLIB_MALLOC_H__ */
