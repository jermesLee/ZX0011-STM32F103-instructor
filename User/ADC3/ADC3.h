#ifndef __ADC3_H_
#define	__ADC3_H_

#include "includes.h"                                         
#include "TIME3.h"                                                        //��ʱ������ADC�Ĳ���

/*********************ADC3�������ݱ����ڴ����***************************/
#define ADC3_OneChannelSampleNum           20                             //һ��adcͨ��������
#define ADC3_ChannelScanNum                4                              //ɨ��ģʽ��ADCͨ����
#define ADC3_MemoryNum                    ( ADC3_OneChannelSampleNum * ADC3_ChannelScanNum )  // ����ADC3�ܵ��ڴ��С


/**********************ADC3 �ⲿ��ʱ��3������ʼ��************************/
#define ADC3_ExternTriggerInit(TimeUs)    TIM3_Init(TimeUs)               //����ADC3���ⲿ���� 


/******************************ADC3 �û����� ****************************/
extern void ADC3_Init(uint16_t SampleFrequency);                          //ADC3��ʼ��
extern uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence );        //��ȡ������ƽ��ֵ


#endif /* __PWM_OUTPUT_H */

/******************************FILE END *********************************/
