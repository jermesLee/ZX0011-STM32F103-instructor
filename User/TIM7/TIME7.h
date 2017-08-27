#ifndef __TIME7_H_
#define	__TIME7_H_

#include "includes.h"

/****************************��ʱ����ʱ�ж����ȼ�*************************/
#define TIM7_TimeTick            2                                        //TIM7��ʱʱ�� ,���ڶ�ʱ�Ļ�׼

/****************************��ʱ����ʱ�ж����ȼ�*************************/
#define TIM7_IRQPreemptionPrio   3                                        //TIM7����ռʽ���ȼ�      
#define TIM7_IRQSubPrio          3                                        //TIM7�Ĵ����ȼ�


/*****************************��ʱ���û�����******************************/
extern void TIM7_Init(void);                                              //TIME6��ʼ�� ,��λ �� ms ����С1ms �����1200ms
extern FlagStatus TIM7_20MsFinish(void);                                  //��ʱ10ms �� ��ʱʱ�䵽 ����SET ,���򷵻�RESET
extern FlagStatus TIM7_50MsFinish(void);                                  //��ʱ50ms �� ��ʱʱ�䵽 ����SET ,���򷵻�RESET
extern FlagStatus TIM7_200MsFinish(void);                                 //��ʱ200ms , ��ʱʱ�䵽 ����SET ,���򷵻�RESET
extern FlagStatus TIM7_1sFinish(void);                                    //��ʱ1S    , ��ʱʱ�䵽 ����SET ,���򷵻�RESET
extern uint32_t TIM7_ReadTimeCount(void);
#endif 



