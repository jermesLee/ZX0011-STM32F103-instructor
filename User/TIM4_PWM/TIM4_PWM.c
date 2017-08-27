#include "TIM4_PWM.h" 

/******************TIM4 重装载寄存器值与PWM输出频率的关系****************/ 
#define TIM4_PWMCounterPeriod     ( SystemCoreClock / (TIM4_PWMOutputFrequency) )  //定时器4的定时周期数


/*******************************TIM4 用户函数****************************/ 
void TIM4_PWM_Init(void);                                                 //TIM4 输出PWM信号初始化总函数                                      
void TIM4_PWM_SetDutyRatio(uint16_t DutyRatio);                           //定时器TIM4产生的pwm输出占空比设置 ，单位 ：0.1%



/*******************************TIM4 内部函数****************************/ 
static void TIM4_GPIO_Config(void);                                       //配置TIM4复用输出PWM时用到的I/O
static void TIM4_Mode_Config(void);                                       //配置TIM1输出的PWM信号的模式，如周期、极性、占空比





 /************************************************************************
  * @brief  配置TIM4复用输出PWM时用到的I/O
  * @param  无
  * @retval 无
	* @notice 无
*************************************************************************/
static void TIM4_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;		                    // 复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                        // I/O引脚速度2M/S
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 ;                            // GPIOB Configuration: TIM4 channel 1  as alternate function push-pull 
  GPIO_Init(GPIOB, &GPIO_InitStructure);                                  // 配置
}




/**************************************************************************
  * @brief  配置TIM1输出的PWM信号的模式，如周期、极性、占空比
  * @param  无
  * @retval 无
	* @notice TIMxCLK/CK_PSC --> TIMxCNT --> TIMx_ARR --> TIMxCNT 重新计数
  *         TIMx_CCR(电平发生变化)
  *         信号周期=(TIMx_ARR +1 ) * 时钟周期
  *         占空比=TIMx_CCR/(TIMx_ARR +1)
  *         初始化默认输出高电平
**************************************************************************/
static void TIM4_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;                         //定时器工作方式初始化变量
	TIM_OCInitTypeDef  TIM_OCInitStructure;                                 //定时器PWM输出方式下的PWM参数变量
  
	/************ Tim4 定时时间配置·**************/		                      //TIM4_PWMCounterPeriod = ( SystemCoreClock / (TIM4_PWMOutputFrequency) - 1 )
	TIM_TimeBaseStructure.TIM_Period        = TIM4_PWMCounterPeriod - 1 ;   //当定时器从0计数到TIM4_PWMCounterPeriod，即为TIM4_PWMCounterPeriod次，为一个定时周期
	TIM_TimeBaseStructure.TIM_Prescaler     = 0;	                          //设置预分频：不预分频，即为72MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	              //设置时钟分频系数：不分频(这里用不到)
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;           //向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/*** Tim4 PWM1 Mode configuration: Channel1 **/
	TIM_OCInitStructure.TIM_OCMode          = TIM_OCMode_PWM1;	            //配置为PWM模式1
	TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable;       //输出使能	
	TIM_OCInitStructure.TIM_OutputNState    = TIM_OutputNState_Disable;   	//互补输出失能（高级定时器才有）
	TIM_OCInitStructure.TIM_OCPolarity      = TIM_OCPolarity_High;          //当定时器计数值小于CCR1_Val时为高电平
	TIM_OCInitStructure.TIM_OCNPolarity     = TIM_OCNPolarity_Low;          //当定时器计数值小于CCR1_Val时为低电平
	TIM_OCInitStructure.TIM_Pulse           = 0;	                          //设置跳变值，当计数器计数到这个值时，电平发生跳变
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);	                              //使能通道1

	/************ TIM4 PWM 使能 ******************/
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);                       //1通道预装载使能          
	TIM_ARRPreloadConfig(TIM4, ENABLE);			                                //使能TIM4重载寄存器ARR
	TIM_Cmd(TIM4, ENABLE);                                                  //使能定时器1	
}




/*************************************************************************
  * @brief  TIM4 输出PWM信号初始化总函数
  * @param  无
  * @retval 无
  * @notice 默认上电输出高电平
***************************************************************************/
void TIM4_PWM_Init(void)                                        
{
	/*****************时钟使能********************/
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4 , ENABLE); 	                // 定时器	TIM4 时钟使能
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);                  // 定时器 TIM4 GPIO 引脚时钟使能

	/*****************硬件配置********************/
	TIM4_GPIO_Config();                                                     //I/0输出引脚配置
	TIM4_Mode_Config();	                                                    //定时器TIM4工作模式配置   
}




/*************************************************************************
  * @brief  定时器TIM4产生的pwm输出占空比设置
  * @param  DutyRatio  ：占空比配置 ，占空比已经放大100倍，即25.9%的占空比应该给DutyRatio为2590
  * @retval 无 
  * @notice 无
**************************************************************************/
	uint16_t  CCR_Data ;
void TIM4_PWM_SetDutyRatio(uint16_t DutyRatio)
{
                                                   //给定时器捕获、比较寄存器的临时存储区 
	
	/*****根据占空比输入的情况计算TIM4 ARR的值*****/	
	if( DutyRatio >= 10000 )                                                //当给的占空比大于100%时，输出一直为高
		CCR_Data   = TIM4_PWMCounterPeriod + 10 ;                             //只要给个比周期值大的值即可，此程序的周期给得是TIM4_PWMCounterPeriod
	else                                                                    //输入数据正确的情况下占空比与定时器TIM4 ARR寄存器值的关系
		CCR_Data =DutyRatio * TIM4_PWMCounterPeriod / 10000;                  //对于给的占空比在0%-100%时，将占空比转换成给捕获/比较寄存器的值 
	TIM4->CCR1 = CCR_Data ;                                                 //将数据写入预装载寄存器
}




/****************************************END OF FILE*******************/


