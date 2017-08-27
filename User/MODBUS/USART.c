#include "USART.h"




/*****************************USART2 �ڴ����******************************/
uint8_t USART2_SendBusyFlag      = 0 ;                                     //����DMA��������æµ��־λ ��1�����ڷ������� ��0��DMA���������ݷ��� ����ʱ���Է�����һ֡����
uint8_t USART2_ReceiveFinishFlag = 0 ;                                     //���ڽ�����һ֡���ݱ�־λ ��1 ����ʾ������һ֡���� ��0����ʾһ֡���ݻ�δ�������
uint8_t USART2_SendData[USART2_SendSizeMax];                               //�������ݷ��ͻ�����
uint8_t USART2_ReceiveData[USART2_ReceiveSizeMax];                         //���ڽ������ݻ�����


/*****************************USART2 �û����� *****************************/ 
void USART2_Init(uint32_t Baudrate);                                       //USART2�������ó�ʼ�� ,Baudrate :������
void USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num);              //���ڷ�������     
void USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num);           //���ڶ�ȡ����
void DMA_SetAndEnableReceiveNum(void);                                     //ʹ�ܴ��ڼ����������� 
FlagStatus USART2_ReceiveOneFrameFinish(void);                             //�����Ƿ���յ�һ֡���� 
//void USART_Debug(void);                                                  //����ʹ�� �����ڽ���ʲô���ݾ͸����ڻ���Ŀ���ݣ�DMA��ʽ��


/*****************************USART2 �ڲ����� *****************************/ 
static void USART2_GPIO_Config(void);                                      //���ô���USART2���ݴ�������I/O��                 
static void USART2_Mode_Config(uint32_t Baudrate);                         //���ô���USART2����ģʽ 
static void USART2_NVIC_Config(void);                                      //�ж����ȼ�����
static void USART2_DMA_Config(void);                                       //USART2 DMA ����






/*************************************************************************                
  * @brief  ���ô���USART2���ݴ�������I/O��
  * @param  ��
  * @retval �� 
  * @notice TXD������� ��RXD�������� 
*************************************************************************/
static void USART2_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		 
	  /* Configure USART2 Tx (PA.02) as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;                           //TXD :PA2
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                      //TXD�������
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //I/O�����ٶ�50M/s
		GPIO_Init(GPIOA, &GPIO_InitStructure);                                //����
	
	  /* Configure USART2 Rx (PA.03) as input floating */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	                            //RXD :PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                 //��������
		GPIO_Init(GPIOA, &GPIO_InitStructure);                                //����

}





/*************************************************************************
  * @brief  ���ô���USART2����ģʽ
  * @param  Baudrate �� ������ 
  * @retval �� 
  * @notice ��
*************************************************************************/
static void USART2_Mode_Config(uint32_t Baudrate)
{
	USART_InitTypeDef USART_InitStructure;

	/* USART2 mode config */	
	USART_InitStructure.USART_BaudRate   = Baudrate ;                       //���ڲ����ʣ�USART2_BAUDRATE
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;             //֡���ݳ��ȣ�8λ
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;                //ֹͣλλ����1λ
	USART_InitStructure.USART_Parity     = USART_Parity_No ;                //��żУ�� ����У��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Ӳ����
	USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;   //���գ�����ʹ��
	USART_Init(USART2, &USART_InitStructure);                               //����
	USART_ITConfig(USART2, USART_IT_IDLE , ENABLE );                        //�������߿����ж�ʹ��
	USART_ITConfig(USART2, USART_IT_TC   , DISABLE );                       //��������жϴ�
	USART_Cmd(USART2, ENABLE);                                              //����USART2ʹ�� 
}





/*************************************************************************
  * @brief  ���ô���USART2�ж����ȼ�
  * @param  ��
  * @retval �� 
  * @notice ���ȼ� �� #include "USART2.h" ���� define �궨��
*************************************************************************/
static void USART2_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
		
	/* USART2_RX :USART2 RX ILDE configuration */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 	                    // ָ�� USART2 �ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART2_IRQPreemptionPrio ;//��ռʽ���ȼ�����
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART2_IRQSubPrio;      //�����ȼ�����
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //�ж�ʹ��
	NVIC_Init(&NVIC_InitStructure);                                         //���üĴ���

	/* USART2_TX :DMA1 channel7 configuration */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;                // ָ�� DMA1_Channel7_IRQn �ж� ������USART2��������жϣ�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Channel7_IRQPreemptionPrio ;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_Channel7_IRQSubPrio;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //�ж�ʹ��
  NVIC_Init(&NVIC_InitStructure);                                         //���üĴ���
	

} 



/*************************************************************************
  * @brief  ���ô���USART2��DMAģʽ 
  * @param  ��
  * @retval �� 
  * @notice ��
*************************************************************************/
static void USART2_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	/* USART2_TX :DMA1 channel7 configuration */
	DMA_DeInit(DMA1_Channel7);	                                            //DMA�Ĵ���ȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR ;      //DMAԴ��ַ   : ����1�����ݷ��͵�ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_SendData;	      //DMAĿ�ĵ�ַ ���ڴ��ַ
	DMA_InitStructure.DMA_DIR            = DMA_DIR_PeripheralDST ;          //���򣺴��ڴ浽����
	DMA_InitStructure.DMA_BufferSize     = USART2_SendSizeMax;              //DMA�����������
	DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;	      //�����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;  		      //�ڴ��ַ��1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte ;//�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte ;        //�ֽ�
	DMA_InitStructure.DMA_Mode           = DMA_Mode_Normal;					        //��������
	DMA_InitStructure.DMA_Priority       = DMA_Priority_High;               //DMA���ȼ�
	DMA_InitStructure.DMA_M2M            = DMA_M2M_Disable;                 //�ڴ浽�ڴ����ݴ��䲻ʹ��                      
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);                            //����

	DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);                           //DMA1 Channel7 ���ݴ�������ж�ʹ��
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);                          //USART2 ����DMAʹ��                
	DMA_Cmd(DMA1_Channel7, DISABLE);                                        //��ʹ��DMA,��������Ҫ����ʱʹ��DMA

	/* USART2_RX :DMA1 channel6 configuration */
	DMA_DeInit(DMA1_Channel6);	                                            //DMA�Ĵ���ȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR ;      //DMAԴ��ַ   : ����1�����ݽ��յ�ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_ReceiveData;    //DMAĿ�ĵ�ַ ���ڴ��ַ
	DMA_InitStructure.DMA_DIR            = DMA_DIR_PeripheralSRC ;          //���򣺴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize     = USART2_ReceiveSizeMax;           //DMA�����������
	DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;	      //�����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;  		      //�ڴ��ַ��1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte ;//�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte ;        //�ֽ�
	DMA_InitStructure.DMA_Mode           = DMA_Mode_Circular;					      //��������
	DMA_InitStructure.DMA_Priority       = DMA_Priority_High;               //DMA���ȼ�
	DMA_InitStructure.DMA_M2M            = DMA_M2M_Disable;                 //�ڴ浽�ڴ����ݴ��䲻ʹ��                      
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);                            //����
	
	DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,DISABLE);                          //DMA1 Channel6 ���ݴ�������жϹر�
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);                          //ʹ�� USART2 ���ݽ���DMAʹ��  
	DMA_Cmd(DMA1_Channel6, ENABLE);                                         //ʹ�ܽ���DMA,�ȴ����ݵĵ���

	
}



/*************************************************************************
  * @brief  USART2��������ȫ����
  * @param  Baudrate �� ������
  * @retval ��
  * @notice �����ô��ڹ���ģʽ������I/O ����Ϊ����I/O�������ô���ʱ��
  *         ������I/O�����ô���֮��ʱI/O���Ż����һ��Ĭ�ϵĵ�ƽ���͵�ƽ���������˴��ڶ��һ������
**************************************************************************/ 
void USART2_Init(uint32_t Baudrate)
{
	/**********USART2 DMA��ʽʱ��ʹ��*************/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1     ,ENABLE);                  //Enable DMA clock 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 ,ENABLE);                  //���ô���USART2��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  ,ENABLE);                  //���ô���GPIO����ʱ��

	/**********USART2 DMA��ʽӲ������*************/
	USART2_NVIC_Config();                                                   //����USART2�ж����ȼ� 
	USART2_Mode_Config(Baudrate);                                           //���ô���USART2����ģ ,����������
	USART2_DMA_Config();                                                    //���ô���USART2 DMA ģʽ
  USART2_GPIO_Config();                                                   //���ô���USART2���ݴ�������I/O��
 
  /************RS485�������ų�ʼ��***************/	
	RS485_ControlPinInit();                                                 //485���߿��ƶ˳�ʼ��
}






/*************************************************************************
  * @brief  �������ݷ��� 
  * @param  Data  �� �跢���������׵�ַ
					  Num   �� �跢�͵�������
  * @retval ��
  * @notice û�����ݷ��ʱ��� ������һ�����ݻ�û�������ʱ������������������
*************************************************************************/
void  USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num)
{
	uint8_t  i ;
	
	while(USART2_SendBusyFlag);                                             //�ж��Ƿ���æµ״̬
	/*************���ͽ���æµ״̬****************/
	USART2_SendBusyFlag  = 1 ;                                              //USART2 DMA ���ڴ���æµ״̬
	RS485_SetSendMode();                                                    //Ӳ������Ϊ����ģʽ
	
	/************���͵��������Ƿ����*************/
	if(Num > USART2_SendSizeMax)                                            //�������ж�
		Num  = USART2_SendSizeMax ;                                           //����̫�� ��������������������������Ҫ
	
	/*******������Ҫ���͵����ݵ����ͻ�����********/
	for( i = 0 ; i < Num ; i++ )                                            //�������ݼ���
		USART2_SendData[i] = Data[i] ;                                        //���跢�͵����ݸ��Ƶ����ͻ�������
	
  /************����DMA���ݷ���******************/
	DMA_Cmd(DMA1_Channel7,DISABLE);                                         //DMA_CNDTR��ҪDMA_CCRx�е�EN=0ʱ����д��
 	DMA_SetCurrDataCounter(DMA1_Channel7,Num);                              //���ô˴�DMA���ݷ��͵���Ŀ
 	DMA_Cmd(DMA1_Channel7,ENABLE);                                          //ʹ��DMA���ݷ���
}




/*************************************************************************
  * @brief  ���ڶ�ȡһ֡���� 
  * @param  Data  ����ȡ���ݱ���λ�� 
  *         Num   ����ȡ����֡���ݵĳ���
  * @retval ��
  * @notice USART2_ReceiveFinishFlag ��ʾ һ֡���ݶ�ȡ��� ��û�д���������� 
*************************************************************************/
void  USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num)
{
	uint8_t  i ;
	
	/*************������յ���������****************/
	*Num = USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6);   //���㴮�ڽ��յ�������

	/*************��ֵ���ؽ��յ�����****************/
	for( i = 0 ; i < *Num ; i++ )                                           //�������ݼ���
		Data[i] = USART2_ReceiveData[i] ;                                     //�����ȡ�����ݸ��Ƶ�ָ����λ��
	
  /*���ý��ձ�־λ ��һ֡���ݴ����꣬û�д����������*/	
	/*��MODBUS�ӻ� ,�����յ�һ�����ݺ󣬱���ö���֡����������Ӧ���ܽ�����һ֡�� �� 
																			���ǰ���һ���ֳ�����ڷ�����MODBUS��Ӧ����ж��� */
//  DMA_SetAndEnableReceiveNum();                                         //ʹ��DMA���ݽ��� ������MODBUSԭ�� ������δ�����ڷ����ж���
	USART2_ReceiveFinishFlag = 0 ;                                          //Ӧ��Ϊһ֡���ݶ�ȡ��� �����Խ�����һ֡���ݵĽ��� ����MODBUS����봦����һ֡���ݲ��ܽ�����һ֡���ݣ�
	                                                                        //�������������˼Ϊ��һ֡���ݶ�ȡ��� ��û�д���������� 
}






/*************************************************************************
  * @brief  �����Ѿ����յ�һ֡���� 
  * @param  ��
  * @retval FlagStatus :���յ�һ֡���ݷ���SET ,���򷵻� RRSET
  * @notice USART2_ReceiveFinishFlag ��ʾ һ֡���ݶ�ȡ��� ��û�д���������� ��
  *         ������һ֡����ʱ��־λ USART2_ReceiveFinishFlag����λ ����ȡ��֡����ʱ��λ������    
*************************************************************************/
FlagStatus USART2_ReceiveOneFrameFinish(void)
{
	/***********�ж��Ƿ���յ�һ֡����************/
	if(USART2_ReceiveFinishFlag == 1 )                                      // �Ѿ����յ�һ֡����                                    
		return SET ;                                                          // ��һ�����ݴ����� ������ SET
	else                                                                    // ��û�н��յ�һ֡���� 
		return RESET ;                                                        // û�д���������� ������ RESET 
}






/*************************************************************************
  * @brief  USART2 �жϷ�����(����DMA���ݽ���ʱ�����ڿ��м��)
  * @param  ��
  * @retval ��
  * @notice �����õ������߿����ж�  �� ֻ��һ���ֽ�ʱ��Ŀ���ʱ�� ������MODBUS֡���ݼ��ʱ�䳤Ϊ3.5�ֽ�ʱ�� ��
  *         ��Ҫ���ж�����򿪶�ʱ�����ж�ʱ �������ʱ�ڼ�û���ٽ��յ����ݾ���Ϊ���յ�һ֡����
*************************************************************************/
extern void TIM6_Open(void);                                              //TIM6��

uint16_t USART2_IdleIRQReceiveDataNum ;                                   //DMA�������ݿ����ж�ʱ���յ���������
uint16_t Time6_IRQReceiveDataNum ;                                        //DMA�������ݿ����ж�ʱ�򿪶�ʱ��TIM6����ʱʱ�䵽ʱ����ʱDMA���յ���������  
void USART2_IRQHandler(void) 
{
	static uint16_t SendDataCount = 0 ;                                     //DMA������ɺ�ԼĴ����ﻹû�з��͵��ֽ������м���  
	if(USART_GetITStatus(USART2,USART_IT_IDLE)!= RESET)                     //�ж��Ƿ�Ϊ���߿����ж�
	{	
		USART_ReceiveData(USART2);                                            //������߿����жϱ�־λ ( ֻ��ͨ���ȶ�USART_SR ,�ٶ�USART_DR�������������) 
    USART2_IdleIRQReceiveDataNum = DMA_GetCurrDataCounter(DMA1_Channel6); //DMA�������ݿ����ж�ʱ��ȡ���ڽ��յ������ ʵ��USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6)���ǽ��յ��������� 	   
		                                                                      //����ֻ��Ҫһ���� TIM6��ʱ�������յ����������Ƿ�һ���󼴿ɣ����յ���������ͬʱ����Ӧʣ���������Ҳ��ͬ ��������ʣ������������棩
		TIM6_Open();                                                          //֡��֡����֮��ʱ�䶨ʱ�򿪣� MODBUS ��                                       	
	}
	else if(USART_GetITStatus(USART2,USART_IT_TC)!= RESET)                  //�ж�һ���ֽ������Ƿ������
	{
		SendDataCount++ ;                                                     //DMA��ʽ������ɵı�־����������ȫ�����͸������裨�� USART��,�������跢�����ݳ�ȥ����Ҫһ��ʱ�� �����ڴ�����˵��
		if( SendDataCount >= 2)                                               //�����������ж�ʱ��ʵ�ʻ��������ֽڵ����ݻ�û�з��ͳ�ȥ �� һ������λ�Ĵ��� ��һ���������ݼĴ����� ��������DMA����ж���򿪴���                       
		{		                                                                  //��������ж� ��Ȼ��Ժ�����յ����ֽ������м��� ��������Ĵ����е������ֽڵ�����ʱ����ʽ����һ֡���ݷ������
			SendDataCount = 0 ;
			/*Ӧ����MODBUS�ӻ� ,�����յ�һ�����ݺ󣬱���ö���֡����������Ӧ���ܽ�����һ֡����*/
			DMA_SetAndEnableReceiveNum();                                         //MODBUSһ֡���ݵ���Ӧ��� �����������һ֡����
			RS485_SetReceiveMode();                                               //Ӳ������Ϊ ����ģʽ	 
	  	USART_ITConfig(USART2, USART_IT_TC   ,DISABLE );                      //��������жϹ�
		}
	}

}



/*************************************************************************
  * @brief  ���ڽ��յ�һ֡���ݺ󣬹ر�DMA���ڽ��� ����DMA������Ӧ���ٴ����ݵĽ���
  * @param  ��
  * @retval ��
  * @notice  �������Ӧ�ö�ʱ��TIM6�ж���
*************************************************************************/
void USART2_TransferOneFrameFinish(void)
{
	/**********�ж�һ֡���������Ƿ����***********/
	 Time6_IRQReceiveDataNum  =  DMA_GetCurrDataCounter(DMA1_Channel6) ;   //DMA�������ݿ����ж�ʱ��ȡ���ڽ��յ������ ʵ��USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6)���ǽ��յ������� ),
	 if(USART2_IdleIRQReceiveDataNum == Time6_IRQReceiveDataNum)           //��DMA�������ݿ����ж�ʱ���յ��������� �� �� �� DMA�������ݿ����ж�ʱ�򿪶�ʱ��TIM6����ʱʱ�䵽ʱ����ʱDMA���յ���������  ����ȱ�ʾ��ʱ����ʱ�ڼ�û���ٽ��յ����� �����ڿ����ж�һ֡���ݽ�����
	 {
			DMA_Cmd(DMA1_Channel6,DISABLE);                                    //һ֡���ݽ������ ���ر�USART DMA ����
		  USART2_ReceiveFinishFlag  = 1 ;                                    //һ֡���ݽ������
	 }	 
}




/*************************************************************************
  * @brief  DMA1_Channel7�жϷ�����(����DMA���ݷ��� ������������ʱ�����ж�
  * @param  ��
  * @retval ��
  * @notice �����յ�һ�����ݺ󣬱���ö���֡����������Ӧ���ܽ�����һ֡���� ��
  *         ���� DMA_SetAndEnableReceiveNum()��������
*************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
	/**********�ж�DMAһ֡�����Ƿ�����**********/
	if(DMA_GetITStatus(DMA1_IT_TC7)!= RESET)                                //�ж��Ƿ�Ϊ��������ж�
  { 
    DMA_ClearITPendingBit(DMA1_IT_TC7);                                   //���DMA�жϱ�־λ 
		DMA_Cmd(DMA1_Channel7,DISABLE);                                       //һ֡���ݷ������ ���ر�DMA���͵����ݴ���
		USART2_SendBusyFlag  = 0 ;                                            //USART2 DMA ���ڲ�����æµ״̬ �����Է�����һ֡����
		USART_ITConfig(USART2, USART_IT_TC   ,ENABLE );                       //��������жϴ�
	}
}



/************************************************************************
  * @brief  �����´�DMA���յ������� �� ʹ��DMA���ݽ���
  * @param  ��
  * @retval ��
  * @notice ÿ��ʹ�����ݵĽ��ն��Ǵ�USART2_ReceiveData��0����ʼ
************************************************************************/
void DMA_SetAndEnableReceiveNum(void)
{
	/**************ʹ��DMA���ݵĽ���**************/
	DMA_Cmd(DMA1_Channel6,DISABLE);                                       //DMA_CNDTR��ҪDMA_CCRx�е�EN=0ʱ����д��
	DMA_SetCurrDataCounter(DMA1_Channel6, USART2_ReceiveSizeMax);         //�����´�DMA���ݽ��յ���Ŀ
	DMA_Cmd(DMA1_Channel6,ENABLE);                                        //ʹ��DMA���ݷ���

}


///************************************************************************
//  * @brief  ����ʹ�� �����ڽ���ʲô���ݾ͸����ڻ�ʲô���ݣ�DMA��ʽ��
//  * @param  ��
//  * @retval ��
//************************************************************************/
//void USART_Debug(void)
//{
//	uint8_t USART2_Dat[100]={0,1,2,3,4,5,6,7,8,9,10,11,1,2,13};
//	uint8_t USART2_Num  ;
//	if(USART2_ReceiveFinishFlag == 1 )                                    //һ֡���ݽ������
//	{
//		USART2_ReadDMAReceiveMulData(USART2_Dat , &USART2_Num );            //��ȡ���յ�һ֡����
//		USART2_WriteDMASendMulData(USART2_Dat , USART2_Num ) ;              //���ͽ��ܵ���һ֡����
//	}
//}


/*********************************************END OF FILE**********************/

