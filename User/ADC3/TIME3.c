#include "TIME3.h" 

/*******************************TIM3 用户函数****************************/ 
void TIM3_Init(uint16_t TimeUs);                                          //TIM3初始化   

/*******************************TIM3 内部函数 ***************************/ 
static void TIM3_MODE_Config(uint16_t TimeUs);                            //TIM3工作模式配置   



/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 49    TIM_Prescaler--71
 * 中断周期为 = 1/(84MHZ /1680) * 50 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
**************************************************************************/
static void TIM3_MODE_Config(uint16_t TimeUs)                             //TIM3工作模式配置（ 单位 ：us ）
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	TIM_OCInitTypeDef  TIM_OCInitStructure;

		/*********TIM3 定时时间配置************/ 
   	TIM_TimeBaseStructure.TIM_Period            = TimeUs - 1 ;            //自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Prescaler         = 71;                     //分配系数 72
	
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1 ;		      //对外部时钟进行采样的时钟分频,这里没有用到 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0 ;                     //没使用
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;     //向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                       //配置 
	
		/* PWM1 Mode configuration: Channel1 */
		TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM1;	      //配置为PWM模式1
		TIM_OCInitStructure.TIM_OutputState         = TIM_OutputState_Enable; //输出使能	
		TIM_OCInitStructure.TIM_OutputNState        = TIM_OutputNState_Disable;	//互补输出失能（高级定时器才有）
		TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;    //当定时器计数值小于CCR1_Val时为高电平
		TIM_OCInitStructure.TIM_OCNPolarity         = TIM_OCNPolarity_Low;    //当定时器计数值小于CCR1_Val时为高电平
		TIM_OCInitStructure.TIM_Pulse               = 1 ;	                    //设置跳变值，当计数器计数到这个值时，电平发生跳变
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);	                            //使能通道1
	  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);                     //1通道预装载使能          

    TIM_Cmd(TIM3, ENABLE);                                                //使能定时器
}






/*************************************************************************
  * @brief  打开定时器TIM3
  * @param  无
  * @retval 无
  * @notice 无
*************************************************************************/
void TIM3_Init(uint16_t TimeUs)
{
	/***********时钟使能***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);                   //使能TIM3的时钟

	/**********硬件配置****************/
	TIM3_MODE_Config(TimeUs);                                               //定时器定时时间配置
}







/*********************************************END OF FILE**********************/


