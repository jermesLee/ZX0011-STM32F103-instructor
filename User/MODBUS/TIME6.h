#ifndef __TIME6_H_
#define	__TIME6_H_

#include "includes.h"

/*************************TIM6 定时中断优先级宏定义**********************/ 
#define TIME6_IRQPreemptionPrio        2                                  //TIM6的抢占式优先级      
#define TIME6_IRQSubPrio               1                                  //TIM6的次优先级


/*************************TIM6 定时用户函数******************************/ 
extern void TIM6_Init(uint8_t MsTime);                                    //TIME6初始化 ，初始化中未使能定时器定时  ,单位 ： ms
extern void TIM6_Open(void);                                              //TIM6打开 
extern void TIM6_Close(void);                                             //TIM6关闭

#endif 


/*****************************FILE END***********************************/
