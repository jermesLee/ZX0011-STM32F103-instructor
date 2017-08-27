#include "SignalChannelSwitch.h"

/************************通道切换GPIO 内部函数****************************/
static void Signal_OutputChannel_GPIO_Config(void);                       //通道切换和信号类型切换的引脚GPIO配置                         
static void Signal_CloseAllOutputChannel(void);                           //关闭所有输出通道

/************************通道切换GPIO 用户函数****************************/
void Signal_OutputChannelGPIO_Init(void);                                 //信号输出通道控制的GPIO引脚初始化
void Signal_OutputChannelSelectClose( uint16_t Channel );                 //关继电器通道
void Signal_OutputChannelSelectOpen( uint16_t Channel ) ;                 //打开继电器通道 
FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel);              //读取通道开关状态
void Signal_SetOutputType(SignalTypeDef SignalType);                      //输出信号类型控制 
SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value );    //int型数据与信号类型的对应关系 


/*************************通道切换 1-20 的GPIO定义************************/
OutputChannelType const OutputChannel[20] =
{
    {GPIOD,GPIO_Pin_7 },{GPIOD,GPIO_Pin_6 },{GPIOD,GPIO_Pin_5 },{GPIOD,GPIO_Pin_4 },{GPIOD,GPIO_Pin_3 },       //通道 01，02，03，04，05
    {GPIOD,GPIO_Pin_2 },{GPIOD,GPIO_Pin_1 },{GPIOD,GPIO_Pin_15},{GPIOD,GPIO_Pin_14},{GPIOD,GPIO_Pin_13},       //通道 06，07，08，09，10
    {GPIOD,GPIO_Pin_12},{GPIOD,GPIO_Pin_11},{GPIOD,GPIO_Pin_10},{GPIOD,GPIO_Pin_9 },{GPIOE,GPIO_Pin_15},       //通道 11，12，13，14，15
    {GPIOE,GPIO_Pin_14},{GPIOE,GPIO_Pin_13},{GPIOE,GPIO_Pin_12},{GPIOE,GPIO_Pin_11},{GPIOE,GPIO_Pin_10}        //通道 16，17，18，19，20
};




/*************************************************************************
  * @brief  通道切换和信号类型切换的引脚GPIO配置        
  * @param  无
  * @retval 无
  * @notice 其中GPIO8 和 GPIO9 用于信号类型的控制 
*************************************************************************/
static void Signal_OutputChannel_GPIO_Config(void)
{		
    GPIO_InitTypeDef GPIO_InitStructure;		                              //定义一个GPIO_InitTypeDef类型的结构体

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //设置引脚模式为通用推挽输出	 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //设置引脚速率为2MHz 
    GPIO_InitStructure.GPIO_Pin   = (uint16_t)0xFEFF;	                    //GPIOD1 - GPIOD7 ,GPIOD9 - GPIOD15 ( GPIOD0 用于控制·FET的输出 )												   	
    GPIO_Init(GPIOD, &GPIO_InitStructure);          	                    //调用库函数初始化

    GPIO_InitStructure.GPIO_Pin   = (uint16_t)0xFF00;	                    //GPIOE8 - GPIOE15 ( 其中GPIOE8 和 GPIOE9 用于信号类型的控制 )												   	
    GPIO_Init(GPIOE, &GPIO_InitStructure);          	                    //调用库函数初始化

    FET_PluseWidthSignalOutputClose();                                    //初始化引脚，通电关闭信号的输出
    Signal_OutputChannelClose(GPIOD,  (uint16_t)0xFEFE);	                //初始化引脚，通电关闭通道输出
    Signal_OutputChannelClose(GPIOE,  (uint16_t)0xFF00);	                //初始化引脚，通电关闭通道输出
}


/*************************************************************************
  * @brief  信号输出通道GPIO初始化总函数      
  * @param  无 
  * @retval 无
  * @notice 无 
*************************************************************************/
void Signal_OutputChannelGPIO_Init(void)
{
	/**********GPIO时钟配置*************/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD |  RCC_APB2Periph_GPIOE , ENABLE);//打开GPIO时钟 

	/**********GPIO硬件配置*************/
	Signal_OutputChannel_GPIO_Config();                                     //通道切换和信号类型切换的引脚GPIO配置  
}


                                  

/*************************************************************************
  * @brief  关闭所用输出通道    
  * @param  无 
  * @retval 无
  * @notice 无 
*************************************************************************/
void Signal_CloseAllOutputChannel(void)
{
	uint8_t i ;                                                             //计数
	for( i = 0 ; i < 20 ; i++ )                                             //需要关闭的20个通道计数
	{
		Signal_OutputChannelClose(OutputChannel[i].GPIOx, OutputChannel[i].GPIO_Pin);	//关闭通道
	}
}


/*************************************************************************
  * @brief  读取通道开关状态    
  * @param  无 
  * @retval 无
  * @notice 无 
*************************************************************************/
FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel)
{
	uint8_t Status ;
	if( Channel == 0 ) return RESET ;
	Status = Signal_ReadChannelStatus(OutputChannel[Channel-1].GPIOx, OutputChannel[Channel-1].GPIO_Pin);	//关闭通道
    return ( (FlagStatus)Status );
}



/*************************************************************************
  * @brief  上位机定义的=数值和信号类型的对应关系   
  * @param  Value ：接收到的上位机信号类型编号 ： 正脉冲 ，负脉冲 ，开关量 
  * @retval 将输入放的信号类型=编号转换为枚举的信号类型
  * @notice 当输入数据 Value输入错误的时候 ，函数返回  ERROR_SIGNAL_TYPE ：信号类型错误 
*************************************************************************/
SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value )
{
    SignalTypeDef   SignalType ;
	
	/********数值与信号类型的对应关系 ************/
	switch( Value )
	{
		case 1 :  /************正脉冲************/                                         
                  SignalType = POSITIVE_PULSE ;  break ;
		case 2 :  /***********负脉冲*************/
                  SignalType = NEGATIVE_PULSE ;  break ;
		case 3 :  /************正电平************/                                         
                  SignalType = POSITIVE_LEVEL ;  break ;
		case 4 :  /***********负电平*************/
                  SignalType = NEGATIVE_LEVEL ;  break ;
		case 5 :  /***********上升沿*************/
			      SignalType = RISING  ;         break ;
		case 6 :  /***********下降沿*************/
			      SignalType = FALLING  ;        break ;
		case 7 :  /*********开关量(断开)*********/
                  SignalType = SWITCH_OFF ;      break ;
		case 8 :  /********开关量(闭合)**********/
                  SignalType = SWITCH_ON  ;      break ;
    		
		default : /********信号类型错误**********/
			      SignalType = ERROR_SIGNAL_TYPE;
	}
    return SignalType ;
}


/*************************************************************************
  * @brief  对于单片机接收到信号类型的实际操作 ，设置输出信号的信号类型  
  * @param  SignalType ： 输出信号类型 
  * @retval 无
  * @notice 无
*************************************************************************/
void Signal_SetOutputType( SignalTypeDef SignalType )
{
	
	/***************信号类型的实际操作*******************/
	switch( SignalType )	
	{
		case POSITIVE_PULSE  :  /************正脉冲************/ 
		case POSITIVE_LEVEL  :  /************正电平************/                                         
		case RISING          :  /***********上升沿*************/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;		
                                break;	
		case NEGATIVE_PULSE  :  /***********负脉冲*************/
		//case NEGATIVE_LEVEL  :  /***********负电平*************///////////////by YZ
	  case FALLING         :  /***********下降沿*************/ 
								GPIO_SetBits(    GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
		case SWITCH_ON       :  /************开关量(闭合)******/
		case NEGATIVE_LEVEL  :  /***********负电平*************/		//////////////by YZ				
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_SetBits(    GPIOE, GPIO_Pin_8) ;
                                break;
		case SWITCH_OFF      :  /************开关量(断开)******/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
		default:                /**********信号类型错误************/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
	}
	
	
}











/************************************************************************
  * @brief  信号输出通道打开   
  * @param  无
  * @retval 无
  * @notice 在打开需要打开的通道的同时也关闭了上一通道 ，当 Channel = 0 时，
  *         只关闭上一通道，不开通道 
************************************************************************/
void Signal_OutputChannelSelectOpen( uint16_t Channel )                   //打开继电器通道
{	
	static uint16_t LastChannel = 0 ;                                       //通电后所有通道关闭
	
	if(( LastChannel != Channel )&&( LastChannel != 0 ))                    //这次打开的电压不等于上一次电压时关闭上一次通道 
		Signal_OutputChannelClose(  OutputChannel[ LastChannel - 1 ].GPIOx, OutputChannel[ LastChannel - 1 ].GPIO_Pin) ;	//关闭上一输出信号的通道
  
	LastChannel = Channel ;                                                 //保存通道数

	if( Channel  != 0 )                                                     // 当不是关闭所有的通道时
		Signal_OutputChannelOpen(  OutputChannel[Channel - 1].GPIOx, OutputChannel[Channel- 1].GPIO_Pin) ;	 // 打开需要输出信号的通道
    else
		Signal_CloseAllOutputChannel();                                       //关闭所有通道
}


/*************************************************************************
  * @brief  信号输出通道关     
  * @param  无
  * @retval 无
  * @notice 通道数最大为20
************************ ************************************************/
void Signal_OutputChannelSelectClose( uint16_t Channel )                 //关继电器通道
{	
	if( Channel  > 20 ) return ;                                           // 通道数错误 
		
	if( Channel  != 0 )                                                    // 当不是关闭所有的通道时
	{
		Channel = Channel - 1 ;
		Signal_OutputChannelClose(  OutputChannel[Channel].GPIOx, OutputChannel[Channel].GPIO_Pin) ;	        // 打开需要输出信号的通道
	}

}

/******************************** FILE END ******************************/


