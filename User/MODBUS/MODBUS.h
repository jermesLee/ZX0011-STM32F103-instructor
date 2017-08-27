#ifndef	_MODBUS_H_
#define	_MODBUS_H_

#include "includes.h"
#include "USART.h"                                    //MODBUS在串口中实现
#include "TIME6.h"                                    //MODBUS帧数据间时间定时（帧间时间至少为3.5个字节）
#include "CRC16.h"                                    //MODBUS通信的校验方式

/******************************************MODBUS通信内存大小分配的宏定义*************************************/ 
//#define MODBUS_FrameData_MaxNumSize           74    //MODBUS发送和接收时允许的最大的一帧数据 （有8个字节是确定的 ， 1从机地址 +1功能吗 + 2起始地址 +2数据量 +2CRC校验） ,可以不定义使用默认值.
                                                      //默认值由MODBUS_Addrx_NumSize 比较得到 ，在四类地址里保证一次性能够读写数据量最大的一类地址里的所有数据 
                                                      //在不需要一次性读写一类地址里所有数据的时候 ，可以自己指定大小 ，减少内存的使用																											
                                                      
																											//MODBUS_Addrx_StartAddr  开始地址最小为  ： 0X0001 ,不能为0X0000
 																											//MODBUS_AddrX_NumSize    每个地址类型下的数据量最小为 ： 1
#define MODBUS_Addr0_StartAddr                0X0001  //MODBUS 地址类型 0XXXX (bit访问，可读可写) 的起始MODBUS地址            
#define MODBUS_Addr0_NumSize                  10       //MODBUS 地址类型 0XXXX 空间大小（根据MODBUS_Addr0_NumSize的大小分配地址类型 0XXXX 空间大小）单位  ：bit

#define MODBUS_Addr1_StartAddr                0X0001  //MODBUS 地址类型 1XXXX (bit访问，只读)的起始MODBUS地址 
#define MODBUS_Addr1_NumSize                  10       //MODBUS 地址类型 1XXXX 空间大小（根据MODBUS_Addr1_NumSize的大小分配地址类型 1XXXX 空间大小） 单位  ：bit

#define MODBUS_Addr3_StartAddr                0X0001  //MODBUS 地址类型 3XXXX (16bit访问，只读)的起始MODBUS地址 
#define MODBUS_Addr3_NumSize                  10      //MODBUS 地址类型 3XXXX 空间大小（根据MODBUS_Addr3_NumSize的大小分配地址类型 3XXXX 空间大小） ，单位 ： 16bit

#define MODBUS_Addr4_StartAddr                0X0001  //MODBUS 地址类型 4XXXX (16bit访问，可读可写)的起始MODBUS地址   
#define MODBUS_Addr4_NumSize                  10      //MODBUS 地址类型 4XXXX 空间大小（根据MODBUS_Addr4_NumSize的大小分配地址类型 4XXXX 空间大小） ，单位 ： 16bit

 
/******************************************MODBUS需要用到的外部函数****************************************/ 
#define MODBUS_FrameDataReceive(FrameData , FrameLength)     USART2_ReadDMAReceiveMulData(FrameData,FrameLength)        //帧数据接收 （从串口的缓冲区读取数据保存到MODBUS_FrameData中）
#define MODBUS_FrameDataSend(FrameData , FrameLength)        USART2_WriteDMASendMulData(FrameData,FrameLength)          //帧数据发送 （MODBUS_FrameData中数据写进串口发送缓冲区进行数据发送）
#define MODBUS_EnableReceiveData()                           DMA_SetAndEnableReceiveNum()                               //是能下一阵数据的接收     
#define MODBUS_USARTInit(Baudrate)                           USART2_Init((uint32_t)Baudrate)                            //MODBUS协议初始化用到的串口初始化函数   ,Baudrate : MODBUS 通信波特率                                               
#define MODBUS_TIMInit(Time)                                 TIM6_Init((uint8_t)Time)                                   //MODBUS协议初始化用到的定时器定时初始化函数，Time ：MODBUS 帧与帧之间的时间间隔，单位：ms                                                         
#define MODBUS_ReceiveOneFrameFinish()                       USART2_ReceiveOneFrameFinish()                             //MODBUS是否接收到一帧数据 ，接收到一阵数据返回 SET,否则返回 RESET

/************************************************用户函数**************************************************/ 
extern void MODBUS_Init(uint32_t Baudrate);                                                                 // MODBUS 初始化总函数 ，设置波特率和帧数据间的时间间隔
extern void MODBUS_ChangeSalveID(uint8_t SalveID);                                                          // 修改从机的ID号 （建议在modbus初始化前进行修改）  ，默认为 ：0X01
extern void MODBUS_HandleFunction(void);                                                                    // MODBUS协议总处理函数    （ 在接收到一帧数据 ，根据数据内容将数据内容保存在MODBUS相应的数据区里）
extern ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );       // CPU读取MODBUS地址数据 ,一个数据对应一个地址   ,MODBUS_StartAddr前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
extern ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr);       // CPU写MODBUS地址数据   ，一个数据对应一个地址  ,MODBUS_StartAddr前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
extern ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );   // 读取MODBUS地址数据 ，  对于0 和 1 类型地址 ，一个数据对于一个地址 ，对于3和4的地址类型 ，两个数据对应于一个地址 ，（ 先写高字节再写低字节） 
extern ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );    // CPU写MODBUS地址数据 ， 对于0 和 1 类型地址 ，一个数据对于一个地址 ，对于3和4的地址类型 ，两个数据对应于一个地址 ，（ 先读高字节再读低字节） 
	


#endif 


/************************************************FILE END**************************************************/ 

