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

#define AT_NO_SLEEP					0
#define AT_SET_APN					1
#define AT_ACIVIATE_PDP				2
#define AT_CREATE_TCP				3
#define AT_SET_PASSTHROUGH			4
#define AT_PASSTHROUGH_TX			5
#define AT_PASSTHROUGH_RX			6
#define AT_CLOSE_SOCKET				7
#define AT_IDLE						8
#define AT_NOP						9

extern rt_device_t uart_dev[];
rt_uint32_t edge1[2],edge2[2],edge3[2];
const rt_uint32_t high = 0;
const rt_uint32_t low = 1;
rt_uint8_t key_flag = 0;
int atstate = AT_NO_SLEEP;
int socketID;
rt_uint8_t pt_rx_buffer[256];
rt_uint8_t pt_tx_buffer[256];
int pt_tx_size = 0;
int pt_rx_size = 0;
static void DumpData(const rt_uint8_t *pcStr,rt_uint8_t *pucBuf,rt_uint32_t usLen)
{

    rt_uint32_t i;
    rt_uint8_t acTmp[17];
    rt_uint8_t *p;
    rt_uint8_t *pucAddr = pucBuf;

    if(pcStr)
    {
        rt_kprintf("%s: length = %d [0x%X]\r\n", pcStr, usLen, usLen);
    }
    if(usLen == 0)
    {
        return;
    }
    p = acTmp;
    rt_kprintf("%p  ", pucAddr);
    for(i=0;i<usLen;i++)
    {

        rt_kprintf("%02X ",pucBuf[i]);
        if((pucBuf[i]>=0x20) && (pucBuf[i]<0x7F))
        {
            *p++ = pucBuf[i];
        }
        else
        {
            *p++ = '.';
        }
        if((i+1)%16==0)
        {
            *p++ = 0;//string end
            rt_kprintf("        | %s", acTmp);
            p = acTmp;

            rt_kprintf("\r\n");

            if((i+1) < usLen)
            {
                pucAddr += 16;
                rt_kprintf("%p  ", pucAddr);
            }
        }
        else if((i+1)%8==0)
        {
            rt_kprintf("- ");
        }
    }
    if(usLen%16!=0)
    {
        for(i=usLen%16;i<16;i++)
        {
            rt_kprintf("   ");
            if(((i+1)%8==0) && ((i+1)%16!=0))
            {
                rt_kprintf("- ");
            }
        }
        *p++ = 0;//string end
        rt_kprintf("        | %s", acTmp);
        rt_kprintf("\r\n");
    }
}

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

		rt_thread_delay(100);
	}
	rt_kprintf("never get here...\r\n");
}

void rt_run_example2_thread_entry(void *parameter)
{

	rt_device_t newdev;
	rt_err_t iRet;
	rt_uint32_t rx_size,tx_size;
	char* ATcmd,PtData;
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
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
		rt_memset(rx_buffer,0,256);
		rt_memset(tx_buffer,0,256);
		switch(atstate)
		{
			case AT_NO_SLEEP:
				rt_kprintf("[AT_NO_SLEEP] enter\r\n");
				ATcmd = "AT+ESLP=0";
				rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
				rt_thread_delay(1);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rt_memcmp(rx_buffer,"OK",2) == 0)
				{
					atstate = AT_SET_APN;
					rt_kprintf("[AT_NO_SLEEP] OK\r\n");
					
				}
				else
				{
					rt_kprintf("[AT_NO_SLEEP] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_NO_SLEEP] exit\r\n");
				break;
			case AT_SET_APN:
				rt_kprintf("[AT_SET_APN] enter\r\n");
				ATcmd = "AT+EGDCONT=0,\"IP\",\"CMNET\"";
				rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
				rt_thread_delay(1);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rt_memcmp(rx_buffer,"OK",2) == 0)
				{
					atstate = AT_ACIVIATE_PDP;
					rt_kprintf("[AT_SET_APN] OK\r\n");
				}
				else
				{
					rt_kprintf("[AT_SET_APN] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_SET_APN] exit\r\n");
				break;
			case AT_ACIVIATE_PDP:
				rt_kprintf("[AT_ACIVIATE_PDP] enter\r\n");
				ATcmd = "AT+ETCPIP=1,0";
				rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
				rt_thread_delay(1);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rt_memcmp(rx_buffer,"OK",2) == 0)
				{
					atstate = AT_CREATE_TCP;
					rt_kprintf("[AT_ACIVIATE_PDP] OK\r\n");
				}
				else
				{
					rt_kprintf("[AT_ACIVIATE_PDP] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_ACIVIATE_PDP] exit\r\n");
				break;
			case AT_CREATE_TCP:
				rt_kprintf("[AT_CREATE_TCP] enter\r\n");
				ATcmd = "AT+ETL=1,0,0,\"121.43.113.60\",6000";
				rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
				rt_thread_delay(1);
				rt_memset(rx_buffer,0,256);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rt_memcmp(rx_buffer,"+ETL:",5)== 0)
				{
					atstate = AT_SET_PASSTHROUGH;
					socketID = rx_buffer[rx_size] -'0';// maybe bug in future
					rt_kprintf("[AT_CREATE_TCP] %s\r\n",rx_buffer);
				}
				else
				{
					rt_kprintf("[AT_CREATE_TCP] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_CREATE_TCP] exit\r\n");
				break;
			case AT_SET_PASSTHROUGH:
				rt_kprintf("[AT_SET_PASSTHROUGH] enter\r\n");
				ATcmd = "AT+ETLTS=";
				rt_snprintf(tx_buffer,strlen(ATcmd)+1,"%s%d",ATcmd,socketID);
				rt_device_write(newdev,0,tx_buffer,strlen(ATcmd)+1);
				rt_thread_delay(1);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rx_size == 0)
				{
					atstate = AT_PASSTHROUGH_TX;
					rt_kprintf("[AT_ACIVIATE_PDP] OK\r\n");
				}
				else
				{
					rt_kprintf("[AT_ACIVIATE_PDP] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_SET_PASSTHROUGH] exit\r\n");
				break;
			case AT_PASSTHROUGH_TX:
				rt_kprintf("[AT_PASSTHROUGH_TX] enter\r\n");
				rt_device_write(newdev,0,pt_tx_buffer,pt_tx_size);
				atstate = AT_PASSTHROUGH_RX;
				DumpData("pt_data_tx",pt_tx_buffer,pt_tx_size);
				pt_tx_size = 0;
				rt_memset(pt_tx_buffer,0,256);
				rt_kprintf("[AT_PASSTHROUGH_TX] exit\r\n");
				break;
			case AT_PASSTHROUGH_RX:
				rt_kprintf("[AT_PASSTHROUGH_RX] enter\r\n");
				rt_memset(pt_rx_buffer,0,256);
				pt_rx_size = rt_device_read(newdev,0,pt_rx_buffer,256);
				atstate = AT_IDLE;
				DumpData("pt_data_rx",pt_rx_buffer,pt_rx_size);
				rt_kprintf("[AT_PASSTHROUGH_RX] exit\r\n");
				break;
			case AT_CLOSE_SOCKET:
				rt_kprintf("[AT_CLOSE_SOCKET] enter\r\n");
				ATcmd = "AT+ETL=0,";
				rt_memset(tx_buffer,0,256);
				rt_snprintf(tx_buffer,strlen(ATcmd)+1,"%s%d",ATcmd,socketID);
				rt_device_write(newdev,0,tx_buffer,strlen(ATcmd)+1);
				rt_thread_delay(1);
				rt_memset(rx_buffer,0,256);
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				if(rt_memcmp(rx_buffer,"OK",2) == 0)
				{
					atstate = AT_NOP;
					rt_kprintf("[AT_CLOSE_SOCKET] OK\r\n");
				}
				else
				{
					rt_kprintf("[AT_CLOSE_SOCKET] %s\r\n",rx_buffer);
				}
				rt_kprintf("[AT_CLOSE_SOCKET] exit\r\n");
				break;
			case AT_IDLE:
				rt_kprintf("[AT_IDLE] enter\r\n");
				if(pt_tx_size) atstate = AT_PASSTHROUGH_TX;
				rt_kprintf("[AT_IDLE] exit\r\n");
				break;
			case AT_NOP:
			default:
				rt_kprintf(".");
				rt_thread_delay(100);
				break;
		}
		rt_thread_delay(1);
		// test code -fill data in passthrough tx buffer
		pt_tx_size = strlen("hello world");
		rt_memset(pt_tx_buffer,0,256);
		rt_memcpy(pt_tx_buffer,"hello world",pt_tx_size);
		
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
	#endif

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
