#include "TIME3.h" 

/*******************************TIM3 �û�����****************************/ 
void TIM3_Init(uint16_t TimeUs);                                          //TIM3��ʼ��   

/*******************************TIM3 �ڲ����� ***************************/ 
static void TIM3_MODE_Config(uint16_t TimeUs);                            //TIM3����ģʽ����   



/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 49    TIM_Prescaler--71
 * �ж�����Ϊ = 1/(84MHZ /1680) * 50 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼��� 
**************************************************************************/
static void TIM3_MODE_Config(uint16_t TimeUs)                             //TIM3����ģʽ���ã� ��λ ��us ��
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	TIM_OCInitTypeDef  TIM_OCInitStructure;

		/*********TIM3 ��ʱʱ������************/ 
   	TIM_TimeBaseStructure.TIM_Period            = TimeUs - 1 ;            //�Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Prescaler         = 71;                     //����ϵ�� 72
	
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1 ;		      //���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0 ;                     //ûʹ��
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;     //���ϼ���
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                       //���� 
	
		/* PWM1 Mode configuration: Channel1 */
		TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM1;	      //����ΪPWMģʽ1
		TIM_OCInitStructure.TIM_OutputState         = TIM_OutputState_Enable; //���ʹ��	
		TIM_OCInitStructure.TIM_OutputNState        = TIM_OutputNState_Disable;	//�������ʧ�ܣ��߼���ʱ�����У�
		TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;    //����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ
		TIM_OCInitStructure.TIM_OCNPolarity         = TIM_OCNPolarity_Low;    //����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ
		TIM_OCInitStructure.TIM_Pulse               = 1 ;	                    //��������ֵ�������������������ֵʱ����ƽ��������
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);	                            //ʹ��ͨ��1
	  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);                     //1ͨ��Ԥװ��ʹ��          

    TIM_Cmd(TIM3, ENABLE);                                                //ʹ�ܶ�ʱ��
}






/*************************************************************************
  * @brief  �򿪶�ʱ��TIM3
  * @param  ��
  * @retval ��
  * @notice ��
*************************************************************************/
void TIM3_Init(uint16_t TimeUs)
{
	/***********ʱ��ʹ��***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);                   //ʹ��TIM3��ʱ��

	/**********Ӳ������****************/
	TIM3_MODE_Config(TimeUs);                                               //��ʱ����ʱʱ������
}







/*********************************************END OF FILE**********************/


