#include "TIME7.h" 

/*******************************TIM7 ��������****************************/ 
uint32_t TIM7_TickCount = 0 ;                                             //�жϴ���������ÿ����һ���жϼ�һ,ע����32λ���ݣ����������ݿ��һ����                            
                                                                          //�����ݲ���32Ϊ����ʱ ����TIM7_xxMsFinish����������Ķ�ʱ�Ƿ��ѵ����жϷ�����һ������ 
																																					//���� ����32λ��ϵͳ�8��16��λ���� - 8��16��λ���� ������ 8��16��λ���� �����ǵ���Ĭ�ϵ�32λ����
/*******************************TIM7 �û�����****************************/ 
void TIM7_Init(void);                                                     //TIME6��ʼ��   
FlagStatus TIM7_20MsFinish(void);                                         //��ʱ10ms �� ��ʱʱ�䵽 ����SET ,���򷵻�RESET
FlagStatus TIM7_50MsFinish(void);                                         //��ʱ50ms �� ��ʱʱ�䵽 ����SET ,���򷵻�RESET
FlagStatus TIM7_200MsFinish(void);                                        //��ʱ100ms , ��ʱʱ�䵽 ����SET ,���򷵻�RESET
FlagStatus TIM7_1sFinish(void);                                           //��ʱ1S ,    ��ʱʱ�䵽 ����SET ,���򷵻�RESET
uint32_t TIM7_ReadTimeCount(void);


/*******************************TIM7 �ڲ����� ***************************/ 
static void TIM7_MODE_Config(void);                                       //TIM7����ģʽ���� 
static void TIM7_NVIC_Config(void);                                       //TIM7�ж�����������
  
	
	
	
/*************************************************************************
  * @brief  TIM7 �ж����ȼ�����
  * @param  ��
  * @retval ��
  * @notice TIME6_IRQPreemptionPrio �� TIME6_IRQSubPrio�ڡ�TIM7.h������#define ���к궨��
**************************************************************************/
static void TIM7_NVIC_Config(void)                                        //�ж�����������
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
		/*********TIM7 �ж����ȼ�����************/ 
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM7_IRQn ;	  //ָ���ж���  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM7_IRQPreemptionPrio;  //������ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIM7_IRQSubPrio;	        //���ô����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //�ж�������ʹ��
    NVIC_Init(&NVIC_InitStructure);                                       //����
}






/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 49    TIM_Prescaler--1439
 * �ж�����Ϊ = 1/(72MHZ /1440) * 50 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼��� 
**************************************************************************/
static void TIM7_MODE_Config(void)                                        //TIM7����ģʽ���ã� ��λ ��ms ��
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
		/*********TIM7 ��ʱʱ������************/ 
   	TIM_TimeBaseStructure.TIM_Period    = TIM7_TimeTick * 50 - 1;         //�Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Prescaler = 1439;                           //����ϵ�� 1439
	
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		              //���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;                        //ûʹ��
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;             //���ϼ���
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);                       //����
	
  	/**************ʹ���ж�****************/
    TIM_ClearFlag(TIM7, TIM_FLAG_Update);	                                //���жϱ�־ 
    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);                              //ʹ���ж�		 
    TIM_Cmd(TIM7, ENABLE);                                                //ʹ�ܶ�ʱ��
}






/*************************************************************************
  * @brief  �򿪶�ʱ��TIM7
  * @param  MsTime ��  ��ʱʱ�� ����λ ��ms
  * @retval ��
  * @notice MsTime ���Ϊ 120ms ���ڴ˺��������޷�
*************************************************************************/
void TIM7_Init(void)
{
	/***********ʱ��ʹ��***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7 , ENABLE);                   //ʹ��TIME6��ʱ��

	/**********Ӳ������****************/
	TIM7_NVIC_Config();                                                     //��ʱ���жϴ� �������ж����ȼ�
	TIM7_MODE_Config();                                                     //��ʱ����ʱʱ������
}



/*************************************************************************
  * @brief  ��ʱ1S
  * @param  ��
  * @retval ��ʱ��ɷ��� SET ,��ʱʱ�仹û������ RESET
  * @notice TIM7_FinishFlag�ڶ�ʱ�ж��ﱻ��λ
*************************************************************************/
FlagStatus TIM7_1sFinish(void)
{
	static uint16_t  TimeTickNum = 1000/TIM7_TimeTick ;                     // 1S��ʱ���� �� ��Ҫ�Զ�ʱ��7�����жϼ���
	static uint32_t  TimeLastCount   = 0 ;                                  // ��ʼֵΪ 0 ��Ӧ��Ϊ32λ���ݣ� �����߿��һ�� ��

	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // �Ƚ�����TIM7_TickCount��ֵ����һ�ν��붨ʱ��ɵ�ֵ
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //��ʱʱ�䵽 ������ζ�ʱ������ֵ��TimeLastCount �������´ζ�ʱ�ļ���
		return SET ;                                                          //��ʱʱ�䵽 ������ SET
	}		
	else  return RESET;                                                     //��ʱʱ�仹û�� ������ RESET
}



/*************************************************************************
  * @brief  ��ʱ200ms
  * @param  ��
  * @retval ��ʱ��ɷ��� SET ,��ʱʱ�仹û������ RESET
  * @notice TIM7_FinishFlag�ڶ�ʱ�ж��ﱻ��λ
*************************************************************************/
FlagStatus TIM7_200MsFinish(void)
{
	static uint16_t  TimeTickNum = 200/TIM7_TimeTick ;                      // 100MS��ʱ���� �� ��Ҫ�Զ�ʱ��6�����жϼ���
	static uint32_t  TimeLastCount   = 0 ;                                  // ��ʼֵΪ 0 ��Ӧ��Ϊ32λ���ݣ� �����߿��һ�� ��

	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // �Ƚ�����TIM7_TickCount��ֵ����һ�ν��붨ʱ��ɵ�ֵ
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //��ʱʱ�䵽 ������ζ�ʱ������ֵ��TimeLastCount �������´ζ�ʱ�ļ���
		return SET ;                                                          //��ʱʱ�䵽 ������ SET
	}		
	else  return RESET;                                                     //��ʱʱ�仹û�� ������ RESET
}




/*************************************************************************
  * @brief  ��ʱ50ms
  * @param  ��
  * @retval ��ʱ��ɷ��� SET ,��ʱʱ�仹û������ RESET
  * @notice TIM7_FinishFlag�ڶ�ʱ�ж��ﱻ��λ
*************************************************************************/
FlagStatus TIM7_50MsFinish(void)
{
	static uint16_t  TimeTickNum = 50/TIM7_TimeTick ;                       // 50MS��ʱ���� �� ��Ҫ�Զ�ʱ��6�����жϼ���
	static uint32_t  TimeLastCount   = 0 ;                                  // ��ʼֵΪ 0 ��Ӧ��Ϊ32λ���ݣ� �����߿��һ�� ��
	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // �Ƚ�����TIM7_TickCount��ֵ����һ�ν��붨ʱ��ɵ�ֵ
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //��ʱʱ�䵽 ������ζ�ʱ������ֵ��TimeLastCount �������´ζ�ʱ�ļ���
		return SET ;                                                          //��ʱʱ�䵽 ������ SET
	}		
	else  return RESET;                                                     //��ʱʱ�仹û�� ������ RESET
}


/*************************************************************************
  * @brief  ��ʱ20ms
  * @param  ��
  * @retval ��ʱ��ɷ��� SET ,��ʱʱ�仹û������ RESET
  * @notice TIM7_FinishFlag�ڶ�ʱ�ж��ﱻ��λ
*************************************************************************/
FlagStatus TIM7_20MsFinish(void)
{
	static uint16_t  TimeTickNum = 20/TIM7_TimeTick ;                       // 10MS��ʱ���� �� ��Ҫ�Զ�ʱ��6�����жϼ���
	static uint32_t  TimeLastCount   = 0 ;                                  // ��ʼֵΪ 0 ��Ӧ��Ϊ32λ���ݣ� �����߿��һ�� ��
	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // �Ƚ�����TIM7_TickCount��ֵ����һ�ν��붨ʱ��ɵ�ֵ
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //��ʱʱ�䵽 ������ζ�ʱ������ֵ��TimeLastCount �������´ζ�ʱ�ļ���
		return SET ;                                                          //��ʱʱ�䵽 ������ SET
	}		
	else  return RESET;                                                     //��ʱʱ�仹û�� ������ RESET
}


uint32_t TIM7_ReadTimeCount(void)
{
	return TIM7_TickCount * TIM7_TimeTick;
}
/*************************************************************************
  * @brief  ��ʱ��TIM7 ISR������
  * @param  ��
  * @retval ��
  * @notice 
*************************************************************************/
void TIM7_IRQHandler(void)                                                //TIM7�жϷ�����
{
	if ( TIM_GetITStatus(TIM7 , TIM_IT_Update) != RESET )                   //����жϱ�־λ
	{
		TIM_ClearITPendingBit(TIM7 , TIM_FLAG_Update);                        //���жϱ�־λ	
		TIM7_TickCount++  ;                                                   //��ʱ��TIM7����
	}	
}






/*********************************************END OF FILE**********************/


