#include "RS485.h" 

/****************************RS485�û�����*******************************/ 
void RS485_ControlPinInit(void);                                         //RS485���ƶ˳�ʼ���ܺ���

/****************************RS485n�ڲ�����******************************/ 
static void RS485_GPIO_Config(void);                                     //RS485���ջ��Ƿ������ݵĿ��ƶ�GPIO����


/*************************************************************************
  * @brief  RS485���ջ��Ƿ������ݵĿ��ƶ�GPIO����
  * @param  ��
  * @retval ��
  * @notice ��
************************************************************************/
static void RS485_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		                              //����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	
  	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;			                      //ѡ��Ҫ���Ƶ�GPIOB����*/															   	
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //��������ģʽΪͨ���������*/		 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;                     //������������Ϊ2MHz */  
		GPIO_Init(GPIOA, &GPIO_InitStructure);          	                    //���ÿ⺯������ʼ��GPIOB0*/
    
	  RS485_SetReceiveMode();                                               //�ϵ�Ĭ��Ϊ����ģʽ
}


/*************************************************************************
  * @brief  RS485���ƶ˳�ʼ������ 
  * @param  ��
  * @retval ��
  * @notice ��
************************************************************************/
void RS485_ControlPinInit(void)
{
	/***********RS485���ƶ�GPIOʱ��ʹ��****************/
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 
	
	/***********RS485���ƶ�GPIOӲ������****************/
	RS485_GPIO_Config();
}


/*********************************************END OF FILE**********************/
