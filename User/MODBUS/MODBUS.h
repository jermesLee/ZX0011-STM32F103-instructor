#ifndef	_MODBUS_H_
#define	_MODBUS_H_

#include "includes.h"
#include "USART.h"                                    //MODBUS�ڴ�����ʵ��
#include "TIME6.h"                                    //MODBUS֡���ݼ�ʱ�䶨ʱ��֡��ʱ������Ϊ3.5���ֽڣ�
#include "CRC16.h"                                    //MODBUSͨ�ŵ�У�鷽ʽ

/******************************************MODBUSͨ���ڴ��С����ĺ궨��*************************************/ 
//#define MODBUS_FrameData_MaxNumSize           74    //MODBUS���ͺͽ���ʱ���������һ֡���� ����8���ֽ���ȷ���� �� 1�ӻ���ַ +1������ + 2��ʼ��ַ +2������ +2CRCУ�飩 ,���Բ�����ʹ��Ĭ��ֵ.
                                                      //Ĭ��ֵ��MODBUS_Addrx_NumSize �Ƚϵõ� ���������ַ�ﱣ֤һ�����ܹ���д����������һ���ַ����������� 
                                                      //�ڲ���Ҫһ���Զ�дһ���ַ���������ݵ�ʱ�� �������Լ�ָ����С �������ڴ��ʹ��																											
                                                      
																											//MODBUS_Addrx_StartAddr  ��ʼ��ַ��СΪ  �� 0X0001 ,����Ϊ0X0000
 																											//MODBUS_AddrX_NumSize    ÿ����ַ�����µ���������СΪ �� 1
#define MODBUS_Addr0_StartAddr                0X0001  //MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д) ����ʼMODBUS��ַ            
#define MODBUS_Addr0_NumSize                  10       //MODBUS ��ַ���� 0XXXX �ռ��С������MODBUS_Addr0_NumSize�Ĵ�С�����ַ���� 0XXXX �ռ��С����λ  ��bit

#define MODBUS_Addr1_StartAddr                0X0001  //MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)����ʼMODBUS��ַ 
#define MODBUS_Addr1_NumSize                  10       //MODBUS ��ַ���� 1XXXX �ռ��С������MODBUS_Addr1_NumSize�Ĵ�С�����ַ���� 1XXXX �ռ��С�� ��λ  ��bit

#define MODBUS_Addr3_StartAddr                0X0001  //MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)����ʼMODBUS��ַ 
#define MODBUS_Addr3_NumSize                  10      //MODBUS ��ַ���� 3XXXX �ռ��С������MODBUS_Addr3_NumSize�Ĵ�С�����ַ���� 3XXXX �ռ��С�� ����λ �� 16bit

#define MODBUS_Addr4_StartAddr                0X0001  //MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)����ʼMODBUS��ַ   
#define MODBUS_Addr4_NumSize                  10      //MODBUS ��ַ���� 4XXXX �ռ��С������MODBUS_Addr4_NumSize�Ĵ�С�����ַ���� 4XXXX �ռ��С�� ����λ �� 16bit

 
/******************************************MODBUS��Ҫ�õ����ⲿ����****************************************/ 
#define MODBUS_FrameDataReceive(FrameData , FrameLength)     USART2_ReadDMAReceiveMulData(FrameData,FrameLength)        //֡���ݽ��� ���Ӵ��ڵĻ�������ȡ���ݱ��浽MODBUS_FrameData�У�
#define MODBUS_FrameDataSend(FrameData , FrameLength)        USART2_WriteDMASendMulData(FrameData,FrameLength)          //֡���ݷ��� ��MODBUS_FrameData������д�����ڷ��ͻ������������ݷ��ͣ�
#define MODBUS_EnableReceiveData()                           DMA_SetAndEnableReceiveNum()                               //������һ�����ݵĽ���     
#define MODBUS_USARTInit(Baudrate)                           USART2_Init((uint32_t)Baudrate)                            //MODBUSЭ���ʼ���õ��Ĵ��ڳ�ʼ������   ,Baudrate : MODBUS ͨ�Ų�����                                               
#define MODBUS_TIMInit(Time)                                 TIM6_Init((uint8_t)Time)                                   //MODBUSЭ���ʼ���õ��Ķ�ʱ����ʱ��ʼ��������Time ��MODBUS ֡��֮֡���ʱ��������λ��ms                                                         
#define MODBUS_ReceiveOneFrameFinish()                       USART2_ReceiveOneFrameFinish()                             //MODBUS�Ƿ���յ�һ֡���� �����յ�һ�����ݷ��� SET,���򷵻� RESET

/************************************************�û�����**************************************************/ 
extern void MODBUS_Init(uint32_t Baudrate);                                                                 // MODBUS ��ʼ���ܺ��� �����ò����ʺ�֡���ݼ��ʱ����
extern void MODBUS_ChangeSalveID(uint8_t SalveID);                                                          // �޸Ĵӻ���ID�� ��������modbus��ʼ��ǰ�����޸ģ�  ��Ĭ��Ϊ ��0X01
extern void MODBUS_HandleFunction(void);                                                                    // MODBUSЭ���ܴ�����    �� �ڽ��յ�һ֡���� �������������ݽ��������ݱ�����MODBUS��Ӧ���������
extern ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );       // CPU��ȡMODBUS��ַ���� ,һ�����ݶ�Ӧһ����ַ   ,MODBUS_StartAddrǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
extern ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr);       // CPUдMODBUS��ַ����   ��һ�����ݶ�Ӧһ����ַ  ,MODBUS_StartAddrǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
extern ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );   // ��ȡMODBUS��ַ���� ��  ����0 �� 1 ���͵�ַ ��һ�����ݶ���һ����ַ ������3��4�ĵ�ַ���� ���������ݶ�Ӧ��һ����ַ ���� ��д���ֽ���д���ֽڣ� 
extern ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );    // CPUдMODBUS��ַ���� �� ����0 �� 1 ���͵�ַ ��һ�����ݶ���һ����ַ ������3��4�ĵ�ַ���� ���������ݶ�Ӧ��һ����ַ ���� �ȶ����ֽ��ٶ����ֽڣ� 
	


#endif 


/************************************************FILE END**************************************************/ 

