#ifndef __TIME7_H_
#define	__TIME7_H_

#include "includes.h"

/****************************定时器定时中断优先级*************************/
#define TIM7_TimeTick            2                                        //TIM7定时时间 ,用于定时的基准

/****************************定时器定时中断优先级*************************/
#define TIM7_IRQPreemptionPrio   3                                        //TIM7的抢占式优先级      
#define TIM7_IRQSubPrio          3                                        //TIM7的次优先级


/*****************************定时器用户函数******************************/
extern void TIM7_Init(void);                                              //TIME6初始化 ,单位 ： ms ，最小1ms ，最大1200ms
extern FlagStatus TIM7_20MsFinish(void);                                  //定时10ms ， 定时时间到 返回SET ,否则返回RESET
extern FlagStatus TIM7_50MsFinish(void);                                  //定时50ms ， 定时时间到 返回SET ,否则返回RESET
extern FlagStatus TIM7_200MsFinish(void);                                 //定时200ms , 定时时间到 返回SET ,否则返回RESET
extern FlagStatus TIM7_1sFinish(void);                                    //定时1S    , 定时时间到 返回SET ,否则返回RESET
extern uint32_t TIM7_ReadTimeCount(void);
#endif 



