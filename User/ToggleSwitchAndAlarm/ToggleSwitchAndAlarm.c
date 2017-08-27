#include "ToggleSwitchAndAlarm.h"

/**************************���뿪�ص��û�����****************************/
void ToggleSwitchAndAlarm_Init(void);                                     // ���뿪�غͱ�����gpio��ʼ���ܺ���      
uint8_t ToggleSwitch_ReadKeyValue(void);                                  // ��ȡ���뿪�ذ���ֵ

/**************************���뿪�ص��ڲ�����****************************/
static void ToggleSwitchAndAlarm_GPIO_Config(void);                       // ���뿪�غͱ�����������GPIO����  



/*************************************************************************
  * @brief  ���뿪�غͱ�����������GPIO����        
  * @param  ��
  * @retval ��
  * @notice ����GPIOA8 - GPIOA11 ���ڲ��뿪�ؼ�⣬��Ϊ����ʹ��
	*             GPIOB2 ��Ϊ�������Ŀ��ƣ���Ϊ���ʹ��
*************************************************************************/
static void ToggleSwitchAndAlarm_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		                              //����һ��GPIO_InitTypeDef���͵Ľṹ��
	
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //��������ģʽΪͨ���������	 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //������������Ϊ2MHz 
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;	                          //GPIOB2 ( ���ڱ������Ŀ��� )												   	
    GPIO_Init(GPIOB, &GPIO_InitStructure);          	                    //���ÿ⺯����ʼ��
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;                        //��������ģʽΪ�������� 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //������������Ϊ2MHz 
	  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 // ( ���ڲ��뿪�صļ�� )												   	
    GPIO_Init(GPIOA, &GPIO_InitStructure);          	                    //���ÿ⺯����ʼ��
	  
	  Alarm_Close();                                                        //��ʼ�����ţ�ͨ��رձ�����
}






/*************************************************************************
  * @brief  ���뿪�غͱ�����gpio��ʼ���ܺ���      
  * @param  �� 
  * @retval ��
  * @notice �� 
*************************************************************************/
void ToggleSwitchAndAlarm_Init(void)
{
  /**********GPIOʱ������*************/
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |  RCC_APB2Periph_GPIOB , ENABLE);//��GPIOʱ�� 

  /**********GPIOӲ������*************/
  ToggleSwitchAndAlarm_GPIO_Config();                                     //���뿪�غͱ�����������GPIO����        

}




/*************************************************************************
  * @brief  ��ȡ���뿪�ذ���ֵ
  * @param  �� 
  * @retval ���뿪�ص�ֵ,���µ�λΪ1 ��δ���µ�ֵΪ0
  * @notice 5ms����������ÿ5ms����һ�Σ������β�����ͬʱ��ʾ�����ɹ�
*************************************************************************/
uint8_t ToggleSwitch_ReadKeyValue(void)
{
  uint16_t KeyValue1 = 0XFFFF , KeyValue2 = 0XFFFF ;                      // ��ʼ�����β�����ֵ
  do
  {
    KeyValue2 = GPIO_ReadInputData(GPIOA);                                // ��ȡ��Ƭ��I/O�ڵ�ѹ
    KeyValue2 = ( KeyValue1 >> 8 )&0X000F ;                               // ����PA8 - PA11 ���ŵ�ѹ���
   
    if(KeyValue1 != KeyValue2)                                            // ���β���ֵ�Ƚ�
    {
      KeyValue1 = KeyValue2 ;                                             // ���ε�ѹֵ��ͬ �����˴ε�ѹ���棬���´�ʹ��
      Delay1Ms(5);                                                        // ��ʱ 5 ms
    } 
    else break ;                                                          // ���β�����ѹһ�� ���˳�Whileѭ��
  
  }while( 1 );
 
	return ((uint8_t)( 15 - KeyValue2));                                    // ���㰴���������
}












