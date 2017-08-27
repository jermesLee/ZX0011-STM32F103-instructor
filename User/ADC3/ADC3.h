#ifndef __ADC3_H_
#define	__ADC3_H_

#include "includes.h"                                         
#include "TIME3.h"                                                        //定时器触发ADC的采样

/*********************ADC3采样数据保存内存分配***************************/
#define ADC3_OneChannelSampleNum           20                             //一个adc通道采样数
#define ADC3_ChannelScanNum                4                              //扫描模式下ADC通道数
#define ADC3_MemoryNum                    ( ADC3_OneChannelSampleNum * ADC3_ChannelScanNum )  // 计算ADC3总的内存大小


/**********************ADC3 外部定时器3触发初始化************************/
#define ADC3_ExternTriggerInit(TimeUs)    TIM3_Init(TimeUs)               //用于ADC3的外部触发 


/******************************ADC3 用户函数 ****************************/
extern void ADC3_Init(uint16_t SampleFrequency);                          //ADC3初始化
extern uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence );        //读取采样的平均值


#endif /* __PWM_OUTPUT_H */

/******************************FILE END *********************************/
