#include "includes.h"

/*****************************宏定义************************************/
#define RELAY_SWITCHOVER_TIMER      20                                    // 继电器切换时间（ 单位 ： ms）
#define HIGH_LEVEL_PULSE_WIDTH      ((uint16_t)0XFFFF )                   // 表示输出脉冲宽度无穷大的标志（高电平）
#define EDGE_PULSE_WIDTH            0                                     // 表示输出边沿

/*************状态机执行状态定义（出错 ，设置完成 ，设置未完成）************/
typedef enum _MyFlagStatus { FLAG_ERROR = 0,FLAG_SET ,FLAG_RESET  } MyFlagStatus ;


/****************************状态机状态定义******************************/
typedef enum _StateDiagramStatusType
{	
   AdjustIdleStatus                  = 0 ,                                // 调节空闲状态，没有需要调节的数据                （ 对于电压输出表示为 ：电压输出空闲 ）
	 NoStartAdjustStatus               = 1 ,                                // 未开始调节状态 ，有数据需要调节但还没有开始调节 （ 对于电压输出表示为 ：电压还未输出 ）
	 StartAdjustStatus                 = 2 ,                                // 开始调节状态                                    （ 对于电压输出表示为 ：电压开始输出 ）
	 AdjustRunWaitStatus               = 3 ,                                // 调节等待状态 ，从开始调节到调节完成需要一段时间 （ 对于电压输出表示为 ：电压正在输出 ）
	 AdjustFinishStatus                = 4 ,	                              // 调节完成                                        （ 对于电压输出表示为 ：电压输出完成 ）
}StateDiagramStatusType;


/***********************各状态机调节是否就绪定义*************************/
typedef struct _StateDiagramFlagType
{
  MyFlagStatus  ControlFinishFlag ;                                       // 整个控制完成标志 
	MyFlagStatus  VoltageAmplitudeFlag ;                                    // 电压幅值调节是否完成 ，FLAG_SET: 调节完成 ，FLAG_RESET :调节未完成 ，FLAG_ERROR：调节出错
	MyFlagStatus  ChannelChangeFlag ;                                       // 输出通道切换是否完成 ，FLAG_SET: 调节完成 ，FLAG_RESET :调节未完成 ，FLAG_ERROR：调节出错
	MyFlagStatus  SignalTypeFlag    ;                                       // 信号类型切换是否完成 LAG_琒ET: 调节完成 ，FLAG_RESET :调节未完成 ，FLAG_ERROR：调节出错
	MyFlagStatus  OutputVoltageFlag ;                                       // 输出电压是否输出完成 LAG_琒ET: 输出完成 ，FLAG_RESET :输出未完成 ，FLAG_ERROR：调节出错
}StateDiagramFlagType ;

/***********************各设计参数与实际测量值***************************/
typedef struct _SystemParameterType 
{
	uint32_t StartAdjustTime ;                                              // 开始调节的时间 ，用于限制最大调节时间使用 
	uint16_t ActualInputVoltage ;                                           // 实际输入电压 ，已经放大100倍 ，即：1LSB = 0.01V
	uint16_t ActualOutputVoltage ;                                          // 实际测量电压 ，已经放大100倍 ，即：1LSB = 0.01V
	uint16_t SetOutputVoltage ;                                             // 设置输出电压 ，已经放大100倍 ，即：1LSB = 0.01V
	uint16_t SetPulseWidth ;                                                // 设置输出脉宽 , 范围： 10 - 60000 。单位为 ：ms
	uint16_t SetSelectChannel ;                                             // 设置输出通道数 ，范围 0 - 20 ， 0表示关闭上次通道
  uint16_t SetSignalTypes ;                                               // 设置信号类型 ， 有正电压、负电压、开关量3种信号类型
}	SystemParameterType;




/*电压调节 、通道调节 、信号类型调节、输出控制 输出完成标志位初始化为SET*/
StateDiagramFlagType StateDiagramFlag ={ FLAG_SET ,FLAG_SET, FLAG_SET , FLAG_SET , FLAG_SET  };// 状态机调节完成与否的标志位
SystemParameterType  SystemParameter  ;                                   // 设置参数和实际参数分配空间



/***************************用户子函数定义*******************************/
void MODBUS_SetSlaveAddress(void);                                                                 // MODBUS 从机地址设置                                                               
uint16_t PWM_SetOutputVoltageAmplitude(uint16_t SetVoltage);                                       // 设置的输出电压 与 PWM占空比设置 的关系 ,并设置了PWM输出占空比
ErrorStatus AdjustAmendOutputVoltage(int16_t SetOutputVoltage , int16_t ActualOutputVoltage);      // 调节修正输出电压 （ 反馈的控制算法 ）

void  StateDiagram_SetVoltageAmplitude(uint16_t SetVoltage, uint16_t ActualVoltage ,StateDiagramFlagType *StateDiagramFlag ) ;//设置电压幅值状态机 
void  StateDiagram_SetChannelChange(uint16_t Channel ,StateDiagramFlagType *StateDiagramFlag) ;    //设置通道状态机 
void  StateDiagram_SetSignalType(uint16_t SignalType ,StateDiagramFlagType *StateDiagramFlag) ;    //设置信号类型状态机 
void  StateDiagram_OutputVoltage(uint16_t PulseWidth_IN ,StateDiagramFlagType *StateDiagramFlag) ;    //输出电压控制状态机 
void  ActualOutputAndInputVoltageMeasure(SystemParameterType  *SystemParameter );                  //输入输出电压采样
void  MODBUS_ReadSystemParameter(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);             //将上位机发送的设置参数保存在相应的变量里
void  AdjustStatusError(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);//输出出错 z
void  ControlFinishFlag_FinishCondition(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);




/*************************************************************************
  * @brief  配置串口USART2工作模式
  * @param  Baudrate ： 波特率 
  * @retval 无 
  * @notice 无
*************************************************************************/
int main(void)
{  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                         // 设置系统中断优先级分组2
  Signal_OutputChannelGPIO_Init();                                        // 信号输出通道GPIO控制引脚初始化
  TIM4_PWM_Init();                                                        // 控制输出电压的PWM输出定时器初始化
  ADC3_Init(1000);                                                        // ADC3电压采集初始化 ，参数为：采样频率（1ms 触发一次扫描采样）
  MODBUS_Init(9600);                                                      // MODBUS通信波特率初始化 
// MODBUS_SetSlaveAddress();                                               // MODBUS从机地址设置
  MODBUS_AddrInit();                                                      // MODBUS地址上电初始化
  TIM5_PluseWidthAdjustInit();                                            // Signal输出脉冲定时器初始化 ，其步进为 ： 1MS
  TIM7_Init();                                                            // 定时器初始化 ，用于主函数查询定时 
  while(1)
  {    
    MODBUS_HandleFunction();                                              // 32us ,MODBUS协议总处理函数    （ 在接收到一帧数据 ，根据数据内容将数据内容保存在MODBUS相应的数据区里）	 
    MODBUS_ReadSystemParameter(&SystemParameter , &StateDiagramFlag);     // 判断是否接受到一帧数据 ，在收到一帧数据的情况下将上位机发送的数据保存到相应变量里，并设置相应状态机调节完成标志位 
    ActualOutputAndInputVoltageMeasure(&SystemParameter);                 // 实际输入输出电压测量 ，将测量值保存到 SystemParameter 中              
    AdjustStatusError(&SystemParameter ,&StateDiagramFlag);               // 错误处理 （电压太低 ，输出调节超时）
    ControlFinishFlag_FinishCondition(&SystemParameter ,&StateDiagramFlag);//z整个控制完成条件处理（即可以处理下一针数据的条件）
     
    StateDiagram_SetVoltageAmplitude(SystemParameter.SetOutputVoltage ,SystemParameter.ActualOutputVoltage,&StateDiagramFlag ) ; // 调节输出电压幅值状态机
    StateDiagram_SetChannelChange(SystemParameter.SetSelectChannel    ,&StateDiagramFlag) ;                                      // 调节输出电压通道状态机
    StateDiagram_SetSignalType(SystemParameter.SetSignalTypes ,&StateDiagramFlag) ;                                              // 调节输出电压类型状态机
    StateDiagram_OutputVoltage(SystemParameter.SetPulseWidth  ,&StateDiagramFlag) ;                                              // 电压输出情况状态机 
 }
}




/*************************************************************************
  * @brief  设置通道状态机 
  * @param  EnableSet ：使能通道切换
  *         Channel   ：需要切换的通道数 
  * @retval 无
  * @notice 无
*************************************************************************/
StateDiagramStatusType    ChannelStateDiagramStatus   = AdjustIdleStatus ;               //通道切换状态机变量定义 
void  StateDiagram_SetChannelChange(uint16_t Channel ,StateDiagramFlagType *StateDiagramFlag ) 
{
  static uint32_t StartAdjustTime ;                                                       //通道切换开始时间
  
  /**************出现调节错误的情况时，自动回到空闲状态**************/
  if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //调节出现错误
  {
    ChannelStateDiagramStatus   = AdjustIdleStatus ;                                      //状态机回到空闲状态
    StateDiagramFlag -> ChannelChangeFlag   = FLAG_ERROR ;                                //调节出错
  } 
  
  /*****************设置通道状态机 *****************/
  switch( ChannelStateDiagramStatus )
  {
    case  AdjustIdleStatus     : //没有需要调节的通道时的空闲状态
                                 if( StateDiagramFlag -> ChannelChangeFlag == FLAG_RESET )// 标志位 ，通道未调节完成
                                 {
                                   ChannelStateDiagramStatus = NoStartAdjustStatus ;      //切换到未开始调节状态				 
                                 }
                                 break ;                                             

    case  NoStartAdjustStatus  : //有需要调节的通道还未开始的状态 （ 开始调节前的准备工作 ）           
                                 MODBUS_WriteChannelStatus( NoReadyStatus );              // 通道未就绪
                                 if( Channel  > 20   )                                    // 选择通道数是否正确 ,通道数最大为20 
                                 {
                                   ChannelStateDiagramStatus   = AdjustIdleStatus  ;      // 选择通道错误 ，切换到调节空闲															 
                                   StateDiagramFlag -> ChannelChangeFlag   = FLAG_ERROR ; // 调节出错
                                 }
                                 else
                                 ChannelStateDiagramStatus   = StartAdjustStatus ;    // 通道数正确 ，切换到开始调节状态     
                                 break ;                                 

  case  StartAdjustStatus    : //开始调节通道 
                                 Signal_OutputChannelSelectOpen( Channel );               // 设置通道数  
                                 StartAdjustTime  = TIM7_ReadTimeCount();                 // 保存通道开始调节的时间  ,单位:ms ,最小步进为 TIM7_TimeTick
                                 ChannelStateDiagramStatus       = AdjustRunWaitStatus  ; // 切换到调节等待状态    
                                 break ;                                

  case  AdjustRunWaitStatus  : //调节等待状态 ，从开始调节到调节完成需要一段时间
                                 if(TIM7_ReadTimeCount() - StartAdjustTime  >=  RELAY_SWITCHOVER_TIMER  ) // 继电器切换完成 （延时RELAY_SWITCHOVER_TIMER ms的继电器切换时间 ）
                                 {
                                   ChannelStateDiagramStatus    =  AdjustFinishStatus  ; // 通道设置完成 ，切换到通道设置完成状态
                                 }
                                 break ;                           

  case  AdjustFinishStatus   : //通道调节完成 
                                 StateDiagramFlag -> ChannelChangeFlag   = FLAG_SET ;     // 标志位 ，通道调节完成
                                 MODBUS_WriteChannelStatus( ReadyStatus );                // 通道就绪
                                 ChannelStateDiagramStatus       =  AdjustIdleStatus   ;  // 通道设置完成 ，没有需要调节的通道 ，切换到调节空闲
                                 break ;                                                  // 调节完成  
                          
  default                    : ChannelStateDiagramStatus       =  AdjustIdleStatus   ;  // 程序跑飞 ，强制切换到调节空闲
  }
}



/*************************************************************************
  * @brief  设置信号类型状态机 
  * @param  StateDiagramFlag ：状态机标志
  *         SignalType：需要的信号类型
  * @retval 无
  * @notice 无
*************************************************************************/
StateDiagramStatusType  SignalTypeStateDiagramStatus  = AdjustIdleStatus ;               //信号类型状态机变量定义 
void StateDiagram_SetSignalType(uint16_t SignalType ,StateDiagramFlagType *StateDiagramFlag) 
{
  static uint32_t StartAdjustTime ;                                                       //信号类型切换开始时间
  static SignalTypeDef SignalTypeID ;
  
	/**************出现调节错误的情况时，自动回到空闲状态**************/
	if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //调节出现错误
	{
     SignalTypeStateDiagramStatus   = AdjustIdleStatus ;                                  //状态机回到空闲状态
     StateDiagramFlag -> SignalTypeFlag =   FLAG_ERROR ;
  }	

	/*****************设置信号类型状态机*****************/	
	switch( SignalTypeStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //没有需要的信号类型的空闲状态
			                           if(  StateDiagramFlag ->SignalTypeFlag  == FLAG_RESET  ) // 信号类型未调节完成
																 {
																   SignalTypeStateDiagramStatus = NoStartAdjustStatus ;   //切换到未开始调节状态
																 }		
			                           break ;                                             

		case  NoStartAdjustStatus  : //有需要调节的信号类型还未开始的状态 （ 开始调节前的准备工作 ）   
                                 SignalTypeID = Signal_MODBUSValueConvertToOutputType(SignalType) ;                       
			                           if( SignalTypeID  == ERROR_SIGNAL_TYPE  )                // 选择信号类型是否正确 
                                 {
                                   SignalTypeStateDiagramStatus = AdjustIdleStatus  ;     // 选择信号类型错误 ，切换到信号类型调节空闲状态
                                   StateDiagramFlag ->SignalTypeFlag  = FLAG_ERROR ;      // 错误输入 
                                 }
                                 else
																		 SignalTypeStateDiagramStatus = StartAdjustStatus ;   // 通道数正确 ，切换到开始调节状态     
		                             break ;                                 

		case  StartAdjustStatus    : //开始切换信号类型                                
			                           Signal_SetOutputType( SignalTypeID );
		                             StartAdjustTime  = TIM7_ReadTimeCount();                 // 保存通道开始调节的时间  ,单位:ms ,最小步进为 TIM7_TimeTick
                                 SignalTypeStateDiagramStatus     = AdjustRunWaitStatus  ;// 切换到调节等待状态    
			                           break ;                                

		case  AdjustRunWaitStatus  : //切换信号类型等待状态 ，从开始调节到切换完成需要一段时间
				                         if(TIM7_ReadTimeCount() - StartAdjustTime  >=  RELAY_SWITCHOVER_TIMER  ) // 继电器切换完成 （延时RELAY_SWITCHOVER_TIMER ms的继电器切换时间 ）
				                         {
															      SignalTypeStateDiagramStatus  =  AdjustFinishStatus  ;// 信号类型设置完成 ，切换到信号类型设置完成状态
																 }
			                           break ;                           

		case  AdjustFinishStatus   : //通道调节完成 
			                           StateDiagramFlag ->SignalTypeFlag  = FLAG_SET ;          // 信号类型调节完成
															   SignalTypeStateDiagramStatus     =  AdjustIdleStatus   ; // 通道设置完成 ，没有需要调节的通道 ，切换到调节空闲
			                           break ;                                                  // 调节完成  
 
		default                    : SignalTypeStateDiagramStatus     =  AdjustIdleStatus   ; // 程序跑飞 ，强制切换到调节空闲
	}
	
}



/*************************************************************************
  * @brief  设置电压幅值状态机 
  * @param  StateDiagramFlag    ：状态机标志
  *         ActualVoltage  ：实际的电压值
  *         SetVoltage     ：设置的电压值
  * @retval 返回电压幅值状态机的状态
  * @notice 无
*************************************************************************/
StateDiagramStatusType    VoltageStateDiagramStatus   = AdjustIdleStatus ;              //电压调节状态机变量定义 
void  StateDiagram_SetVoltageAmplitude( uint16_t SetVoltage, uint16_t ActualVoltage ,StateDiagramFlagType *StateDiagramFlag ) 
{
  static uint32_t StartAdjustTime ;                                                       //电压调节定时
  static uint16_t VoltageAdjustFinishCount = 0 ;   
	
	/**************出现调节错误的情况时，自动回到空闲状态**************/
	if(StateDiagramFlag -> ControlFinishFlag  == FLAG_ERROR )                               //调节出现错误
	{
     VoltageStateDiagramStatus   = AdjustIdleStatus ;                                     //状态机回到空闲状态
     StateDiagramFlag ->VoltageAmplitudeFlag  =   FLAG_ERROR ;
  }	

	
	/*****************设置幅值状态机*****************/		
	switch( VoltageStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //没有需要调节电压的空闲状态
			                           /**************判断是否允许设置电压************/
			                           if(  StateDiagramFlag ->VoltageAmplitudeFlag == FLAG_RESET  )    // 电压幅值未调节完成
																 {
																		VoltageStateDiagramStatus = NoStartAdjustStatus ;     //切换到未开始调节状态
																 } 
			                           break ;                                             

		case  NoStartAdjustStatus  : //电压还未开始调节
			                           /************开始调节前的准备工作**************/
	                             	 VoltageAdjustFinishCount  = 0 ; 
		                             MODBUS_WriteVoltageStatus( NoReadyStatus );              // 电压未就绪 
 
		                             /*************设置电压限幅判断***************/
			                           if( ( SetVoltage  > 5000 ) || ( SetVoltage  < 500 ))			// 电压设置是否正确   
																 {
                                 	   VoltageStateDiagramStatus   = AdjustIdleStatus  ;    // 电压设置错误 ，切换到调节空闲
		                                 StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_ERROR;// 设置有误 
                                 }
                                 else
                                 {
																	 	 VoltageStateDiagramStatus   = StartAdjustStatus ;    // 通道数正确 ，切换到开始调节状态     
		                             }
                                 break ;                                 

		case  StartAdjustStatus    : //开始调节电压
																 PWM_SetOutputVoltageAmplitude(SetVoltage) ;              // pwm 调节输出电压  
		                             StartAdjustTime  = TIM7_ReadTimeCount();                 // 保存电压开始调节的时间  ,单位:ms ,最小步进为 TIM7_TimeTick
 		                             VoltageStateDiagramStatus       = AdjustRunWaitStatus;   // 切换到调节等待中
			                           break ;                                

		case  AdjustRunWaitStatus  : //调节等待状态 ，一次不一定调节成功 ，加入反馈
			                           /************200ms调节一次电压******************/
                                 if(TIM7_ReadTimeCount() - StartAdjustTime  >=  200 )     // 200ms调节一次
																 {
																	 	 StartAdjustTime  = TIM7_ReadTimeCount();             // 保存时间  ,单位:ms ,最小步进为 TIM7_TimeTick
																	   VoltageAdjustFinishCount++ ;
//																		 /***************负反馈电压调节*************/
//																		 if(AdjustAmendOutputVoltage(SetVoltage ,ActualVoltage) == SUCCESS )//反馈调节
//																		 {
																			  VoltageStateDiagramStatus = AdjustFinishStatus;   // 电压调节完成 ，切换到调节完成
//																		 } else
																			 if( VoltageAdjustFinishCount > 30 )                // 6s超时检测
																			 {
																			     VoltageStateDiagramStatus   = AdjustIdleStatus;// 调节超时，切换到调节空闲
		                                       StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_ERROR ; // 设置有误 
																			 }
																 }
																 break ;                           

		case  AdjustFinishStatus   : //电压调节完成 
		                             StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_SET ;     // 电压幅值调节完成
																 MODBUS_WriteVoltageStatus(ReadyStatus );                 // 电压就绪  
															   VoltageStateDiagramStatus         = AdjustIdleStatus ;   // 切换到调节空闲状态
			                           break ;                                                          
 
		default                    : VoltageStateDiagramStatus         = AdjustIdleStatus ;   // 程序跑飞 ，强制切换到调节空闲
	}
	
}




/*************************************************************************
  * @brief  输出电压控制状态机 
  * @param  StateDiagramFlag：状态机标志
  *         ActualVoltage  ：实际的电压值
  * @retval 无
  * @notice  0XFFFF :表示输出脉宽无冲大 ，即：输出高低电平
*************************************************************************/
 StateDiagramStatusType  OutputStateDiagramStatus      = AdjustIdleStatus ;               //输出电压状态机变量定义 
void  StateDiagram_OutputVoltage( uint16_t PulseWidth_IN ,StateDiagramFlagType *StateDiagramFlag) 
{
  static uint16_t  PulseWidth ;

	/**************出现调节错误的情况时，自动回到空闲状态**************/
	if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //调节出现错误
	{
     OutputStateDiagramStatus  = AdjustIdleStatus ;                                       //状态机回到空闲状态
     StateDiagramFlag -> OutputVoltageFlag  =   FLAG_ERROR ;
  }	

	switch( OutputStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //没有需要输出的空闲时候
			                           /**************判断电压是否已经输出************/
                                 PulseWidth  = PulseWidth_IN ;
			                           if( StateDiagramFlag -> OutputVoltageFlag == FLAG_RESET) // 电压还未输出
																 {
 																	  MODBUS_WriteOutputStatus(NoStartOutputStatus);        // 未开始输出电压 
																		OutputStateDiagramStatus = NoStartAdjustStatus ;      // 切换到未开始调节状态 
																 }
			                           break ;                                             

		case  NoStartAdjustStatus  : //电压还未开始输出电压 ，电压还未调节完成 
		                             /*************设置电压、通道、信号类型是否调节完成***************/
		                             if(( StateDiagramFlag -> VoltageAmplitudeFlag == FLAG_SET )&&\
																	  ( StateDiagramFlag -> ChannelChangeFlag    == FLAG_SET )&&\
																	  ( StateDiagramFlag -> SignalTypeFlag       == FLAG_SET ))
                                 {
																   if ( PulseWidth    >= 10 && PulseWidth <= 60000 ) 
                                   {                                     
		                                 OutputStateDiagramStatus = StartAdjustStatus  ;      // 切换到开始输出电压 
                                   }
                                   else if( PulseWidth == HIGH_LEVEL_PULSE_WIDTH )        // 输出高低电平
                                   {
                                      FET_PluseWidthSignalOutputOpen();                   // 打开FET管
                                      MODBUS_WriteOutputStatus(NowRunOutputStatus);       // 置位标志位 ： 正在输出电压状态  
                                      StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;// 电压输出完成  
                                      OutputStateDiagramStatus  =  AdjustIdleStatus  ;    // 输出空闲，可以进行下一次数据的读取
                                   }
																	 else if(PulseWidth == EDGE_PULSE_WIDTH )
																	 {
																	 	  OutputStateDiagramStatus = StartAdjustStatus  ;     // 切换到开始输出电压 
                                      PulseWidth  = 2 ;                                   // 2ms脉宽表示边沿
																	 }
                                   else 
                                   {
                                      StateDiagramFlag -> OutputVoltageFlag = FLAG_ERROR ;// 数据输入有误
                                      OutputStateDiagramStatus  = AdjustIdleStatus ;      // 状态机回到空闲状态
                                   }
                                 }
		                             break ;                                 

		case  StartAdjustStatus    : //开始输出电压
                                 TIM5_SetOutputPluseWidth(PulseWidth);	                  // 设置Signal输出脉冲时间 ，单位 ：ms ，实际值比设置值大450us 
			                           MODBUS_WriteOutputStatus(NowRunOutputStatus);            // 置位标志位 ： 正在输出电压状态    
			                           OutputStateDiagramStatus = AdjustRunWaitStatus  ;        // 切换到电压正在输出状态 
		                             break ;                                

		case  AdjustRunWaitStatus  : //电压输出等待状态
				                         if( TIM5_OutputPluseWidthFinish() == SET)                // 检测电压脉冲是否输出完成
	                                  OutputStateDiagramStatus = AdjustFinishStatus  ;      // 电压输出完成 ，切换到信号输出完成状态                                       
																 break ;                           

		case  AdjustFinishStatus   : //电压输出完成
																 Delay1Us(500);                                           // FET 开关管的关断时间约为450us  
		                             StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET  ;    // 电压输出完成  
																 MODBUS_WriteOutputStatus(EndOutputStatus);               // 电压输出完成状态 
		                             OutputStateDiagramStatus  =  AdjustIdleStatus  ;         // 输出空闲，可以进行下一次数据的读取
			                           break ;                                                          
 
		default                    : OutputStateDiagramStatus  =  AdjustIdleStatus  ;         // 程序跑飞 ，强制切换到输出空闲
	}

	
}





/*************************************************************************
  * @brief  将上位机发送的设置参数保存在相应的变量里
  * @param  SystemParameter   ：需要设置的变量
  *         StateDiagramFlag  ：各状态机调节完成标志
  * @retval 无
  * @notice 进入函数前先检测上一次脉冲是否输出完成 ，在输出完成的条件下
  *         再检测是否有输出使能标志
*************************************************************************/
void MODBUS_ReadSystemParameter(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{
	static uint8_t OutputFlag =  0 ; 
  static SystemParameterType  SystemLastParameter = {0};                                  // 保存每40ms读取一次MODBUS的数据 

	/********在接收到输出信号后需要先关闭上次输出才能操作下一步*****/
	if ( ( OutputFlag == 1 ) && ( TIM7_ReadTimeCount() - SystemParameter  -> StartAdjustTime > 2  ) ) //给FET管的关断设置设置时间为2ms
	{
     OutputFlag =  0 ;
		 StateDiagramFlag ->ControlFinishFlag   = FLAG_RESET  ;                               // 总输出未完成 （相当于打开总输出开关）                    
	}
  
  /*************正在控制输出 ，不允许读取MODBUS数据***************/
  if( (StateDiagramFlag ->ControlFinishFlag   == FLAG_RESET  ) || (OutputFlag == 1 ) )		return ;     // 输出完成的条件下读取是否输出的状态
	
  /*************每 20ms 读取一次MODBUS数据保存到变量中************/
  if( TIM7_20MsFinish() != SET )   return ;
  
  /**********************保存上一次读取MIDBUS内存数据*************/
  (void)memcpy(&SystemLastParameter ,SystemParameter ,sizeof(SystemParameterType));       // 保存上一次的数据 
 
   /******************读取MODBUS内存数据保存到变量****************/
  SystemParameter -> SetOutputVoltage = MODBUS_ReadSetOutputVoltage() * 10;               // 读取设置的电压值
  SystemParameter -> SetSignalTypes   = MODBUS_ReadSetSignalTypes()  ;                    // 读取设置的信号类型
  SystemParameter -> SetPulseWidth    = MODBUS_ReadSetPulseWidth()   ;                    // 读取设置的输出信号脉宽
  SystemParameter -> SetSelectChannel = MODBUS_ReadSetSelectChannel();                    // 读取设置的输出通道 


  /*********************判断那些参数需要重新调节*****************/
  if(StateDiagramFlag -> ControlFinishFlag    == FLAG_ERROR)  
	{
		StateDiagramFlag -> VoltageAmplitudeFlag = FLAG_RESET ;                               // 电压幅值未调节好
		StateDiagramFlag -> ChannelChangeFlag    = FLAG_RESET ;                               // 通道未调节好
		StateDiagramFlag -> SignalTypeFlag       = FLAG_RESET ;                               // 信号类型未调节好
	}
	else
	{
		if( ( SystemParameter-> SetOutputVoltage !=  SystemLastParameter.SetOutputVoltage))// ||\
			 (abs( SystemParameter -> SetOutputVoltage - SystemParameter -> ActualOutputVoltage ) > 5 ))	// 输出电压有改动 或 实际输出电压和设置电压压差大于50mV
			StateDiagramFlag -> VoltageAmplitudeFlag = FLAG_RESET ;                                     // 电压幅值未调节好
	 
		if(( SystemParameter -> SetSelectChannel != SystemLastParameter.SetSelectChannel ))//||\
			(  Signal_ReadChannelSwitchStatus(SystemParameter -> SetSelectChannel) != SET ) )		// 通道有改动 或 通道未打开 
			StateDiagramFlag -> ChannelChangeFlag    = FLAG_RESET ;                             // 通道未调节好
	 
		if( SystemParameter-> SetSignalTypes  != SystemLastParameter.SetSignalTypes )         // 信号类型有改动
			StateDiagramFlag -> SignalTypeFlag       = FLAG_RESET ;                             // 信号类型未调节好
		
	}
	


  /************************读取输出使能端 ，为1表示输出*************************/
  if( MODBUS_ReadSwitchOutputStatus() == SET )                                            // 判断是否接收到输出使能位置1                              
  {
	
		/****************根据信号类型决定输出电压状态机的控制 ***********************/    
			switch(Signal_MODBUSValueConvertToOutputType(SystemParameter -> SetSignalTypes))
			{
				case POSITIVE_PULSE  :  /************正脉冲************/ 	
				case NEGATIVE_PULSE  :  /***********负脉冲*************/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // 信号还未输出 
																if( SystemParameter  -> SetPulseWidth   == HIGH_LEVEL_PULSE_WIDTH  ) 
																	 SystemParameter  -> SetPulseWidth    = HIGH_LEVEL_PULSE_WIDTH - 1 ;//防止和电平输出冲突，于是在此减1       
                                else  if( SystemParameter  -> SetPulseWidth   == EDGE_PULSE_WIDTH  )	
																	 SystemParameter  -> SetPulseWidth    = EDGE_PULSE_WIDTH  + 1 ;     //防止和边沿输出冲突，于是在此加1     
																	
																break ;
				case RISING          :  /***********上升沿*************/
				case FALLING         :  /***********下降沿*************/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // 信号还未输出 
																SystemParameter  -> SetPulseWidth       = EDGE_PULSE_WIDTH ;         // 上升和下降沿用最小的输出脉宽表示 ，在这里用 2MS表示
																break ;
				case POSITIVE_LEVEL  :  /************正电平************/                                         
				//case NEGATIVE_LEVEL  :  /***********负电平*************////////////by YZ
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // 信号还未输出   
																SystemParameter  -> SetPulseWidth       = HIGH_LEVEL_PULSE_WIDTH ;   // 表示输出的脉宽无穷大 
																break;      
				case SWITCH_ON       :  /************开关量(闭合)******/
				case SWITCH_OFF      :  /************开关量(断开)******/
				case NEGATIVE_LEVEL  :  /***********负电平*************///////////////by YZ
				
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;      // 不需要输出电压 ，设置电压已经完成 
																break;
				default:                /**********信号类型错误********/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;      // 不需要输出电压 ，设置电压已经完成 
																break;
			}
			
		  OutputFlag    =  1 ;                                                                // 接收到输出信号时，需先关上步输出才能操作下一步 ，OutputFlag 作为之间结束的标志
      SystemParameter  -> StartAdjustTime   = TIM7_ReadTimeCount();                       // 保存时间  ,单位:ms ,最小步进为 TIM7_TimeTick
      FET_PluseWidthSignalOutputClose();                                                  //

		}
}



/*************************************************************************
  * @brief  设置的输出电压 与 PWM占空比设置 的关系 ,并设置了PWM输出占空比
  * @param  SetVoltage ： 设置的输出电压（ 放大100倍 ）
  * @retval 返回PWM的占空比 （ 单位： 万分之一 ）
  * @notice 占空比单位 ：万分之一  ，电压的单位 ： 0.01V
*************************************************************************/
uint16_t PWM_SetOutputVoltageAmplitude(uint16_t SetVoltage)
{
	uint16_t PWM_SetDutyRatio;                                                              //PWM的占空比
	
	/****************输出电压限幅*****************/
	if( SetVoltage > 5275 ) SetVoltage = 5275 ;                                             //最大输出电压为52.75V : 5275 = 96741000 /  18337 
	if( SetVoltage < 150  ) SetVoltage = 150  ;                                             //最小输出电压为1.50V
	
	/****设置的输出电压 与 PWM占空比设置 的关系***/
	PWM_SetDutyRatio = (uint16_t)(( 96741000 -  18337 * SetVoltage ) / 10000 );             //电压与PWM占空比的关系  
	
 	/*****************设置PWM输出*****************/
  TIM4_PWM_SetDutyRatio(PWM_SetDutyRatio);                                                //配置PWM占空比 ，单位： 万分之一  
	return PWM_SetDutyRatio ;                                                               // 返回实际PWM输出占空比
}







/*************************************************************************
  * @brief  调节修正输出电压
  * @param  SetOutputVoltage    ： 设置的输出电压 （ 电压放大100倍 ）
  *         ActualOutputVoltage ： 实际输出电压   （ 电压放大100倍 ）
  * @retval 是否调节成功, ERROR ：未调节成功 ，等待下一次的调节
                        SUCCESS ：调节成功 
	* @notice 无
*************************************************************************/
ErrorStatus AdjustAmendOutputVoltage(int16_t SetOutputVoltage , int16_t ActualOutputVoltage)
{
	static uint16_t  ActualSetOutputVoltage  ;                                              // 通过修正后实际需要设置的输出电压值                               
	static int16_t  SetOutputVoltage2 = 0   ;                                               // 备份设置的输出电压值
	static int16_t  LastOutputVoltageError ;                                                // 上次实际电压和设置电压的差值 
  int16_t  OutputVoltageError;                                                            // 这次实际电压和设置电压的差值
	
	/***********保存设置电压值*****************/
	if(SetOutputVoltage2  != SetOutputVoltage  )                                            // 判断是否从新设置了输出电压
	{
		SetOutputVoltage2       =  SetOutputVoltage ;                                         // 备份这次设置的电压值
		ActualSetOutputVoltage  =  SetOutputVoltage ;                                         // 实际设置的输出电压先按理论值给，后面再调节
    LastOutputVoltageError	=  0 	;                                                       // 上次偏差归零
	}	

	 /***调节成功的标志为：连续两次电压在20mv内***/
  OutputVoltageError       =  ActualOutputVoltage    -  SetOutputVoltage ;                // 计算实测电压和设置电压的差值   
	if( ( OutputVoltageError < 2 ) && ( OutputVoltageError > -2  ) )  
	{
		if( ( LastOutputVoltageError < 2 ) && ( LastOutputVoltageError > -2  ) )  
		 return SUCCESS;                                                                      // 调解成功
	}

	/*******只有在压差在1v范围内才允许调节********/
	if( ( OutputVoltageError > 100 ) ||( OutputVoltageError < -100  ) ) return ERROR;       // 电压压差大于1V不允许调节
  
	/****************输出调节限幅*****************/
  if( OutputVoltageError > 10 )       OutputVoltageError = 10  ;                          // 一次最大调节100mV电压      
	else if( OutputVoltageError < -10 ) OutputVoltageError = -10 ;                          // 一次最大调节100mV电压   
 
	LastOutputVoltageError     =  OutputVoltageError ;                                      // 保存这次电压差值                            
  ActualSetOutputVoltage   =  ActualSetOutputVoltage -  OutputVoltageError ;              // 计算实际设置的输出电压值
	PWM_SetOutputVoltageAmplitude( ActualSetOutputVoltage) ;                                // PWM电压调节输出
	return ERROR ;
}




/*************************************************************************
  * @brief  对实际输入输出电压进行采样计算，得到实际的输入输出电压值 ，电压单位 ： 1LSB = 0.01 V
  * @param  SystemParameter    ：实际电压的保存位置
  * @retval 无
	* @notice 无
*************************************************************************/
void  ActualOutputAndInputVoltageMeasure(SystemParameterType  *SystemParameter )
{
  if(TIM7_50MsFinish() == SET )       //每50ms更新一次实际输出电压
	{
		SystemParameter -> ActualOutputVoltage   = ADC3_SampleOutputVoltageValue();           //输出电压
	  MODBUS_WriteActualOutputVoltage (SystemParameter -> ActualOutputVoltage);
	}
	if(TIM7_200MsFinish() == SET )      //每200Ms钟跟新一次输入电压的值
	{ 
		SystemParameter -> ActualInputVoltage    =  ADC3_SampleInputVoltageValue();           // 输入电压
	}
}


/*************************************************************************
  * @brief  出现调节出错的情况 ，如电压一直无法调节成功 ，输入电压太低
  * @param  SystemParameter    ：系统参数
  *         StateDiagramFlag   ：系统标志
  * @retval 无
	* @notice 无
*************************************************************************/
void AdjustStatusError(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{	
	/************ 输入电压太低 或 调节超时( 1S 检测一次 ) **********/ 
	if( TIM7_1sFinish() == SET )
	{
		if((SystemParameter -> ActualInputVoltage - SystemParameter -> SetOutputVoltage < 300 )||\
			 (SystemParameter -> ActualInputVoltage < 1000 ))                                   // 输入电压太低( (输入输出压差3V)||(输入电压小于10V)  )                  
		{
			StateDiagramFlag -> ControlFinishFlag    = FLAG_ERROR ;                             // 调节出错                                  
		}
		
	}
	
	/*************控制完成条件（错误情况）********************/
	if( StateDiagramFlag -> ControlFinishFlag  != FLAG_SET  )
	if(( StateDiagramFlag -> VoltageAmplitudeFlag == FLAG_ERROR  ) || ( StateDiagramFlag -> ChannelChangeFlag == FLAG_ERROR  ) ||\
	  	(StateDiagramFlag -> SignalTypeFlag       == FLAG_ERROR  ) || ( StateDiagramFlag -> OutputVoltageFlag == FLAG_ERROR  ))  
	{
     StateDiagramFlag -> ControlFinishFlag  = FLAG_SET ;                                  // 设置有误 ，不做任何处理 ，等待处理下一针数据                       {
     MODBUS_WriteSwitchOutputStatus(RESET);                                               // 写0 ，表示输出完成，可以处理下一阵数据 
	}	
}







/*************************************************************************
  * @brief  正常通信的情况下 ，控制完成条件（正常情况）
  * @param  SystemParameter    ：系统参数
  *         StateDiagramFlag   ：系统标志
  * @retval 无
	* @notice 无
*************************************************************************/
void ControlFinishFlag_FinishCondition(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{
	/***************只有在正在调节输出才检测调节是否完成****************/
	if( StateDiagramFlag -> ControlFinishFlag  != FLAG_RESET ) return ;

  /***************输出不同信号类型，控制完成条件不同*******************/	
  switch(Signal_MODBUSValueConvertToOutputType(SystemParameter -> SetSignalTypes))
  {
    case POSITIVE_PULSE  :  /***********正脉冲*************/ 	
    case NEGATIVE_PULSE  :  /***********负脉冲*************/
		case RISING          :  /***********上升沿*************/
	  case FALLING         :  /***********下降沿*************/
    case POSITIVE_LEVEL  :  /************正电平************/                                         
    //case NEGATIVE_LEVEL  :  /***********负电平*************/////////////////////////by YZ
                            if(StateDiagramFlag -> OutputVoltageFlag == FLAG_SET )        // 电压输出完成 
                              StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;  
                            break;      
    case SWITCH_ON       :  /************开关量(闭合)******/
    case SWITCH_OFF      :  /************开关量(断开)******/
		case NEGATIVE_LEVEL  :  /***********负电平*************////////////////////////////////by YZ

		
                          if(( StateDiagramFlag -> SignalTypeFlag == FLAG_SET )&&\
                               ( StateDiagramFlag -> ChannelChangeFlag == FLAG_SET ))     // 信号类型 和 通道完成 
                               StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;  

                            break;
    default:                /**********信号类型错误********/
                            StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;          // 不需要输出电压 ，设置电压已经完成 
                            break;
  }
  
	/******************判断控制是否完成****************/
  if( StateDiagramFlag -> ControlFinishFlag  != FLAG_RESET )                              //总控制完成(或调节出错)的条件下写MODBUS可以接收下一帧数据的标志                        
  {
     MODBUS_WriteSwitchOutputStatus(RESET);                                               // 写0 ，表示输出完成，可以处理下一阵数据 
  }

}




/*************************************************************************
  * @brief  MODBUS 从机地址设置 ，通过拨码开关的值设置MODBUS地址 
  * @param  无
  * @retval 无
	* @notice 无
*************************************************************************/
void MODBUS_SetSlaveAddress(void)
{
	uint8_t KeyValue ;                                                                      // 拨码开关按键值临时保存区
  KeyValue = ToggleSwitch_ReadKeyValue();                                                 // 读取拨码开关按键值
  MODBUS_ChangeSalveID(KeyValue);                                                         // 设置MODBUS从机地址      
}






