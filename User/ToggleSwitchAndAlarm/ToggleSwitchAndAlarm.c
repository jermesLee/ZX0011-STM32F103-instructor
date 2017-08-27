#include "ToggleSwitchAndAlarm.h"

/**************************拨码开关的用户函数****************************/
void ToggleSwitchAndAlarm_Init(void);                                     // 拨码开关和报警器gpio初始化总函数      
uint8_t ToggleSwitch_ReadKeyValue(void);                                  // 读取拨码开关按键值

/**************************拨码开关的内部函数****************************/
static void ToggleSwitchAndAlarm_GPIO_Config(void);                       // 拨码开关和报警器的引脚GPIO配置  



/*************************************************************************
  * @brief  拨码开关和报警器的引脚GPIO配置        
  * @param  无
  * @retval 无
  * @notice 其中GPIOA8 - GPIOA11 用于拨码开关检测，作为输入使用
	*             GPIOB2 作为报警器的控制，作为输出使用
*************************************************************************/
static void ToggleSwitchAndAlarm_GPIO_Config(void)
{		
		GPIO_InitTypeDef GPIO_InitStructure;		                              //定义一个GPIO_InitTypeDef类型的结构体
	
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //设置引脚模式为通用推挽输出	 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //设置引脚速率为2MHz 
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;	                          //GPIOB2 ( 用于报警器的控制 )												   	
    GPIO_Init(GPIOB, &GPIO_InitStructure);          	                    //调用库函数初始化
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;                        //设置引脚模式为上拉输入 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //设置引脚速率为2MHz 
	  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 // ( 用于拨码开关的检测 )												   	
    GPIO_Init(GPIOA, &GPIO_InitStructure);          	                    //调用库函数初始化
	  
	  Alarm_Close();                                                        //初始化引脚，通电关闭报警器
}






/*************************************************************************
  * @brief  拨码开关和报警器gpio初始化总函数      
  * @param  无 
  * @retval 无
  * @notice 无 
*************************************************************************/
void ToggleSwitchAndAlarm_Init(void)
{
  /**********GPIO时钟配置*************/
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |  RCC_APB2Periph_GPIOB , ENABLE);//打开GPIO时钟 

  /**********GPIO硬件配置*************/
  ToggleSwitchAndAlarm_GPIO_Config();                                     //拨码开关和报警器的引脚GPIO配置        

}




/*************************************************************************
  * @brief  读取拨码开关按键值
  * @param  无 
  * @retval 拨码开关的值,按下的位为1 ，未按下的值为0
  * @notice 5ms采样消抖，每5ms采样一次，当两次采样相同时表示采样成功
*************************************************************************/
uint8_t ToggleSwitch_ReadKeyValue(void)
{
  uint16_t KeyValue1 = 0XFFFF , KeyValue2 = 0XFFFF ;                      // 初始化两次采样的值
  do
  {
    KeyValue2 = GPIO_ReadInputData(GPIOA);                                // 读取单片机I/O口电压
    KeyValue2 = ( KeyValue1 >> 8 )&0X000F ;                               // 计算PA8 - PA11 引脚电压情况
   
    if(KeyValue1 != KeyValue2)                                            // 两次采样值比较
    {
      KeyValue1 = KeyValue2 ;                                             // 两次电压值不同 ，将此次电压保存，供下次使用
      Delay1Ms(5);                                                        // 延时 5 ms
    } 
    else break ;                                                          // 两次采样电压一样 ，退出While循环
  
  }while( 1 );
 
	return ((uint8_t)( 15 - KeyValue2));                                    // 计算按键按下情况
}












