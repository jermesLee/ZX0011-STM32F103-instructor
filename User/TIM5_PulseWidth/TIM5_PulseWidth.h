#ifndef __TIM5_PULSEWIDTH_H_
#define	__TIM5_PULSEWIDTH_H_

#include "includes.h"

/****************************��ʱ����ʱ�ж����ȼ�*************************/
#define TIM5_IRQPreemptionPrio   1                                        //TIM5����ռʽ���ȼ�      
#define TIM5_IRQSubPrio          2                                        //TIM5�Ĵ����ȼ�


/*****************************��ʱ���û�����******************************/
extern void TIM5_PluseWidthAdjustInit(void);                              //TIM5��ʼ�� 
extern void TIM5_SetOutputPluseWidth(uint16_t PluseWidth );	
extern FlagStatus TIM5_OutputPluseWidthFinish(void);


#endif 



