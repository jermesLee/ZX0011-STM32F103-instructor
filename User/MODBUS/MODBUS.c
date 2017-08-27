#include "MODBUS.h"

/******************************************MODBUS可能的最大一帧数据计算*****************************************/ 
#ifndef MODBUS_FrameData_MaxNumSize                                                                       // 如果没有宏定义MODBUS_FrameData_MaxNumSize  ，则使用默认的MODBUS_FrameData_MaxNumSize 
#define MODBUS_FrameData_MaxNumSize01          ( MODBUS_Addr0_NumSize > MODBUS_Addr1_NumSize ? ((MODBUS_Addr0_NumSize -1 ) >> 3 ) +1  : ((MODBUS_Addr1_NumSize  -1 ) >> 3 )+1 )               //MODBUS 类型0 和 类型 1 内存大小比较
#define MODBUS_FrameData_MaxNumSize34          ( MODBUS_Addr3_NumSize > MODBUS_Addr4_NumSize ? 2*MODBUS_Addr3_NumSize : 2*MODBUS_Addr4_NumSize )                                              //MODBUS 类型3 和 类型 4 内存大小比较
#define MODBUS_FrameData_MaxNumSize            ( MODBUS_FrameData_MaxNumSize01 > MODBUS_FrameData_MaxNumSize34 ? MODBUS_FrameData_MaxNumSize01 + 10 : MODBUS_FrameData_MaxNumSize34  + 10  )  // 加10是modbus 别的数据流出空间 ：
#endif                                                                                                    // 1从机地址 +1功能吗 + 2起始地址 +2数据量 +2CRC校验（ 默认的MODBUS_FrameData_MaxNumSize为读取一类地址里所有数据的最大值 ）

/******************************************MODBUS通信各种地址类型下最大最小地址限制****************************/ 
#define MODBUS_Addr0_MinAddr                   ( MODBUS_Addr0_StartAddr )                                 // MODBUS 地址类型 0XXXX (bit访问，可读可写) 的最小MODBUS地址  
#define MODBUS_Addr0_MaxAddr                   ( MODBUS_Addr0_StartAddr + MODBUS_Addr0_NumSize  )         // MODBUS 地址类型 0XXXX (bit访问，可读可写) 的最大MODBUS地址  
#define MODBUS_Addr1_MinAddr                   ( MODBUS_Addr1_StartAddr )                                 // MODBUS 地址类型 1XXXX (bit访问，只读)的最小MODBUS地址  
#define MODBUS_Addr1_MaxAddr                   ( MODBUS_Addr1_StartAddr + MODBUS_Addr1_NumSize  )         // MODBUS 地址类型 1XXXX (bit访问，只读)的最大MODBUS地址  
#define MODBUS_Addr3_MinAddr                   ( MODBUS_Addr3_StartAddr )                                 // MODBUS 地址类型 3XXXX (16bit访问，只读)的最小MODBUS地址  
#define MODBUS_Addr3_MaxAddr                   ( MODBUS_Addr3_StartAddr + MODBUS_Addr3_NumSize  )         // MODBUS 地址类型 3XXXX (16bit访问，只读)的最大MODBUS地址  
#define MODBUS_Addr4_MinAddr                   ( MODBUS_Addr4_StartAddr )                                 // MODBUS 地址类型 4XXXX (16bit访问，可读可写)的最小MODBUS地址   
#define MODBUS_Addr4_MaxAddr                   ( MODBUS_Addr4_StartAddr + MODBUS_Addr4_NumSize  )         // MODBUS 地址类型 4XXXX (16bit访问，可读可写)的最大MODBUS地址   



/******************************************MODBUS内存分配******************************************************/ 
uint8_t MODBUS_Addr0[MODBUS_Addr0_NumSize] ;                                                              // MODBUS 地址类型 0XXXX (bit访问，可读可写) 
uint8_t MODBUS_Addr1[MODBUS_Addr1_NumSize] ;                                                              // MODBUS 地址类型 1XXXX (bit访问，只读)
uint8_t MODBUS_Addr3[MODBUS_Addr3_NumSize * 2 ] ;                                                         // MODBUS 地址类型 3XXXX (16bit访问，只读)
uint8_t MODBUS_Addr4[MODBUS_Addr4_NumSize * 2 ] ;                                                         // MODBUS 地址类型 4XXXX (16bit访问，可读可写)

uint8_t MODBUS_FrameData[MODBUS_FrameData_MaxNumSize] ;                                                   // MODBUS接收和发送一帧数据临时保存位置
uint8_t MODBUS_FrameLength ;                                                                              // MODBUS一帧数据的长度
uint8_t MODBUS_SlaveID  = 0X01;                                                                           // MODBUS 从机ID号(默认为0X01)
	



/******************************** MODBUS 地址类型 0XXXX (bit访问，可读可写)  **********************************/
static void MODBUS_Function_01(void);                                                                     // 读线圈
static void MODBUS_Function_05(void);                                                                     // 写单个线圈
static void MODBUS_Function_15(void);                                                                     // 写多个线圈

/******************************** MODBUS 地址类型 1XXXX (bit访问，只读)  **************************************/
static void MODBUS_Function_02(void);                                                                     // 读输入离散量  

/******************************** MODBUS 地址类型 3XXXX (16bit访问，只读)  ************************************/
static void MODBUS_Function_04(void);                                                                     // 读输入寄存器

/******************************** MODBUS 地址类型 4XXXX (16bit访问，可读可写)  ********************************/
static void MODBUS_Function_03(void);                                                                     // 读多个寄存器
static void MODBUS_Function_06(void);                                                                     // 写单个寄存器
static void MODBUS_Function_16(void);                                                                     // 写多个寄存器

/************************************************基本功能子函数*************************************************/
static void Modbus_Function_Error(uint8_t com , uint8_t error);                                           //MODBUS 协议出错时，根据情况回应主机出错原因（如数据地址溢出 ，数据量太大，不支持此功能码）
static void BITArrayCopyByteArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t *OUT_Num);          // 把位数组中数据每8个一组转换为字节数组
static void ByteArrayCopyBITArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t OUT_Num);           // 把字节数组每位拆开为一个位数组
static void ByteArrayCopyByteArray( uint8_t *IN , uint8_t *OUT ,uint8_t Num);                             // 字节数组数据从IN数组复制到OUT数组
static void HalfWordArrayCopyByteArray( uint16_t *IN , uint8_t *OUT ,uint8_t IN_Num ,uint8_t CopyType);   // 半字数据按一定的规则复制到字节数组中
static void ByteArrayCopyHalfWordArray( uint8_t *IN , uint16_t *OUT ,uint8_t OUT_Num ,uint8_t CopyType);  // 字节数组数据按一定的规则复制到半字数组中


/************************************************用户函数******************************************************/
void MODBUS_Init(uint32_t Baudrate);                                                                      // MODBUS 初始化总函数 ，设置波特率和帧数据间的时间间隔
void MODBUS_HandleFunction(void);                                                                         // MODBUS协议总处理函数 (串口接收到一帧数据对其处理保存到对应的地址里，并回应主机的响应)
void MODBUS_ChangeSalveID(uint8_t SalveID);                                                               // 修改从机的ID号 （最好在modbus初始化前进行修改）         
ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );            // CPU读取MODBUS地址数据 ,一个数据对应一个地址
ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr);            // CPU写MODBUS地址数据   ，一个数据对应一个地址
ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );        // 读取MODBUS地址数据 ，  对于0 和 1 类型地址 ，一个数据对于一个地址 ，对于3和4的地址类型 ，两个数据对应于一个地址 ，（ 先写高字节再写低字节） 
ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );         // CPU写MODBUS地址数据 ， 对于0 和 1 类型地址 ，一个数据对于一个地址 ，对于3和4的地址类型 ，两个数据对应于一个地址 ，（ 先读高字节再读低字节） 
	



/****************************************************************************************************************
  * @brief  修改从机的ID号 （建议在modbus初始化前进行修改）     
  * @param  SalveID ：新的从机ID号
  * @retval 无
  * @notice 建议在modbus初始化前进行修改
*****************************************************************************************************************/
void MODBUS_ChangeSalveID(uint8_t SalveID)                                                                // 修改从机的ID号 （最好在modbus初始化前进行修改）         
{
	MODBUS_SlaveID = SalveID ;                                                                              // 将数据保存到从机ID变量里
}





/****************************************************************************************************************
  * @brief  MODBUS协议总处理函数
  * @param  无
  * @retval 无
  * @notice 地址0x00为广播地址 ，不需要数据响应，否者将通信冲突出错 
*****************************************************************************************************************/
void MODBUS_HandleFunction(void)
{
	uint8_t crc16[2];
  
	/*********判断是否接收到一帧数据*************/	
	if( MODBUS_ReceiveOneFrameFinish() == RESET ) return ;                                // 还没有接收到一帧数据 ，退出函数    	
 
	/**********读取接收到的一帧数据**************/	
	MODBUS_FrameDataReceive(MODBUS_FrameData, &MODBUS_FrameLength);                       // 读取MODBUS接收到的一帧数据 到 MODBUS_FrameData 中 ，下面是对这帧数据进行处理
	if((MODBUS_FrameData[0] != MODBUS_SlaveID )&&(MODBUS_FrameData[0] != 0X00))	          // 0 :表示广播模式 ，MODBUS_SlaveID 为从机地址 
		goto FrameError;                                                                    // 地址不匹配，跳到数据接收错误的处理位置
    CRC16_CheckCompute( MODBUS_FrameData , MODBUS_FrameLength- 2 ,crc16);				          // crc计算	
	if( CRC16_Check( &MODBUS_FrameData[MODBUS_FrameLength - 2 ]  ,crc16 ) != SUCCESS)     // crc比较，正确进行应答 ，否则表示传输过程出错，此时什么也不处理
	  goto FrameError;                                                                    // 数据帧校验错误 ，跳到数据接收错误的处理位置   
	
	switch(MODBUS_FrameData[1])                                                           //不同功能码执行不同的程序
	{	    
		/******************************** MODBUS 地址类型 0XXXX (bit访问，可读可写)  **********************************/
		case 1  : MODBUS_Function_01();break ;                                              // 读线圈
		case 5  : MODBUS_Function_05();break ;                                              // 写单个线圈
		case 15 : MODBUS_Function_15();break ;                                              // 写多个线圈
				
		/******************************** MODBUS 地址类型 1XXXX (bit访问，只读)  **************************************/
		case 2  : MODBUS_Function_02();break ;                                              // 读输入离散量                                    

		/******************************** MODBUS 地址类型 3XXXX (16bit访问，只读)  ************************************/
		case 4  : MODBUS_Function_04();break ;                                              // 读输入寄存器
		
		/******************************** MODBUS 地址类型 4XXXX (16bit访问，可读可写)  ********************************/
		case 3  : MODBUS_Function_03();break ;                                              // 读多个寄存器
		case 6  : MODBUS_Function_06();break ;                                              // 写单个寄存器
		case 16 : MODBUS_Function_16();break ;                                              // 写多个寄存器
			
		/******************************** MODBUS 不支持此功能  ********************************************************/
		default : Modbus_Function_Error(MODBUS_FrameData[1] ,0x01);                         // 不支持此功能
	}
	
	if(MODBUS_FrameData[0] != 0 )                                                         // 0X00地址为广播地址，不需要响应数据  , 和 FrameError 处理一样 ，为下一帧数据接受做准备
  {		                                                                                  // 在一切数据正确的情况下 ，在响应完成中断里打开下一帧数据的接收函数      
		MODBUS_FrameDataSend(MODBUS_FrameData, MODBUS_FrameLength);   	                    // 此时地址为从机地址 ，发送准备好的一帧数据	
		return ;                                                                            // 正常收发数据 ，退出函数
  }                 
  
	/***************接收数据错误 或 接收的不是本机数据 或 为广播地址 需要为下一帧数据的接收做的准备工作**************/                                                                         
FrameError :                                                                            // 接收数据出错后的准备工作
	  MODBUS_EnableReceiveData();                                                         // 为下一帧数据的接收做准备	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
}





/**************************************************************************
  * @brief  MODBUS 初始化总函数 ，设置波特率和帧数据间的时间间隔
  * @param  Baudrate ： 通信波特率
  * @retval 无
  * @notice 帧时间间隔为 TimeMs =  40000  / Baudrate ; 当时间间隔小于2Ms时，强制为2Ms
**************************************************************************/
void MODBUS_Init(uint32_t Baudrate)
{
	 uint8_t TimeMs ; 
	 
	 /************计算帧数据间的时间间隔***********/
 	 TimeMs =  40000  / Baudrate ;                                                        //帧数据间定时时间计算                                                 
     if( TimeMs < 2 )  TimeMs = 2 ;                                                       //限制帧数据间最小时间为2Ms
   
	 /********串口波特率与定时器定时时间配置*******/
	 MODBUS_USARTInit(Baudrate);                                                          //串口波特率配置
	 MODBUS_TIMInit(TimeMs);                                                              //定时器定时时间配置 
} 









/**************************************************************************
  * @brief  维护MODBUS的一段数据区，往MODBUS内存写数据 （8bit访问）
  * @param  Data： 需要写进MODBUS数据区的数据
  *         Num ： 需要写进的数据量 
  *         MODBUS_StartAddr ：需写数据的MODBUS地址 
  * @retval ERROR :   输入数据有误,不做任何操作，退出函数 ;  
  *         SUCCESS : s输入数据正确 ，读写数据区
  * @notice MODBUS_StartAddr 的前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
  *         对于0 和1 类型地址数据的写，一个数据对应一个地址 
  *         对于3 和4 类型地址数据的写 ，两个数据对应一个地址 （ 先写高字节数据 ，再写低字节数据）
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                                             // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	uint16_t StartAddrIndex ;                                                             // 地址索引号
	uint8_t  *p ;                                                                         // 指上需要目的首地址
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                                          // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                                        // 在一个地址类型下的真实MODBUS地址
  switch(AddrTypeSelect)                                                                // 不同的地址类型下数据保存位置不一样
	{
		case 0 :                                                                                                            //  MODBUS 地址类型 0XXXX (bit访问，可读可写)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // 地址访问溢出溢出 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;     break ;     // 指针p指上需要填写数据目的首地址
	  case 1 :                                                                                                            // MODBUS 地址类型 1XXXX (bit访问，只读) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // 地址访问溢出溢出      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;     break ;     // 指针p指上需要填写数据目的首地址 
		case 3 :                                                                                                            // MODBUS 地址类型 3XXXX (16bit访问，只读)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr3_MaxAddr )) return ERROR;  // 地址访问溢出溢出     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ; break ;     // 指针p指上需要填写数据目的首地址 
		case 4 :                                                                                                            // MODBUS 地址类型 4XXXX (16bit访问，可读可写)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr4_MaxAddr )) return ERROR;  // 地址访问溢出溢出   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ; break ;     // 指针p指上需要填写数据目的首地址
		default:  return ERROR ; 
	}
	ByteArrayCopyByteArray( Data, p ,Num) ;                                                                               //数据复制填充 
	return SUCCESS ;
}
		



/**************************************************************************
  * @brief  维护MODBUS的一段数据区，从MODBUS内存读数据 （ 8bit访问）
  * @param  Data： 读取的数据保存位置
  *         Num ： 需要读取的数据量
  *         MODBUS_StartAddr ： 读取MODBUS数据位置
  * @retval ERROR :   输入数据有误,不做任何操作，退出函数 ;  
  *         SUCCESS : s输入数据正确 ，读写数据区
  * @notice MODBUS_StartAddr 的前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
  *         对于0 和1 类型地址数据的读，一个数据对应一个地址 
  *         对于3 和4 类型地址数据的读，两个数据对应一个地址 （ 先读高字节数据 ，再读低字节数据）
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                                             // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	uint16_t StartAddrIndex ;                                                             // 地址索引号
	uint8_t  *p ;                                                                         // 指上需要目的首地址
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                                          // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	MODBUS_StartAddr = MODBUS_StartAddr & 0XFFFF ;                                        // 在一个地址类型下的真实MODBUS地址
  switch(AddrTypeSelect)                                                                // 不同的地址类型下数据保存位置不一样
	{
		case 0 :                                                                                                            //  MODBUS 地址类型 0XXXX (bit访问，可读可写)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // 地址访问溢出溢出 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;     break ;     // 指针p指上需要填写数据目的首地址
	  case 1 :                                                                                                            // MODBUS 地址类型 1XXXX (bit访问，只读) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // 地址访问溢出溢出      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;     break ;     // 指针p指上需要填写数据目的首地址 
		case 3 :                                                                                                            // MODBUS 地址类型 3XXXX (16bit访问，只读)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr3_MaxAddr )) return ERROR;  // 地址访问溢出溢出     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ; break ;     // 指针p指上需要填写数据目的首地址 
		case 4 :                                                                                                            // MODBUS 地址类型 4XXXX (16bit访问，可读可写)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr4_MaxAddr )) return ERROR;  // 地址访问溢出溢出   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ; break ;     // 指针p指上需要填写数据目的首地址
		default:  return ERROR; 
	}
	ByteArrayCopyByteArray( p, Data ,Num) ;                                                                                        //数据复制填充
  return SUCCESS ;
}



/**************************************************************************
  * @brief  维护MODBUS的一段数据区，往MODBUS内存写数据 （ 16bit访问）
  * @param  Data： 需要写进MODBUS数据区的数据
  *         Num ： 需要写进的数据量 
  *         MODBUS_StartAddr ：需写数据的MODBUS地址 
  * @retval ERROR :   输入数据有误,不做任何操作，退出函数 ;  
  *         SUCCESS : s输入数据正确 ，读写数据区
  * @notice MODBUS_StartAddr 的前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
  *         对于0 和1 类型地址数据的写，一个数据对应一个地址 （只写16位数据的低8位）
  *         对于3 和4 类型地址数据的写，一个数据对应一个地址 ( 高低字节全部写入)
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                               // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	uint16_t StartAddrIndex ;                                               // 地址索引号
	uint8_t  *p ;                                                           // 指上需要目的首地址
	uint8_t  CopyType ;                                                     // 复制方式
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                            // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                          // 在一个地址类型下的真实MODBUS地址
  switch(AddrTypeSelect)                                                  // 不同的地址类型下数据保存位置不一样
	{
		case 0 :                                                                                                            //  MODBUS 地址类型 0XXXX (bit访问，可读可写)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // 地址访问溢出溢出 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;                 // 指针p指上需要填写数据目的首地址
		    CopyType = 0 ;  break ;                                                                                           // 复制方式选择，只复制16位诗句的低8位
	  case 1 :                                                                                                            // MODBUS 地址类型 1XXXX (bit访问，只读) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // 地址访问溢出溢出      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;                 // 指针p指上需要填写数据目的首地址 
		    CopyType = 0 ;  break ;                                                                                           // 复制方式选择，只复制16位诗句的低8位
		case 3 :                                                                                                            // MODBUS 地址类型 3XXXX (16bit访问，只读)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr3_MaxAddr )) return ERROR;  // 地址访问溢出溢出     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ;             // 指针p指上需要填写数据目的首地址 
		    CopyType = 1 ; break ;                                                                                            // 复制方式选择，高低字节交差复制   
		case 4 :                                                                                                            // MODBUS 地址类型 4XXXX (16bit访问，可读可写)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr4_MaxAddr )) return ERROR;  // 地址访问溢出溢出   
	  	    StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ;             // 指针p指上需要填写数据目的首地址
		    CopyType = 1 ; break ;                                                                                            // 复制方式选择，高低字节交差复制     
		default:  return ERROR ;                                                     
	}
	HalfWordArrayCopyByteArray( Data , p, Num,CopyType);                                                                  // CopyType = 0 ： 复制低字节· ，CopyType ！=0 ：非零表示高低字节交差复制   
	return SUCCESS ;
}
		

/**************************************************************************
  * @brief  维护MODBUS的一段数据区，从MODBUS内存读数据 （ 16bit访问）
  * @param  Data： 读取的数据保存位置
  *         Num ： 需要读取的数据量
  *         MODBUS_StartAddr ： 读取MODBUS数据位置
  * @retval ERROR :   输入数据有误,不做任何操作，退出函数 ;  
  *         SUCCESS : s输入数据正确 ，读写数据区
  * @notice MODBUS_StartAddr 的前16位表示地址类型 ，后16位表示在这个数据类型下的真实地址 
  *         对于0 和1 类型地址数据的读，一个数据对应一个地址  （只填充地质界，高字节补零）
  *         对于3 和4 类型地址数据的读，一个数据对应一个地址  （高低字节数据都填写）
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                               // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	uint16_t StartAddrIndex ;                                               // 地址索引号
	uint8_t  *p ;                                                           // 指上需要目的首地址
	uint8_t  CopyType ;                                                     // 复制方式
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                            // MODBUS地址类型选择（ 0 ， 1 ， 3 ，4 ）
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                          // 在一个地址类型下的真实MODBUS地址
  switch(AddrTypeSelect)                                                  // 不同的地址类型下数据保存位置不一样
	{
		case 0 :                                                                                                            //  MODBUS 地址类型 0XXXX (bit访问，可读可写)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // 地址访问溢出溢出 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;                 // 指针p指上需要填写数据目的首地址
		  CopyType = 0 ;  break ;                                                                                           // 复制方式选择，只填充低字节 ，高字节补零· 
	  case 1 :                                                                                                            // MODBUS 地址类型 1XXXX (bit访问，只读) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // 地址访问溢出溢出      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;                 // 指针p指上需要填写数据目的首地址 
		  CopyType = 0 ;  break ;                                                                                           // 复制方式选择，只填充低字节 ，高字节补零· 
		case 3 :                                                                                                            // MODBUS 地址类型 3XXXX (16bit访问，只读)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr3_MaxAddr )) return ERROR;  // 地址访问溢出溢出     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ;             // 指针p指上需要填写数据目的首地址 
		  CopyType = 1 ; break ;                                                                                            // 复制方式选择，两个8位数据组成一个16位数据                  
		case 4 :                                                                                                            // MODBUS 地址类型 4XXXX (16bit访问，可读可写)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr4_MaxAddr )) return ERROR;  // 地址访问溢出溢出   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ;             // 指针p指上需要填写数据目的首地址
		  CopyType = 1 ; break ;                                                                                            // 复制方式选择，两个8位数据组成一个16位数据                  
		default:  return ERROR ;                                                     
	}
	ByteArrayCopyHalfWordArray( p , Data, Num , CopyType);                                                                // CopyType = 0 ： 只填充低字节 ，高字节补零· ，CopyType ！=0 ：两个8位数据组成一个16位数据                                                    
  return SUCCESS; 
}
	




/******************************** MODBUS 地址类型 0XXXX (bit访问，可读可写)  ***************************************/

/************************************************************************
  * @brief  MODBUS 功能码 0x01 ：  读线圈
  * @param  无
  * @retval 无
  * @notice 无
************************************************************************/
static void MODBUS_Function_01(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ;                                              // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr0_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x01 , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address + data_num > MODBUS_Addr0_MaxAddr) )  // 数据地址错误
	{
		Modbus_Function_Error(0x01 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr0_StartAddr ;                 // 计算发送的地址在数组中索引号
		
	BITArrayCopyByteArray(&MODBUS_Addr0[addr_index] , data_num ,&MODBUS_FrameData[3] ,&Senddata_num);//位数据转换为字节数据
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1地址码+1功能码+1字节数 + Senddata_num 输出状态+2校验码  =  Senddata_num + 5字节
                                                                        // 从机地址 MODBUS_FrameData[0] 直接使用接收到的地址
	MODBUS_FrameData[1]= 0X01 ;                                           // 功能码
  MODBUS_FrameData[2]= Senddata_num ;                                   // 需要发送的字节数 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// crc16计算

}




/***********************************************************************
  * @brief  MODBUS 功能码 0x05 ：  写单个线圈
  * @param  无
  * @retval 无
  * @notice 接收到的数据只能是0xff00（0） 和 0x0000（1） ； 响应：直接发送接收到的数据即可
  *       数据地址错误  ： ( start_address +NUM的没有等于号  ，只有start——address的需要等于号)
************************************************************************/
static void MODBUS_Function_05(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  OutValue ;                                                  // 输出值
	uint16_t  addr_index ;                                                // 地址索引号
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	OutValue      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 需要对地址写的值
	
	if( (OutValue  != 0X0000  )&&  (OutValue  != 0XFF00  ) )              // 传送值错误 （ 只能是 0x0000 和 0XFF00 ）
	{ 
		Modbus_Function_Error(0x05 , 0x03);                                 // 传送值错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address  >= MODBUS_Addr0_MaxAddr) )    // 数据地址错误 ( start_address +NUM的没有等于号  ，只有start——address的需要等于号)
	{
		Modbus_Function_Error(0x05 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr0_StartAddr ;                 // 计算发送的地址在数组中索引号 
	if( OutValue == 0x0000)                                               // 判断需往地址里写的数据
		MODBUS_Addr0[addr_index] = 	0 ;                                     // 0X0000 代表 OFF , 写 0
	else
		MODBUS_Addr0[addr_index] = 	1 ;                                     // 0XFF00 代表 ON  , 写 1

	MODBUS_FrameLength = 8;                                               // 1地址码+1功能码+2输出地址 + 2输出值+2校验码  = 8字节
                                                                        // 响应直接发送接收到的数据即可
}




/************************************************************************
  * @brief  MODBUS 功能码 0x0f ：  写多个线圈
  * @param  无
  * @retval 无
  * @notice 响应 ：发送接收到的一帧数据的前6个字节 和2个字节的CRC校验
************************************************************************/
static void MODBUS_Function_15(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ; 	                                            // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0

	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr0_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x0F , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address + data_num > MODBUS_Addr0_MaxAddr) )  // 数据地址错误
	{
		Modbus_Function_Error(0x0F , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	Senddata_num  = MODBUS_FrameData[6] ;                                 // 字节数 
	addr_index   = start_address - MODBUS_Addr0_StartAddr ;               // 计算填充的地址在数组中索引号
	ByteArrayCopyBITArray(&MODBUS_FrameData[7] , Senddata_num ,&MODBUS_Addr0[addr_index],data_num);	//字节数据转换为位数据
  
	MODBUS_FrameLength = 8;                                               // 1地址码+1功能码+2起始地址 +2输出数据 +2校验码 =  8字节
	CRC16_CheckCompute( MODBUS_FrameData, 6, &MODBUS_FrameData[6]);       // crc16计算
                                                                        // 响应发送接收到的一帧数据的前6个字节 和2个字节的CRC校验
}





/******************************** MODBUS 地址类型 1XXXX (bit访问，只读)  ********************************************/

/************************************************************************
  * @brief  MODBUS 功能码 0x02 ：  读输入离散量
  * @param  无
  * @retval 无
  * @notice 无
*************************************************************************/
static void MODBUS_Function_02(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ;   	                                          // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr1_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x02 , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr1_MinAddr)||( start_address + data_num > MODBUS_Addr1_MaxAddr) ) // 数据地址错误
	{
		Modbus_Function_Error(0x02 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr1_StartAddr ;                 // 计算发送的地址在数组中索引号
		
	BITArrayCopyByteArray(&MODBUS_Addr1[addr_index] , data_num ,&MODBUS_FrameData[3] ,&Senddata_num);//位数据转换为字节数据
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1地址码+1功能码+1字节数 + Senddata_num 输出状态+2校验码  =  Senddata_num + 5字节
                                                                        // 从机地址 MODBUS_FrameData[0] 直接使用接收到的地址
	MODBUS_FrameData[1]= 0X02 ;                                           // 功能码
  MODBUS_FrameData[2]= Senddata_num ;                                   // 需要发送的字节数 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// crc16计算

}




/******************************** MODBUS 地址类型 3XXXX (16bit访问，只读)  **********************************/

/*************************************************************************
  * @brief  MODBUS 功能码 0x04 ：  读输入寄存器
  * @param  无
  * @retval 无
  * @notice 16bit访问
*************************************************************************/
static void MODBUS_Function_04(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ; 	                                            // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr3_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x04 , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr3_MinAddr)||( start_address +  data_num > MODBUS_Addr3_MaxAddr) )  // 数据地址错误
	{
		Modbus_Function_Error(0x04 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //响应的字节数是输入寄存器数目的两倍（16bit访问） 
	addr_index   = (start_address - MODBUS_Addr3_StartAddr ) << 1 ;       // 计算发送的地址在数组中索引号
	ByteArrayCopyByteArray(&MODBUS_Addr3[addr_index] ,&MODBUS_FrameData[3] ,Senddata_num ); //复制数据
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1地址码+1功能码+1字节数 + Senddata_num 输出状态+2校验码  =  Senddata_num + 5字节
                                                                        // 从机地址 MODBUS_FrameData[0] 直接使用接收到的地址
	MODBUS_FrameData[1]= 0X04 ;                                           // 功能码
  MODBUS_FrameData[2]= Senddata_num ;                                   // 需要发送的字节数 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// 发送数据crc16计算

}






/******************************** MODBUS 地址类型 4XXXX (16bit访问，可读可写)  **********************************/

/*************************************************************************
  * @brief  MODBUS 功能码 0x03 ：  读多个寄存器
  * @param  无
  * @retval 无
  * @notice 16bit访问
*************************************************************************/
static void MODBUS_Function_03(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ; 	                                            // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr4_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x03 , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address +  data_num > MODBUS_Addr4_MaxAddr) )  // 数据地址错误
	{
		Modbus_Function_Error(0x03 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //响应的字节数是输入寄存器数目的两倍（16bit访问） 
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // 计算发送的地址在数组中索引号
	ByteArrayCopyByteArray(&MODBUS_Addr4[addr_index] ,&MODBUS_FrameData[3] ,Senddata_num ); //复制数据
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1地址码+1功能码+1字节数 + Senddata_num 输出状态+2校验码  =  Senddata_num + 5字节
                                                                        // 从机地址 MODBUS_FrameData[0] 直接使用接收到的地址
	MODBUS_FrameData[1]= 0X03 ;                                           // 功能码
  MODBUS_FrameData[2]= Senddata_num ;                                   // 需要发送的字节数 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);//crc16计算

}




/*************************************************************************
  * @brief  MODBUS 功能码 0x06 ：  写单个寄存器
  * @param  无
  * @retval 无
  * @notice 数据地址错误  ： ( start_address +NUM的没有等于号  ，只有start_address的需要等于号)
*************************************************************************/
static void MODBUS_Function_06(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  addr_index ;                                                // 地址索引号

	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address >= MODBUS_Addr4_MaxAddr) )  // 数据地址错误 ，此处有等于号
	{
		Modbus_Function_Error(0x06 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // 计算发送的地址在数组中索引号 (16bit访问)
	ByteArrayCopyByteArray(&MODBUS_FrameData[4], &MODBUS_Addr4[addr_index],2 );    //复制数据
  
	MODBUS_FrameLength = 8;                                               // 1地址码+1功能码+2字节地址 +2字节数据 +2校验码  =  8字节
                                                                        // 响应直接发送接收到的数据即可
}




/************************************************************************
  * @brief  MODBUS 功能码 0x10 ：  写多个寄存器
  * @param  无
  * @retval 无
  * @notice 无
************************************************************************/

static void MODBUS_Function_16(void)
{
	uint16_t  start_address ;                                             // 起始地址
	uint16_t  data_num ;                                                  // 数据量
	uint16_t  addr_index ;                                                // 地址索引号
  uint8_t   Senddata_num ; 	                                            // 发送响应时里的字节数	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // 开始地址   ,高位在前
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // 读取数据量 ,高位在前,data_num最大在256以内，所以RXDBuff[4]肯定为0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr4_NumSize) )          // 数据量错误
	{
		Modbus_Function_Error(0x10 , 0x03);                                 // 数据量错误 ，返回错误码 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address +  data_num > MODBUS_Addr4_MaxAddr) )  // 数据地址错误
	{
		Modbus_Function_Error(0x10 , 0x02);                                 // 数据地址错误 ，返回错误码 0x02
		return ;
	}
	
	if( MODBUS_FrameLength <  MODBUS_FrameData[6] + 9 )                   // 主机发送下来的数据量不够 ，回应主机写失败 （ 在16功能码里存在 data_num 与实际接收到的数据量不同的情况）
	{                                                                     // 1从机地址 + 1功能码 + 2起始地址 + 2数据量 + 1字节数 + 2CRC校验 = 9 字节
		Modbus_Function_Error(0x10 , 0x04);                                 // 主机发送下来的数据量不够 ，返回错误码 0x04 :写失败
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //响应的字节数是输入寄存器数目的两倍（16bit访问） 
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // 计算发送的地址在数组中索引号
	ByteArrayCopyByteArray(&MODBUS_FrameData[7], &MODBUS_Addr4[addr_index] ,Senddata_num ); //复制数据
  
	MODBUS_FrameLength = 8  ;                                             // 1地址码+1功能码+2字节地址 +2字节寄存器数量 +2校验码  =  8字节
	CRC16_CheckCompute( MODBUS_FrameData, 6, &MODBUS_FrameData[6]);       // 发送数据crc16计算
}






/************************************************基本功能子函数*****************************************************/

/*************************************************************************
  * @brief    从机接受数据错误，回应主机数据错误
  * @param    com : 接受到的功能码
  *           error：错误码 
  * @retval   无
  * @notice   错误功能码是接收到的功能码加0X80
**************************************************************************/
static void Modbus_Function_Error(uint8_t com , uint8_t error)
{
	MODBUS_FrameLength = 5;                                               // 1地址码+1功能码+1字节数+2校验码=5字节
                                                                        // 从机地址 MODBUS_FrameData[0] 直接使用接收到的地址
	MODBUS_FrameData[1]= com + 0x80 ;                                     // 错误功能码
	MODBUS_FrameData[2]= error ;                                          // 错误异常码
	CRC16_CheckCompute( MODBUS_FrameData, 3, &MODBUS_FrameData[3]);       // 发送数据crc16计算
}



/*************************************************************************
  * @brief  把位数组中数据每8个一组转换为字节数组
  * @param  IN      :BITArray 头指针
  *         IN_Num  :需要转换的位数
  *         OUT     :转换后输出数组 ByteArray 头指针
  *         OUT_Num :转换后的字节数
  * @retval 无
  * @notice 无
*************************************************************************/
static void BITArrayCopyByteArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t *OUT_Num)
{
	uint8_t i ,j ,dat ;                    
	
	*OUT_Num  = ( IN_Num - 1 ) / 8 + 1 ;                                  // 计算相应的字节数
	
	j   = 0 ;                                                             //数据保存位置计数 
	dat = 0 ;                                                             //给dat赋初值
	for( i = 0 ; i < IN_Num ; i++ )        
	{ 
		if(IN[i] & 0X01)                                                    //该位为1 ，则最高位补1
			dat = dat | 0X80 ;                                                //8位数据组成一个字节数据 （地址大的数据放在byte的高位）

		if( i % 8 == 7 )                                                    //读取到8个数据
		{
			 OUT[j++] = dat ;                                                 //保存数据
			 dat      = 0   ;                                                 //数据清零               
		}	
		else
		{
		  dat = dat >> 1 ;                                                  // 还没够8位数据 ，右移把最高位留给下一位
		}
	}
	                                                                      //下面几行程序为对最后一个不完整字节数据的处理方法
	i = IN_Num % 8 ;                                                      //判断需要组合的数据是否为8的倍数
  if( i != 0 )	                                                        //不是8倍数时将还有一部分数据没保存，对未保存数据进行处理再保存
	{
		dat  = dat >> ( 7 - i ) ;                                           //不是8的倍数时，需要把数据进行右移        
    OUT[j] = dat ;                                                      //保存最后一个数据
	}		
}




/************************************************************************
  * @brief  把字节数组按位拆开，从新组成一个位数组
  * @param  IN      :ByteArray 头指针
  *         IN_Num  :需要转换字节数
  *         OUT     :转换后输出数组BITArray头指针
  *         OUT_Num :字节数不一等转换完后全部输出，OUT_Num表示转换后需要输出的位数
  * @retval 无
  * @notice 无
************************************************************************/
static void ByteArrayCopyBITArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t OUT_Num)
{
	uint8_t i ,j ,dat ;                    
	j = 0 ;                                                               //数据保存位置计数 
  dat = IN[0];                                                          //把第一个数据放变量dat中	
	for( i = 0 ; i < OUT_Num ; i++ )                                      //需要填充的数据量计数      
	{ 
		OUT[i] = dat & 0X01 ;                                               //将byte各位数据写入到bit数组中 （字节各位中，先写低位）                  
    dat    = dat >> 1 ;                                                 //右移，下一个地址对应的的数据                                                   
		if( i % 8 == 7 )                                                    //读取到8个数据
		{	                                                                  //保存数据 
			if( ++j > IN_Num ) return ;                                       //访问溢出，退出函数
		  dat = IN[j] ;	                                                    //在没有溢出的条件下，把下一字节的数据给dat
		}	              
	}
}




/*************************************************************************
  * @brief  数组复制
  * @param  IN ： 输入数组头指针 ；  OUT ： 输出数组头指针 ；   Num ：需要复制的数据量
  * @retval 无
  * @notice 无
*************************************************************************/
static void ByteArrayCopyByteArray( uint8_t *IN , uint8_t *OUT ,uint8_t Num)
{
  while( Num-- )
	{
	  *OUT++ = *IN++ ; 
	}
}



/*************************************************************************
  * @brief  16位数组复制到字符数组
  * @param  IN ： 输入数组头指针 ；  OUT ： 输出数组头指针 ；   Num ：需要复制的数据量
  * @retval 无
  * @notice 先保存16位数据的高字节，再保存低字节 （ 对于16为数据在小端存储模式下，小地址保存低字节数据 ，大地址保存高字节数据）
  *         CopyType = 0 ： 复制低字节· ，CopyType ！=0 ：非零表示高低字节交差复制   
*************************************************************************/
static void HalfWordArrayCopyByteArray( uint16_t *IN , uint8_t *OUT ,uint8_t IN_Num ,uint8_t CopyType)
{
	uint8_t *In ;                                                           
	uint8_t i ;                                                              
	In  = (uint8_t *)IN ;                                                     // 将输入的16位数据指针转换为8位数据指针进行访问
  if( CopyType != 0 )                                                       // 非零表示高低字节交差复制                                                     
	{
		for( i = 0 ; i < 2 * IN_Num ; i = i + 2 )                               // 数据填充计数 ，下面为交叉填充
		{
			OUT[i+1] = In[i] ;                                                    // [1] <-- [0] ,  [3] <-- [2] , [5] <-- [4]  ......  
			OUT[i]   = In[i+1] ;                                                  // [0] <-- [1] ,  [2] <-- [3] , [4] <-- [5] ......  
		}
	}	
  else                                                                      // 0表示复制低字节·
	{
		for( i = 0 ; i < IN_Num ; i = i + 1 )                                   // 数据填充计数 ，直接填充 ，高字节舍弃不要
		{ 
			OUT[i] = IN[i] ;                                                      // [0]（ 8位） <-- [0]（16位） ,  [1] <-- [1] , [2] <-- [2] ......  
		}
	}		
}


/*************************************************************************
  * @brief  8位数组复制到16位数组
  * @param  IN ： 输入数组头指针 ；  OUT ： 输出数组头指针 ；   Num ：需要复制的数据量
  * @retval 无
  * @notice 先保存16位数据的高字节，再保存低字节 （ 对于16为数据在小端存储模式下，小地址保存低字节数据 ，大地址保存高字节数据）
  *         CopyType = 0 ： 只填充低字节 ，高字节补零· ，CopyType ！=0 ：两个8位数据组成一个16位数据
*************************************************************************/
static void ByteArrayCopyHalfWordArray( uint8_t *IN , uint16_t *OUT ,uint8_t OUT_Num ,uint8_t CopyType)
{                                                        
	uint8_t i,j ;                                                           
  if( CopyType != 0 )                                                       // 非零表示两个8位数据组成一个16位数据                                                  
	{
		for( i = 0 , j = 0 ; i < 2 * OUT_Num ; i = i + 2 ,j++)                  // 数据填充计数 ，下面为交叉填充
		{
			OUT[j] = IN[i] << 8 | IN[i+1];                                        // [0] <-- [0]<<8|[1] , [1] <-- [2]<<8|[3]  ......  
		}
	}	
  else                                                                      // 0表示只填充低字节 ，高字节补零
	{
		for( i = 0 ; i < OUT_Num ; i = i + 1 )                                  // 数据填充计数 ，直接填充 ，高字节舍弃不要
		{ 
			OUT[i] = IN[i] ;                                                      // [0] <-- [0],  [1] <-- [1] , [2] <-- [2] ......  
		}
	
	}		
}



