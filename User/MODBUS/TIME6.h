#ifndef __TIME6_H_
#define	__TIME6_H_

#include "includes.h"

/*************************TIM6 ��ʱ�ж����ȼ��궨��**********************/ 
#define TIME6_IRQPreemptionPrio        2                                  //TIM6����ռʽ���ȼ�      
#define TIME6_IRQSubPrio               1                                  //TIM6�Ĵ����ȼ�


/*************************TIM6 ��ʱ�û�����******************************/ 
extern void TIM6_Init(uint8_t MsTime);                                    //TIME6��ʼ�� ����ʼ����δʹ�ܶ�ʱ����ʱ  ,��λ �� ms
extern void TIM6_Open(void);                                              //TIM6�� 
extern void TIM6_Close(void);                                             //TIM6�ر�

#endif 


/*****************************FILE END***********************************/
