#ifndef __RS485_H_
#define	__RS485_H_

#include "stm32f10x.h"

#define RS485_SetSendMode()      GPIO_SetBits(   GPIOA , GPIO_Pin_1 )   // 控制端为 1 ，RS485处于发送模式
#define RS485_SetReceiveMode()   GPIO_ResetBits( GPIOA , GPIO_Pin_1 )   // 控制端为 0 ，RS485处于接收模式

extern void RS485_ControlPinInit(void);



#endif /* LED.H*/

