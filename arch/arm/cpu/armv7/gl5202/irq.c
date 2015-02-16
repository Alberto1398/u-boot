
#include <common.h>
#include <asm/io.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>

#define GL520X_PERIPH_BASE  0xB0020000

#define IRQ_ICCICR_ADDR     (GL520X_PERIPH_BASE + 0x0100)
#define IRQ_ICCPMR_ADDR     (GL520X_PERIPH_BASE + 0x0104)
#define IRQ_ICCIAD_ADDR     (GL520X_PERIPH_BASE + 0x010C)
#define IRQ_ICCEOIR_ADDR    (GL520X_PERIPH_BASE + 0x0110)
#define IRQ_ICDDCR_ADDR     (GL520X_PERIPH_BASE + 0x1000)
#define IRQ_ICDISER_ADDR    (GL520X_PERIPH_BASE + 0x1100)
#define IRQ_ICDICER_ADDR    (GL520X_PERIPH_BASE + 0x1180)
#define IRQ_ICDIPR_ADDR     (GL520X_PERIPH_BASE + 0x1400)
#define IRQ_ICDITARGET_ADDR (GL520X_PERIPH_BASE + 0x1800)

#define IRQ_TARGET_MASK     (0xFF << 0)
#define IRQ_TARGET_CPU0     (0x1 << 0)
#define IRQ_TARGET_CPU1     (0x1 << 1)
#define IRQ_TARGET_CPU2     (0x1 << 2)
#define IRQ_TARGET_CPU3     (0x1 << 3)



typedef struct {
    interrupt_handler_t *p_func;
    void *p_usr_data;
} act_irq_register_rec_t;

static act_irq_register_rec_t s_irq_reg_tbl[256];



static void _default_int_handler(void *p_data)
{
    printf("unhandled irq %u\n", (uint)p_data);
}

static inline uint _chk_irq_id_valid(uint irq_id)
{
    return (irq_id < ARRAY_SIZE(s_irq_reg_tbl));
}

static void _irq_enable_id(uint irq_id)
{
    uint32_t bit, offset;

    offset = ((irq_id >> 5) << 2);
    bit = 0x1 << (irq_id & 0x1F);
    writel(bit, IRQ_ICDISER_ADDR + offset);

    debug("bit(0x%x), off(0x%x) add(0x%x), val(0x%x)\n", bit, offset,
        IRQ_ICDISER_ADDR + offset,
        readl(IRQ_ICDISER_ADDR + offset));
}

static void _irq_disable_id(uint irq_id)
{
    u32 bit, offset;

    offset = ((irq_id >> 5) << 2);
    bit = 0x1 << (irq_id & 0x1F);
    writel(bit, IRQ_ICDICER_ADDR + offset);
    readl(IRQ_ICDICER_ADDR + offset);
}

static void _irq_set_priority(uint irq_id, uint priority)
{
    int addr, shift, clr, set;

    shift = (irq_id & 0x3) << 3;
    set = (priority & 0x1F) << (3 + shift);
    clr = 0xFF << shift;
    addr = (irq_id & ~0x3) + IRQ_ICDIPR_ADDR;

    clrsetbits_le32(addr, clr, set);
}

static inline void _irq_set_target(uint irq_id)
{
    int addr, shift, clr, set;

    shift = (irq_id & 0x3) << 3;
    set = IRQ_TARGET_CPU0 << shift;
    clr = 0xFF << shift;
    addr = ((irq_id >> 2) << 2) + IRQ_ICDITARGET_ADDR;

    debug("set target addr(0x%x), clr(0x%x), set(0x%x)\n",
            addr, clr, set);

    clrsetbits_le32(addr, clr, set);
}

/*  Read the Interrupt Acknowledge Register  (ICCIAR) */
static inline uint32_t irq_read_ack(void)
{
    // 注意:此ICCIAR寄存器cpu读一次后便会回到默认值0x3ff
    return readl(IRQ_ICCIAD_ADDR);
}

/* Acknowlege the Interrupt, (ICCEOIR) */
static void _irq_eoi_ack(uint32_t irq_reg)
{
    writel(irq_reg, IRQ_ICCEOIR_ADDR);
    readl(IRQ_ICCEOIR_ADDR);
}

void do_irq(struct pt_regs *pt_regs)
{
    uint32_t irq_reg, irq_id;

    irq_reg = irq_read_ack();
    irq_id = irq_reg & 0x3ffU;
    if(likely(_chk_irq_id_valid(irq_id)))
    {
        (s_irq_reg_tbl[irq_id].p_func)(s_irq_reg_tbl[irq_id].p_usr_data);
    }
    else
    {
        puts("spurious irq\n");
    }
    _irq_eoi_ack(irq_reg);
}

void irq_install_handler(int irq_id, interrupt_handler_t *p_func, void *p_usr_data)
{
    if(! _chk_irq_id_valid(irq_id))
    {
        return;
    }

    s_irq_reg_tbl[irq_id].p_func = p_func;
    s_irq_reg_tbl[irq_id].p_usr_data = p_usr_data;

    _irq_set_target(irq_id);
    _irq_set_priority(irq_id, 0x0);
    _irq_enable_id(irq_id);
}

void irq_free_handler(int irq_id)
{
    if(! _chk_irq_id_valid(irq_id))
    {
        return;
    }

    _irq_disable_id(irq_id);

    s_irq_reg_tbl[irq_id].p_func = _default_int_handler;
    s_irq_reg_tbl[irq_id].p_usr_data = (void*)irq_id;
}

int arch_interrupt_init(void)
{
    uint i, reg_val;

    for(i=0; i<ARRAY_SIZE(s_irq_reg_tbl); i++)
    {
        s_irq_reg_tbl[i].p_func = _default_int_handler;
        s_irq_reg_tbl[i].p_usr_data = (void*)i;
    }

    /* disable all src from forwarding */
    for(i=0; i<32; i++)
    {
        reg_val = readl(IRQ_ICDICER_ADDR + (i<<2));
        if(reg_val != 0)
        {
            writel(reg_val, IRQ_ICDICER_ADDR + (i<<2));
        }
    }

    /* enable global irq gic */
    setbits_le32(IRQ_ICDDCR_ADDR, 0x1);

    /* enable processor interface */
#define SECURE_BIT      (0x1 << 0)
#define NON_SECURE_BIT  (0x1 << 1)
    setbits_le32(IRQ_ICCICR_ADDR, SECURE_BIT | NON_SECURE_BIT);
    writel(0x1F, IRQ_ICCPMR_ADDR);

    return 0;
}

