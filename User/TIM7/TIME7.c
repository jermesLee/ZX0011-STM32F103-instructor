#include "TIME7.h" 

/*******************************TIM7 计数变量****************************/ 
uint32_t TIM7_TickCount = 0 ;                                             //中断次数变量，每进入一次中断加一,注意是32位数据（和总线数据宽度一样）                            
                                                                          //当数据不是32为数据时 ，在TIM7_xxMsFinish（）函数里的定时是否已到的判断方法有一点区别 
																																					//区别 ：在32位的系统里，8（16）位数据 - 8（16）位数据 不等于 8（16）位数据 ，而是等于默认的32位数据
/*******************************TIM7 用户函数****************************/ 
void TIM7_Init(void);                                                     //TIME6初始化   
FlagStatus TIM7_20MsFinish(void);                                         //定时10ms ， 定时时间到 返回SET ,否则返回RESET
FlagStatus TIM7_50MsFinish(void);                                         //定时50ms ， 定时时间到 返回SET ,否则返回RESET
FlagStatus TIM7_200MsFinish(void);                                        //定时100ms , 定时时间到 返回SET ,否则返回RESET
FlagStatus TIM7_1sFinish(void);                                           //定时1S ,    定时时间到 返回SET ,否则返回RESET
uint32_t TIM7_ReadTimeCount(void);


/*******************************TIM7 内部函数 ***************************/ 
static void TIM7_MODE_Config(void);                                       //TIM7工作模式配置 
static void TIM7_NVIC_Config(void);                                       //TIM7中断向量表配置
  
	
	
	
/*************************************************************************
  * @brief  TIM7 中断优先级配置
  * @param  无
  * @retval 无
  * @notice TIME6_IRQPreemptionPrio 和 TIME6_IRQSubPrio在“TIM7.h“里用#define 进行宏定义
**************************************************************************/
static void TIM7_NVIC_Config(void)                                        //中断向量表配置
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
		/*********TIM7 中断优先级配置************/ 
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM7_IRQn ;	  //指明中断名  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM7_IRQPreemptionPrio;  //配置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIM7_IRQSubPrio;	        //配置次优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //中断向量表使能
    NVIC_Init(&NVIC_InitStructure);                                       //配置
}






/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 49    TIM_Prescaler--1439
 * 中断周期为 = 1/(72MHZ /1440) * 50 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
**************************************************************************/
static void TIM7_MODE_Config(void)                                        //TIM7工作模式配置（ 单位 ：ms ）
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
		/*********TIM7 定时时间配置************/ 
   	TIM_TimeBaseStructure.TIM_Period    = TIM7_TimeTick * 50 - 1;         //自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Prescaler = 1439;                           //分配系数 1439
	
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		              //对外部时钟进行采样的时钟分频,这里没有用到 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;                        //没使用
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;             //向上计数
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);                       //配置
	
  	/**************使能中断****************/
    TIM_ClearFlag(TIM7, TIM_FLAG_Update);	                                //清中断标志 
    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);                              //使能中断		 
    TIM_Cmd(TIM7, ENABLE);                                                //使能定时器
}






/*************************************************************************
  * @brief  打开定时器TIM7
  * @param  MsTime ：  定时时间 ，单位 ：ms
  * @retval 无
  * @notice MsTime 最大为 120ms ，在此函数里有限幅
*************************************************************************/
void TIM7_Init(void)
{
	/***********时钟使能***************/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7 , ENABLE);                   //使能TIME6的时钟

	/**********硬件配置****************/
	TIM7_NVIC_Config();                                                     //定时器中断打开 ，配置中断优先级
	TIM7_MODE_Config();                                                     //定时器定时时间配置
}



/*************************************************************************
  * @brief  定时1S
  * @param  无
  * @retval 定时完成返回 SET ,定时时间还没到返回 RESET
  * @notice TIM7_FinishFlag在定时中断里被置位
*************************************************************************/
FlagStatus TIM7_1sFinish(void)
{
	static uint16_t  TimeTickNum = 1000/TIM7_TimeTick ;                     // 1S定时计算 ： 需要对定时器7进入中断计数
	static uint32_t  TimeLastCount   = 0 ;                                  // 初始值为 0 ，应该为32位数据（ 和总线宽度一样 ）

	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // 比较现在TIM7_TickCount的值和上一次进入定时完成的值
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //定时时间到 ，将这次定时器计数值给TimeLastCount ，用于下次定时的计算
		return SET ;                                                          //定时时间到 ，返回 SET
	}		
	else  return RESET;                                                     //定时时间还没到 ，返回 RESET
}



/*************************************************************************
  * @brief  定时200ms
  * @param  无
  * @retval 定时完成返回 SET ,定时时间还没到返回 RESET
  * @notice TIM7_FinishFlag在定时中断里被置位
*************************************************************************/
FlagStatus TIM7_200MsFinish(void)
{
	static uint16_t  TimeTickNum = 200/TIM7_TimeTick ;                      // 100MS定时计算 ： 需要对定时器6进入中断计数
	static uint32_t  TimeLastCount   = 0 ;                                  // 初始值为 0 ，应该为32位数据（ 和总线宽度一样 ）

	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // 比较现在TIM7_TickCount的值和上一次进入定时完成的值
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //定时时间到 ，将这次定时器计数值给TimeLastCount ，用于下次定时的计算
		return SET ;                                                          //定时时间到 ，返回 SET
	}		
	else  return RESET;                                                     //定时时间还没到 ，返回 RESET
}




/*************************************************************************
  * @brief  定时50ms
  * @param  无
  * @retval 定时完成返回 SET ,定时时间还没到返回 RESET
  * @notice TIM7_FinishFlag在定时中断里被置位
*************************************************************************/
FlagStatus TIM7_50MsFinish(void)
{
	static uint16_t  TimeTickNum = 50/TIM7_TimeTick ;                       // 50MS定时计算 ： 需要对定时器6进入中断计数
	static uint32_t  TimeLastCount   = 0 ;                                  // 初始值为 0 ，应该为32位数据（ 和总线宽度一样 ）
	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // 比较现在TIM7_TickCount的值和上一次进入定时完成的值
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //定时时间到 ，将这次定时器计数值给TimeLastCount ，用于下次定时的计算
		return SET ;                                                          //定时时间到 ，返回 SET
	}		
	else  return RESET;                                                     //定时时间还没到 ，返回 RESET
}


/*************************************************************************
  * @brief  定时20ms
  * @param  无
  * @retval 定时完成返回 SET ,定时时间还没到返回 RESET
  * @notice TIM7_FinishFlag在定时中断里被置位
*************************************************************************/
FlagStatus TIM7_20MsFinish(void)
{
	static uint16_t  TimeTickNum = 20/TIM7_TimeTick ;                       // 10MS定时计算 ： 需要对定时器6进入中断计数
	static uint32_t  TimeLastCount   = 0 ;                                  // 初始值为 0 ，应该为32位数据（ 和总线宽度一样 ）
	if( TIM7_TickCount - TimeLastCount >= TimeTickNum )                     // 比较现在TIM7_TickCount的值和上一次进入定时完成的值
	{
	  TimeLastCount	= TIM7_TickCount ;                                      //定时时间到 ，将这次定时器计数值给TimeLastCount ，用于下次定时的计算
		return SET ;                                                          //定时时间到 ，返回 SET
	}		
	else  return RESET;                                                     //定时时间还没到 ，返回 RESET
}


uint32_t TIM7_ReadTimeCount(void)
{
	return TIM7_TickCount * TIM7_TimeTick;
}
/*************************************************************************
  * @brief  定时器TIM7 ISR服务函数
  * @param  无
  * @retval 无
  * @notice 
*************************************************************************/
void TIM7_IRQHandler(void)                                                //TIM7中断服务函数
{
	if ( TIM_GetITStatus(TIM7 , TIM_IT_Update) != RESET )                   //检测中断标志位
	{
		TIM_ClearITPendingBit(TIM7 , TIM_FLAG_Update);                        //清中断标志位	
		TIM7_TickCount++  ;                                                   //定时器TIM7计数
	}	
}






/*********************************************END OF FILE**********************/


