#include <common.h>
#include "tiny_heap.h"
#include "mxml_malloc.h"


// The static version

static void *s_my_tiny_heap;

uint mxml_init_heap(uint32_t heap_base, uint32_t heap_size)
{
    s_my_tiny_heap = tiny_heap_init(heap_base, heap_size);
    if(s_my_tiny_heap == NULL)
    {
        return 1;
    }
    return 0;
}

void * mxml_malloc(uint32_t rq_size)
{
    void *p_new = tiny_heap_malloc(s_my_tiny_heap, rq_size);
#if _DEBUG
    if(p_new == NULL)
    {
        debug("%s: no mem, need %u\n", __FUNCTION__, rq_size);
    }
#endif
    return p_new;
}

void * mxml_calloc(uint32_t n_elements, uint32_t elem_size)
{
    uint32_t rq_size = elem_size * n_elements;
    void *p_new = tiny_heap_malloc(s_my_tiny_heap, rq_size);
    if(p_new != NULL)
    {
        memset(p_new, 0, rq_size);
    }
#if _DEBUG
    else
    {
        debug("%s: no mem, need %u\n", __FUNCTION__, rq_size);
    }
#endif
    return p_new;
}

void * mxml_realloc(void *p_old, uint32_t new_size)
{
    void *p_new;
    uint32_t old_size;

    if(new_size != 0)
    {
        old_size = tiny_heap_get_alloc_size(s_my_tiny_heap, p_old);
        if(new_size > old_size)
        {
            p_new = tiny_heap_malloc(s_my_tiny_heap, new_size);
            if(p_new != NULL)
            {
                memcpy(p_new, p_old, old_size);
                memset((uint8_t*)p_new + old_size, 0, new_size - old_size);
                tiny_heap_mfree(s_my_tiny_heap, p_old);
            }
#if _DEBUG
            else
            {
                debug("%s: no mem, need %u\n", __FUNCTION__, new_size);
            }
#endif
        }
        else
        {
            p_new = p_old;
        }
    }
    else
    {
        tiny_heap_mfree(s_my_tiny_heap, p_old);
        p_new = NULL;
    }
    return p_new;
}

void mxml_mfree(void *pbuf)
{
#if _DEBUG
    if(pbuf == NULL)
    {
        debug("%s: request to free a NULL ptr!\n", __FUNCTION__);
    }
#endif
    tiny_heap_mfree(s_my_tiny_heap, pbuf);
}



//==============================================================================

// hook for tiny_heap debug

void __attribute__((weak)) tiny_heap_cb_assert_halt(UINT32 line_id)
{
    printf("mxml_heap halt @ line %u\n", line_id);
    hang();
}

void __attribute__((weak)) tiny_heap_cb_node_chk_err(UINT32 hdr, UINT32 err_type)
{
    printf("mxml_heap node chk err, hdr=0x%x type=%u\n", hdr, err_type);
    hang();
}

void __attribute__((weak)) tiny_heap_cb_dump_node(
        UINT32 hdr, UINT32 prev, UINT32 next, UINT32 base, UINT32 size)
{
    printf("mxml_heap dump: hdr=0x%08x prev=0x%08x next=0x%08x base=0x%08x size=%u\n",
            hdr, prev, next, base, size);
}
