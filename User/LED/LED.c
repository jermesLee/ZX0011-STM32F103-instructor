#include "LED.h" 

void LED_GPIO_Config(void);



void LED_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		          /*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		                                                  /*����GPIOB��GPIOF������ʱ��*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD , ENABLE); 
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			  /*ѡ��Ҫ���Ƶ�GPIOB����*/															   	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /*��������ģʽΪͨ���������*/		 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;  /*������������Ϊ2MHz */  
		GPIO_Init(GPIOD, &GPIO_InitStructure);          	/*���ÿ⺯������ʼ��GPIOB0*/
	  
	  GPIO_SetBits(GPIOD, GPIO_Pin_All);	              /*��ʼ������*/
}




/*********************************************END OF FILE**********************/
