#ifndef __LED_H_
#define	__LED_H_

#include "includes.h"

/* 直接操作寄存器的方法控制IO */
#define	digitalHi(p,i)				{p->BSRR=i;}			//输出高电平		
#define digitalLo(p,i)				{p->BRR	=i;}		  //输出低电平
#define digitalToggle(p,i)		{p->ODR ^=i;}			//输出反转状态


/* 定义控制IO的宏 */
#define LED1_TOGGLE()		digitalToggle(GPIOD,GPIO_Pin_1)
#define LED1_OFF()			digitalHi(GPIOD,GPIO_Pin_1)
#define LED1_ON()		   	digitalLo(GPIOD,GPIO_Pin_1)

#define LED2_TOGGLE()		digitalToggle(GPIOD,GPIO_Pin_2)
#define LED2_OFF()			digitalHi(GPIOD,GPIO_Pin_2)
#define LED2_ON()		   	digitalLo(GPIOD,GPIO_Pin_2)

#define LED3_TOGGLE()		digitalToggle(GPIOD,GPIO_Pin_3)
#define LED3_OFF()			digitalHi(GPIOD,GPIO_Pin_3)
#define LED3_ON()		   	digitalLo(GPIOD,GPIO_Pin_3)






extern void LED_GPIO_Config(void);


#endif /* LED.H*/

