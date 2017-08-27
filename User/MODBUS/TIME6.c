#include "TIME6.h" 

/**************************TIM6 定时MS级别用户函数***********************/ 
void TIM6_Init(uint8_t MsTime);                                           //TIME6初始化   , MsTime ： 定时时间（ 单位： ms） ,z最大 120ms
void TIM6_Open(void);                                                     //TIM6打开
void TIM6_Close(void);                                                    //TIM6关闭
 
 
/**************************TIM6 定时MS级别内部函数***********************/ 
static void TIM6_MODE_Config(uint8_t MsTime);                             //TIM6工作模式配置 ，MsTime ： 定时时间（ 单位： ms）
static void TIM6_NVIC_Config(void);                                       //TIM6中断向量表配置
	
	
	
	
	
/*************************************************************************
  * @brief  TIM6 中断优先级配置
  * @param  无
  * @retval 无
  * @notice TIME6_IRQPreemptionPrio 和 TIME6_IRQSubPrio在“TIM6.h“里用#define 进行宏定义
**************************************************************************/
static void TIM6_NVIC_Config(void)                                        //中断向量表配置
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
	  /**********TIM6定时 中断优先级配置**********/
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM6_IRQn;	    //指明中断名  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME6_IRQPreemptionPrio;  //配置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = TIME6_IRQSubPrio;	       //配置次优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;        //中断向量表使能
    NVIC_Init(&NVIC_InitStructure);                                       //配置
}




/*************************************************************************
 * TIM_Period / Auto Reload Register(ARR) = 499   TIM_Prescaler--143
 * 中断周期为 = 1/(72MHZ /144) * 500 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
**************************************************************************/
static void TIM6_MODE_Config(uint8_t MsTime )                             //TIM6工作模式配置（ 单位 ：ms ）
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	 
	  /*************TIM6定时时间配置************/  
	  TIM_TimeBaseStructure.TIM_Period            = MsTime * 500 - 1;       //自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Prescaler         = 143;                    //分配系数 144
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;		        //对外部时钟进行采样的时钟分频,这里没有用到 
	  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                      //没使用
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;     //向上计数
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);                       //配置
	
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);	                                //清中断标志 
    TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);                              //使能中断		 
    TIM_Cmd(TIM6, DISABLE);                                               //不使能定时器，需要使用时使能定时器	                       																	  
}


/*************************************************************************
  * @brief  打开定时器TIM6
  * @param  MsTime ：  定时时间 ，单位 ：ms
  * @retval 无
  * @notice MsTime 最大为 120ms ，在此函数里有限幅
*************************************************************************/
void TIM6_Init(uint8_t MsTime)
{
	  /********定时时间限幅 ，最大为120ms*********/
    if( MsTime > 120 ) MsTime = 120  ;                                    //限幅 ，最大延时时间为 ： 120ms	 
	
	  /**********定时器 TIM6时钟使能**************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 , ENABLE);                 //使能TIME6的时钟 
	
	  /**********定时器 TIM6硬件配置**************/	  
		TIM6_NVIC_Config();                                                   //定时器中断打开 ，配置中断优先级
    TIM6_MODE_Config(MsTime);                                             //定时器定时时间配置
}




/*************************************************************************
  * @brief  从新设置定时器TIM6 ,并重新打开它 ，
  * @param  无
  * @retval 无
  * @notice 重新设置TIM6->CNT 为 0 ，TIM6 为从 0 向上计数 到  TIM6->ARR
*************************************************************************/
void TIM6_Open(void)
{
	/*************** 开定时器 TIM6 ***************/
	TIM_Cmd(TIM6, DISABLE);                                                 //关定时器 ， TIMx->CNT只能在定时器关的条件下写入
	TIM_SetCounter(TIM6,0);                                                 //重新设置TIM6->CNT 为 0 ，TIM6 为从 0 向上计数 从新计数到 TIM6->ARR
	TIM_Cmd(TIM6, ENABLE);                                                  //开定时器 ，定时器开始计数 
}






/*************************************************************************
  * @brief  关闭定时器TIM6 
  * @param  无
  * @retval 无
  * @notice 无
*************************************************************************/
void TIM6_Close(void)
{
	/*************** 关定时器 TIM6 ***************/
	TIM_Cmd(TIM6, DISABLE);                                                 //不使能定时器TIM6计数
}



/*************************************************************************
  * @brief  定时器TIM6 ISR服务函数
  * @param  无
  * @retval 无
  * @notice 用于MODBUS一帧数据的检测（帧之间至少3.5个字节的时间）
*************************************************************************/
extern void USART2_TransferOneFrameFinish(void);                          //函数在USART1.c中 ，TIM6用于MODBUS里帧间时间定时（至少3.5字节时间），只有检测到空闲时间至少3.5字节的时间才能确定一帧数据接收完成

void TIM6_IRQHandler(void)                                                //TIM6中断服务函数
{	
	if ( TIM_GetITStatus(TIM6 , TIM_IT_Update) != RESET )                   //检测中断标志位
	{	
		TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);                        //清中断标志位
		TIM6_Close();                                             		        //关闭定时器
    USART2_TransferOneFrameFinish();                                      //Usart1 一帧数据接收完成（接收完一帧数据关闭串口的数据接收，防止这帧数据没处理完又接收到别的数据）                                                            
	}	
}










/*********************************************END OF FILE**********************/


