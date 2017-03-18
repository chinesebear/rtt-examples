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
#include "gpio.h"
extern rt_device_t uart_dev[];
rt_uint32_t edge1[2],edge2[2],edge3[2];
const rt_uint32_t high = 0;
const rt_uint32_t low = 1;
rt_uint8_t key_flag = 0;

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
void key_soft_interrupt_thread_entry(void *parameter)
{
	rt_device_t KEY1,KEY2,KEY3;
	rt_err_t iRet;
	rt_kprintf("key soft interrupt thread start...\r\n");
	KEY1 = rt_device_find("KEY1");
	rt_device_open(KEY1, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(KEY1, GPIO_READ_MODE , RT_NULL);
	KEY2 = rt_device_find("KEY2");
	rt_device_open(KEY2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(KEY2, GPIO_READ_MODE , RT_NULL);
	KEY3 = rt_device_find("KEY3");
	rt_device_open(KEY3, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(KEY3, GPIO_READ_MODE , RT_NULL);

	for(;;)
	{
		rt_device_read(KEY1,0,edge1,1);
		rt_device_read(KEY2,0,edge2,1);
		rt_device_read(KEY3,0,edge3,1);
		rt_thread_delay(10);
		rt_device_read(KEY1,0,edge1+1,1);
		rt_device_read(KEY2,0,edge2+1,1);
		rt_device_read(KEY3,0,edge3+1,1);
		if(edge1[0] == 0 && edge1[1] == 1)
		{
			key_flag = 0;//flush led
			rt_kprintf("key1 interrupt\n");
		}
		if(edge2[0] == 0 && edge2[1] == 1)
		{
			key_flag = 1;//power off led
			rt_kprintf("key2 interrupt\n");
		}
		if(edge3[0] == 0 && edge3[1] == 1)
		{
			key_flag = 2;//power on led
			rt_kprintf("key3 interrupt\n");
		}
		rt_thread_delay(10);
	}
	
}

void rt_run_example3_thread_entry(void *parameter)
{
	rt_device_t LED1,LED2,LED3,LED4,LED5;
	rt_err_t iRet;
	
	rt_uint32_t flush_tick = 30;
	rt_kprintf("LDE flush thread start...\r\n");
	LED1 = rt_device_find("LED1");
	rt_device_open(LED1, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(LED1, GPIO_WRITE_MODE , RT_NULL);
	LED2 = rt_device_find("LED2");
	rt_device_open(LED2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(LED2, GPIO_WRITE_MODE , RT_NULL);
	LED3 = rt_device_find("LED3");
	rt_device_open(LED3, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(LED3, GPIO_WRITE_MODE , RT_NULL);
	LED4 = rt_device_find("LED4");
	rt_device_open(LED4, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(LED4, GPIO_WRITE_MODE , RT_NULL);
	LED5 = rt_device_find("LED5");
	rt_device_open(LED5, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	rt_device_control(LED5, GPIO_WRITE_MODE , RT_NULL);
	//rt_pad_show();
	for(;;)
	{
		if(key_flag == 0)//led flush
		{
			rt_device_write(LED1,0,&high,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED1,0,&low,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED2,0,&high,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED2,0,&low,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED3,0,&high,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED3,0,&low,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED4,0,&high,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED4,0,&low,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED5,0,&high,1);
			rt_thread_delay(flush_tick);
			rt_device_write(LED5,0,&low,1);
			rt_thread_delay(flush_tick);
		}
		else if(key_flag == 1)//led low
		{
			rt_device_write(LED1,0,&low,1);
			rt_device_write(LED2,0,&low,1);
			rt_device_write(LED3,0,&low,1);
			rt_device_write(LED4,0,&low,1);
			rt_device_write(LED5,0,&low,1);
			rt_thread_delay(flush_tick);
		}
		else//led high
		{
			rt_device_write(LED1,0,&high,1);
			rt_device_write(LED2,0,&high,1);
			rt_device_write(LED3,0,&high,1);
			rt_device_write(LED4,0,&high,1);
			rt_device_write(LED5,0,&high,1);
			rt_thread_delay(flush_tick);
		}
		
	}
	rt_kprintf("never get here...\r\n");
}

int rt_application_init(void)
{
	rt_thread_t tid,tid1,tid2,tid3;
	rt_thread_t sid;
	/* create initialization thread */
	tid = rt_thread_create("init",
							rt_init_thread_entry, RT_NULL,
							4096, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	else
		return -1;
#if 0
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
#endif
	/* create key_soft_interrupt thread */
	sid = rt_thread_create("key",
							key_soft_interrupt_thread_entry, RT_NULL,
							4096, 10, 20);
	if (sid != RT_NULL)
		rt_thread_startup(sid);
	else
		return -1;

	/*create example3 thread*/
	tid3 = rt_thread_create("example3",
							rt_run_example3_thread_entry, RT_NULL,
							4096, 20, 20);
	if (tid3 != RT_NULL)
		rt_thread_startup(tid3);
	else
		return -1;

	return 0;
}
