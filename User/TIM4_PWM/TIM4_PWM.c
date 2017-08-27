#include "TIM4_PWM.h" 

/******************TIM4 ��װ�ؼĴ���ֵ��PWM���Ƶ�ʵĹ�ϵ****************/ 
#define TIM4_PWMCounterPeriod     ( SystemCoreClock / (TIM4_PWMOutputFrequency) )  //��ʱ��4�Ķ�ʱ������


/*******************************TIM4 �û�����****************************/ 
void TIM4_PWM_Init(void);                                                 //TIM4 ���PWM�źų�ʼ���ܺ���                                      
void TIM4_PWM_SetDutyRatio(uint16_t DutyRatio);                           //��ʱ��TIM4������pwm���ռ�ձ����� ����λ ��0.1%



/*******************************TIM4 �ڲ�����****************************/ 
static void TIM4_GPIO_Config(void);                                       //����TIM4�������PWMʱ�õ���I/O
static void TIM4_Mode_Config(void);                                       //����TIM1�����PWM�źŵ�ģʽ�������ڡ����ԡ�ռ�ձ�





 /************************************************************************
  * @brief  ����TIM4�������PWMʱ�õ���I/O
  * @param  ��
  * @retval ��
	* @notice ��
*************************************************************************/
static void TIM4_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;		                    // �����������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                        // I/O�����ٶ�2M/S
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 ;                            // GPIOB Configuration: TIM4 channel 1  as alternate function push-pull 
  GPIO_Init(GPIOB, &GPIO_InitStructure);                                  // ����
}




/**************************************************************************
  * @brief  ����TIM1�����PWM�źŵ�ģʽ�������ڡ����ԡ�ռ�ձ�
  * @param  ��
  * @retval ��
	* @notice TIMxCLK/CK_PSC --> TIMxCNT --> TIMx_ARR --> TIMxCNT ���¼���
  *         TIMx_CCR(��ƽ�����仯)
  *         �ź�����=(TIMx_ARR +1 ) * ʱ������
  *         ռ�ձ�=TIMx_CCR/(TIMx_ARR +1)
  *         ��ʼ��Ĭ������ߵ�ƽ
**************************************************************************/
static void TIM4_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;                         //��ʱ��������ʽ��ʼ������
	TIM_OCInitTypeDef  TIM_OCInitStructure;                                 //��ʱ��PWM�����ʽ�µ�PWM��������
  
	/************ Tim4 ��ʱʱ�����á�**************/		                      //TIM4_PWMCounterPeriod = ( SystemCoreClock / (TIM4_PWMOutputFrequency) - 1 )
	TIM_TimeBaseStructure.TIM_Period        = TIM4_PWMCounterPeriod - 1 ;   //����ʱ����0������TIM4_PWMCounterPeriod����ΪTIM4_PWMCounterPeriod�Σ�Ϊһ����ʱ����
	TIM_TimeBaseStructure.TIM_Prescaler     = 0;	                          //����Ԥ��Ƶ����Ԥ��Ƶ����Ϊ72MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	              //����ʱ�ӷ�Ƶϵ��������Ƶ(�����ò���)
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;           //���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/*** Tim4 PWM1 Mode configuration: Channel1 **/
	TIM_OCInitStructure.TIM_OCMode          = TIM_OCMode_PWM1;	            //����ΪPWMģʽ1
	TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable;       //���ʹ��	
	TIM_OCInitStructure.TIM_OutputNState    = TIM_OutputNState_Disable;   	//�������ʧ�ܣ��߼���ʱ�����У�
	TIM_OCInitStructure.TIM_OCPolarity      = TIM_OCPolarity_High;          //����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ
	TIM_OCInitStructure.TIM_OCNPolarity     = TIM_OCNPolarity_Low;          //����ʱ������ֵС��CCR1_ValʱΪ�͵�ƽ
	TIM_OCInitStructure.TIM_Pulse           = 0;	                          //��������ֵ�������������������ֵʱ����ƽ��������
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);	                              //ʹ��ͨ��1

	/************ TIM4 PWM ʹ�� ******************/
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);                       //1ͨ��Ԥװ��ʹ��          
	TIM_ARRPreloadConfig(TIM4, ENABLE);			                                //ʹ��TIM4���ؼĴ���ARR
	TIM_Cmd(TIM4, ENABLE);                                                  //ʹ�ܶ�ʱ��1	
}




/*************************************************************************
  * @brief  TIM4 ���PWM�źų�ʼ���ܺ���
  * @param  ��
  * @retval ��
  * @notice Ĭ���ϵ�����ߵ�ƽ
***************************************************************************/
void TIM4_PWM_Init(void)                                        
{
	/*****************ʱ��ʹ��********************/
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4 , ENABLE); 	                // ��ʱ��	TIM4 ʱ��ʹ��
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);                  // ��ʱ�� TIM4 GPIO ����ʱ��ʹ��

	/*****************Ӳ������********************/
	TIM4_GPIO_Config();                                                     //I/0�����������
	TIM4_Mode_Config();	                                                    //��ʱ��TIM4����ģʽ����   
}




/*************************************************************************
  * @brief  ��ʱ��TIM4������pwm���ռ�ձ�����
  * @param  DutyRatio  ��ռ�ձ����� ��ռ�ձ��Ѿ��Ŵ�100������25.9%��ռ�ձ�Ӧ�ø�DutyRatioΪ2590
  * @retval �� 
  * @notice ��
**************************************************************************/
	uint16_t  CCR_Data ;
void TIM4_PWM_SetDutyRatio(uint16_t DutyRatio)
{
                                                   //����ʱ�����񡢱ȽϼĴ�������ʱ�洢�� 
	
	/*****����ռ�ձ�������������TIM4 ARR��ֵ*****/	
	if( DutyRatio >= 10000 )                                                //������ռ�ձȴ���100%ʱ�����һֱΪ��
		CCR_Data   = TIM4_PWMCounterPeriod + 10 ;                             //ֻҪ����������ֵ���ֵ���ɣ��˳�������ڸ�����TIM4_PWMCounterPeriod
	else                                                                    //����������ȷ�������ռ�ձ��붨ʱ��TIM4 ARR�Ĵ���ֵ�Ĺ�ϵ
		CCR_Data =DutyRatio * TIM4_PWMCounterPeriod / 10000;                  //���ڸ���ռ�ձ���0%-100%ʱ����ռ�ձ�ת���ɸ�����/�ȽϼĴ�����ֵ 
	TIM4->CCR1 = CCR_Data ;                                                 //������д��Ԥװ�ؼĴ���
}




/****************************************END OF FILE*******************/


