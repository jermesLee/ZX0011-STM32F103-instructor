#ifndef __LED_H_
#define	__LED_H_

#include "includes.h"

/* ֱ�Ӳ����Ĵ����ķ�������IO */
#define	digitalHi(p,i)				{p->BSRR=i;}			//����ߵ�ƽ		
#define digitalLo(p,i)				{p->BRR	=i;}		  //����͵�ƽ
#define digitalToggle(p,i)		{p->ODR ^=i;}			//�����ת״̬


/* �������IO�ĺ� */
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

