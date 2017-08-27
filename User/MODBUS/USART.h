#ifndef _USART_H_
#define	_USART_H_

#include "includes.h"
#include "RS485.h"                                                        //485����

/***********************USART2 ���ͽ��ջ����ڴ����**********************/ 
#define USART2_SendSizeMax                 128                            //���ڷ���һ�η��͵�����ֽ���(DMA��ʽ)
#define USART2_ReceiveSizeMax              128                            //���ڽ���һ���Խ��յ�����ֽ�����DMA��ʽ��


/********************USART2 DMA��ʽ�ж����ȼ��궨��**********************/ 
#define USART2_IRQPreemptionPrio            0                             //USART2��ռʽ���ȼ� ( ���ڽ��� )
#define USART2_IRQSubPrio                   2                             //USART2�����ȼ�

#define DMA1_Channel7_IRQPreemptionPrio     0                             //USART2 DMA1_Channel7 ������ռʽ���ȼ� �� ���ڷ��� ��
#define DMA1_Channel7_IRQSubPrio            3                             //USART2 DMA1_Channel7 ��������ȼ�                   



/*****************************USART2 �û����� ****************************/ 
extern void USART2_Init(uint32_t Baudrate);                               //USART2���� �� Baudrate ��������
extern void DMA_SetAndEnableReceiveNum(void);                             //��������DMA�´���Ҫ���ܵ���������ʹ��DMA���ݽ���
extern FlagStatus USART2_ReceiveOneFrameFinish(void);                     //�����Ƿ���յ�һ֡���� 
extern void USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num);      //���ڷ�������     
extern void USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num);   //���ڶ�ȡ����
//extern void USART_Debug(void) ;                                         //����ʹ�� �����ڽ���ʲô���ݾ͸����ڻ���Ŀ���ݣ�DMA��ʽ��



#endif /* __USART2_H */
