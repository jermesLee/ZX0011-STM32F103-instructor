#ifndef _USART_H_
#define	_USART_H_

#include "includes.h"
#include "RS485.h"                                                        //485总线

/***********************USART2 发送接收缓冲内存分配**********************/ 
#define USART2_SendSizeMax                 128                            //串口发送一次发送的最大字节数(DMA方式)
#define USART2_ReceiveSizeMax              128                            //串口接收一次性接收的最大字节数（DMA方式）


/********************USART2 DMA方式中断优先级宏定义**********************/ 
#define USART2_IRQPreemptionPrio            0                             //USART2抢占式优先级 ( 用于接收 )
#define USART2_IRQSubPrio                   2                             //USART2次优先级

#define DMA1_Channel7_IRQPreemptionPrio     0                             //USART2 DMA1_Channel7 传输抢占式优先级 （ 用于发送 ）
#define DMA1_Channel7_IRQSubPrio            3                             //USART2 DMA1_Channel7 传输次优先级                   



/*****************************USART2 用户函数 ****************************/ 
extern void USART2_Init(uint32_t Baudrate);                               //USART2配置 ， Baudrate ：波特率
extern void DMA_SetAndEnableReceiveNum(void);                             //重新设置DMA下次需要接受的数据量和使能DMA数据接收
extern FlagStatus USART2_ReceiveOneFrameFinish(void);                     //串口是否接收到一帧数据 
extern void USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num);      //串口发送数据     
extern void USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num);   //串口读取数据
//extern void USART_Debug(void) ;                                         //调试使用 ，串口接收什么数据就给串口回数目数据（DMA方式）



#endif /* __USART2_H */
