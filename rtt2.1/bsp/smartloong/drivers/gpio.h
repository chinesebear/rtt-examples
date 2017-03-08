/*
 * File      : gpio.h
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                Author             Notes
 * 2015-07-20     chinesebear    first version
 */


#ifndef __GPIO_H__
#define __GPIO_H__
 
#include "ls1c.h"


#define  GPIO_READ_MODE     (0)
#define  GPIO_WRITE_MODE   (1)

void rt_hw_gpio_init(void);



#endif



