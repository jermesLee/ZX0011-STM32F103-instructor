#ifndef __RS485_H_
#define	__RS485_H_

#include "stm32f10x.h"

#define RS485_SetSendMode()      GPIO_SetBits(   GPIOA , GPIO_Pin_1 )   // ���ƶ�Ϊ 1 ��RS485���ڷ���ģʽ
#define RS485_SetReceiveMode()   GPIO_ResetBits( GPIOA , GPIO_Pin_1 )   // ���ƶ�Ϊ 0 ��RS485���ڽ���ģʽ

extern void RS485_ControlPinInit(void);



#endif /* LED.H*/

