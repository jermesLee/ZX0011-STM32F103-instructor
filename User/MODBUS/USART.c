#include "USART.h"




/*****************************USART2 内存分配******************************/
uint8_t USART2_SendBusyFlag      = 0 ;                                     //串口DMA发送数据忙碌标志位 ，1：正在发送数据 ，0：DMA不处于数据发送 ，此时可以发送下一帧数据
uint8_t USART2_ReceiveFinishFlag = 0 ;                                     //串口接受完一帧数据标志位 ，1 ：表示接受完一帧数据 ，0：表示一帧数据还未接收完成
uint8_t USART2_SendData[USART2_SendSizeMax];                               //串口数据发送缓冲区
uint8_t USART2_ReceiveData[USART2_ReceiveSizeMax];                         //串口接收数据缓冲区


/*****************************USART2 用户函数 *****************************/ 
void USART2_Init(uint32_t Baudrate);                                       //USART2串口配置初始化 ,Baudrate :波特率
void USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num);              //串口发送数据     
void USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num);           //串口读取数据
void DMA_SetAndEnableReceiveNum(void);                                     //使能串口继续接收数据 
FlagStatus USART2_ReceiveOneFrameFinish(void);                             //串口是否接收到一帧数据 
//void USART_Debug(void);                                                  //调试使用 ，串口接收什么数据就给串口回数目数据（DMA方式）


/*****************************USART2 内部函数 *****************************/ 
static void USART2_GPIO_Config(void);                                      //配置串口USART2数据传送引脚I/O口                 
static void USART2_Mode_Config(uint32_t Baudrate);                         //配置串口USART2工作模式 
static void USART2_NVIC_Config(void);                                      //中断优先级设置
static void USART2_DMA_Config(void);                                       //USART2 DMA 配置






/*************************************************************************                
  * @brief  配置串口USART2数据传送引脚I/O口
  * @param  无
  * @retval 无 
  * @notice TXD推挽输出 ，RXD悬空输入 
*************************************************************************/
static void USART2_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		 
	  /* Configure USART2 Tx (PA.02) as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;                           //TXD :PA2
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                      //TXD推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //I/O引脚速度50M/s
		GPIO_Init(GPIOA, &GPIO_InitStructure);                                //配置
	
	  /* Configure USART2 Rx (PA.03) as input floating */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	                            //RXD :PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                 //悬空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);                                //配置

}





/*************************************************************************
  * @brief  配置串口USART2工作模式
  * @param  Baudrate ： 波特率 
  * @retval 无 
  * @notice 无
*************************************************************************/
static void USART2_Mode_Config(uint32_t Baudrate)
{
	USART_InitTypeDef USART_InitStructure;

	/* USART2 mode config */	
	USART_InitStructure.USART_BaudRate   = Baudrate ;                       //串口波特率：USART2_BAUDRATE
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;             //帧数据长度：8位
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;                //停止位位数：1位
	USART_InitStructure.USART_Parity     = USART_Parity_No ;                //奇偶校验 ：无校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //硬件流
	USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;   //接收，发送使能
	USART_Init(USART2, &USART_InitStructure);                               //配置
	USART_ITConfig(USART2, USART_IT_IDLE , ENABLE );                        //接收总线空闲中断使能
	USART_ITConfig(USART2, USART_IT_TC   , DISABLE );                       //发送完成中断打开
	USART_Cmd(USART2, ENABLE);                                              //串口USART2使能 
}





/*************************************************************************
  * @brief  配置串口USART2中断优先级
  * @param  无
  * @retval 无 
  * @notice 优先级 在 #include "USART2.h" 中用 define 宏定义
*************************************************************************/
static void USART2_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
		
	/* USART2_RX :USART2 RX ILDE configuration */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 	                    // 指定 USART2 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART2_IRQPreemptionPrio ;//抢占式优先级设置
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART2_IRQSubPrio;      //次优先级设置
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //中断使能
	NVIC_Init(&NVIC_InitStructure);                                         //配置寄存器

	/* USART2_TX :DMA1 channel7 configuration */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;                // 指定 DMA1_Channel7_IRQn 中断 （串口USART2发送完成中断）
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Channel7_IRQPreemptionPrio ;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_Channel7_IRQSubPrio;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //中断使能
  NVIC_Init(&NVIC_InitStructure);                                         //配置寄存器
	

} 



/*************************************************************************
  * @brief  配置串口USART2的DMA模式 
  * @param  无
  * @retval 无 
  * @notice 无
*************************************************************************/
static void USART2_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	/* USART2_TX :DMA1 channel7 configuration */
	DMA_DeInit(DMA1_Channel7);	                                            //DMA寄存器缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR ;      //DMA源地址   : 串口1的数据发送地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_SendData;	      //DMA目的地址 ：内存地址
	DMA_InitStructure.DMA_DIR            = DMA_DIR_PeripheralDST ;          //方向：从内存到外设
	DMA_InitStructure.DMA_BufferSize     = USART2_SendSizeMax;              //DMA传输的数据量
	DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;	      //外设地址固定
	DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;  		      //内存地址加1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte ;//字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte ;        //字节
	DMA_InitStructure.DMA_Mode           = DMA_Mode_Normal;					        //正常发送
	DMA_InitStructure.DMA_Priority       = DMA_Priority_High;               //DMA优先级
	DMA_InitStructure.DMA_M2M            = DMA_M2M_Disable;                 //内存到内存数据传输不使能                      
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);                            //配置

	DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);                           //DMA1 Channel7 数据传输完成中断使能
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);                          //USART2 发送DMA使能                
	DMA_Cmd(DMA1_Channel7, DISABLE);                                        //不使能DMA,在数据需要发送时使能DMA

	/* USART2_RX :DMA1 channel6 configuration */
	DMA_DeInit(DMA1_Channel6);	                                            //DMA寄存器缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR ;      //DMA源地址   : 串口1的数据接收地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_ReceiveData;    //DMA目的地址 ：内存地址
	DMA_InitStructure.DMA_DIR            = DMA_DIR_PeripheralSRC ;          //方向：从外设到内存
	DMA_InitStructure.DMA_BufferSize     = USART2_ReceiveSizeMax;           //DMA传输的数据量
	DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;	      //外设地址固定
	DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;  		      //内存地址加1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte ;//字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte ;        //字节
	DMA_InitStructure.DMA_Mode           = DMA_Mode_Circular;					      //正常接收
	DMA_InitStructure.DMA_Priority       = DMA_Priority_High;               //DMA优先级
	DMA_InitStructure.DMA_M2M            = DMA_M2M_Disable;                 //内存到内存数据传输不使能                      
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);                            //配置
	
	DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,DISABLE);                          //DMA1 Channel6 数据传输完成中断关闭
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);                          //使能 USART2 数据接收DMA使能  
	DMA_Cmd(DMA1_Channel6, ENABLE);                                         //使能接收DMA,等待数据的到来

	
}



/*************************************************************************
  * @brief  USART2串口配置全过程
  * @param  Baudrate ： 波特率
  * @retval 无
  * @notice 先配置串口工作模式再配置I/O ，因为配置I/O口再配置串口时，
  *         在配置I/O与配置串口之间时I/O引脚会输出一个默认的电平：低电平，因此造成了串口多出一个数据
**************************************************************************/ 
void USART2_Init(uint32_t Baudrate)
{
	/**********USART2 DMA方式时钟使能*************/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1     ,ENABLE);                  //Enable DMA clock 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 ,ENABLE);                  //配置串口USART2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  ,ENABLE);                  //配置串口GPIO引脚时钟

	/**********USART2 DMA方式硬件配置*************/
	USART2_NVIC_Config();                                                   //配置USART2中断优先级 
	USART2_Mode_Config(Baudrate);                                           //配置串口USART2工作模 ,波特率设置
	USART2_DMA_Config();                                                    //配置串口USART2 DMA 模式
  USART2_GPIO_Config();                                                   //配置串口USART2数据传送引脚I/O口
 
  /************RS485控制引脚初始化***************/	
	RS485_ControlPinInit();                                                 //485总线控制端初始化
}






/*************************************************************************
  * @brief  串口数据发送 
  * @param  Data  ： 需发送数据量首地址
					  Num   ： 需发送的数据量
  * @retval 无
  * @notice 没加数据访问保护 ，在上一阵数据还没发送完成时调用这个函数将会出错
*************************************************************************/
void  USART2_WriteDMASendMulData(uint8_t *Data , uint8_t Num)
{
	uint8_t  i ;
	
	while(USART2_SendBusyFlag);                                             //判断是否处于忙碌状态
	/*************发送进入忙碌状态****************/
	USART2_SendBusyFlag  = 1 ;                                              //USART2 DMA 现在处于忙碌状态
	RS485_SetSendMode();                                                    //硬件配置为发送模式
	
	/************发送的数据量是否溢出*************/
	if(Num > USART2_SendSizeMax)                                            //数据量判断
		Num  = USART2_SendSizeMax ;                                           //数据太多 ，多出最大数据量的数据舍弃不要
	
	/*******复制需要发送的数据到发送缓冲区********/
	for( i = 0 ; i < Num ; i++ )                                            //复制数据计数
		USART2_SendData[i] = Data[i] ;                                        //将需发送的数据复制到发送缓冲区中
	
  /************触发DMA数据发送******************/
	DMA_Cmd(DMA1_Channel7,DISABLE);                                         //DMA_CNDTR需要DMA_CCRx中的EN=0时才能写入
 	DMA_SetCurrDataCounter(DMA1_Channel7,Num);                              //设置此次DMA数据发送的数目
 	DMA_Cmd(DMA1_Channel7,ENABLE);                                          //使能DMA数据发送
}




/*************************************************************************
  * @brief  串口读取一帧数据 
  * @param  Data  ：读取数据保存位置 
  *         Num   ：读取到这帧数据的长度
  * @retval 无
  * @notice USART2_ReceiveFinishFlag 表示 一帧数据读取完成 ，没有待处理的数据 
*************************************************************************/
void  USART2_ReadDMAReceiveMulData(uint8_t *Data , uint8_t *Num)
{
	uint8_t  i ;
	
	/*************计算接收到的数据量****************/
	*Num = USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6);   //计算串口接收的数据量

	/*************幅值返回接收的数据****************/
	for( i = 0 ; i < *Num ; i++ )                                           //复制数据计数
		Data[i] = USART2_ReceiveData[i] ;                                     //将需读取的数据复制到指定的位置
	
  /*设置接收标志位 ，一帧数据处理完，没有待处理的数据*/	
	/*在MODBUS从机 ,当接收到一阵数据后，必须得对这帧数据做出回应才能接收下一帧数 ， 
																			于是把这一部分程序放在发送完MODBUS回应后的中断里 */
//  DMA_SetAndEnableReceiveNum();                                         //使能DMA数据接收 ，由于MODBUS原因 ，把这段代码放在发送中断了
	USART2_ReceiveFinishFlag = 0 ;                                          //应该为一帧数据读取完成 ，可以进行下一帧数据的接收 ，在MODBUS里必须处理完一帧数据才能接受下一帧数据，
	                                                                        //所以在这里的意思为：一帧数据读取完成 ，没有待处理的数据 
}






/*************************************************************************
  * @brief  串口已经接收到一帧数据 
  * @param  无
  * @retval FlagStatus :接收到一帧数据返回SET ,否则返回 RRSET
  * @notice USART2_ReceiveFinishFlag 表示 一帧数据读取完成 ，没有待处理的数据 ；
  *         接收完一帧数据时标志位 USART2_ReceiveFinishFlag被置位 ，读取这帧数据时该位被清零    
*************************************************************************/
FlagStatus USART2_ReceiveOneFrameFinish(void)
{
	/***********判断是否接收到一帧数据************/
	if(USART2_ReceiveFinishFlag == 1 )                                      // 已经接收到一帧数据                                    
		return SET ;                                                          // 有一阵数据待处理 ，返回 SET
	else                                                                    // 还没有接收到一帧数据 
		return RESET ;                                                        // 没有待处理的数据 ，返回 RESET 
}






/*************************************************************************
  * @brief  USART2 中断服务函数(用于DMA数据接收时，串口空闲检测)
  * @param  无
  * @retval 无
  * @notice 这里用的是总线空闲中断  ， 只用一个字节时间的空闲时间 ，对于MODBUS帧数据间的时间长为3.5字节时间 ，
  *         需要在中断里面打开定时器进行定时 ，如果定时期间没有再接收到数据就认为接收到一帧数据
*************************************************************************/
extern void TIM6_Open(void);                                              //TIM6打开

uint16_t USART2_IdleIRQReceiveDataNum ;                                   //DMA接收数据空闲中断时接收到的数据量
uint16_t Time6_IRQReceiveDataNum ;                                        //DMA接收数据空闲中断时打开定时器TIM6，定时时间到时，此时DMA接收到的数据量  
void USART2_IRQHandler(void) 
{
	static uint16_t SendDataCount = 0 ;                                     //DMA发送完成后对寄存器里还没有发送的字节数进行计数  
	if(USART_GetITStatus(USART2,USART_IT_IDLE)!= RESET)                     //判断是否为总线空闲中断
	{	
		USART_ReceiveData(USART2);                                            //清除总线空闲中断标志位 ( 只能通过先读USART_SR ,再读USART_DR软件序列来清零) 
    USART2_IdleIRQReceiveDataNum = DMA_GetCurrDataCounter(DMA1_Channel6); //DMA接收数据空闲中断时读取串口接收的数据里（ 实际USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6)才是接收的数据量， 	   
		                                                                      //这里只需要一个与 TIM6定时到达后接收到的数据量是否一样大即可，接收到的数据相同时，相应剩余的数据量也相同 ，这里用剩余的数据量代替）
		TIM6_Open();                                                          //帧与帧数据之间时间定时打开（ MODBUS ）                                       	
	}
	else if(USART_GetITStatus(USART2,USART_IT_TC)!= RESET)                  //判断一个字节数据是否发送完成
	{
		SendDataCount++ ;                                                     //DMA方式发送完成的标志是所有数据全部传送给了外设（即 USART）,由于外设发送数据出去还需要一段时间 ，对于串口来说，
		if( SendDataCount >= 2)                                               //当发生发送中断时，实际还有两个字节的数据还没有发送出去 （ 一个在移位寄存器 ，一个·在数据寄存器） ，所以在DMA完成中断里打开串口                       
		{		                                                                  //发送完成中断 ，然后对后面接收到的字节数进行计数 ，发送完寄存器中的两个字节的数据时才正式代表一帧数据发送完成
			SendDataCount = 0 ;
			/*应用于MODBUS从机 ,当接收到一阵数据后，必须得对这帧数据做出回应才能接收下一帧数据*/
			DMA_SetAndEnableReceiveNum();                                         //MODBUS一帧数据的响应完成 ，允许接受下一帧数据
			RS485_SetReceiveMode();                                               //硬件配置为 接收模式	 
	  	USART_ITConfig(USART2, USART_IT_TC   ,DISABLE );                      //发送完成中断关
		}
	}

}



/*************************************************************************
  * @brief  串口接收到一帧数据后，关闭DMA串口接收 ，在DMA发送响应后再打开数据的接收
  * @param  无
  * @retval 无
  * @notice  这个函数应用定时器TIM6中断中
*************************************************************************/
void USART2_TransferOneFrameFinish(void)
{
	/**********判断一帧数据数据是否结束***********/
	 Time6_IRQReceiveDataNum  =  DMA_GetCurrDataCounter(DMA1_Channel6) ;   //DMA接收数据空闲中断时读取串口接收的数据里（ 实际USART2_ReceiveSizeMax - DMA_GetCurrDataCounter(DMA1_Channel6)才是接收的数据量 ),
	 if(USART2_IdleIRQReceiveDataNum == Time6_IRQReceiveDataNum)           //”DMA接收数据空闲中断时接收到的数据量 “ 与 “ DMA接收数据空闲中断时打开定时器TIM6，定时时间到时，此时DMA接收到的数据量  ”相等表示定时器定时期间没有再接收到数据 ，现在可以判断一帧数据接受完
	 {
			DMA_Cmd(DMA1_Channel6,DISABLE);                                    //一帧数据接收完成 ，关闭USART DMA 接收
		  USART2_ReceiveFinishFlag  = 1 ;                                    //一帧数据接受完成
	 }	 
}




/*************************************************************************
  * @brief  DMA1_Channel7中断服务函数(用于DMA数据发送 ，发送完数据时产生中断
  * @param  无
  * @retval 无
  * @notice 当接收到一阵数据后，必须得对这帧数据做出回应才能接收下一帧数据 ，
  *         所以 DMA_SetAndEnableReceiveNum()放在这里
*************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
	/**********判断DMA一帧数据是否发送完**********/
	if(DMA_GetITStatus(DMA1_IT_TC7)!= RESET)                                //判断是否为发送完成中断
  { 
    DMA_ClearITPendingBit(DMA1_IT_TC7);                                   //清除DMA中断标志位 
		DMA_Cmd(DMA1_Channel7,DISABLE);                                       //一帧数据发送完成 ，关闭DMA发送的数据传送
		USART2_SendBusyFlag  = 0 ;                                            //USART2 DMA 现在不处于忙碌状态 ，可以发送下一帧数据
		USART_ITConfig(USART2, USART_IT_TC   ,ENABLE );                       //发送完成中断打开
	}
}



/************************************************************************
  * @brief  设置下次DMA接收的数据量 和 使能DMA数据接收
  * @param  无
  * @retval 无
  * @notice 每次使能数据的接收都是从USART2_ReceiveData【0】开始
************************************************************************/
void DMA_SetAndEnableReceiveNum(void)
{
	/**************使能DMA数据的接收**************/
	DMA_Cmd(DMA1_Channel6,DISABLE);                                       //DMA_CNDTR需要DMA_CCRx中的EN=0时才能写入
	DMA_SetCurrDataCounter(DMA1_Channel6, USART2_ReceiveSizeMax);         //设置下次DMA数据接收的数目
	DMA_Cmd(DMA1_Channel6,ENABLE);                                        //使能DMA数据发送

}


///************************************************************************
//  * @brief  调试使用 ，串口接收什么数据就给串口回什么数据（DMA方式）
//  * @param  无
//  * @retval 无
//************************************************************************/
//void USART_Debug(void)
//{
//	uint8_t USART2_Dat[100]={0,1,2,3,4,5,6,7,8,9,10,11,1,2,13};
//	uint8_t USART2_Num  ;
//	if(USART2_ReceiveFinishFlag == 1 )                                    //一帧数据接受完成
//	{
//		USART2_ReadDMAReceiveMulData(USART2_Dat , &USART2_Num );            //读取接收的一帧数据
//		USART2_WriteDMASendMulData(USART2_Dat , USART2_Num ) ;              //发送接受到的一帧数据
//	}
//}


/*********************************************END OF FILE**********************/

