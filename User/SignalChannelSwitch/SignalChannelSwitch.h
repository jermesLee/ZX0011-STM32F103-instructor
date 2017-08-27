#ifndef _SIGNALCHANNELSWITCH_H_
#define _SIGNALCHANNELSWITCH_H_

#include "includes.h"

/**********************开关管控制信号的输出******************************/
#define FET_PluseWidthSignalOutputOpen()       GPIO_SetBits(  GPIOD, GPIO_Pin_0)            // 高电平开输出 
#define FET_PluseWidthSignalOutputClose()      GPIO_ResetBits(GPIOD, GPIO_Pin_0)            // 低电平关输出 


/*******************************开关通道宏定义***************************/
#define Signal_OutputChannelOpen( GPIOX, GPIO_Pin)   GPIO_SetBits(  GPIOX, GPIO_Pin)        // 高电平开输出 
#define Signal_OutputChannelClose(GPIOX, GPIO_Pin)   GPIO_ResetBits(GPIOX, GPIO_Pin)        // 低电平关输出 
#define Signal_ReadChannelStatus(GPIOX, GPIO_Pin)    GPIO_ReadOutputDataBit(GPIOX, GPIO_Pin)// 低电平关输出 


/**************************通道结构体类型定义***************************/
typedef struct _OutputChannelType                                          //定义通道类型
{
	GPIO_TypeDef* GPIOx    ;                                              //GPIO组别
	uint16_t      GPIO_Pin ;                                              //GPIO引脚
}	OutputChannelType ;                                                          

/**************************信号类型定义********************************/
typedef enum SignalTypeDef{ERROR_SIGNAL_TYPE = 0 ,POSITIVE_PULSE = 1 ,NEGATIVE_PULSE , SWITCH_ON,SWITCH_OFF , \
                           POSITIVE_LEVEL ,NEGATIVE_LEVEL ,FALLING ,RISING }SignalTypeDef ;



/****************************通道用户函数 *******************************/
extern void Signal_OutputChannelGPIO_Init(void);                          //信号输出通道控制的GPIO引脚初始化
extern void Signal_OutputChannelSelectClose( uint16_t Channel );          //关继电器通道
extern void Signal_OutputChannelSelectOpen( uint16_t Channel ) ;          //打开继电器通道 ，在关继电器通道的同时也关了上一次的通道  
extern FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel);       //读取通道开关状态
extern void Signal_SetOutputType( SignalTypeDef SignalType);              //输出信号类型控制 
extern SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value );     //int型数据与信号类型的对应关系 


#endif


/*****************************FILE END *********************************/

