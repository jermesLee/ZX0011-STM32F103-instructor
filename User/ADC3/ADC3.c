#include "ADC3.h" 

/******************************分配ADC3采样值的内存区*********************/
static uint16_t ADC3_ConvertedValue[ ADC3_MemoryNum ];                    //ADC3数据寄存器通过DMA转移到SRAM中的目的地址

/*******************************ADC3采样的用户函数************************/
void ADC3_Init(uint16_t SampleFrequency);                                 //ADC3初始化
uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence );               //读取采样的平均值

/********************************ADC3采样的内部**************************/
static void ADC3_GPIO_Config(void);                                       //配置ADC3的GPIO工作
static void ADC3_Mode_Config(void);                                       //配置ADC3的工作模式 和 DMA传输



/*************************************************************************
  * @brief  配置ADC3的GPIO工作
  * @param  无
  * @retval 无
	* @notice 无
*************************************************************************/
static void ADC3_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/***************ADC3 GPIO 配置****************/	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;  // Configure PC0,1,2,3  as analog input  
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;                          // 引脚模式 ： 模拟输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;                      // 引脚速度 ： 10Mhz   
	GPIO_Init(GPIOC, &GPIO_InitStructure);			                          	// 配置
}






/************************************************************************
  * @brief  配置ADC3的工作模式 和 DMA传输
  * @param  无
  * @retval 无
	* @notice 无
*************************************************************************/
static void ADC3_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/***************DMA2 Channel5 配置*************/
	DMA_DeInit(DMA2_Channel5);	                                                 //DMA寄存器缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3 -> DR ; 			     //DMA源地址   :ADC数据地址 
	DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)&ADC3_ConvertedValue;	 //DMA目的地址 ：内存地址
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;	           //方向：从外设到内存
	DMA_InitStructure.DMA_BufferSize         = ADC3_MemoryNum ;                  //DMA传输的数据量
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;	       //外设地址固定
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;  			  	 //内存地址加1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	 //半字
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;      //半字
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;								 //循环传输
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;                //DMA优先级
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;                  //内存到内存数据传输不使能                      
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);                                 //配置
	DMA_Cmd(DMA2_Channel5, ENABLE);                                              // Enable DMA channel1 

	/******************ADC3采样配置***************/	
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;		         //独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode       = ENABLE ; 	 		                   //使能扫描模式，扫描模式用于多通道采集
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;			                     //关闭连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_T3_CC1 ;	   //使用定时器3更新触发
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right; 	           //采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel       = ADC3_ChannelScanNum ;	 					 //要转换的通道数目为 3
	ADC_Init(ADC3, &ADC_InitStructure);                                          //配置	
	
	/****************ADC3采样时钟配置*************/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 	                                         //配置ADC时钟，为PCLK2的8分频，即9MHz
	
	/*****ADC3扫描模式扫描通道和次序配置*********/	
	ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 4, ADC_SampleTime_28Cycles5); //PC0引脚,采样时间与输入阻抗有关 ，28.5周期的采样的输入阻抗最大为41K
	ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 3, ADC_SampleTime_28Cycles5); //PC1引脚,采样时间与输入阻抗有关 ，28.5周期的采样的输入阻抗最大为41K
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 2, ADC_SampleTime_28Cycles5); //PC2引脚
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1, ADC_SampleTime_28Cycles5); //PC3引脚

  /***********ADC3模式和DMA使能 ***************/	 
  ADC_DiscModeChannelCountConfig(ADC3, ADC3_ChannelScanNum);                   //间断模式 ，一次触发转换3路通道的数据 
  ADC_DiscModeCmd(ADC3, ENABLE);                                               //使能间断模式 
	ADC_Cmd(ADC3, ENABLE);	                                                     // Enable ADC3 
	ADC_DMACmd(ADC3, ENABLE);                                                    // Enable ADC3 DMA 

  /**************ADC3复位校准 *****************/	 
  ADC_ResetCalibration(ADC3);	                                                 //复位校准寄存器   
	while(ADC_GetResetCalibrationStatus(ADC3));                    	             //等待校准寄存器复位完成 
	ADC_StartCalibration(ADC3);	                                                 //ADC校准 
	while(ADC_GetCalibrationStatus(ADC3)){};	                                   //等待校准完成
	 
	/************使能ADC3外部触发****************/	 
  ADC_ExternalTrigConvCmd(ADC3, ENABLE);                                       //使能外部触发转换 

		
}



/**
  * @brief  ADC3初始化
  * @param  无
  * @retval 无
  */
void ADC3_Init(uint16_t SampleFrequency)
{
	/**************ADC3 DMA方式时钟配置*************/
	RCC_AHBPeriphClockCmd(  RCC_AHBPeriph_DMA2  , ENABLE);	                // Enable DMA clock 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3 , ENABLE);	                // Enable ADC3 clock 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);	                // Enable GPIOC clock 

	/****************ADC3 硬件配置 *****************/
	ADC3_GPIO_Config();                                                     // 配置ADC3的GPIO工作                                              
	ADC3_Mode_Config();                                                     // 配置ADC3的工作模式 和 DMA传输 
	
	/**********ADC3 外部触发定时器初始化************/
	ADC3_ExternTriggerInit(1000000 / SampleFrequency ) ;                    //配置外部触发频率 ，参数为 ：时间（ 单位 ：us ）
}





/*************************************************************************
  * @brief  求取一组数据的平均值
  * @param  SampleChannel ：读取转换的通道数 ，通道数范围 ： 1 - ADC3_ChannelScanNum 
  * @retval 通道数采集到的数据的平均值 
 	* @notice 当通道数输入错误时 ，默认返回通道1的值
  *         SampleSequence 为 采样的顺序 ，而不是通道号 ；第 1 个采样的不一定为第一个通道  
*************************************************************************/
uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence )
{
	uint16_t StartIndex , i ;
	uint32_t SampleAverageValue = 0 ;
	
	/*************确定通道数的开始索引号***************/
	if( ( SampleSequence > ADC3_ChannelScanNum )||( SampleSequence == 0 ) )
		StartIndex = 0 ;                                                       // 输入通道数错误时 ，默认使用通道1的开始索引号
	else
		StartIndex = SampleSequence - 1 ;                                      // 输入通道数据的开始索引号 = 采样的顺序  - 1

	/********************计算平均值*******************/
	for( i = StartIndex ; i < ADC3_MemoryNum ; i = i + ADC3_ChannelScanNum ) // 内存步进为 ADC3_ChannelScanNum = 3           
		SampleAverageValue += ADC3_ConvertedValue[ i ] ;                       // 求和
	SampleAverageValue  = SampleAverageValue / ADC3_OneChannelSampleNum ;    // 求平均 ADC3_OneChannelSampleNum = 10
	
	return (uint16_t)SampleAverageValue ;                                    // 返回平均值 
}




/***************************************END OF FILE**********************/


