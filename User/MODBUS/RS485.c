#include "RS485.h" 

/****************************RS485用户函数*******************************/ 
void RS485_ControlPinInit(void);                                         //RS485控制端初始化总函数

/****************************RS485n内部函数******************************/ 
static void RS485_GPIO_Config(void);                                     //RS485接收还是发送数据的控制端GPIO配置


/*************************************************************************
  * @brief  RS485接收还是发送数据的控制端GPIO配置
  * @param  无
  * @retval 无
  * @notice 无
************************************************************************/
static void RS485_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		                              //定义一个GPIO_InitTypeDef类型的结构体*/
	
  	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;			                      //选择要控制的GPIOB引脚*/															   	
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //设置引脚模式为通用推挽输出*/		 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;                     //设置引脚速率为2MHz */  
		GPIO_Init(GPIOA, &GPIO_InitStructure);          	                    //调用库函数，初始化GPIOB0*/
    
	  RS485_SetReceiveMode();                                               //上电默认为接收模式
}


/*************************************************************************
  * @brief  RS485控制端初始化函数 
  * @param  无
  * @retval 无
  * @notice 无
************************************************************************/
void RS485_ControlPinInit(void)
{
	/***********RS485控制端GPIO时钟使能****************/
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 
	
	/***********RS485控制端GPIO硬件配置****************/
	RS485_GPIO_Config();
}


/*********************************************END OF FILE**********************/
