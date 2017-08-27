#include "ADC3.h" 

/******************************����ADC3����ֵ���ڴ���*********************/
static uint16_t ADC3_ConvertedValue[ ADC3_MemoryNum ];                    //ADC3���ݼĴ���ͨ��DMAת�Ƶ�SRAM�е�Ŀ�ĵ�ַ

/*******************************ADC3�������û�����************************/
void ADC3_Init(uint16_t SampleFrequency);                                 //ADC3��ʼ��
uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence );               //��ȡ������ƽ��ֵ

/********************************ADC3�������ڲ�**************************/
static void ADC3_GPIO_Config(void);                                       //����ADC3��GPIO����
static void ADC3_Mode_Config(void);                                       //����ADC3�Ĺ���ģʽ �� DMA����



/*************************************************************************
  * @brief  ����ADC3��GPIO����
  * @param  ��
  * @retval ��
	* @notice ��
*************************************************************************/
static void ADC3_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/***************ADC3 GPIO ����****************/	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;  // Configure PC0,1,2,3  as analog input  
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;                          // ����ģʽ �� ģ������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;                      // �����ٶ� �� 10Mhz   
	GPIO_Init(GPIOC, &GPIO_InitStructure);			                          	// ����
}






/************************************************************************
  * @brief  ����ADC3�Ĺ���ģʽ �� DMA����
  * @param  ��
  * @retval ��
	* @notice ��
*************************************************************************/
static void ADC3_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/***************DMA2 Channel5 ����*************/
	DMA_DeInit(DMA2_Channel5);	                                                 //DMA�Ĵ���ȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3 -> DR ; 			     //DMAԴ��ַ   :ADC���ݵ�ַ 
	DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)&ADC3_ConvertedValue;	 //DMAĿ�ĵ�ַ ���ڴ��ַ
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;	           //���򣺴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize         = ADC3_MemoryNum ;                  //DMA�����������
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;	       //�����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;  			  	 //�ڴ��ַ��1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	 //����
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;      //����
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;								 //ѭ������
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;                //DMA���ȼ�
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;                  //�ڴ浽�ڴ����ݴ��䲻ʹ��                      
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);                                 //����
	DMA_Cmd(DMA2_Channel5, ENABLE);                                              // Enable DMA channel1 

	/******************ADC3��������***************/	
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;		         //����ADCģʽ
	ADC_InitStructure.ADC_ScanConvMode       = ENABLE ; 	 		                   //ʹ��ɨ��ģʽ��ɨ��ģʽ���ڶ�ͨ���ɼ�
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;			                     //�ر�����ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_T3_CC1 ;	   //ʹ�ö�ʱ��3���´���
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right; 	           //�ɼ������Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel       = ADC3_ChannelScanNum ;	 					 //Ҫת����ͨ����ĿΪ 3
	ADC_Init(ADC3, &ADC_InitStructure);                                          //����	
	
	/****************ADC3����ʱ������*************/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 	                                         //����ADCʱ�ӣ�ΪPCLK2��8��Ƶ����9MHz
	
	/*****ADC3ɨ��ģʽɨ��ͨ���ʹ�������*********/	
	ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 4, ADC_SampleTime_28Cycles5); //PC0����,����ʱ���������迹�й� ��28.5���ڵĲ����������迹���Ϊ41K
	ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 3, ADC_SampleTime_28Cycles5); //PC1����,����ʱ���������迹�й� ��28.5���ڵĲ����������迹���Ϊ41K
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 2, ADC_SampleTime_28Cycles5); //PC2����
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1, ADC_SampleTime_28Cycles5); //PC3����

  /***********ADC3ģʽ��DMAʹ�� ***************/	 
  ADC_DiscModeChannelCountConfig(ADC3, ADC3_ChannelScanNum);                   //���ģʽ ��һ�δ���ת��3·ͨ�������� 
  ADC_DiscModeCmd(ADC3, ENABLE);                                               //ʹ�ܼ��ģʽ 
	ADC_Cmd(ADC3, ENABLE);	                                                     // Enable ADC3 
	ADC_DMACmd(ADC3, ENABLE);                                                    // Enable ADC3 DMA 

  /**************ADC3��λУ׼ *****************/	 
  ADC_ResetCalibration(ADC3);	                                                 //��λУ׼�Ĵ���   
	while(ADC_GetResetCalibrationStatus(ADC3));                    	             //�ȴ�У׼�Ĵ�����λ��� 
	ADC_StartCalibration(ADC3);	                                                 //ADCУ׼ 
	while(ADC_GetCalibrationStatus(ADC3)){};	                                   //�ȴ�У׼���
	 
	/************ʹ��ADC3�ⲿ����****************/	 
  ADC_ExternalTrigConvCmd(ADC3, ENABLE);                                       //ʹ���ⲿ����ת�� 

		
}



/**
  * @brief  ADC3��ʼ��
  * @param  ��
  * @retval ��
  */
void ADC3_Init(uint16_t SampleFrequency)
{
	/**************ADC3 DMA��ʽʱ������*************/
	RCC_AHBPeriphClockCmd(  RCC_AHBPeriph_DMA2  , ENABLE);	                // Enable DMA clock 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3 , ENABLE);	                // Enable ADC3 clock 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);	                // Enable GPIOC clock 

	/****************ADC3 Ӳ������ *****************/
	ADC3_GPIO_Config();                                                     // ����ADC3��GPIO����                                              
	ADC3_Mode_Config();                                                     // ����ADC3�Ĺ���ģʽ �� DMA���� 
	
	/**********ADC3 �ⲿ������ʱ����ʼ��************/
	ADC3_ExternTriggerInit(1000000 / SampleFrequency ) ;                    //�����ⲿ����Ƶ�� ������Ϊ ��ʱ�䣨 ��λ ��us ��
}





/*************************************************************************
  * @brief  ��ȡһ�����ݵ�ƽ��ֵ
  * @param  SampleChannel ����ȡת����ͨ���� ��ͨ������Χ �� 1 - ADC3_ChannelScanNum 
  * @retval ͨ�����ɼ��������ݵ�ƽ��ֵ 
 	* @notice ��ͨ�����������ʱ ��Ĭ�Ϸ���ͨ��1��ֵ
  *         SampleSequence Ϊ ������˳�� ��������ͨ���� ���� 1 �������Ĳ�һ��Ϊ��һ��ͨ��  
*************************************************************************/
uint16_t ADC3_SampleAverageValue(uint16_t SampleSequence )
{
	uint16_t StartIndex , i ;
	uint32_t SampleAverageValue = 0 ;
	
	/*************ȷ��ͨ�����Ŀ�ʼ������***************/
	if( ( SampleSequence > ADC3_ChannelScanNum )||( SampleSequence == 0 ) )
		StartIndex = 0 ;                                                       // ����ͨ��������ʱ ��Ĭ��ʹ��ͨ��1�Ŀ�ʼ������
	else
		StartIndex = SampleSequence - 1 ;                                      // ����ͨ�����ݵĿ�ʼ������ = ������˳��  - 1

	/********************����ƽ��ֵ*******************/
	for( i = StartIndex ; i < ADC3_MemoryNum ; i = i + ADC3_ChannelScanNum ) // �ڴ沽��Ϊ ADC3_ChannelScanNum = 3           
		SampleAverageValue += ADC3_ConvertedValue[ i ] ;                       // ���
	SampleAverageValue  = SampleAverageValue / ADC3_OneChannelSampleNum ;    // ��ƽ�� ADC3_OneChannelSampleNum = 10
	
	return (uint16_t)SampleAverageValue ;                                    // ����ƽ��ֵ 
}




/***************************************END OF FILE**********************/


