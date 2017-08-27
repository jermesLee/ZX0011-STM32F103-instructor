#ifndef _SIGNALCHANNELSWITCH_H_
#define _SIGNALCHANNELSWITCH_H_

#include "includes.h"

/**********************���عܿ����źŵ����******************************/
#define FET_PluseWidthSignalOutputOpen()       GPIO_SetBits(  GPIOD, GPIO_Pin_0)            // �ߵ�ƽ����� 
#define FET_PluseWidthSignalOutputClose()      GPIO_ResetBits(GPIOD, GPIO_Pin_0)            // �͵�ƽ����� 


/*******************************����ͨ���궨��***************************/
#define Signal_OutputChannelOpen( GPIOX, GPIO_Pin)   GPIO_SetBits(  GPIOX, GPIO_Pin)        // �ߵ�ƽ����� 
#define Signal_OutputChannelClose(GPIOX, GPIO_Pin)   GPIO_ResetBits(GPIOX, GPIO_Pin)        // �͵�ƽ����� 
#define Signal_ReadChannelStatus(GPIOX, GPIO_Pin)    GPIO_ReadOutputDataBit(GPIOX, GPIO_Pin)// �͵�ƽ����� 


/**************************ͨ���ṹ�����Ͷ���***************************/
typedef struct _OutputChannelType                                          //����ͨ������
{
	GPIO_TypeDef* GPIOx    ;                                              //GPIO���
	uint16_t      GPIO_Pin ;                                              //GPIO����
}	OutputChannelType ;                                                          

/**************************�ź����Ͷ���********************************/
typedef enum SignalTypeDef{ERROR_SIGNAL_TYPE = 0 ,POSITIVE_PULSE = 1 ,NEGATIVE_PULSE , SWITCH_ON,SWITCH_OFF , \
                           POSITIVE_LEVEL ,NEGATIVE_LEVEL ,FALLING ,RISING }SignalTypeDef ;



/****************************ͨ���û����� *******************************/
extern void Signal_OutputChannelGPIO_Init(void);                          //�ź����ͨ�����Ƶ�GPIO���ų�ʼ��
extern void Signal_OutputChannelSelectClose( uint16_t Channel );          //�ؼ̵���ͨ��
extern void Signal_OutputChannelSelectOpen( uint16_t Channel ) ;          //�򿪼̵���ͨ�� ���ڹؼ̵���ͨ����ͬʱҲ������һ�ε�ͨ��  
extern FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel);       //��ȡͨ������״̬
extern void Signal_SetOutputType( SignalTypeDef SignalType);              //����ź����Ϳ��� 
extern SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value );     //int���������ź����͵Ķ�Ӧ��ϵ 


#endif


/*****************************FILE END *********************************/

