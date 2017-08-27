#include "MODBUS.h"

/******************************************MODBUS���ܵ����һ֡���ݼ���*****************************************/ 
#ifndef MODBUS_FrameData_MaxNumSize                                                                       // ���û�к궨��MODBUS_FrameData_MaxNumSize  ����ʹ��Ĭ�ϵ�MODBUS_FrameData_MaxNumSize 
#define MODBUS_FrameData_MaxNumSize01          ( MODBUS_Addr0_NumSize > MODBUS_Addr1_NumSize ? ((MODBUS_Addr0_NumSize -1 ) >> 3 ) +1  : ((MODBUS_Addr1_NumSize  -1 ) >> 3 )+1 )               //MODBUS ����0 �� ���� 1 �ڴ��С�Ƚ�
#define MODBUS_FrameData_MaxNumSize34          ( MODBUS_Addr3_NumSize > MODBUS_Addr4_NumSize ? 2*MODBUS_Addr3_NumSize : 2*MODBUS_Addr4_NumSize )                                              //MODBUS ����3 �� ���� 4 �ڴ��С�Ƚ�
#define MODBUS_FrameData_MaxNumSize            ( MODBUS_FrameData_MaxNumSize01 > MODBUS_FrameData_MaxNumSize34 ? MODBUS_FrameData_MaxNumSize01 + 10 : MODBUS_FrameData_MaxNumSize34  + 10  )  // ��10��modbus ������������ռ� ��
#endif                                                                                                    // 1�ӻ���ַ +1������ + 2��ʼ��ַ +2������ +2CRCУ�飨 Ĭ�ϵ�MODBUS_FrameData_MaxNumSizeΪ��ȡһ���ַ���������ݵ����ֵ ��

/******************************************MODBUSͨ�Ÿ��ֵ�ַ�����������С��ַ����****************************/ 
#define MODBUS_Addr0_MinAddr                   ( MODBUS_Addr0_StartAddr )                                 // MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д) ����СMODBUS��ַ  
#define MODBUS_Addr0_MaxAddr                   ( MODBUS_Addr0_StartAddr + MODBUS_Addr0_NumSize  )         // MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д) �����MODBUS��ַ  
#define MODBUS_Addr1_MinAddr                   ( MODBUS_Addr1_StartAddr )                                 // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)����СMODBUS��ַ  
#define MODBUS_Addr1_MaxAddr                   ( MODBUS_Addr1_StartAddr + MODBUS_Addr1_NumSize  )         // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)�����MODBUS��ַ  
#define MODBUS_Addr3_MinAddr                   ( MODBUS_Addr3_StartAddr )                                 // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)����СMODBUS��ַ  
#define MODBUS_Addr3_MaxAddr                   ( MODBUS_Addr3_StartAddr + MODBUS_Addr3_NumSize  )         // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)�����MODBUS��ַ  
#define MODBUS_Addr4_MinAddr                   ( MODBUS_Addr4_StartAddr )                                 // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)����СMODBUS��ַ   
#define MODBUS_Addr4_MaxAddr                   ( MODBUS_Addr4_StartAddr + MODBUS_Addr4_NumSize  )         // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)�����MODBUS��ַ   



/******************************************MODBUS�ڴ����******************************************************/ 
uint8_t MODBUS_Addr0[MODBUS_Addr0_NumSize] ;                                                              // MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д) 
uint8_t MODBUS_Addr1[MODBUS_Addr1_NumSize] ;                                                              // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)
uint8_t MODBUS_Addr3[MODBUS_Addr3_NumSize * 2 ] ;                                                         // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)
uint8_t MODBUS_Addr4[MODBUS_Addr4_NumSize * 2 ] ;                                                         // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)

uint8_t MODBUS_FrameData[MODBUS_FrameData_MaxNumSize] ;                                                   // MODBUS���պͷ���һ֡������ʱ����λ��
uint8_t MODBUS_FrameLength ;                                                                              // MODBUSһ֡���ݵĳ���
uint8_t MODBUS_SlaveID  = 0X01;                                                                           // MODBUS �ӻ�ID��(Ĭ��Ϊ0X01)
	



/******************************** MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)  **********************************/
static void MODBUS_Function_01(void);                                                                     // ����Ȧ
static void MODBUS_Function_05(void);                                                                     // д������Ȧ
static void MODBUS_Function_15(void);                                                                     // д�����Ȧ

/******************************** MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)  **************************************/
static void MODBUS_Function_02(void);                                                                     // ��������ɢ��  

/******************************** MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)  ************************************/
static void MODBUS_Function_04(void);                                                                     // ������Ĵ���

/******************************** MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)  ********************************/
static void MODBUS_Function_03(void);                                                                     // ������Ĵ���
static void MODBUS_Function_06(void);                                                                     // д�����Ĵ���
static void MODBUS_Function_16(void);                                                                     // д����Ĵ���

/************************************************���������Ӻ���*************************************************/
static void Modbus_Function_Error(uint8_t com , uint8_t error);                                           //MODBUS Э�����ʱ�����������Ӧ��������ԭ�������ݵ�ַ��� ��������̫�󣬲�֧�ִ˹����룩
static void BITArrayCopyByteArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t *OUT_Num);          // ��λ����������ÿ8��һ��ת��Ϊ�ֽ�����
static void ByteArrayCopyBITArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t OUT_Num);           // ���ֽ�����ÿλ��Ϊһ��λ����
static void ByteArrayCopyByteArray( uint8_t *IN , uint8_t *OUT ,uint8_t Num);                             // �ֽ��������ݴ�IN���鸴�Ƶ�OUT����
static void HalfWordArrayCopyByteArray( uint16_t *IN , uint8_t *OUT ,uint8_t IN_Num ,uint8_t CopyType);   // �������ݰ�һ���Ĺ����Ƶ��ֽ�������
static void ByteArrayCopyHalfWordArray( uint8_t *IN , uint16_t *OUT ,uint8_t OUT_Num ,uint8_t CopyType);  // �ֽ��������ݰ�һ���Ĺ����Ƶ�����������


/************************************************�û�����******************************************************/
void MODBUS_Init(uint32_t Baudrate);                                                                      // MODBUS ��ʼ���ܺ��� �����ò����ʺ�֡���ݼ��ʱ����
void MODBUS_HandleFunction(void);                                                                         // MODBUSЭ���ܴ����� (���ڽ��յ�һ֡���ݶ��䴦���浽��Ӧ�ĵ�ַ�����Ӧ��������Ӧ)
void MODBUS_ChangeSalveID(uint8_t SalveID);                                                               // �޸Ĵӻ���ID�� �������modbus��ʼ��ǰ�����޸ģ�         
ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );            // CPU��ȡMODBUS��ַ���� ,һ�����ݶ�Ӧһ����ַ
ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr);            // CPUдMODBUS��ַ����   ��һ�����ݶ�Ӧһ����ַ
ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );        // ��ȡMODBUS��ַ���� ��  ����0 �� 1 ���͵�ַ ��һ�����ݶ���һ����ַ ������3��4�ĵ�ַ���� ���������ݶ�Ӧ��һ����ַ ���� ��д���ֽ���д���ֽڣ� 
ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr );         // CPUдMODBUS��ַ���� �� ����0 �� 1 ���͵�ַ ��һ�����ݶ���һ����ַ ������3��4�ĵ�ַ���� ���������ݶ�Ӧ��һ����ַ ���� �ȶ����ֽ��ٶ����ֽڣ� 
	



/****************************************************************************************************************
  * @brief  �޸Ĵӻ���ID�� ��������modbus��ʼ��ǰ�����޸ģ�     
  * @param  SalveID ���µĴӻ�ID��
  * @retval ��
  * @notice ������modbus��ʼ��ǰ�����޸�
*****************************************************************************************************************/
void MODBUS_ChangeSalveID(uint8_t SalveID)                                                                // �޸Ĵӻ���ID�� �������modbus��ʼ��ǰ�����޸ģ�         
{
	MODBUS_SlaveID = SalveID ;                                                                              // �����ݱ��浽�ӻ�ID������
}





/****************************************************************************************************************
  * @brief  MODBUSЭ���ܴ�����
  * @param  ��
  * @retval ��
  * @notice ��ַ0x00Ϊ�㲥��ַ ������Ҫ������Ӧ�����߽�ͨ�ų�ͻ���� 
*****************************************************************************************************************/
void MODBUS_HandleFunction(void)
{
	uint8_t crc16[2];
  
	/*********�ж��Ƿ���յ�һ֡����*************/	
	if( MODBUS_ReceiveOneFrameFinish() == RESET ) return ;                                // ��û�н��յ�һ֡���� ���˳�����    	
 
	/**********��ȡ���յ���һ֡����**************/	
	MODBUS_FrameDataReceive(MODBUS_FrameData, &MODBUS_FrameLength);                       // ��ȡMODBUS���յ���һ֡���� �� MODBUS_FrameData �� �������Ƕ���֡���ݽ��д���
	if((MODBUS_FrameData[0] != MODBUS_SlaveID )&&(MODBUS_FrameData[0] != 0X00))	          // 0 :��ʾ�㲥ģʽ ��MODBUS_SlaveID Ϊ�ӻ���ַ 
		goto FrameError;                                                                    // ��ַ��ƥ�䣬�������ݽ��մ���Ĵ���λ��
    CRC16_CheckCompute( MODBUS_FrameData , MODBUS_FrameLength- 2 ,crc16);				          // crc����	
	if( CRC16_Check( &MODBUS_FrameData[MODBUS_FrameLength - 2 ]  ,crc16 ) != SUCCESS)     // crc�Ƚϣ���ȷ����Ӧ�� �������ʾ������̳�����ʱʲôҲ������
	  goto FrameError;                                                                    // ����֡У����� ���������ݽ��մ���Ĵ���λ��   
	
	switch(MODBUS_FrameData[1])                                                           //��ͬ������ִ�в�ͬ�ĳ���
	{	    
		/******************************** MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)  **********************************/
		case 1  : MODBUS_Function_01();break ;                                              // ����Ȧ
		case 5  : MODBUS_Function_05();break ;                                              // д������Ȧ
		case 15 : MODBUS_Function_15();break ;                                              // д�����Ȧ
				
		/******************************** MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)  **************************************/
		case 2  : MODBUS_Function_02();break ;                                              // ��������ɢ��                                    

		/******************************** MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)  ************************************/
		case 4  : MODBUS_Function_04();break ;                                              // ������Ĵ���
		
		/******************************** MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)  ********************************/
		case 3  : MODBUS_Function_03();break ;                                              // ������Ĵ���
		case 6  : MODBUS_Function_06();break ;                                              // д�����Ĵ���
		case 16 : MODBUS_Function_16();break ;                                              // д����Ĵ���
			
		/******************************** MODBUS ��֧�ִ˹���  ********************************************************/
		default : Modbus_Function_Error(MODBUS_FrameData[1] ,0x01);                         // ��֧�ִ˹���
	}
	
	if(MODBUS_FrameData[0] != 0 )                                                         // 0X00��ַΪ�㲥��ַ������Ҫ��Ӧ����  , �� FrameError ����һ�� ��Ϊ��һ֡���ݽ�����׼��
  {		                                                                                  // ��һ��������ȷ������� ������Ӧ����ж������һ֡���ݵĽ��պ���      
		MODBUS_FrameDataSend(MODBUS_FrameData, MODBUS_FrameLength);   	                    // ��ʱ��ַΪ�ӻ���ַ ������׼���õ�һ֡����	
		return ;                                                                            // �����շ����� ���˳�����
  }                 
  
	/***************�������ݴ��� �� ���յĲ��Ǳ������� �� Ϊ�㲥��ַ ��ҪΪ��һ֡���ݵĽ�������׼������**************/                                                                         
FrameError :                                                                            // �������ݳ�����׼������
	  MODBUS_EnableReceiveData();                                                         // Ϊ��һ֡���ݵĽ�����׼��	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
}





/**************************************************************************
  * @brief  MODBUS ��ʼ���ܺ��� �����ò����ʺ�֡���ݼ��ʱ����
  * @param  Baudrate �� ͨ�Ų�����
  * @retval ��
  * @notice ֡ʱ����Ϊ TimeMs =  40000  / Baudrate ; ��ʱ����С��2Msʱ��ǿ��Ϊ2Ms
**************************************************************************/
void MODBUS_Init(uint32_t Baudrate)
{
	 uint8_t TimeMs ; 
	 
	 /************����֡���ݼ��ʱ����***********/
 	 TimeMs =  40000  / Baudrate ;                                                        //֡���ݼ䶨ʱʱ�����                                                 
     if( TimeMs < 2 )  TimeMs = 2 ;                                                       //����֡���ݼ���Сʱ��Ϊ2Ms
   
	 /********���ڲ������붨ʱ����ʱʱ������*******/
	 MODBUS_USARTInit(Baudrate);                                                          //���ڲ���������
	 MODBUS_TIMInit(TimeMs);                                                              //��ʱ����ʱʱ������ 
} 









/**************************************************************************
  * @brief  ά��MODBUS��һ������������MODBUS�ڴ�д���� ��8bit���ʣ�
  * @param  Data�� ��Ҫд��MODBUS������������
  *         Num �� ��Ҫд���������� 
  *         MODBUS_StartAddr ����д���ݵ�MODBUS��ַ 
  * @retval ERROR :   ������������,�����κβ������˳����� ;  
  *         SUCCESS : s����������ȷ ����д������
  * @notice MODBUS_StartAddr ��ǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
  *         ����0 ��1 ���͵�ַ���ݵ�д��һ�����ݶ�Ӧһ����ַ 
  *         ����3 ��4 ���͵�ַ���ݵ�д ���������ݶ�Ӧһ����ַ �� ��д���ֽ����� ����д���ֽ����ݣ�
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataWriteByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                                             // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	uint16_t StartAddrIndex ;                                                             // ��ַ������
	uint8_t  *p ;                                                                         // ָ����ҪĿ���׵�ַ
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                                          // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                                        // ��һ����ַ�����µ���ʵMODBUS��ַ
  switch(AddrTypeSelect)                                                                // ��ͬ�ĵ�ַ���������ݱ���λ�ò�һ��
	{
		case 0 :                                                                                                            //  MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // ��ַ���������� 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;     break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
	  case 1 :                                                                                                            // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // ��ַ����������      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;     break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		case 3 :                                                                                                            // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr3_MaxAddr )) return ERROR;  // ��ַ����������     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ; break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		case 4 :                                                                                                            // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr4_MaxAddr )) return ERROR;  // ��ַ����������   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ; break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		default:  return ERROR ; 
	}
	ByteArrayCopyByteArray( Data, p ,Num) ;                                                                               //���ݸ������ 
	return SUCCESS ;
}
		



/**************************************************************************
  * @brief  ά��MODBUS��һ������������MODBUS�ڴ������ �� 8bit���ʣ�
  * @param  Data�� ��ȡ�����ݱ���λ��
  *         Num �� ��Ҫ��ȡ��������
  *         MODBUS_StartAddr �� ��ȡMODBUS����λ��
  * @retval ERROR :   ������������,�����κβ������˳����� ;  
  *         SUCCESS : s����������ȷ ����д������
  * @notice MODBUS_StartAddr ��ǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
  *         ����0 ��1 ���͵�ַ���ݵĶ���һ�����ݶ�Ӧһ����ַ 
  *         ����3 ��4 ���͵�ַ���ݵĶ����������ݶ�Ӧһ����ַ �� �ȶ����ֽ����� ���ٶ����ֽ����ݣ�
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataReadByte(uint8_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                                             // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	uint16_t StartAddrIndex ;                                                             // ��ַ������
	uint8_t  *p ;                                                                         // ָ����ҪĿ���׵�ַ
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                                          // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	MODBUS_StartAddr = MODBUS_StartAddr & 0XFFFF ;                                        // ��һ����ַ�����µ���ʵMODBUS��ַ
  switch(AddrTypeSelect)                                                                // ��ͬ�ĵ�ַ���������ݱ���λ�ò�һ��
	{
		case 0 :                                                                                                            //  MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // ��ַ���������� 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;     break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
	  case 1 :                                                                                                            // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // ��ַ����������      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;     break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		case 3 :                                                                                                            // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr3_MaxAddr )) return ERROR;  // ��ַ����������     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ; break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		case 4 :                                                                                                            // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + ( ( Num - 1 ) >> 1 ) + 1  > MODBUS_Addr4_MaxAddr )) return ERROR;  // ��ַ����������   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ; break ;     // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		default:  return ERROR; 
	}
	ByteArrayCopyByteArray( p, Data ,Num) ;                                                                                        //���ݸ������
  return SUCCESS ;
}



/**************************************************************************
  * @brief  ά��MODBUS��һ������������MODBUS�ڴ�д���� �� 16bit���ʣ�
  * @param  Data�� ��Ҫд��MODBUS������������
  *         Num �� ��Ҫд���������� 
  *         MODBUS_StartAddr ����д���ݵ�MODBUS��ַ 
  * @retval ERROR :   ������������,�����κβ������˳����� ;  
  *         SUCCESS : s����������ȷ ����д������
  * @notice MODBUS_StartAddr ��ǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
  *         ����0 ��1 ���͵�ַ���ݵ�д��һ�����ݶ�Ӧһ����ַ ��ֻд16λ���ݵĵ�8λ��
  *         ����3 ��4 ���͵�ַ���ݵ�д��һ�����ݶ�Ӧһ����ַ ( �ߵ��ֽ�ȫ��д��)
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataWrite(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                               // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	uint16_t StartAddrIndex ;                                               // ��ַ������
	uint8_t  *p ;                                                           // ָ����ҪĿ���׵�ַ
	uint8_t  CopyType ;                                                     // ���Ʒ�ʽ
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                            // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                          // ��һ����ַ�����µ���ʵMODBUS��ַ
  switch(AddrTypeSelect)                                                  // ��ͬ�ĵ�ַ���������ݱ���λ�ò�һ��
	{
		case 0 :                                                                                                            //  MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // ��ַ���������� 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;                 // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		    CopyType = 0 ;  break ;                                                                                           // ���Ʒ�ʽѡ��ֻ����16λʫ��ĵ�8λ
	  case 1 :                                                                                                            // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // ��ַ����������      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;                 // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		    CopyType = 0 ;  break ;                                                                                           // ���Ʒ�ʽѡ��ֻ����16λʫ��ĵ�8λ
		case 3 :                                                                                                            // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr3_MaxAddr )) return ERROR;  // ��ַ����������     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ;             // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		    CopyType = 1 ; break ;                                                                                            // ���Ʒ�ʽѡ�񣬸ߵ��ֽڽ����   
		case 4 :                                                                                                            // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr4_MaxAddr )) return ERROR;  // ��ַ����������   
	  	    StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ;             // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		    CopyType = 1 ; break ;                                                                                            // ���Ʒ�ʽѡ�񣬸ߵ��ֽڽ����     
		default:  return ERROR ;                                                     
	}
	HalfWordArrayCopyByteArray( Data , p, Num,CopyType);                                                                  // CopyType = 0 �� ���Ƶ��ֽڡ� ��CopyType ��=0 �������ʾ�ߵ��ֽڽ����   
	return SUCCESS ;
}
		

/**************************************************************************
  * @brief  ά��MODBUS��һ������������MODBUS�ڴ������ �� 16bit���ʣ�
  * @param  Data�� ��ȡ�����ݱ���λ��
  *         Num �� ��Ҫ��ȡ��������
  *         MODBUS_StartAddr �� ��ȡMODBUS����λ��
  * @retval ERROR :   ������������,�����κβ������˳����� ;  
  *         SUCCESS : s����������ȷ ����д������
  * @notice MODBUS_StartAddr ��ǰ16λ��ʾ��ַ���� ����16λ��ʾ��������������µ���ʵ��ַ 
  *         ����0 ��1 ���͵�ַ���ݵĶ���һ�����ݶ�Ӧһ����ַ  ��ֻ�����ʽ磬���ֽڲ��㣩
  *         ����3 ��4 ���͵�ַ���ݵĶ���һ�����ݶ�Ӧһ����ַ  ���ߵ��ֽ����ݶ���д��
**************************************************************************/
ErrorStatus MODBUS_AddrMapDataRead(uint16_t *Data , uint8_t Num , uint32_t MODBUS_StartAddr )
{
	uint32_t AddrTypeSelect ;                                               // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	uint16_t StartAddrIndex ;                                               // ��ַ������
	uint8_t  *p ;                                                           // ָ����ҪĿ���׵�ַ
	uint8_t  CopyType ;                                                     // ���Ʒ�ʽ
	AddrTypeSelect    = MODBUS_StartAddr >> 16 ;                            // MODBUS��ַ����ѡ�� 0 �� 1 �� 3 ��4 ��
	MODBUS_StartAddr  = MODBUS_StartAddr &0XFFFF ;                          // ��һ����ַ�����µ���ʵMODBUS��ַ
  switch(AddrTypeSelect)                                                  // ��ͬ�ĵ�ַ���������ݱ���λ�ò�һ��
	{
		case 0 :                                                                                                            //  MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)                                    
			if(( MODBUS_StartAddr < MODBUS_Addr0_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr0_MaxAddr )) return ERROR;  // ��ַ���������� 
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr0_StartAddr  ; p = &MODBUS_Addr0[StartAddrIndex] ;                 // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		  CopyType = 0 ;  break ;                                                                                           // ���Ʒ�ʽѡ��ֻ�����ֽ� �����ֽڲ��㡤 
	  case 1 :                                                                                                            // MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��) 
			if(( MODBUS_StartAddr < MODBUS_Addr1_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr1_MaxAddr )) return ERROR;  // ��ַ����������      
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr1_StartAddr  ; p = &MODBUS_Addr1[StartAddrIndex] ;                 // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		  CopyType = 0 ;  break ;                                                                                           // ���Ʒ�ʽѡ��ֻ�����ֽ� �����ֽڲ��㡤 
		case 3 :                                                                                                            // MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)
			if(( MODBUS_StartAddr < MODBUS_Addr3_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr3_MaxAddr )) return ERROR;  // ��ַ����������     
			StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr3_StartAddr  ; p = &MODBUS_Addr3[StartAddrIndex * 2] ;             // ָ��pָ����Ҫ��д����Ŀ���׵�ַ 
		  CopyType = 1 ; break ;                                                                                            // ���Ʒ�ʽѡ������8λ�������һ��16λ����                  
		case 4 :                                                                                                            // MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)
			if(( MODBUS_StartAddr < MODBUS_Addr4_MinAddr )||(MODBUS_StartAddr + Num  > MODBUS_Addr4_MaxAddr )) return ERROR;  // ��ַ����������   
	  	StartAddrIndex = MODBUS_StartAddr - MODBUS_Addr4_StartAddr  ; p = &MODBUS_Addr4[StartAddrIndex * 2] ;             // ָ��pָ����Ҫ��д����Ŀ���׵�ַ
		  CopyType = 1 ; break ;                                                                                            // ���Ʒ�ʽѡ������8λ�������һ��16λ����                  
		default:  return ERROR ;                                                     
	}
	ByteArrayCopyHalfWordArray( p , Data, Num , CopyType);                                                                // CopyType = 0 �� ֻ�����ֽ� �����ֽڲ��㡤 ��CopyType ��=0 ������8λ�������һ��16λ����                                                    
  return SUCCESS; 
}
	




/******************************** MODBUS ��ַ���� 0XXXX (bit���ʣ��ɶ���д)  ***************************************/

/************************************************************************
  * @brief  MODBUS ������ 0x01 ��  ����Ȧ
  * @param  ��
  * @retval ��
  * @notice ��
************************************************************************/
static void MODBUS_Function_01(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ;                                              // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr0_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x01 , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address + data_num > MODBUS_Addr0_MaxAddr) )  // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x01 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr0_StartAddr ;                 // ���㷢�͵ĵ�ַ��������������
		
	BITArrayCopyByteArray(&MODBUS_Addr0[addr_index] , data_num ,&MODBUS_FrameData[3] ,&Senddata_num);//λ����ת��Ϊ�ֽ�����
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1��ַ��+1������+1�ֽ��� + Senddata_num ���״̬+2У����  =  Senddata_num + 5�ֽ�
                                                                        // �ӻ���ַ MODBUS_FrameData[0] ֱ��ʹ�ý��յ��ĵ�ַ
	MODBUS_FrameData[1]= 0X01 ;                                           // ������
  MODBUS_FrameData[2]= Senddata_num ;                                   // ��Ҫ���͵��ֽ��� 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// crc16����

}




/***********************************************************************
  * @brief  MODBUS ������ 0x05 ��  д������Ȧ
  * @param  ��
  * @retval ��
  * @notice ���յ�������ֻ����0xff00��0�� �� 0x0000��1�� �� ��Ӧ��ֱ�ӷ��ͽ��յ������ݼ���
  *       ���ݵ�ַ����  �� ( start_address +NUM��û�е��ں�  ��ֻ��start����address����Ҫ���ں�)
************************************************************************/
static void MODBUS_Function_05(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  OutValue ;                                                  // ���ֵ
	uint16_t  addr_index ;                                                // ��ַ������
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	OutValue      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��Ҫ�Ե�ַд��ֵ
	
	if( (OutValue  != 0X0000  )&&  (OutValue  != 0XFF00  ) )              // ����ֵ���� �� ֻ���� 0x0000 �� 0XFF00 ��
	{ 
		Modbus_Function_Error(0x05 , 0x03);                                 // ����ֵ���� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address  >= MODBUS_Addr0_MaxAddr) )    // ���ݵ�ַ���� ( start_address +NUM��û�е��ں�  ��ֻ��start����address����Ҫ���ں�)
	{
		Modbus_Function_Error(0x05 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr0_StartAddr ;                 // ���㷢�͵ĵ�ַ�������������� 
	if( OutValue == 0x0000)                                               // �ж�������ַ��д������
		MODBUS_Addr0[addr_index] = 	0 ;                                     // 0X0000 ���� OFF , д 0
	else
		MODBUS_Addr0[addr_index] = 	1 ;                                     // 0XFF00 ���� ON  , д 1

	MODBUS_FrameLength = 8;                                               // 1��ַ��+1������+2�����ַ + 2���ֵ+2У����  = 8�ֽ�
                                                                        // ��Ӧֱ�ӷ��ͽ��յ������ݼ���
}




/************************************************************************
  * @brief  MODBUS ������ 0x0f ��  д�����Ȧ
  * @param  ��
  * @retval ��
  * @notice ��Ӧ �����ͽ��յ���һ֡���ݵ�ǰ6���ֽ� ��2���ֽڵ�CRCУ��
************************************************************************/
static void MODBUS_Function_15(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ; 	                                            // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0

	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr0_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x0F , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr0_MinAddr)||( start_address + data_num > MODBUS_Addr0_MaxAddr) )  // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x0F , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	Senddata_num  = MODBUS_FrameData[6] ;                                 // �ֽ��� 
	addr_index   = start_address - MODBUS_Addr0_StartAddr ;               // �������ĵ�ַ��������������
	ByteArrayCopyBITArray(&MODBUS_FrameData[7] , Senddata_num ,&MODBUS_Addr0[addr_index],data_num);	//�ֽ�����ת��Ϊλ����
  
	MODBUS_FrameLength = 8;                                               // 1��ַ��+1������+2��ʼ��ַ +2������� +2У���� =  8�ֽ�
	CRC16_CheckCompute( MODBUS_FrameData, 6, &MODBUS_FrameData[6]);       // crc16����
                                                                        // ��Ӧ���ͽ��յ���һ֡���ݵ�ǰ6���ֽ� ��2���ֽڵ�CRCУ��
}





/******************************** MODBUS ��ַ���� 1XXXX (bit���ʣ�ֻ��)  ********************************************/

/************************************************************************
  * @brief  MODBUS ������ 0x02 ��  ��������ɢ��
  * @param  ��
  * @retval ��
  * @notice ��
*************************************************************************/
static void MODBUS_Function_02(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ;   	                                          // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr1_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x02 , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr1_MinAddr)||( start_address + data_num > MODBUS_Addr1_MaxAddr) ) // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x02 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	addr_index = start_address - MODBUS_Addr1_StartAddr ;                 // ���㷢�͵ĵ�ַ��������������
		
	BITArrayCopyByteArray(&MODBUS_Addr1[addr_index] , data_num ,&MODBUS_FrameData[3] ,&Senddata_num);//λ����ת��Ϊ�ֽ�����
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1��ַ��+1������+1�ֽ��� + Senddata_num ���״̬+2У����  =  Senddata_num + 5�ֽ�
                                                                        // �ӻ���ַ MODBUS_FrameData[0] ֱ��ʹ�ý��յ��ĵ�ַ
	MODBUS_FrameData[1]= 0X02 ;                                           // ������
  MODBUS_FrameData[2]= Senddata_num ;                                   // ��Ҫ���͵��ֽ��� 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// crc16����

}




/******************************** MODBUS ��ַ���� 3XXXX (16bit���ʣ�ֻ��)  **********************************/

/*************************************************************************
  * @brief  MODBUS ������ 0x04 ��  ������Ĵ���
  * @param  ��
  * @retval ��
  * @notice 16bit����
*************************************************************************/
static void MODBUS_Function_04(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ; 	                                            // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr3_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x04 , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr3_MinAddr)||( start_address +  data_num > MODBUS_Addr3_MaxAddr) )  // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x04 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //��Ӧ���ֽ���������Ĵ�����Ŀ��������16bit���ʣ� 
	addr_index   = (start_address - MODBUS_Addr3_StartAddr ) << 1 ;       // ���㷢�͵ĵ�ַ��������������
	ByteArrayCopyByteArray(&MODBUS_Addr3[addr_index] ,&MODBUS_FrameData[3] ,Senddata_num ); //��������
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1��ַ��+1������+1�ֽ��� + Senddata_num ���״̬+2У����  =  Senddata_num + 5�ֽ�
                                                                        // �ӻ���ַ MODBUS_FrameData[0] ֱ��ʹ�ý��յ��ĵ�ַ
	MODBUS_FrameData[1]= 0X04 ;                                           // ������
  MODBUS_FrameData[2]= Senddata_num ;                                   // ��Ҫ���͵��ֽ��� 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);// ��������crc16����

}






/******************************** MODBUS ��ַ���� 4XXXX (16bit���ʣ��ɶ���д)  **********************************/

/*************************************************************************
  * @brief  MODBUS ������ 0x03 ��  ������Ĵ���
  * @param  ��
  * @retval ��
  * @notice 16bit����
*************************************************************************/
static void MODBUS_Function_03(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ; 	                                            // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr4_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x03 , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address +  data_num > MODBUS_Addr4_MaxAddr) )  // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x03 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //��Ӧ���ֽ���������Ĵ�����Ŀ��������16bit���ʣ� 
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // ���㷢�͵ĵ�ַ��������������
	ByteArrayCopyByteArray(&MODBUS_Addr4[addr_index] ,&MODBUS_FrameData[3] ,Senddata_num ); //��������
  
	MODBUS_FrameLength = Senddata_num + 5;                                // 1��ַ��+1������+1�ֽ��� + Senddata_num ���״̬+2У����  =  Senddata_num + 5�ֽ�
                                                                        // �ӻ���ַ MODBUS_FrameData[0] ֱ��ʹ�ý��յ��ĵ�ַ
	MODBUS_FrameData[1]= 0X03 ;                                           // ������
  MODBUS_FrameData[2]= Senddata_num ;                                   // ��Ҫ���͵��ֽ��� 
	CRC16_CheckCompute( MODBUS_FrameData, Senddata_num + 3, &MODBUS_FrameData[Senddata_num + 3]);//crc16����

}




/*************************************************************************
  * @brief  MODBUS ������ 0x06 ��  д�����Ĵ���
  * @param  ��
  * @retval ��
  * @notice ���ݵ�ַ����  �� ( start_address +NUM��û�е��ں�  ��ֻ��start_address����Ҫ���ں�)
*************************************************************************/
static void MODBUS_Function_06(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  addr_index ;                                                // ��ַ������

	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address >= MODBUS_Addr4_MaxAddr) )  // ���ݵ�ַ���� ���˴��е��ں�
	{
		Modbus_Function_Error(0x06 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // ���㷢�͵ĵ�ַ�������������� (16bit����)
	ByteArrayCopyByteArray(&MODBUS_FrameData[4], &MODBUS_Addr4[addr_index],2 );    //��������
  
	MODBUS_FrameLength = 8;                                               // 1��ַ��+1������+2�ֽڵ�ַ +2�ֽ����� +2У����  =  8�ֽ�
                                                                        // ��Ӧֱ�ӷ��ͽ��յ������ݼ���
}




/************************************************************************
  * @brief  MODBUS ������ 0x10 ��  д����Ĵ���
  * @param  ��
  * @retval ��
  * @notice ��
************************************************************************/

static void MODBUS_Function_16(void)
{
	uint16_t  start_address ;                                             // ��ʼ��ַ
	uint16_t  data_num ;                                                  // ������
	uint16_t  addr_index ;                                                // ��ַ������
  uint8_t   Senddata_num ; 	                                            // ������Ӧʱ����ֽ���	
	start_address = ( MODBUS_FrameData[2] << 8 ) + MODBUS_FrameData[3] ;  // ��ʼ��ַ   ,��λ��ǰ
	data_num      = ( MODBUS_FrameData[4] << 8 ) + MODBUS_FrameData[5] ;  // ��ȡ������ ,��λ��ǰ,data_num�����256���ڣ�����RXDBuff[4]�϶�Ϊ0
	
	if( ( data_num < 1 ) || ( data_num > MODBUS_Addr4_NumSize) )          // ����������
	{
		Modbus_Function_Error(0x10 , 0x03);                                 // ���������� �����ش����� 0x03
		return ;
	}
	 
	if((start_address < MODBUS_Addr4_MinAddr)||( start_address +  data_num > MODBUS_Addr4_MaxAddr) )  // ���ݵ�ַ����
	{
		Modbus_Function_Error(0x10 , 0x02);                                 // ���ݵ�ַ���� �����ش����� 0x02
		return ;
	}
	
	if( MODBUS_FrameLength <  MODBUS_FrameData[6] + 9 )                   // ������������������������ ����Ӧ����дʧ�� �� ��16����������� data_num ��ʵ�ʽ��յ�����������ͬ�������
	{                                                                     // 1�ӻ���ַ + 1������ + 2��ʼ��ַ + 2������ + 1�ֽ��� + 2CRCУ�� = 9 �ֽ�
		Modbus_Function_Error(0x10 , 0x04);                                 // ������������������������ �����ش����� 0x04 :дʧ��
		return ;
	}
	
	Senddata_num =  data_num  <<  1 ;                                     //��Ӧ���ֽ���������Ĵ�����Ŀ��������16bit���ʣ� 
	addr_index   = (start_address - MODBUS_Addr4_StartAddr ) << 1 ;       // ���㷢�͵ĵ�ַ��������������
	ByteArrayCopyByteArray(&MODBUS_FrameData[7], &MODBUS_Addr4[addr_index] ,Senddata_num ); //��������
  
	MODBUS_FrameLength = 8  ;                                             // 1��ַ��+1������+2�ֽڵ�ַ +2�ֽڼĴ������� +2У����  =  8�ֽ�
	CRC16_CheckCompute( MODBUS_FrameData, 6, &MODBUS_FrameData[6]);       // ��������crc16����
}






/************************************************���������Ӻ���*****************************************************/

/*************************************************************************
  * @brief    �ӻ��������ݴ��󣬻�Ӧ�������ݴ���
  * @param    com : ���ܵ��Ĺ�����
  *           error�������� 
  * @retval   ��
  * @notice   ���������ǽ��յ��Ĺ������0X80
**************************************************************************/
static void Modbus_Function_Error(uint8_t com , uint8_t error)
{
	MODBUS_FrameLength = 5;                                               // 1��ַ��+1������+1�ֽ���+2У����=5�ֽ�
                                                                        // �ӻ���ַ MODBUS_FrameData[0] ֱ��ʹ�ý��յ��ĵ�ַ
	MODBUS_FrameData[1]= com + 0x80 ;                                     // ��������
	MODBUS_FrameData[2]= error ;                                          // �����쳣��
	CRC16_CheckCompute( MODBUS_FrameData, 3, &MODBUS_FrameData[3]);       // ��������crc16����
}



/*************************************************************************
  * @brief  ��λ����������ÿ8��һ��ת��Ϊ�ֽ�����
  * @param  IN      :BITArray ͷָ��
  *         IN_Num  :��Ҫת����λ��
  *         OUT     :ת����������� ByteArray ͷָ��
  *         OUT_Num :ת������ֽ���
  * @retval ��
  * @notice ��
*************************************************************************/
static void BITArrayCopyByteArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t *OUT_Num)
{
	uint8_t i ,j ,dat ;                    
	
	*OUT_Num  = ( IN_Num - 1 ) / 8 + 1 ;                                  // ������Ӧ���ֽ���
	
	j   = 0 ;                                                             //���ݱ���λ�ü��� 
	dat = 0 ;                                                             //��dat����ֵ
	for( i = 0 ; i < IN_Num ; i++ )        
	{ 
		if(IN[i] & 0X01)                                                    //��λΪ1 �������λ��1
			dat = dat | 0X80 ;                                                //8λ�������һ���ֽ����� ����ַ������ݷ���byte�ĸ�λ��

		if( i % 8 == 7 )                                                    //��ȡ��8������
		{
			 OUT[j++] = dat ;                                                 //��������
			 dat      = 0   ;                                                 //��������               
		}	
		else
		{
		  dat = dat >> 1 ;                                                  // ��û��8λ���� �����ư����λ������һλ
		}
	}
	                                                                      //���漸�г���Ϊ�����һ���������ֽ����ݵĴ�����
	i = IN_Num % 8 ;                                                      //�ж���Ҫ��ϵ������Ƿ�Ϊ8�ı���
  if( i != 0 )	                                                        //����8����ʱ������һ��������û���棬��δ�������ݽ��д����ٱ���
	{
		dat  = dat >> ( 7 - i ) ;                                           //����8�ı���ʱ����Ҫ�����ݽ�������        
    OUT[j] = dat ;                                                      //�������һ������
	}		
}




/************************************************************************
  * @brief  ���ֽ����鰴λ�𿪣��������һ��λ����
  * @param  IN      :ByteArray ͷָ��
  *         IN_Num  :��Ҫת���ֽ���
  *         OUT     :ת�����������BITArrayͷָ��
  *         OUT_Num :�ֽ�����һ��ת�����ȫ�������OUT_Num��ʾת������Ҫ�����λ��
  * @retval ��
  * @notice ��
************************************************************************/
static void ByteArrayCopyBITArray(uint8_t *IN , uint8_t IN_Num ,uint8_t *OUT ,uint8_t OUT_Num)
{
	uint8_t i ,j ,dat ;                    
	j = 0 ;                                                               //���ݱ���λ�ü��� 
  dat = IN[0];                                                          //�ѵ�һ�����ݷű���dat��	
	for( i = 0 ; i < OUT_Num ; i++ )                                      //��Ҫ��������������      
	{ 
		OUT[i] = dat & 0X01 ;                                               //��byte��λ����д�뵽bit������ ���ֽڸ�λ�У���д��λ��                  
    dat    = dat >> 1 ;                                                 //���ƣ���һ����ַ��Ӧ�ĵ�����                                                   
		if( i % 8 == 7 )                                                    //��ȡ��8������
		{	                                                                  //�������� 
			if( ++j > IN_Num ) return ;                                       //����������˳�����
		  dat = IN[j] ;	                                                    //��û������������£�����һ�ֽڵ����ݸ�dat
		}	              
	}
}




/*************************************************************************
  * @brief  ���鸴��
  * @param  IN �� ��������ͷָ�� ��  OUT �� �������ͷָ�� ��   Num ����Ҫ���Ƶ�������
  * @retval ��
  * @notice ��
*************************************************************************/
static void ByteArrayCopyByteArray( uint8_t *IN , uint8_t *OUT ,uint8_t Num)
{
  while( Num-- )
	{
	  *OUT++ = *IN++ ; 
	}
}



/*************************************************************************
  * @brief  16λ���鸴�Ƶ��ַ�����
  * @param  IN �� ��������ͷָ�� ��  OUT �� �������ͷָ�� ��   Num ����Ҫ���Ƶ�������
  * @retval ��
  * @notice �ȱ���16λ���ݵĸ��ֽڣ��ٱ�����ֽ� �� ����16Ϊ������С�˴洢ģʽ�£�С��ַ������ֽ����� �����ַ������ֽ����ݣ�
  *         CopyType = 0 �� ���Ƶ��ֽڡ� ��CopyType ��=0 �������ʾ�ߵ��ֽڽ����   
*************************************************************************/
static void HalfWordArrayCopyByteArray( uint16_t *IN , uint8_t *OUT ,uint8_t IN_Num ,uint8_t CopyType)
{
	uint8_t *In ;                                                           
	uint8_t i ;                                                              
	In  = (uint8_t *)IN ;                                                     // �������16λ����ָ��ת��Ϊ8λ����ָ����з���
  if( CopyType != 0 )                                                       // �����ʾ�ߵ��ֽڽ����                                                     
	{
		for( i = 0 ; i < 2 * IN_Num ; i = i + 2 )                               // ���������� ������Ϊ�������
		{
			OUT[i+1] = In[i] ;                                                    // [1] <-- [0] ,  [3] <-- [2] , [5] <-- [4]  ......  
			OUT[i]   = In[i+1] ;                                                  // [0] <-- [1] ,  [2] <-- [3] , [4] <-- [5] ......  
		}
	}	
  else                                                                      // 0��ʾ���Ƶ��ֽڡ�
	{
		for( i = 0 ; i < IN_Num ; i = i + 1 )                                   // ���������� ��ֱ����� �����ֽ�������Ҫ
		{ 
			OUT[i] = IN[i] ;                                                      // [0]�� 8λ�� <-- [0]��16λ�� ,  [1] <-- [1] , [2] <-- [2] ......  
		}
	}		
}


/*************************************************************************
  * @brief  8λ���鸴�Ƶ�16λ����
  * @param  IN �� ��������ͷָ�� ��  OUT �� �������ͷָ�� ��   Num ����Ҫ���Ƶ�������
  * @retval ��
  * @notice �ȱ���16λ���ݵĸ��ֽڣ��ٱ�����ֽ� �� ����16Ϊ������С�˴洢ģʽ�£�С��ַ������ֽ����� �����ַ������ֽ����ݣ�
  *         CopyType = 0 �� ֻ�����ֽ� �����ֽڲ��㡤 ��CopyType ��=0 ������8λ�������һ��16λ����
*************************************************************************/
static void ByteArrayCopyHalfWordArray( uint8_t *IN , uint16_t *OUT ,uint8_t OUT_Num ,uint8_t CopyType)
{                                                        
	uint8_t i,j ;                                                           
  if( CopyType != 0 )                                                       // �����ʾ����8λ�������һ��16λ����                                                  
	{
		for( i = 0 , j = 0 ; i < 2 * OUT_Num ; i = i + 2 ,j++)                  // ���������� ������Ϊ�������
		{
			OUT[j] = IN[i] << 8 | IN[i+1];                                        // [0] <-- [0]<<8|[1] , [1] <-- [2]<<8|[3]  ......  
		}
	}	
  else                                                                      // 0��ʾֻ�����ֽ� �����ֽڲ���
	{
		for( i = 0 ; i < OUT_Num ; i = i + 1 )                                  // ���������� ��ֱ����� �����ֽ�������Ҫ
		{ 
			OUT[i] = IN[i] ;                                                      // [0] <-- [0],  [1] <-- [1] , [2] <-- [2] ......  
		}
	
	}		
}



