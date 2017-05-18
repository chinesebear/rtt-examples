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
#define AT_START					0
#define AT_NO_SLEEP					1
#define AT_SET_IEMI					2
#define AT_SET_APN					3
#define AT_ACIVIATE_PDP				4
#define AT_CREATE_TCP				5
#define AT_SET_PASSTHROUGH			6
#define AT_PASSTHROUGH_TX			7
#define AT_PASSTHROUGH_RX			8
#define AT_CLOSE_SOCKET				9
#define AT_IDLE						10
#define AT_NOP						11

#define AT_DEBUG					0
#define SEND_TO_GSM					0
#define RECV_FROM_GSM				1
extern rt_device_t uart_dev[];
rt_uint32_t edge1[2],edge2[2],edge3[2];
const rt_uint32_t high = 0;
const rt_uint32_t low = 1;
rt_uint8_t key_flag = 0;
int atstate = AT_START;
int linkgsm = SEND_TO_GSM;
int socketID;
rt_uint8_t pt_rx_buffer[256];
rt_uint8_t pt_tx_buffer[256];
int pt_tx_size = 0;
int pt_rx_size = 0;

static const rt_uint16_t crctab16[] =
{
0X0000, 0X1189, 0X2312, 0X329B, 0X4624, 0X57AD, 0X6536, 0X74BF,
0X8C48, 0X9DC1, 0XAF5A, 0XBED3, 0XCA6C, 0XDBE5, 0XE97E, 0XF8F7,
0X1081, 0X0108, 0X3393, 0X221A, 0X56A5, 0X472C, 0X75B7, 0X643E,
0X9CC9, 0X8D40, 0XBFDB, 0XAE52, 0XDAED, 0XCB64, 0XF9FF, 0XE876,
0X2102, 0X308B, 0X0210, 0X1399, 0X6726, 0X76AF, 0X4434, 0X55BD,
0XAD4A, 0XBCC3, 0X8E58, 0X9FD1, 0XEB6E, 0XFAE7, 0XC87C, 0XD9F5,
0X3183, 0X200A, 0X1291, 0X0318, 0X77A7, 0X662E, 0X54B5, 0X453C,
0XBDCB, 0XAC42, 0X9ED9, 0X8F50, 0XFBEF, 0XEA66, 0XD8FD, 0XC974,
0X4204, 0X538D, 0X6116, 0X709F, 0X0420, 0X15A9, 0X2732, 0X36BB,
0XCE4C, 0XDFC5, 0XED5E, 0XFCD7, 0X8868, 0X99E1, 0XAB7A, 0XBAF3,
0X5285, 0X430C, 0X7197, 0X601E, 0X14A1, 0X0528, 0X37B3, 0X263A,
0XDECD, 0XCF44, 0XFDDF, 0XEC56, 0X98E9, 0X8960, 0XBBFB, 0XAA72,
0X6306, 0X728F, 0X4014, 0X519D, 0X2522, 0X34AB, 0X0630, 0X17B9,
0XEF4E, 0XFEC7, 0XCC5C, 0XDDD5, 0XA96A, 0XB8E3, 0X8A78, 0X9BF1,
0X7387, 0X620E, 0X5095, 0X411C, 0X35A3, 0X242A, 0X16B1, 0X0738,
0XFFCF, 0XEE46, 0XDCDD, 0XCD54, 0XB9EB, 0XA862, 0X9AF9, 0X8B70,
0X8408, 0X9581, 0XA71A, 0XB693, 0XC22C, 0XD3A5, 0XE13E, 0XF0B7,
0X0840, 0X19C9, 0X2B52, 0X3ADB, 0X4E64, 0X5FED, 0X6D76, 0X7CFF,
0X9489, 0X8500, 0XB79B, 0XA612, 0XD2AD, 0XC324, 0XF1BF, 0XE036,
0X18C1, 0X0948, 0X3BD3, 0X2A5A, 0X5EE5, 0X4F6C, 0X7DF7, 0X6C7E,
0XA50A, 0XB483, 0X8618, 0X9791, 0XE32E, 0XF2A7, 0XC03C, 0XD1B5,
0X2942, 0X38CB, 0X0A50, 0X1BD9, 0X6F66, 0X7EEF, 0X4C74, 0X5DFD,
0XB58B, 0XA402, 0X9699, 0X8710, 0XF3AF, 0XE226, 0XD0BD, 0XC134,
0X39C3, 0X284A, 0X1AD1, 0X0B58, 0X7FE7, 0X6E6E, 0X5CF5, 0X4D7C,
0XC60C, 0XD785, 0XE51E, 0XF497, 0X8028, 0X91A1, 0XA33A, 0XB2B3,
0X4A44, 0X5BCD, 0X6956, 0X78DF, 0X0C60, 0X1DE9, 0X2F72, 0X3EFB,
0XD68D, 0XC704, 0XF59F, 0XE416, 0X90A9, 0X8120, 0XB3BB, 0XA232,
0X5AC5, 0X4B4C, 0X79D7, 0X685E, 0X1CE1, 0X0D68, 0X3FF3, 0X2E7A,
0XE70E, 0XF687, 0XC41C, 0XD595, 0XA12A, 0XB0A3, 0X8238, 0X93B1,
0X6B46, 0X7ACF, 0X4854, 0X59DD, 0X2D62, 0X3CEB, 0X0E70, 0X1FF9,
0XF78F, 0XE606, 0XD49D, 0XC514, 0XB1AB, 0XA022, 0X92B9, 0X8330,
0X7BC7, 0X6A4E, 0X58D5, 0X495C, 0X3DE3, 0X2C6A, 0X1EF1, 0X0F78,
};
//GT02A info feild
struct gt02a_info_field{
	rt_uint8_t datetime[6];
	rt_uint8_t latitude[4];
	rt_uint8_t longitude[4];
	rt_uint8_t speed[1];
	rt_uint8_t direction[2];
	rt_uint8_t MNC[1];
	rt_uint8_t cellID[2];
	rt_uint8_t state[4];
};
//GT02A protocol
struct gt02a_packet{
	rt_uint8_t start[2];
	rt_uint8_t length;
	rt_uint8_t LAC[2];
	rt_uint8_t terID[8];
	rt_uint8_t infoSN[2];
	rt_uint8_t protocolNO[1];
	struct gt02a_info_field info;
	rt_uint8_t exit[2];
}GpsPacket;

static rt_uint32_t infoSnCnt = 1;
#define  atCmdGapTime  		300 //tick
#define  atUart3RxTimeOut   400 //tick


static void DumpData(const rt_uint8_t *pcStr,rt_uint8_t *pucBuf,rt_uint32_t usLen)
{

    rt_uint32_t i;
    rt_uint8_t acTmp[17];
    rt_uint8_t *p;
    rt_uint8_t *pucAddr = pucBuf;
    if(usLen == 0)
    {
        return;
    }
	    if(pcStr)
    {
        rt_kprintf("%s: length = %d [0x%X]\r\n", pcStr, usLen, usLen);
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
static int AtStr2Hex(const char* src, unsigned char* dest)
{
	int srcLen,destLen;
	unsigned char val;
	unsigned char tmp;
	int i =0;
	//rt_kprintf("Entering AtStr2Hex()\r\n");
	if(src == NULL || dest == NULL)
	{
		return -1;
	}
	srcLen = strlen(src);
	//rt_kprintf("srcLen = %d\r\n",srcLen);
	if(srcLen < 1)
	{
		return -2;
	}

	if(srcLen%2)
		destLen = srcLen/2 + 1;
	else
		destLen = srcLen/2;
	rt_memset(dest,0x00,destLen);	//clr dest buf
	//rt_kprintf("AtStr2Hex()--1\r\n");
	for(i=0;i < srcLen;i++)
	{
		tmp = *(src+ i);
		if(tmp >= '0' && tmp <= '9')
		{
			val = tmp -'0';
		}
		else if(tmp >= 'A' && tmp <= 'F')
		{
			val = tmp -'A' + 10;
		}
		else 
			return -3;
		if(i%2 == 0)//high half byte
		{
			*(dest+ i/2) |= (val<<4);
		}
		else // low half byte
		{
			*(dest+ i/2) |= val;
		}
		//rt_kprintf("dest(%d) = 0x%02X\r\n",i/2,*(dest+ i/2));
	}
	return 0;
}
static rt_uint16_t AtGetCrc16(const rt_uint8_t* pData, int nLength)
{
	rt_uint16_t fcs = 0xffff; 
	while(nLength>0){
	fcs = (fcs >> 8) ^ crctab16[(fcs ^ *pData) & 0xff];
	nLength--;
	pData++;
}
return ~fcs;
}

static int AtReponseDataCompare(rt_uint8_t* rxdata, int rxsize,const char* comparedta,int * pos)
{
	int i =0;
	int tpos = 0;
	int cmpStrSize = 0,flagCmp = 0;
	if(rxsize == 0)
	{
		rt_kprintf("compare rxsize=%d\r\n",rxsize);
		return 0;
	}
	while(i<rxsize)
	{

		if(rxdata[i] != '\r' && rxdata[i] != '\n')
		{
			*pos = i;
			tpos = i;
			break;
		}
		i++;
	}
	i =0;
	while(i<rxsize )
	{
		if(rxdata[i] == '\r' || rxdata[i] == '\n')
		{
			rxdata[i] = '\0';
		}
		i++;
	}
	cmpStrSize =strlen(comparedta);
	#if AT_DEBUG
	DumpData("DataCompare",(rxdata+tpos),(rxsize-tpos));
	#endif
	for(i = 0; i<(rxsize-tpos) ;i++)
	{
		if(rt_memcmp(comparedta,(rxdata+tpos),cmpStrSize)== 0)
		{
			flagCmp  = 1;
			break;
		}
	}
	if(flagCmp)
		return 0;
	else
		return tpos;// response data stata postion
}
static int AtStrCompare(const void * recvBuf,const int recvSize,const void* cmpStr)
{
	int i,iRet =0;
	int cmpStrSize;
	cmpStrSize = strlen(cmpStr);
	#if AT_DEBUG
	{
		DumpData("StrCompare",(rt_uint8_t *)recvBuf,recvSize);
		DumpData("StrCmpwith",(rt_uint8_t *)cmpStr,cmpStrSize);
	}
	#endif
	for(i = 0; i< recvSize;i++)
	{
		if(rt_memcmp(recvBuf+i,cmpStr,cmpStrSize)==0)
		{
			iRet= 1;
			break;
		}
	}
	return iRet;
}
static int AtReturnSocketId(const rt_uint8_t * recvBuf,const int recvSize)
{
	int i,socketId = -1,offset = 0;
	const rt_uint8_t* cmpStr ="+ETL: ";
	int cmpStrSize = strlen(cmpStr);
	for(i = 0; i< recvSize;i++)
	{
		if(rt_memcmp(recvBuf+i,cmpStr,cmpStrSize)==0)
		{
			offset = i;
			break;
		}
	}
	socketId = recvBuf[offset+6] -'0';
	return socketId;
}

static void AtCombineGpsData(struct gt02a_packet * packet,rt_uint8_t* txbuffer)
{
	if(packet == NULL || txbuffer == NULL)return ;
	const char* ptDat = "686825266A03586880000001580001100A0C1E0A2E05027AC8390C4657C5000156001DF1000000060D0A";
	AtStr2Hex(ptDat,txbuffer);
	#if 0
	rt_memset(txbuffer,0x68,2);//start
	rt_memset(txbuffer+2,0x25,1);//length 37
	rt_memcpy(txbuffer+3,packet->LAC,2);
	rt_memcpy(txbuffer+5,packet->terID,8);
	rt_memcpy(txbuffer+13,packet->infoSN,2);
	rt_memcpy(txbuffer+15,packet->protocolNO,1);
	rt_memcpy(txbuffer+16,packet->info.datetime,6);
	rt_memcpy(txbuffer+22,packet->info.latitude,4);
	rt_memcpy(txbuffer+26,packet->info.longitude,4);
	rt_memcpy(txbuffer+30,packet->info.speed,1);
	rt_memcpy(txbuffer+31,packet->info.direction,2);
	rt_memcpy(txbuffer+33,packet->info.MNC,1);
	rt_memcpy(txbuffer+34,packet->info.cellID,2);
	rt_memcpy(txbuffer+36,packet->info.state,4);
	rt_memset(txbuffer+40,0x0D,1);//exit
	rt_memset(txbuffer+41,0x0A,1);
	#endif
}
static rt_size_t AtUart3Read(rt_device_t dev,void *buffer,rt_size_t   size)
{
	int rx_size = 0,rx_size1 =0;
	int i=0,j=0,flag = 0;
	unsigned char * tmp;
	tmp = (unsigned char*)buffer;
	while(i < atUart3RxTimeOut)
	{
		// check string with key words "ERROR" & "OK"
		if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
		{
			break;
		}
		rt_thread_delay(2);
		rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
		rx_size += rx_size1;//update rx_size value
		if(rx_size)break;
		i++;
	}
	DumpData("Uart3Rx",buffer,rx_size);
	return rx_size;
}

static rt_size_t AtUart3ReadTimeOut(rt_device_t dev,void *buffer,rt_size_t   size,int TimeOutMs)
{
	int rx_size = 0,rx_size1 =0;
	int i=0,j=0,flag = 0;
	int flagLoop=1;
	int ticktimeout=0;
	unsigned char * tmp;
	tmp = (unsigned char*)buffer;
	if(TimeOutMs == -1)
	{
		while(1)
		{
			// check string with key words "ERROR" & "OK"
			if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
			{
				break;
			}
			rt_thread_delay(2);
			rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
			rx_size += rx_size1;//update rx_size value
		}
	}
	else
	{
		ticktimeout = TimeOutMs / 20;
		while(i < ticktimeout)
		{
			// check string with key words "ERROR" & "OK"
			if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
			{
				break;
			}
			rt_thread_delay(2);
			rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
			rx_size += rx_size1;//update rx_size value
			i++;
		}
	}
	DumpData("AtUart3Rx",buffer,rx_size);
	return rx_size;
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
		GpsPacket.length = 0x25;//37
		GpsPacket.LAC[0] = 0xFF;
		GpsPacket.LAC[1] = 0xFE;
		rt_memcpy(GpsPacket.terID,"\x01\x23\x45\x67\x89\x01\x23\x45",8);
		/*
		GpsPacket.terID[0] = 0x01;
		GpsPacket.terID[1] = 0x23;
		GpsPacket.terID[2] = 0x45;
		GpsPacket.terID[3] = 0x67;
		GpsPacket.terID[4] = 0x89;
		GpsPacket.terID[5] = 0x01;
		GpsPacket.terID[6] = 0x23;
		GpsPacket.terID[7] = 0x45;
		*/
		GpsPacket.infoSN[0] = infoSnCnt&0xFF;
		GpsPacket.infoSN[1] = ((infoSnCnt&0xFF00)>>8);
		rt_memcpy(GpsPacket.info.datetime,"\x0A\x03\x17\x0F\x32\x17",6);
		rt_memcpy(GpsPacket.info.latitude,"\x02\x6B\x3F\x3E",4);
		rt_memcpy(GpsPacket.info.longitude,"\x09\xA7\xEC\x80",4);
		rt_memset(GpsPacket.info.speed,0x10,1);
		rt_memcpy(GpsPacket.info.direction,"\x15\x4c",2);
		rt_memset(GpsPacket.info.MNC,0x00,1);
		rt_memcpy(GpsPacket.info.cellID,"\x00\x00\x00\x01",4);
		rt_memcpy(GpsPacket.info.state,"\x00\x00\x00\x07",4);
		if(pt_tx_size == 0)
		{
			AtCombineGpsData(&GpsPacket,pt_tx_buffer);
			pt_tx_size = 42;
		}
		rt_thread_delay(100);
	}
	rt_kprintf("never get here...\r\n");
}

void rt_run_example2_thread_entry(void *parameter)
{

	rt_device_t newdev;
	rt_uint32_t rx_size,tx_size;
	char* ATcmd,PtData;
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	int pos,iRet;
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
			case AT_START:
				rx_size = rt_device_read(newdev,0,rx_buffer,256);
				iRet = AtStrCompare(rx_buffer,rx_size,"+EUSIM: 1");
				if(iRet)
				{
					atstate = AT_NO_SLEEP;
					rt_kprintf("[AT_START] start gsm module\r\n");
				}
				break;
			case AT_NO_SLEEP:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_NO_SLEEP] enter\r\n");
					ATcmd = "AT+ESLP=0\n";
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_SET_IEMI;
						rt_kprintf("[AT_NO_SLEEP] OK\r\n");
						rt_thread_delay(atCmdGapTime);
						
					}
					else
					{
						rt_kprintf("[AT_NO_SLEEP](error)\r\n");
					}
					rt_kprintf("[AT_NO_SLEEP] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_NO_SLEEP] linkgsm state err\r\n");
				}
				break;
			case AT_SET_IEMI:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_SET_IEMI] enter\r\n");
					ATcmd = "AT+EGMR=1,7,\"868120114072751\"\n";
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_SET_APN;
						rt_kprintf("[AT_SET_IEMI] OK\r\n");
						rt_thread_delay(atCmdGapTime);
						
					}
					else
					{
						rt_kprintf("[AT_SET_IEMI] (error)\r\n");
					}
					rt_kprintf("[AT_SET_IEMI] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_SET_IEMI] linkgsm state err\r\n");
				}
				break;
			case AT_SET_APN:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_SET_APN] enter\r\n");
					ATcmd = "AT+EGDCONT=0,\"IP\",\"CMNET\"\n";
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_ACIVIATE_PDP;
						rt_kprintf("[AT_SET_APN] OK\r\n");
						rt_thread_delay(atCmdGapTime);
					}
					else
					{
						rt_kprintf("[AT_SET_APN] (error)\r\n");
					}
					rt_kprintf("[AT_SET_APN] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_SET_APN] linkgsm state err\r\n");
				}
				break;
			case AT_ACIVIATE_PDP:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_ACIVIATE_PDP] enter\r\n");
					ATcmd = "AT+ETCPIP=1,0\n";
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_CREATE_TCP;
						rt_kprintf("[AT_ACIVIATE_PDP] OK\r\n");
						rt_thread_delay(atCmdGapTime);
					}
					else
					{
						rt_kprintf("[AT_ACIVIATE_PDP](error)\r\n");
					}
					rt_kprintf("[AT_ACIVIATE_PDP] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_ACIVIATE_PDP] linkgsm state err\r\n");
				}
				break;
			case AT_CREATE_TCP:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_CREATE_TCP] enter\r\n");
					ATcmd = "AT+ETL=1,0,0,\"114.215.111.138\",5000\n";
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_SET_PASSTHROUGH;
						socketID = AtReturnSocketId(rx_buffer,rx_size);
						rt_kprintf("[AT_CREATE_TCP] socketID=%d\r\n",socketID);
						rt_thread_delay(atCmdGapTime);
					}
					else
					{
						rt_kprintf("[AT_CREATE_TCP] (ERROR)\r\n");
					}
					rt_kprintf("[AT_CREATE_TCP] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_CREATE_TCP] linkgsm state err\r\n");
				}
				
				break;
			case AT_SET_PASSTHROUGH:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_SET_PASSTHROUGH] enter\r\n");
					ATcmd = "AT+ETLTS=";
					rt_snprintf(tx_buffer,strlen(ATcmd)+2,"%s%d",ATcmd,socketID);
					tx_buffer[10] = 0x0A;
					DumpData("ATcmd",tx_buffer,strlen(tx_buffer));
					rt_device_write(newdev,0,tx_buffer,strlen(tx_buffer));
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,4000);
					//iRet = AtReponseDataCompare(rx_buffer,rx_size,"ERROR",&pos);
					{
						atstate = AT_PASSTHROUGH_TX;
						rt_kprintf("[AT_SET_PASSTHROUGH] OK\r\n");
						rt_thread_delay(atCmdGapTime);
					}
					rt_kprintf("[AT_SET_PASSTHROUGH] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_SET_PASSTHROUGH] linkgsm state err\r\n");
				}
				
				break;
			case AT_PASSTHROUGH_TX:
				rt_kprintf("[AT_PASSTHROUGH_TX] enter\r\n");
				rt_device_write(newdev,0,pt_tx_buffer,pt_tx_size);
				atstate = AT_PASSTHROUGH_RX;
				DumpData("pt_data_tx",pt_tx_buffer,pt_tx_size);
				infoSnCnt++;
				pt_tx_size = 0;
				rt_memset(pt_tx_buffer,0,256);
				rt_kprintf("[AT_PASSTHROUGH_TX] exit\r\n");
				break;
			case AT_PASSTHROUGH_RX:
				rt_kprintf("[AT_PASSTHROUGH_RX] enter\r\n");
				rt_thread_delay(100);
				rt_memset(pt_rx_buffer,0,256);
				pt_rx_size = rt_device_read(newdev,0,pt_rx_buffer,256);
				if(pt_rx_size)
				{
					atstate = AT_IDLE;
					DumpData("pt_data_rx",pt_rx_buffer,pt_rx_size);
				}
				else
				{
					atstate = AT_PASSTHROUGH_RX;
					rt_kprintf("[AT_PASSTHROUGH_RX] receive no data\r\n");
				}
				rt_kprintf("[AT_PASSTHROUGH_RX] exit\r\n");
				break;
			case AT_CLOSE_SOCKET:
				if(linkgsm == SEND_TO_GSM)
				{
					rt_kprintf("[AT_CLOSE_SOCKET] enter\r\n");
					ATcmd = "AT+ETL=0,";
					rt_memset(tx_buffer,0,256);
					rt_snprintf(tx_buffer,strlen(ATcmd)+1,"%s%d\n",ATcmd,socketID);
					DumpData("ATcmd",ATcmd,strlen(ATcmd));
					rt_device_write(newdev,0,tx_buffer,strlen(ATcmd)+1);
					linkgsm = RECV_FROM_GSM;
				}
				else if(linkgsm == RECV_FROM_GSM)
				{
					rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
					iRet = AtReponseDataCompare(rx_buffer,rx_size,"OK",&pos);
					if(iRet == 0 && rx_size)
					{
						atstate = AT_NOP;
						rt_kprintf("[AT_CLOSE_SOCKET] OK\r\n");
						rt_thread_delay(300);
					}
					else
					{
						rt_kprintf("[AT_CLOSE_SOCKET](error)\r\n");
					}
					rt_kprintf("[AT_CLOSE_SOCKET] exit\r\n");
					linkgsm = SEND_TO_GSM;
				}
				else
				{
					rt_kprintf("[AT_CLOSE_SOCKET] linkgsm state err\r\n");
				}
				break;
			case AT_IDLE:
				rt_kprintf("[AT_IDLE] enter\r\n");
				if(pt_tx_size) 
				{
					atstate = AT_PASSTHROUGH_TX;
					linkgsm = SEND_TO_GSM;
				}
				rt_kprintf("[AT_IDLE] exit\r\n");
				break;
			case AT_NOP:
			default:
				rt_kprintf(".");
				rt_thread_delay(100);
				break;
		}
		rt_thread_delay(1);		
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
