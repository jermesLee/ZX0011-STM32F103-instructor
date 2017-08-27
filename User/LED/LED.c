#include "LED.h" 

void LED_GPIO_Config(void);



void LED_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		          /*定义一个GPIO_InitTypeDef类型的结构体*/
		                                                  /*开启GPIOB和GPIOF的外设时钟*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD , ENABLE); 
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			  /*选择要控制的GPIOB引脚*/															   	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /*设置引脚模式为通用推挽输出*/		 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;  /*设置引脚速率为2MHz */  
		GPIO_Init(GPIOD, &GPIO_InitStructure);          	/*调用库函数，初始化GPIOB0*/
	  
	  GPIO_SetBits(GPIOD, GPIO_Pin_All);	              /*初始化引脚*/
}




/*********************************************END OF FILE**********************/
