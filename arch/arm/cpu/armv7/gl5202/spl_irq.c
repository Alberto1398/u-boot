
#include <common.h>
#include <asm/io.h>

/* We reuse the exception code in BROM in SPL stage, to save RAM */

typedef struct
{
    uint8_t irq_id;   // 0xff means free
    interrupt_handler_t *p_func;
    void *p_usr_data;
} act_irq_register_rec_t;

static act_irq_register_rec_t s_irq_reg_tbl[2];

static act_irq_register_rec_t * _find_irq_reg(uint irq_id)
{
    uint i;
    act_irq_register_rec_t *p;

    p = s_irq_reg_tbl;
    i = ARRAY_SIZE(s_irq_reg_tbl);
    while(i-- != 0)
    {
        if(p->irq_id == irq_id)
        {
            return p;
        }
        p++;
    }
    return NULL;
}

void act_do_irq_inner(uint irq_id)
{
    /* IRQ ACK is done outside this function. */

    act_irq_register_rec_t *p = _find_irq_reg(irq_id);
    if(unlikely(p == NULL))
    {
        printf("unhandled irq %u\n", irq_id);
    }
    else
    {
        (p->p_func)(p->p_usr_data);
    }
}

void do_irq(struct pt_regs *pt_regs)
{

}

void irq_install_handler(int irq_id, interrupt_handler_t *p_func, void *p_usr_data)
{
    act_irq_register_rec_t *p;

    p = _find_irq_reg(0xff);
    if(p == NULL)
    {
        puts("no irq_reg space\n");
        hang();
    }
    p->irq_id = irq_id;
    p->p_func = p_func;
    p->p_usr_data = p_usr_data;
}

void irq_free_handler(int irq_id)
{
    act_irq_register_rec_t *p;

    p = _find_irq_reg(irq_id);
    if(p != NULL)
    {
        p->irq_id = 0xff;
    }
}

int arch_interrupt_init(void)
{
    memset(s_irq_reg_tbl, 0xff, sizeof(s_irq_reg_tbl)); // all invalid
    return 0;
}

