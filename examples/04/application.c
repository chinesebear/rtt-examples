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
int GetComma(int num,char *str)//返回逗号所在的位置
{
	int i,j = 0;
	int len=strlen(str);
	for(i = 0;i < len;i ++)
	{
		if(str[i] == ',')
			j++;
		if(j == num)
			return i + 1;	
	}

	return 0;	
}
#define GPS_START	0
#define GPS_FEILD1	1
#define GPS_FEILD2	2
#define GPS_FEILD3	3
#define GPS_FEILD4	4
#define GPS_FEILD5	5
#define GPS_FEILD6	6
#define GPS_FEILD7	7
#define GPS_FEILD8	8
#define GPS_FEILD9	9
#define GPS_FEILD10	10
#define GPS_FEILD11	11
#define GPS_FEILD12	12
#define GPS_END		13
void rt_run_example_thread_entry(void *parameter)//我需要在这里加入代码（URT1）
{
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	
//	rt_uint8_t print_buffer[256];
//	rt_uint8_t comma[]=","; 
	rt_device_t newdev;
	rt_err_t iRet;
	rt_uint32_t rx_size,tx_size;

	rt_kprintf("uart1 thread start...\r\n");
	 newdev = rt_device_find("uart1");//按照指定设备名称查找设备（uart1）
	if(newdev == NULL)
	{
		rt_kprintf("find no uart1\r\n");
		return ;
	}
	//打开设备，（设备句柄，访问模式）
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart1 failure\r\n");
		return ;
	}
	//rt_pad_show();
	char ch;
	char NS[12],EW[13];
	char NS_FLAG,EW_FLAG;
	int i;
	unsigned char state = GPS_START;
	rt_memset(rx_buffer,0,256);
	rt_memset(tx_buffer,0,256);
	rt_memset(NS,0,12);
	rt_memset(EW,0,13);
	for(;;)
	{
		
		
		switch(state)
		{
			case GPS_START:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == '$')
				{
					rt_device_read(newdev,0,rx_buffer,5);
					if(rt_memcmp(rx_buffer,"GPRMC",5)== 0)
					{
						state = GPS_FEILD1;
					}
				}
				break;
			case GPS_FEILD1://UTC time
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD2;
				}
				break;
			case GPS_FEILD2:// A/V
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD3;
				}
				break;
			case GPS_FEILD3:// 纬度
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					rt_device_read(newdev,0,rx_buffer,11);
					rt_memset(NS,0,12);
					rt_memcpy(NS,rx_buffer,11);
					state == GPS_FEILD4;
				}
				break;
			case GPS_FEILD4:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					rt_device_read(newdev,0,rx_buffer,1);
					rt_memcpy(&NS_FLAG,rx_buffer,1);
					state == GPS_FEILD5;
				}
				break;
			case GPS_FEILD5:// 经度
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					rt_device_read(newdev,0,rx_buffer,12);
					rt_memset(EW,0,12);
					rt_memcpy(EW,rx_buffer,12);
					state == GPS_FEILD6;
				}
				break;
			case GPS_FEILD6:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					rt_device_read(newdev,0,rx_buffer,1);
					rt_memcpy(&EW_FLAG,rx_buffer,1);
					state == GPS_FEILD7;
				}
				break;
			case GPS_FEILD7:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD8;
				}
				break;
			case GPS_FEILD8:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD9;
				}
				break;
			case GPS_FEILD9:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD10;
				}
				break;
			case GPS_FEILD10:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD11;
				}
				break;
			case GPS_FEILD11:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_FEILD12;
				}
				break;
			case GPS_FEILD12:
				rt_device_read(newdev,0,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state == GPS_END;
				}
				break;
			case GPS_END:
				rt_kprintf("%c:%s %c:%s\r\n",
				NS_FLAG,NS,EW_FLAG,EW);
				state = GPS_START;
				rt_thread_delay(300);//分配时间片给其他线程
				
				break;
			default:
				state = GPS_START;
				rt_thread_delay(300);//分配时间片给其他线程
				break;
		}

		
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
#if 1
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
							4096, 12, 20);
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
