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
 * 2017-05-18		   jiangchuan Li  online debug by li(from nuaa), coding by chinesebear
 */

#include <rtthread.h>
#include <components.h>
#include "net/synopGMAC.h"
#include <lwip/api.h>

#define TCP_ECHO_PORT   7
#define UDP_ECHO_PORT   7

#define printf rt_kprintf

static void DumpData(const rt_uint8_t *pcStr,rt_uint8_t *pucBuf,rt_uint32_t usLen)
{

    rt_uint32_t i;
    rt_uint8_t acTmp[17];
    rt_uint8_t *p;
    rt_uint8_t *pucAddr = pucBuf;

    if(pcStr)
    {
        printf("%s: length = %d [0x%X]\r\n", pcStr, usLen, usLen);
    }
    if(usLen == 0)
    {
        return;
    }
    p = acTmp;
    printf("%p  ", pucAddr);
    for(i=0;i<usLen;i++)
    {

        printf("%02X ",pucBuf[i]);
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
            printf("        | %s", acTmp);
            p = acTmp;

            printf("\r\n");

            if((i+1) < usLen)
            {
                pucAddr += 16;
                printf("%p  ", pucAddr);
            }
        }
        else if((i+1)%8==0)
        {
            printf("- ");
        }
    }
    if(usLen%16!=0)
    {
        for(i=usLen%16;i<16;i++)
        {
            printf("   ");
            if(((i+1)%8==0) && ((i+1)%16!=0))
            {
                printf("- ");
            }
        }
        *p++ = 0;//string end
        printf("        | %s", acTmp);
        printf("\r\n");
    }
}

static int rt_netconn_recv(struct netconn *conn, struct netbuf **new_buf)
{
	err_t err;
	err = netconn_recv(conn,new_buf);
	if(*new_buf) 
		return 1;
	else 
		return 0;
}
void rt_tcpecho_entry(void *parameter)
{
	struct netconn *conn, *newconn;
	err_t err;
	struct netbuf *buf;
	void *data;
	u16_t len;
	rt_kprintf("tcpecho server start...\r\n");
	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);
	/* Bind connection to well known port number 7. */
	netconn_bind(conn, IP_ADDR_ANY, TCP_ECHO_PORT);
	/* Tell connection to go into listening mode. */
	netconn_listen(conn);
	for(;;)
	{
		/* Grab new connection. */
		err = netconn_accept(conn,&newconn);
		/* Process the new connection. */
		if(newconn != NULL)
		{
			while(rt_netconn_recv(newconn,&buf))
			{
				do
				{
					netbuf_data(buf, &data, &len);
					//DumpData("payload_rx",data,len);
					err = netconn_write(newconn, data, len, NETCONN_COPY);
					if(err != ERR_OK)
					{
						rt_kprintf("netconn_write(),ret=%d\r\n",err);
					}
				}while(netbuf_next(buf) >= 0);
				netbuf_delete(buf);
				buf = NULL;
			}
		/* Close connection and discard connection identifier. */
		netconn_delete(newconn);
		newconn = NULL;
		}
	}
}
void rt_udpecho_entry(void *parameter)
{	
	struct netconn *conn;	
	struct netbuf *buf;	
	struct ip_addr *addr;	
	unsigned short port;	
	rt_kprintf("udpecho server start...\r\n");
	conn = netconn_new(NETCONN_UDP);	
	netconn_bind(conn, IP_ADDR_ANY, UDP_ECHO_PORT);	
	for(;;)	
	{       
		/* received data to buffer */		
		netconn_recv(conn,&buf);
		addr = netbuf_fromaddr(buf);		
		port = netbuf_fromport(buf);        
		/* send the data to buffer */		
		netconn_connect(conn, addr, port);		
		/* reset address, and send to client */	
		buf->addr.addr = 0;
		netconn_send(conn, buf); 
		/* release buffer */		
		netbuf_delete(buf);
	}
}

void rt_init_thread_entry(void *parameter)
{
	
	/* initialization RT-Thread Components */
	rt_components_init();
	rt_hw_eth_init();
}

int rt_application_init(void)
{
	rt_thread_t tid,echo_tid,udpecho_tid;

	/* create initialization thread */
	tid = rt_thread_create("init",
							rt_init_thread_entry, RT_NULL,
							4096, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);

	echo_tid = rt_thread_create("tcpecho",
							rt_tcpecho_entry, RT_NULL,
				            4096, 30, 5);
    if (echo_tid != RT_NULL)
            rt_thread_startup(echo_tid);

	udpecho_tid = rt_thread_create("udpecho",
							rt_udpecho_entry, RT_NULL,
				            4096, 30, 5);
    if (udpecho_tid != RT_NULL)
            rt_thread_startup(udpecho_tid);

	return 0;
}
