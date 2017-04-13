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
 * 2017-04-13          mingyan liu    example for GPS module with NMEA0183 protocol
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

void rt_init_thread_entry(void *parameter)
{
	/* initialization RT-Thread Components */
	rt_components_init();
}


static void uart1_read(rt_device_t newdev, rt_uint8_t *rx_buffer, int n) //n位固定读取的字节数
{
	
	rt_uint32_t rx_size;
	rt_uint8_t buf;
	int i=0;
	while(i<n)
	{
		while (1)//从串口中一次只读取一个字符
		{
     
			rx_size=rt_device_read(newdev,0,&buf,1);
			if (rx_size == 1) break;
		}
		rx_buffer[i]=buf;
		i++;
		
	}
	return;	
}
void rt_run_example_thread_entry(void *parameter)//我需要在这里加入代码（URT1）
{
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	
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
	char ch;
	char GNRMC[6];
	char UTChms[10],UTCdmy[6];
	char NS[12],EW[13];
	char NS_FLAG,EW_FLAG;
	int i;
	unsigned char state = GPS_START;//串口状态机状态标志
	rt_memset(rx_buffer,0,256);
	rt_memset(tx_buffer,0,256);
	rt_memset(GNRMC,0,6);
	rt_memset(UTChms,0,10);
	rt_memset(UTCdmy,0,6);
	rt_memset(NS,0,12);
	rt_memset(EW,0,13);
	for(;;)
	{
		
		
		switch(state)
		{
			case GPS_START:
				
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == '$')
				{
					uart1_read(newdev,rx_buffer,5);	
					if(rt_memcmp(rx_buffer,"GNRMC",5)== 0)
					{
						rt_memcpy(GNRMC,rx_buffer,6);
						//rt_kprintf("%s\n",GNRMC);
						state = GPS_FEILD1;
						
					}
				}
				break;
			case GPS_FEILD1://UTC time
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,10);
					rt_memcpy(UTChms,rx_buffer,10);
					state = GPS_FEILD2;
					
				}
				break;
			case GPS_FEILD2:// A/V
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,1);
						state = GPS_FEILD3;
				}
				break;
			case GPS_FEILD3:// 纬度
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,11);
					rt_memset(NS,0,12);
					rt_memcpy(NS,rx_buffer,11);
					state = GPS_FEILD4;
				}
				break;
			case GPS_FEILD4:
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,1);
					rt_memcpy(&NS_FLAG,rx_buffer,1);
					state = GPS_FEILD5;
				}
				break;
			case GPS_FEILD5:// 经度
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,12);
					rt_memset(EW,0,12);
					rt_memcpy(EW,rx_buffer,12);
					state = GPS_FEILD6;
				}
				break;
			case GPS_FEILD6:
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,1);
					rt_memcpy(&EW_FLAG,rx_buffer,1);
					state = GPS_FEILD7;
				}
				break;
			case GPS_FEILD7: //地面速率  000.0~999.9
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,5);
					state = GPS_FEILD8;
				}
				break;
			case GPS_FEILD8://地面航向
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,7);
					state = GPS_FEILD9;
				}
				break;
			case GPS_FEILD9://日月年
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,6);
					state = GPS_FEILD10;
				}
				break;
			case GPS_FEILD10:
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state = GPS_FEILD11;
				}
				break;
			case GPS_FEILD11://磁偏角
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					uart1_read(newdev,rx_buffer,1);
					state = GPS_FEILD12;
				}
				break;
			case GPS_FEILD12://磁偏角方向
				uart1_read(newdev,rx_buffer,1);
				if(rx_buffer[0] == ',')
				{
					state = GPS_END;
				}
				break;
			case GPS_END:
				rt_kprintf("%c:%s %c:%s\t\r\n",NS_FLAG,NS,EW_FLAG,EW);
				state = GPS_START;
				rt_thread_delay(10);//分配时间片给其他线程
				break;
			default:
				state = GPS_START;
				rt_thread_delay(10);//分配时间片给其他线程
				break;
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
	/*create example thread*/
	tid1 = rt_thread_create("example",
							rt_run_example_thread_entry, RT_NULL,
							4096, 16, 20);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);
	else
		return -1;
	
}
