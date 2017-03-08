/*
 * File      : gpio.c
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                 Author       Notes
 * 2015-07-20     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "gpio.h"

#define GPIO_MAX 17

struct rt_device gpio[GPIO_MAX];
rt_uint32_t gpio_num[GPIO_MAX] = {0,1,2,3,4,5,48,49,50,51,52,53,87,88,89,90,91};
char* gpio_string[GPIO_MAX] = {"GPIO0","GPIO1","GPIO2","GPIO3","GPIO4","GPIO5",\
	"GPIO48","GPIO49","GPIO50","GPIO51","GPIO52","GPIO53",\
	"GPIO87","GPIO88","GPIO89","GPIO90","GPIO91"};
rt_err_t rt_gpio_num(rt_device_t dev, rt_uint32_t * base_addr, rt_uint32_t * ctlbit_pos);


static rt_err_t rt_gpio_init(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);
	
	
	return RT_EOK;
}
static rt_err_t rt_gpio_open(rt_device_t dev, rt_uint16_t oflag)
{
	RT_ASSERT(dev != RT_NULL);
	return RT_EOK;
}
static rt_err_t rt_gpio_close(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);

	return RT_EOK;
}
static rt_size_t rt_gpio_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	rt_uint32_t  *p_buf =(rt_uint32_t*) buffer ;
	rt_gpio_num(dev, &p_gpio_base,&p_ctlbit_pos);
	rt_kprintf("p_gpio_base=0x%x,p_ctlbit_pos=%d\n",p_gpio_base,p_ctlbit_pos);
	rt_kprintf("GPIO_EN(p_gpio_base) is 0x%x\n",GPIO_EN(p_gpio_base));
	if (!(GPIO_EN(p_gpio_base)&(1<<p_ctlbit_pos)))
		rt_kprintf("GPIO is not read mode \n");
	*p_buf=(GPIO_IN(p_gpio_base)&(1<<p_ctlbit_pos))==0?0:1;
	rt_kprintf("GPIO input is %d\n",*p_buf);
	
	return 1;
}
static rt_size_t rt_gpio_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	rt_uint32_t  *p_buf =(rt_uint32_t*) buffer ;
	//rt_uint32_t  p_value= *p_buf ;
	rt_gpio_num(dev, &p_gpio_base,&p_ctlbit_pos);
	rt_kprintf("p_gpio_base=0x%x,p_ctlbit_pos=%d\n",p_gpio_base,p_ctlbit_pos);
	rt_kprintf("GPIO_EN(p_gpio_base) is 0x%x\n",GPIO_EN(p_gpio_base));
	if ((GPIO_EN(p_gpio_base)&(1<<p_ctlbit_pos)))
		rt_kprintf("GPIO is not write mode \n");
	rt_kprintf("GPIO_OUT(p_gpio_base) is 0x%x\n",GPIO_OUT(p_gpio_base));
	if( *p_buf  == 1)
		GPIO_OUT(p_gpio_base) |= (1<<p_ctlbit_pos);
	else if(*p_buf  == 0)
		GPIO_OUT(p_gpio_base) &= ~(1<<p_ctlbit_pos);
	else
		rt_kprintf("GPIO output value is wrong \n");
	rt_kprintf("GPIO_OUT(p_gpio_base) is 0x%x now\n",GPIO_OUT(p_gpio_base));
	return 1;
}

rt_err_t rt_gpio_num(rt_device_t dev,  rt_uint32_t * base_addr, rt_uint32_t * ctlbit_pos)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t * p_tmp = (rt_uint32_t *)dev->user_data;
	rt_uint32_t p_gpio_num = *p_tmp;
	if (!((p_gpio_num >=0 && p_gpio_num <= 5) \
		||(p_gpio_num >=48 && p_gpio_num <= 53)\
		||(p_gpio_num >=87 && p_gpio_num <= 91)))
	return RT_EIO;

	if (p_gpio_num >=0 && p_gpio_num <= 31)
	{
		*base_addr = GPIO_BASE0;
		*ctlbit_pos  = p_gpio_num -0;
	}
	else if(p_gpio_num >=32 && p_gpio_num <= 63)
	{
		*base_addr = GPIO_BASE1;
		*ctlbit_pos  = p_gpio_num -32;
	}
	else if(p_gpio_num >=64 && p_gpio_num <= 95)
	{
		*base_addr = GPIO_BASE2;
		*ctlbit_pos  = p_gpio_num -64;
	}
	else if(p_gpio_num >=96 && p_gpio_num <= 127)
	{
		*base_addr = GPIO_BASE3;
		*ctlbit_pos  = p_gpio_num -96;
	}
	else
		return RT_EIO;
}

rt_err_t  rt_gpio_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	
	if(rt_gpio_num(dev, &p_gpio_base,&p_ctlbit_pos) == RT_EIO)
		return RT_EIO;
	
	switch(cmd)
	{
		case GPIO_READ_MODE :
			GPIO_EN(p_gpio_base) |= (1<<p_ctlbit_pos);//1= in
			break;
		case GPIO_WRITE_MODE :
			GPIO_EN(p_gpio_base) &= ~(1<<p_ctlbit_pos);//0=out
			break;
		default: break;
	}
	return RT_EOK;
}
void rt_hw_gpio_init(void)
{
	rt_uint32_t i;
	rt_device_t dev;
	for(i=0; i < GPIO_MAX; i++)
	{
		dev = &gpio[i];
		dev->type       = RT_Device_Class_Char;
		dev->init         = rt_gpio_init;
		dev->open       = rt_gpio_open;
		dev->close       = rt_gpio_close;
		dev->read        = rt_gpio_read;
		dev->write       = rt_gpio_write;
		dev->control     = rt_gpio_control;
		dev->user_data  = &gpio_num[i];
		rt_device_register(&gpio[i], gpio_string[i],RT_DEVICE_FLAG_RDWR );
	}

}




