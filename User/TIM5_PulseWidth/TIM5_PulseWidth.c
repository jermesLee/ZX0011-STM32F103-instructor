#include "TIM5_PulseWidth.h" 

/*******************************TIM5 ��������****************************/ 
int32_t TIM5_TickCount = 0 ;      
int16_t TIM5_FinishFlag= 0 ;                                             // ��ʱʱ����ɱ�־ 

/*******************************TIM5 �û�����****************************/ 
void TIM5_PluseWidthAdjustInit(void);                                    //TIM5��ʼ��   
void TIM5_SetOutputPluseWidth(uint16_t PluseWidth );	
FlagStatus TIM5_OutputPluseWidthFinish(void);


/*******************************TIM5 �ڲ����� ***************************/ 
static void TIM5_MODE_Config(void);                                       //TIM5����ģʽ���� 
static void TIM5_NVIC_Config(void);                                       //TIM5�ж�����������
  
	
	
	
/*************************************************************************
  * @brief  TIM5 �ж����ȼ�����
  * @param  ��
  * @retval ��
  * @notice TIM5_IRQPreemptionPrio �� TIM5_IRQSubPrio�ڡ�TIM5.h������#define ���к궨��
**************************************************************************/
static void TIM5_NVIC_Config(void)                                        //�ж�����������
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
		/*********TIM5 �ж����ȼ�����************/ 
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn ;	  //ָ���ж���  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM5_IRQPreemptionPrio;  //������ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIM5_IRQSubPrio;	        //���ô����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //�ж�������ʹ��
    NVIC_Init(&NVIC_InitStructure);                                       //����
}






/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 999    TIM_Prescaler--71
 * �ж�����Ϊ = 1MS
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼��� 
**************************************************************************/
static void TIM5_MODE_Config(void)                                        //TIM5����ģʽ���ã� ��λ ��ms ��
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
		/*********TIM5 ��ʱʱ������************/ 
   	TIM_TimeBaseStructure.TIM_Period    = 1000 - 1 ;                       //�Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Prescaler = 71;                              //����ϵ�� 0
	
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		              //���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;                        //ûʹ��
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;             //���ϼ���
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);                       //����
	
  	/**************ʹ���ж�****************/
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);	                                //���жϱ�־ 
    TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                              //ʹ���ж�		 
    TIM_Cmd(TIM5, DISABLE);                                               //��ʹ�ܶ�ʱ�� ������Ҫʹ��ʱ��ʹ�ܶ�ʱ��
}






/*************************************************************************
  * @brief  �򿪶�ʱ��TIM5
  * @param  ��
  * @retval ��
  * @notice ��
*************************************************************************/
void TIM5_PluseWidthAdjustInit(void)
{
	/***********ʱ��ʹ��***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);                   //ʹ��TIME6��ʱ��

	/**********Ӳ������****************/
	TIM5_NVIC_Config();                                                     //��ʱ���жϴ� �������ж����ȼ�
	TIM5_MODE_Config();                                                     //��ʱ����ʱʱ������
}


FlagStatus TIM5_OutputPluseWidthFinish(void)
{
	if( TIM5_FinishFlag ) 
	{
		TIM5_FinishFlag = 0 ;
		return SET ;
	}
	else
	 return  RESET ;
}




void TIM5_SetOutputPluseWidth(uint16_t PluseWidth )   
{
	TIM5_TickCount = PluseWidth ;
	TIM_Cmd(TIM5, DISABLE);                                                 //�ض�ʱ�� �� TIMx->CNTֻ���ڶ�ʱ���ص�������д��
	TIM_SetCounter(TIM5,0);                                                 //��������TIM5->CNT Ϊ 0 ��TIM5 Ϊ�� 0 ���ϼ��� ���¼����� TIM5->ARR
	TIM_Cmd(TIM5, ENABLE);                                                  //����ʱ�� ����ʱ����ʼ���� 
  FET_PluseWidthSignalOutputOpen();                                       //��FET�ܣ���ʼ��� 
}

/*************************************************************************
  * @brief  ��ʱ��TIM5 ISR������
  * @param  ��
  * @retval ��
  * @notice 
*************************************************************************/
void TIM5_IRQHandler(void)                                                //TIM5�жϷ�����
{
	if ( TIM_GetITStatus(TIM5 , TIM_IT_Update) != RESET )                   //����жϱ�־λ
	{
		TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update);                        //���жϱ�־λ	
    if( ( --TIM5_TickCount ) ==0 )		
		{
			FET_PluseWidthSignalOutputClose();                                  //�����ź�ʱ�䵽 ������� 
			TIM_Cmd(TIM5, DISABLE);                                             //�ض�ʱ�� ������һ����Ҫ�������ʱ�ٴ򿪶�ʱ��
		  TIM5_FinishFlag  = 1 ;                                              //��ʱʱ����� 
		}
	}	
}






/*********************************************END OF FILE**********************/


