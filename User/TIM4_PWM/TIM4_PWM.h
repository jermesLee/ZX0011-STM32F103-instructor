#ifndef __TIM4_PWM_H_
#define	__TIM4_PWM_H_

#include "stm32f10x.h"

#define TIM4_PWMOutputFrequency   1200                                    //PWM���Ƶ�� ����λ ��Hz ��,��СΪ ��1100 

/*******************************TIM4 �û�����****************************/ 
extern void  TIM4_PWM_Init(void);                                         //TIM4 ���PWM�źų�ʼ���ܺ���
extern void  TIM4_PWM_SetDutyRatio(uint16_t DutyRatio);                   //��ʱ��TIM4������pwm���ռ�ձ����� ����λ ��0.1%


#endif /* __PWM_OUTPUT_H */





