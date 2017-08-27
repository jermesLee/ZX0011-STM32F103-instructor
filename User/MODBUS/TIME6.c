#include "TIME6.h" 

/**************************TIM6 ��ʱMS�����û�����***********************/ 
void TIM6_Init(uint8_t MsTime);                                           //TIME6��ʼ��   , MsTime �� ��ʱʱ�䣨 ��λ�� ms�� ,z��� 120ms
void TIM6_Open(void);                                                     //TIM6��
void TIM6_Close(void);                                                    //TIM6�ر�
 
 
/**************************TIM6 ��ʱMS�����ڲ�����***********************/ 
static void TIM6_MODE_Config(uint8_t MsTime);                             //TIM6����ģʽ���� ��MsTime �� ��ʱʱ�䣨 ��λ�� ms��
static void TIM6_NVIC_Config(void);                                       //TIM6�ж�����������
	
	
	
	
	
/*************************************************************************
  * @brief  TIM6 �ж����ȼ�����
  * @param  ��
  * @retval ��
  * @notice TIME6_IRQPreemptionPrio �� TIME6_IRQSubPrio�ڡ�TIM6.h������#define ���к궨��
**************************************************************************/
static void TIM6_NVIC_Config(void)                                        //�ж�����������
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
	  /**********TIM6��ʱ �ж����ȼ�����**********/
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM6_IRQn;	    //ָ���ж���  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME6_IRQPreemptionPrio;  //������ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIME6_IRQSubPrio;	       //���ô����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //�ж�������ʹ��
    NVIC_Init(&NVIC_InitStructure);                                       //����
}




/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 499   TIM_Prescaler--143
 * �ж�����Ϊ = 1/(72MHZ /144) * 500 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼��� 
**************************************************************************/
static void TIM6_MODE_Config(uint8_t MsTime )                             //TIM6����ģʽ���ã� ��λ ��ms ��
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	 
	  /*************TIM6��ʱʱ������************/  
	  TIM_TimeBaseStructure.TIM_Period            = MsTime * 500 - 1;       //�Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Prescaler         = 143;                    //����ϵ�� 144
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;		        //���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                      //ûʹ��
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;     //���ϼ���
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);                       //����
	
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);	                                //���жϱ�־ 
    TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);                              //ʹ���ж�		 
    TIM_Cmd(TIM6, DISABLE);                                               //��ʹ�ܶ�ʱ������Ҫʹ��ʱʹ�ܶ�ʱ��	                       																	  
}


/*************************************************************************
  * @brief  �򿪶�ʱ��TIM6
  * @param  MsTime ��  ��ʱʱ�� ����λ ��ms
  * @retval ��
  * @notice MsTime ���Ϊ 120ms ���ڴ˺��������޷�
*************************************************************************/
void TIM6_Init(uint8_t MsTime)
{
	  /********��ʱʱ���޷� �����Ϊ120ms*********/
    if( MsTime > 120 ) MsTime = 120  ;                                    //�޷� �������ʱʱ��Ϊ �� 120ms	 
	
	  /**********��ʱ�� TIM6ʱ��ʹ��**************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 , ENABLE);                 //ʹ��TIME6��ʱ�� 
	
	  /**********��ʱ�� TIM6Ӳ������**************/	  
		TIM6_NVIC_Config();                                                   //��ʱ���жϴ� �������ж����ȼ�
    TIM6_MODE_Config(MsTime);                                             //��ʱ����ʱʱ������
}




/*************************************************************************
  * @brief  �������ö�ʱ��TIM6 ,�����´��� ��
  * @param  ��
  * @retval ��
  * @notice ��������TIM6->CNT Ϊ 0 ��TIM6 Ϊ�� 0 ���ϼ��� ��  TIM6->ARR
*************************************************************************/
void TIM6_Open(void)
{
	/*************** ����ʱ�� TIM6 ***************/
	TIM_Cmd(TIM6, DISABLE);                                                 //�ض�ʱ�� �� TIMx->CNTֻ���ڶ�ʱ���ص�������д��
	TIM_SetCounter(TIM6,0);                                                 //��������TIM6->CNT Ϊ 0 ��TIM6 Ϊ�� 0 ���ϼ��� ���¼����� TIM6->ARR
	TIM_Cmd(TIM6, ENABLE);                                                  //����ʱ�� ����ʱ����ʼ���� 
}






/*************************************************************************
  * @brief  �رն�ʱ��TIM6 
  * @param  ��
  * @retval ��
  * @notice ��
*************************************************************************/
void TIM6_Close(void)
{
	/*************** �ض�ʱ�� TIM6 ***************/
	TIM_Cmd(TIM6, DISABLE);                                                 //��ʹ�ܶ�ʱ��TIM6����
}



/*************************************************************************
  * @brief  ��ʱ��TIM6 ISR������
  * @param  ��
  * @retval ��
  * @notice ����MODBUSһ֡���ݵļ�⣨֮֡������3.5���ֽڵ�ʱ�䣩
*************************************************************************/
extern void USART2_TransferOneFrameFinish(void);                          //������USART1.c�� ��TIM6����MODBUS��֡��ʱ�䶨ʱ������3.5�ֽ�ʱ�䣩��ֻ�м�⵽����ʱ������3.5�ֽڵ�ʱ�����ȷ��һ֡���ݽ������

void TIM6_IRQHandler(void)                                                //TIM6�жϷ�����
{	
	if ( TIM_GetITStatus(TIM6 , TIM_IT_Update) != RESET )                   //����жϱ�־λ
	{	
		TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);                        //���жϱ�־λ
		TIM6_Close();                                             		        //�رն�ʱ��
    USART2_TransferOneFrameFinish();                                      //Usart1 һ֡���ݽ�����ɣ�������һ֡���ݹرմ��ڵ����ݽ��գ���ֹ��֡����û�������ֽ��յ�������ݣ�                                                            
	}	
}










/*********************************************END OF FILE**********************/


