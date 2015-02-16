/*
 * tiny_heap.c
 *
 *  Created on: Nov 4, 2013
 *      Author: clamshell
 */


#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>
#include "tiny_heap.h"

#ifndef TINY_HEAP_DBG_EN
#define TINY_HEAP_DBG_EN    0
#endif

#if TINY_HEAP_DBG_EN
#define TINY_HEAP_OV_CHK_EN                 1
#define DASSERT(EXP)                        do{if(!(EXP)){tiny_heap_cb_assert_halt(__LINE__);}}while(0)
#define DBG_DEFINE_OBJ_TYPE_ID              uint32_t __obj_type_id
#define DBG_SET_OBJ_TYPE_ID(P_OBJ,VAL)      ((P_OBJ)->__obj_type_id = (VAL))
#define DBG_CHK_OBJ_TYPE_ID(P_OBJ,VAL)      DASSERT(((P_OBJ)!=NULL)&&((P_OBJ)->__obj_type_id==(VAL)))
#define DBG_CHK_OBJ_TYPE_ID_NA(P_OBJ,VAL)   (((P_OBJ)!=NULL)&&((P_OBJ)->__obj_type_id==(VAL)))
#else
#define TINY_HEAP_OV_CHK_EN                 0
#define DASSERT(EXP)                        do{}while(0)
#define DBG_DEFINE_OBJ_TYPE_ID              uint32_t __obj_type_id
#define DBG_SET_OBJ_TYPE_ID(P_OBJ,VAL)      do{}while(0)
#define DBG_CHK_OBJ_TYPE_ID(P_OBJ,VAL)      do{}while(0)
#define DBG_CHK_OBJ_TYPE_ID_NA(P_OBJ,VAL)   (TRUE)
#endif

#define TINY_HEAP_OBJ_SIGNATURE         0xe75e5929U
#define TINY_HEAP_ALIGNED_SIZE          32U     /* SET THIS to cache-line size of your CPU! */

typedef struct _tag_TINY_HEAP_ALLOC_REC
{
    UINT32  ov_chk[4];
    struct _tag_TINY_HEAP_ALLOC_REC *p_prev;
    struct _tag_TINY_HEAP_ALLOC_REC *p_next;
    UINT32  size;
    DBG_DEFINE_OBJ_TYPE_ID;
} TINY_HEAP_ALLOC_REC_t; /* fixed 32 bytes */


#if TINY_HEAP_DBG_EN
__attribute__((weak)) void tiny_heap_assert_halt(UINT32 line_id)
{
    while(1);
}
static void _tiny_heap_zero_node_chk(TINY_HEAP_ALLOC_REC_t *p_rec)
{
    p_rec->ov_chk[0] = 0;
    p_rec->ov_chk[1] = 0;
    p_rec->ov_chk[2] = 0;
    p_rec->ov_chk[3] = 0;
    DBG_SET_OBJ_TYPE_ID(p_rec, 0);
}
static void _tiny_heap_init_node_chk(TINY_HEAP_ALLOC_REC_t *p_rec)
{
    p_rec->ov_chk[0] = 0x55555555U;
    p_rec->ov_chk[1] = 0xaaaaaaaaU;
    p_rec->ov_chk[2] = 0xa5a5a5a5U;
    p_rec->ov_chk[3] = 0x5a5a5a5aU;
    DBG_SET_OBJ_TYPE_ID(p_rec, TINY_HEAP_OBJ_SIGNATURE);
}
static void _tiny_heap_node_chk(TINY_HEAP_ALLOC_REC_t *p_rec)
{
    UINT32 err_type = 0;
    while(1)
    {
        if(p_rec == NULL)
        {
            err_type = 1;
            break;
        }
        if(((UINT32)p_rec & (TINY_HEAP_ALIGNED_SIZE-1U)) != 0)
        {
            err_type = 2;
            break;
        }
        if(! DBG_CHK_OBJ_TYPE_ID_NA(p_rec, TINY_HEAP_OBJ_SIGNATURE))
        {
            err_type = 3;
            break;
        }
        if(!(p_rec->ov_chk[0] == 0x55555555U &&
                p_rec->ov_chk[1] == 0xaaaaaaaaU &&
                p_rec->ov_chk[2] == 0xa5a5a5a5U &&
                p_rec->ov_chk[3] == 0x5a5a5a5aU))
        {
            err_type = 4;
            break;
        }
        if((p_rec->size & (TINY_HEAP_ALIGNED_SIZE-1U)) != 0)
        {
            err_type = 5;
            break;
        }
        return;
    }
    tiny_heap_cb_node_chk_err((UINT32)p_rec, err_type);
}
#else
#define _tiny_heap_zero_node_chk(P_REC) do{}while(0)
#define _tiny_heap_init_node_chk(P_REC) do{}while(0)
#define _tiny_heap_node_chk(P_REC)      do{}while(0)
#endif

#if TINY_HEAP_OV_CHK_EN
static UINT32 _tiny_heap_memchk(UINT32 *pbuf, UINT32 val, UINT32 size)
{
    size /= 4U;
    while(size-- != 0)
    {
        if(*pbuf++ != val)
        {
            return FALSE;
        }
    }
    return TRUE;
}
static void _tiny_heap_node_init_ov_chk(TINY_HEAP_ALLOC_REC_t *p_rec)
{
    DASSERT(p_rec->size > TINY_HEAP_ALIGNED_SIZE);
    memset((UINT8*)(p_rec+1) + p_rec->size - TINY_HEAP_ALIGNED_SIZE,
            0x5a, TINY_HEAP_ALIGNED_SIZE);
}
static void _tiny_heap_node_do_ov_chk(TINY_HEAP_ALLOC_REC_t *p_rec)
{
    UINT32 ret;
    DASSERT(p_rec->size > TINY_HEAP_ALIGNED_SIZE);
    ret = _tiny_heap_memchk(
            (UINT32*)((UINT8*)(p_rec+1) + p_rec->size - TINY_HEAP_ALIGNED_SIZE),
            0x5a5a5a5aU, TINY_HEAP_ALIGNED_SIZE);
    DASSERT(ret);
}
#else
#define _tiny_heap_node_init_ov_chk(P_REC)  do{}while(0)
#define _tiny_heap_node_do_ov_chk(P_REC)    do{}while(0)
#endif


void * tiny_heap_init(UINT32 heap_base, UINT32 heap_size)
{
    TINY_HEAP_ALLOC_REC_t *p_head, *p_tail;
    UINT32 heap_base_old;

    DASSERT((sizeof(TINY_HEAP_ALLOC_REC_t) & (TINY_HEAP_ALIGNED_SIZE-1U)) == 0);

    if(heap_size > 2U * 1024U * 1024U * 1024U)
    {
        return NULL;
    }

    // cache line align.
    heap_base_old = heap_base;
    heap_base = (heap_base + (TINY_HEAP_ALIGNED_SIZE-1U)) & ~(TINY_HEAP_ALIGNED_SIZE-1U);
    heap_size -= heap_base - heap_base_old;
    heap_size = heap_size & ~(TINY_HEAP_ALIGNED_SIZE-1U);

    p_head = (TINY_HEAP_ALLOC_REC_t *)heap_base;
    heap_base += sizeof(TINY_HEAP_ALLOC_REC_t);
    heap_size -= 2 * sizeof(TINY_HEAP_ALLOC_REC_t);

    p_tail = (TINY_HEAP_ALLOC_REC_t *)(heap_base + heap_size);

    p_head->p_next = p_tail;
    p_head->p_prev = NULL;
    p_head->size = 0;
    _tiny_heap_init_node_chk(p_head);

    p_tail->p_next = NULL;
    p_tail->p_prev = p_head;
    p_tail->size = 0;
    _tiny_heap_init_node_chk(p_tail);

    return p_head;
}

void * tiny_heap_malloc(void *p_obj, UINT32 rq_size)
{
    TINY_HEAP_ALLOC_REC_t *p_head, *p_prev, *p;
    TINY_HEAP_ALLOC_REC_t *p_new;

    p_head = (TINY_HEAP_ALLOC_REC_t *)p_obj;
    _tiny_heap_node_chk(p_head);
    DASSERT(p_head->size == 0);

    if(rq_size == 0)
    {
        return NULL;
    }
#if TINY_HEAP_OV_CHK_EN
    rq_size += TINY_HEAP_ALIGNED_SIZE;
#endif
    rq_size = (rq_size + (TINY_HEAP_ALIGNED_SIZE-1U)) & ~(TINY_HEAP_ALIGNED_SIZE-1U);

    p_prev = p_head;
    p = p_head->p_next;
    while(p != NULL)
    {
        UINT32 gap_size, prev_end_addr;

        _tiny_heap_node_chk(p);

        prev_end_addr = (UINT32)(p_prev + 1) + p_prev->size;
        gap_size = (UINT32)p - prev_end_addr;
        if(gap_size >= rq_size + sizeof(TINY_HEAP_ALLOC_REC_t))
        {
            p_new = (TINY_HEAP_ALLOC_REC_t *)prev_end_addr;
            p_new->size = rq_size;
            _tiny_heap_init_node_chk(p_new);
            _tiny_heap_node_init_ov_chk(p_new);

            p_prev->p_next = p_new;
            p_new->p_prev = p_prev;
            p_new->p_next = p;
            p->p_prev = p_new;

            return (void*)(p_new + 1);
        }
        p_prev = p;
        p = p->p_next;
    }

    return NULL;
}

void tiny_heap_mfree(void *p_obj, void *pbuf)
{
    TINY_HEAP_ALLOC_REC_t * __attribute__((unused)) p_head;
    TINY_HEAP_ALLOC_REC_t *p_next, *p_prev, *p;

    p_head = (TINY_HEAP_ALLOC_REC_t *)p_obj;
    _tiny_heap_node_chk(p_head);
    DASSERT(p_head->size == 0);

    if(pbuf == NULL)
    {
        return;
    }

    p = (TINY_HEAP_ALLOC_REC_t *)((UINT32)pbuf - sizeof(TINY_HEAP_ALLOC_REC_t));
    _tiny_heap_node_chk(p);
    _tiny_heap_node_do_ov_chk(p);

    p_next = p->p_next;
    _tiny_heap_node_chk(p_next);

    p_prev = p->p_prev;
    _tiny_heap_node_chk(p_prev);

    p_prev->p_next = p_next;
    p_next->p_prev = p_prev;

#if TINY_HEAP_DBG_EN
    p->p_next = NULL;
    p->p_prev = NULL;
    _tiny_heap_zero_node_chk(p);
#endif
}

uint32_t tiny_heap_get_alloc_size(void *p_obj, void *pbuf)
{
    TINY_HEAP_ALLOC_REC_t * __attribute__((unused)) p_head;
    TINY_HEAP_ALLOC_REC_t *p;

    p_head = (TINY_HEAP_ALLOC_REC_t *)p_obj;
    _tiny_heap_node_chk(p_head);
    DASSERT(p_head->size == 0);

    if(pbuf == NULL)
    {
        return 0;
    }

    p = (TINY_HEAP_ALLOC_REC_t *)((UINT32)pbuf - sizeof(TINY_HEAP_ALLOC_REC_t));
    _tiny_heap_node_chk(p);
    _tiny_heap_node_do_ov_chk(p);

#if TINY_HEAP_OV_CHK_EN
    DASSERT(p->size > TINY_HEAP_ALIGNED_SIZE);
    return p->size - TINY_HEAP_ALIGNED_SIZE;
#else
    return p->size;
#endif
}

void tiny_heap_dump(void *p_obj)
{
    TINY_HEAP_ALLOC_REC_t *p_head, *p;

    p_head = (TINY_HEAP_ALLOC_REC_t *)p_obj;
    _tiny_heap_node_chk(p_head);
    DASSERT(p_head->size == 0);

    p = p_head;
    while(p != NULL)
    {
        _tiny_heap_node_chk(p);
        tiny_heap_cb_dump_node(
                (UINT32)p, (UINT32)(p->p_prev), (UINT32)(p->p_next),
                (UINT32)(p + 1), p->size);
        p = p->p_next;
    }
}
