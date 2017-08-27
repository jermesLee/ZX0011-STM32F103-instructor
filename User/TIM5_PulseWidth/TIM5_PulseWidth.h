#ifndef __TIM5_PULSEWIDTH_H_
#define	__TIM5_PULSEWIDTH_H_

#include "includes.h"

/****************************定时器定时中断优先级*************************/
#define TIM5_IRQPreemptionPrio   1                                        //TIM5的抢占式优先级      
#define TIM5_IRQSubPrio          2                                        //TIM5的次优先级


/*****************************定时器用户函数******************************/
extern void TIM5_PluseWidthAdjustInit(void);                              //TIM5初始化 
extern void TIM5_SetOutputPluseWidth(uint16_t PluseWidth );	
extern FlagStatus TIM5_OutputPluseWidthFinish(void);


#endif 



