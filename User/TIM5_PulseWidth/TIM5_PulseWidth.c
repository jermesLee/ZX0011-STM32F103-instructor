#include "TIM5_PulseWidth.h" 

/*******************************TIM5 计数变量****************************/ 
int32_t TIM5_TickCount = 0 ;      
int16_t TIM5_FinishFlag= 0 ;                                             // 定时时间完成标志 

/*******************************TIM5 用户函数****************************/ 
void TIM5_PluseWidthAdjustInit(void);                                    //TIM5初始化   
void TIM5_SetOutputPluseWidth(uint16_t PluseWidth );	
FlagStatus TIM5_OutputPluseWidthFinish(void);


/*******************************TIM5 内部函数 ***************************/ 
static void TIM5_MODE_Config(void);                                       //TIM5工作模式配置 
static void TIM5_NVIC_Config(void);                                       //TIM5中断向量表配置
  
	
	
	
/*************************************************************************
  * @brief  TIM5 中断优先级配置
  * @param  无
  * @retval 无
  * @notice TIM5_IRQPreemptionPrio 和 TIM5_IRQSubPrio在“TIM5.h“里用#define 进行宏定义
**************************************************************************/
static void TIM5_NVIC_Config(void)                                        //中断向量表配置
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
		/*********TIM5 中断优先级配置************/ 
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn ;	  //指明中断名  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM5_IRQPreemptionPrio;  //配置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIM5_IRQSubPrio;	        //配置次优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //中断向量表使能
    NVIC_Init(&NVIC_InitStructure);                                       //配置
}






/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 999    TIM_Prescaler--71
 * 中断周期为 = 1MS
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
**************************************************************************/
static void TIM5_MODE_Config(void)                                        //TIM5工作模式配置（ 单位 ：ms ）
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
		/*********TIM5 定时时间配置************/ 
   	TIM_TimeBaseStructure.TIM_Period    = 1000 - 1 ;                       //自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Prescaler = 71;                              //分配系数 0
	
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		              //对外部时钟进行采样的时钟分频,这里没有用到 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;                        //没使用
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;             //向上计数
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);                       //配置
	
  	/**************使能中断****************/
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);	                                //清中断标志 
    TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                              //使能中断		 
    TIM_Cmd(TIM5, DISABLE);                                               //不使能定时器 ，在需要使用时再使能定时器
}






/*************************************************************************
  * @brief  打开定时器TIM5
  * @param  无
  * @retval 无
  * @notice 无
*************************************************************************/
void TIM5_PluseWidthAdjustInit(void)
{
	/***********时钟使能***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);                   //使能TIME6的时钟

	/**********硬件配置****************/
	TIM5_NVIC_Config();                                                     //定时器中断打开 ，配置中断优先级
	TIM5_MODE_Config();                                                     //定时器定时时间配置
}


FlagStatus TIM5_OutputPluseWidthFinish(void)
{
	if( TIM5_FinishFlag ) 
	{
		TIM5_FinishFlag = 0 ;
		return SET ;
	}
	else
	 return  RESET ;
}




void TIM5_SetOutputPluseWidth(uint16_t PluseWidth )   
{
	TIM5_TickCount = PluseWidth ;
	TIM_Cmd(TIM5, DISABLE);                                                 //关定时器 ， TIMx->CNT只能在定时器关的条件下写入
	TIM_SetCounter(TIM5,0);                                                 //重新设置TIM5->CNT 为 0 ，TIM5 为从 0 向上计数 从新计数到 TIM5->ARR
	TIM_Cmd(TIM5, ENABLE);                                                  //开定时器 ，定时器开始计数 
  FET_PluseWidthSignalOutputOpen();                                       //打开FET管，开始输出 
}

/*************************************************************************
  * @brief  定时器TIM5 ISR服务函数
  * @param  无
  * @retval 无
  * @notice 
*************************************************************************/
void TIM5_IRQHandler(void)                                                //TIM5中断服务函数
{
	if ( TIM_GetITStatus(TIM5 , TIM_IT_Update) != RESET )                   //检测中断标志位
	{
		TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update);                        //清中断标志位	
    if( ( --TIM5_TickCount ) ==0 )		
		{
			FET_PluseWidthSignalOutputClose();                                  //脉冲信号时间到 ，关输出 
			TIM_Cmd(TIM5, DISABLE);                                             //关定时器 ，在下一次需要输出脉冲时再打开定时器
		  TIM5_FinishFlag  = 1 ;                                              //定时时间完成 
		}
	}	
}






/*********************************************END OF FILE**********************/


