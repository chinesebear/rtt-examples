/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2012, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date                Author         Notes
 * 2010-06-25          Bernard        first version
 * 2011-08-08          lgnq           modified for Loongson LS1B
 * 2015-07-06          chinesebear    modified for Loongson LS1C
 */

#include <rtthread.h>
#include <components.h>
#include "uart.h"
extern rt_device_t uart_dev[];
void rt_init_thread_entry(void *parameter)
{
	/* initialization RT-Thread Components */
	rt_components_init();
}

void rt_run_example_thread_entry(void *parameter)
{
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	rt_device_t newdev;
	rt_err_t iRet;
	rt_uint32_t rx_size,tx_size;
	rt_kprintf("uart1 thread start...\r\n");
	 newdev = rt_device_find("uart1");
	if(newdev == NULL)
	{
		rt_kprintf("find no uart1\r\n");
		return ;
	}
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart1 failure\r\n");
		return ;
	}
	//rt_pad_show();
	for(;;)
	{
		#if 1
		rt_memset(rx_buffer,0,256);
		rt_memset(tx_buffer,0,256);
		rx_size = rt_device_read(newdev,0,rx_buffer,256);
		if(rx_size)
		{
			rt_kprintf("uart1 rx_size = %d\r\n",rx_size);
		}
		tx_size = rx_size;
		rt_memcpy(tx_buffer,rx_buffer,rx_size);
		rt_device_write(newdev,0,rx_buffer,tx_size);
		#endif
		//rt_device_write(newdev,0,"hello world!",12);
		//UART_DAT(UART1_BASE)= 'h';
		rt_thread_delay(100);
	}
	rt_kprintf("never get here...\r\n");
}


void rt_run_example2_thread_entry(void *parameter)
{
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	rt_device_t newdev;
	rt_err_t iRet;
	rt_uint32_t rx_size,tx_size;
	rt_kprintf("uart3 thread start...\r\n");
	 newdev = rt_device_find("uart3");
	if(newdev == NULL)
	{
		rt_kprintf("find no uart3\r\n");
		return ;
	}
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart3 failure\r\n");
		return ;
	}
	//rt_pad_show();
	for(;;)
	{
		#if 1
		rt_memset(rx_buffer,0,256);
		rt_memset(tx_buffer,0,256);
		rx_size = rt_device_read(newdev,0,rx_buffer,256);
		if(rx_size)
		{
			rt_kprintf("uart3 rx_size = %d\r\n",rx_size);
		}
		tx_size = rx_size;
		rt_memcpy(tx_buffer,rx_buffer,rx_size);
		rt_device_write(newdev,0,rx_buffer,tx_size);
		#endif
		//rt_device_write(newdev,0,"hello world!",12);
		//UART_DAT(UART1_BASE)= 'h';
		rt_thread_delay(100);
	}
	rt_kprintf("never get here...\r\n");
}


int rt_application_init(void)
{
	rt_thread_t tid,tid1,tid2;

	/* create initialization thread */
	tid = rt_thread_create("init",
							rt_init_thread_entry, RT_NULL,
							4096, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	else
		return -1;

	/*create example thread*/
	tid1 = rt_thread_create("example",
							rt_run_example_thread_entry, RT_NULL,
							4096, 16, 20);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);
	else
		return -1;

	/*create example2 thread*/
	tid2 = rt_thread_create("example2",
							rt_run_example2_thread_entry, RT_NULL,
							4096, 17, 20);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);
	else
		return -1;

	return 0;
}
