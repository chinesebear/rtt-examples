/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date                  Author       Notes
 * 2010-10-15     Bernard      first version
 * 2010-10-15     lgnq           modified for LS1B
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 * 2015-07-06     chinesebear modified for loongson 1c
 * 2017-04-06     chinesebear  add int1~int4 registers
 */

#include <rtthread.h>
#include <rthw.h>
#include "ls1c.h"

#define MAX_INTR 160

extern rt_uint32_t rt_interrupt_nest;
rt_uint32_t rt_interrupt_from_thread;
rt_uint32_t rt_interrupt_to_thread;
rt_uint32_t rt_thread_switch_interrupt_flag;

static struct rt_irq_desc irq_handle_table[MAX_INTR];
void rt_interrupt_dispatch(void *ptreg);
void rt_hw_timer_handler();

static struct ls1c_intc_regs volatile *ls1c_hw0_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INT0_BASE);

static struct ls1c_intc_regs volatile *ls1c_hw1_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INT1_BASE);

static struct ls1c_intc_regs volatile *ls1c_hw2_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INT2_BASE);

static struct ls1c_intc_regs volatile *ls1c_hw3_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INT3_BASE);

static struct ls1c_intc_regs volatile *ls1c_hw4_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INT4_BASE);


/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

static void rt_hw_interrupt_handler(int vector, void *param)
{
    rt_kprintf("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void rt_hw_interrupt_init(void)
{
    rt_int32_t idx;

    /* pci active low */
    ls1c_hw0_icregs->int_pol = -1;	   //must be done here 20110802 lgnq
	ls1c_hw1_icregs->int_pol = -1;
	ls1c_hw2_icregs->int_pol = -1;
	ls1c_hw3_icregs->int_pol = -1;
	ls1c_hw4_icregs->int_pol = -1;
	/* make all interrupts level triggered */
    (ls1c_hw0_icregs+0)->int_edge = 0x0000e000;
	 ls1c_hw1_icregs->int_edge = 0x00000000;
	 ls1c_hw2_icregs->int_edge = 0x00000000;
	 ls1c_hw3_icregs->int_edge = 0x00000000;
	 ls1c_hw4_icregs->int_edge = 0x00000000;
    /* mask all interrupts */
    (ls1c_hw0_icregs+0)->int_clr = 0xffffffff;
	ls1c_hw1_icregs->int_clr = 0xffffffff;
	ls1c_hw2_icregs->int_clr = 0xffffffff;
	ls1c_hw3_icregs->int_clr = 0xffffffff;
	ls1c_hw4_icregs->int_clr = 0xffffffff;
    rt_memset(irq_handle_table, 0x00, sizeof(irq_handle_table));
    for (idx = 0; idx < MAX_INTR; idx ++)
    {
        irq_handle_table[idx].handler = rt_hw_interrupt_handler;
    }

    /* init interrupt nest, and context in thread sp */
    rt_interrupt_nest = 0;
    rt_interrupt_from_thread = 0;
    rt_interrupt_to_thread = 0;
    rt_thread_switch_interrupt_flag = 0;
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_mask(int vector)
{
    /* mask interrupt */
    //(ls1c_hw0_icregs+(vector>>5))->int_en &= ~(1 << (vector&0x1f));
	struct ls1c_intc_regs volatile *ls1c_irq_reg;
	int posbit=0;
	if(vector >= 0 && vector <=31)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT0_BASE);
		posbit = vector;
	}
	else if(vector >= 32 && vector <=63)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT1_BASE);
		posbit = vector -32;
	}
	else if(vector >= 64 && vector <=95)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT2_BASE);
		posbit = vector -64;
	}
	else if(vector >= 96 && vector <=127)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT3_BASE);
		posbit = vector -96;
	}
	else if(vector >= 128 && vector <=160)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT4_BASE);
		posbit = vector -128;
	}
	else
		return ;
	ls1c_irq_reg->int_en &= ~(1 << posbit);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_umask(int vector)
{
    //(ls1c_hw0_icregs+(vector>>5))->int_en |= (1 << (vector&0x1f));
    struct ls1c_intc_regs volatile *ls1c_irq_reg;
	int posbit=0;
	if(vector >= 0 && vector <=31)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT0_BASE);
		posbit = vector;
	}
	else if(vector >= 32 && vector <=63)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT1_BASE);
		posbit = vector -32;
	}
	else if(vector >= 64 && vector <=95)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT2_BASE);
		posbit = vector -64;
	}
	else if(vector >= 96 && vector <=127)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT3_BASE);
		posbit = vector -96;
	}
	else if(vector >= 128 && vector <=160)
	{
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT4_BASE);
		posbit = vector -128;
	}
	else
		return ;
	ls1c_irq_reg->int_en |= (1 << posbit);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if (vector >= 0 && vector < MAX_INTR)
    {
        old_handler = irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
        rt_strncpy(irq_handle_table[vector].name, name, RT_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
        irq_handle_table[vector].handler = handler;
        irq_handle_table[vector].param = param;
    }

    return old_handler;
}

void rt_interrupt_dispatch(void *ptreg)
{
    int irq;
    void *param;
    rt_isr_handler_t irq_func;
    static rt_uint32_t status = 0;
    rt_uint32_t c0_status;
    rt_uint32_t c0_cause;
    volatile rt_uint32_t cause_im;
    volatile rt_uint32_t status_im;
    rt_uint32_t pending_im;
	int ipflag = 0;
	int irqMaxNo=0;
	int irqMinNo=0;
	int posbit = 0;
	struct ls1c_intc_regs volatile *ls1c_irq_reg;
    /* check os timer */
    c0_status = read_c0_status();
    c0_cause = read_c0_cause();

    cause_im = c0_cause & ST0_IM;
    status_im = c0_status & ST0_IM;
    pending_im = cause_im & status_im;

    if (pending_im & CAUSEF_IP7)
    {
        rt_hw_timer_handler();
    }

    if (pending_im & CAUSEF_IP2)
    {
		ipflag = 1;//0~31
		irqMinNo = 0;
		irqMaxNo = 31;
		/* the hardware interrupt */
        status = ls1c_hw0_icregs->int_isr;
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT0_BASE);
    }
    else if (pending_im & CAUSEF_IP3)
    {
        ipflag = 2;//32~63
        irqMinNo = 32;
		irqMaxNo = 63;
		/* the hardware interrupt */
        status = ls1c_hw1_icregs->int_isr;
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT1_BASE);
    }
    else if (pending_im & CAUSEF_IP4)
    {
        ipflag = 3;//64~95
        irqMinNo = 64;
		irqMaxNo = 95;
		/* the hardware interrupt */
        status = ls1c_hw2_icregs->int_isr;
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT2_BASE);
    }
    else if (pending_im & CAUSEF_IP5)
    {
        ipflag = 4;//96~127
        irqMinNo = 96;
		irqMaxNo = 127;
		/* the hardware interrupt */
        status = ls1c_hw3_icregs->int_isr;
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT3_BASE);
    }
    else if (pending_im & CAUSEF_IP6)
    {
        ipflag = 5;//128~159
        irqMinNo = 128;
		irqMaxNo = 159;
		/* the hardware interrupt */
        status = ls1c_hw4_icregs->int_isr;
		ls1c_irq_reg = (struct ls1c_intc_regs volatile *)(LS1C_INT4_BASE);
    }
	if(ipflag)
	{
		
        if (!status)
            return;

        for (irq = irqMinNo,posbit=0; irq <= irqMaxNo && posbit < 32; irq++,posbit++)
        {
            if ((status & (1 << posbit)))
            {
                status &= ~(1 << posbit);

                irq_func = irq_handle_table[irq].handler;
                param = irq_handle_table[irq].param;

                /* do interrupt */
                irq_func(irq, param);

#ifdef RT_USING_INTERRUPT_INFO
                irq_handle_table[irq].counter++;
#endif /* RT_USING_INTERRUPT_INFO */

                /* ack interrupt */
                ls1c_irq_reg->int_clr |= (1 << posbit);
            }
        }
	}
}

/*@}*/


