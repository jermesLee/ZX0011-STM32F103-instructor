#ifndef __TIM4_PWM_H_
#define	__TIM4_PWM_H_

#include "stm32f10x.h"

#define TIM4_PWMOutputFrequency   1200                                    //PWM输出频率 ，单位 ：Hz ，,最小为 ：1100 

/*******************************TIM4 用户函数****************************/ 
extern void  TIM4_PWM_Init(void);                                         //TIM4 输出PWM信号初始化总函数
extern void  TIM4_PWM_SetDutyRatio(uint16_t DutyRatio);                   //定时器TIM4产生的pwm输出占空比设置 ，单位 ：0.1%


#endif /* __PWM_OUTPUT_H */





