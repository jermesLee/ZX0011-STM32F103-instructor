#include "SignalChannelSwitch.h"

/************************ͨ���л�GPIO �ڲ�����****************************/
static void Signal_OutputChannel_GPIO_Config(void);                       //ͨ���л����ź������л�������GPIO����                         
static void Signal_CloseAllOutputChannel(void);                           //�ر��������ͨ��

/************************ͨ���л�GPIO �û�����****************************/
void Signal_OutputChannelGPIO_Init(void);                                 //�ź����ͨ�����Ƶ�GPIO���ų�ʼ��
void Signal_OutputChannelSelectClose( uint16_t Channel );                 //�ؼ̵���ͨ��
void Signal_OutputChannelSelectOpen( uint16_t Channel ) ;                 //�򿪼̵���ͨ�� 
FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel);              //��ȡͨ������״̬
void Signal_SetOutputType(SignalTypeDef SignalType);                      //����ź����Ϳ��� 
SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value );    //int���������ź����͵Ķ�Ӧ��ϵ 


/*************************ͨ���л� 1-20 ��GPIO����************************/
OutputChannelType const OutputChannel[20] =
{
    {GPIOD,GPIO_Pin_7 },{GPIOD,GPIO_Pin_6 },{GPIOD,GPIO_Pin_5 },{GPIOD,GPIO_Pin_4 },{GPIOD,GPIO_Pin_3 },       //ͨ�� 01��02��03��04��05
    {GPIOD,GPIO_Pin_2 },{GPIOD,GPIO_Pin_1 },{GPIOD,GPIO_Pin_15},{GPIOD,GPIO_Pin_14},{GPIOD,GPIO_Pin_13},       //ͨ�� 06��07��08��09��10
    {GPIOD,GPIO_Pin_12},{GPIOD,GPIO_Pin_11},{GPIOD,GPIO_Pin_10},{GPIOD,GPIO_Pin_9 },{GPIOE,GPIO_Pin_15},       //ͨ�� 11��12��13��14��15
    {GPIOE,GPIO_Pin_14},{GPIOE,GPIO_Pin_13},{GPIOE,GPIO_Pin_12},{GPIOE,GPIO_Pin_11},{GPIOE,GPIO_Pin_10}        //ͨ�� 16��17��18��19��20
};




/*************************************************************************
  * @brief  ͨ���л����ź������л�������GPIO����        
  * @param  ��
  * @retval ��
  * @notice ����GPIO8 �� GPIO9 �����ź����͵Ŀ��� 
*************************************************************************/
static void Signal_OutputChannel_GPIO_Config(void)
{		
    GPIO_InitTypeDef GPIO_InitStructure;		                              //����һ��GPIO_InitTypeDef���͵Ľṹ��

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;                     //��������ģʽΪͨ���������	 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;                      //������������Ϊ2MHz 
    GPIO_InitStructure.GPIO_Pin   = (uint16_t)0xFEFF;	                    //GPIOD1 - GPIOD7 ,GPIOD9 - GPIOD15 ( GPIOD0 ���ڿ��ơ�FET����� )												   	
    GPIO_Init(GPIOD, &GPIO_InitStructure);          	                    //���ÿ⺯����ʼ��

    GPIO_InitStructure.GPIO_Pin   = (uint16_t)0xFF00;	                    //GPIOE8 - GPIOE15 ( ����GPIOE8 �� GPIOE9 �����ź����͵Ŀ��� )												   	
    GPIO_Init(GPIOE, &GPIO_InitStructure);          	                    //���ÿ⺯����ʼ��

    FET_PluseWidthSignalOutputClose();                                    //��ʼ�����ţ�ͨ��ر��źŵ����
    Signal_OutputChannelClose(GPIOD,  (uint16_t)0xFEFE);	                //��ʼ�����ţ�ͨ��ر�ͨ�����
    Signal_OutputChannelClose(GPIOE,  (uint16_t)0xFF00);	                //��ʼ�����ţ�ͨ��ر�ͨ�����
}


/*************************************************************************
  * @brief  �ź����ͨ��GPIO��ʼ���ܺ���      
  * @param  �� 
  * @retval ��
  * @notice �� 
*************************************************************************/
void Signal_OutputChannelGPIO_Init(void)
{
	/**********GPIOʱ������*************/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD |  RCC_APB2Periph_GPIOE , ENABLE);//��GPIOʱ�� 

	/**********GPIOӲ������*************/
	Signal_OutputChannel_GPIO_Config();                                     //ͨ���л����ź������л�������GPIO����  
}


                                  

/*************************************************************************
  * @brief  �ر��������ͨ��    
  * @param  �� 
  * @retval ��
  * @notice �� 
*************************************************************************/
void Signal_CloseAllOutputChannel(void)
{
	uint8_t i ;                                                             //����
	for( i = 0 ; i < 20 ; i++ )                                             //��Ҫ�رյ�20��ͨ������
	{
		Signal_OutputChannelClose(OutputChannel[i].GPIOx, OutputChannel[i].GPIO_Pin);	//�ر�ͨ��
	}
}


/*************************************************************************
  * @brief  ��ȡͨ������״̬    
  * @param  �� 
  * @retval ��
  * @notice �� 
*************************************************************************/
FlagStatus Signal_ReadChannelSwitchStatus(uint16_t Channel)
{
	uint8_t Status ;
	if( Channel == 0 ) return RESET ;
	Status = Signal_ReadChannelStatus(OutputChannel[Channel-1].GPIOx, OutputChannel[Channel-1].GPIO_Pin);	//�ر�ͨ��
    return ( (FlagStatus)Status );
}



/*************************************************************************
  * @brief  ��λ�������=��ֵ���ź����͵Ķ�Ӧ��ϵ   
  * @param  Value �����յ�����λ���ź����ͱ�� �� ������ �������� �������� 
  * @retval ������ŵ��ź�����=���ת��Ϊö�ٵ��ź�����
  * @notice ���������� Value��������ʱ�� ����������  ERROR_SIGNAL_TYPE ���ź����ʹ��� 
*************************************************************************/
SignalTypeDef  Signal_MODBUSValueConvertToOutputType(uint16_t Value )
{
    SignalTypeDef   SignalType ;
	
	/********��ֵ���ź����͵Ķ�Ӧ��ϵ ************/
	switch( Value )
	{
		case 1 :  /************������************/                                         
                  SignalType = POSITIVE_PULSE ;  break ;
		case 2 :  /***********������*************/
                  SignalType = NEGATIVE_PULSE ;  break ;
		case 3 :  /************����ƽ************/                                         
                  SignalType = POSITIVE_LEVEL ;  break ;
		case 4 :  /***********����ƽ*************/
                  SignalType = NEGATIVE_LEVEL ;  break ;
		case 5 :  /***********������*************/
			      SignalType = RISING  ;         break ;
		case 6 :  /***********�½���*************/
			      SignalType = FALLING  ;        break ;
		case 7 :  /*********������(�Ͽ�)*********/
                  SignalType = SWITCH_OFF ;      break ;
		case 8 :  /********������(�պ�)**********/
                  SignalType = SWITCH_ON  ;      break ;
    		
		default : /********�ź����ʹ���**********/
			      SignalType = ERROR_SIGNAL_TYPE;
	}
    return SignalType ;
}


/*************************************************************************
  * @brief  ���ڵ�Ƭ�����յ��ź����͵�ʵ�ʲ��� ����������źŵ��ź�����  
  * @param  SignalType �� ����ź����� 
  * @retval ��
  * @notice ��
*************************************************************************/
void Signal_SetOutputType( SignalTypeDef SignalType )
{
	
	/***************�ź����͵�ʵ�ʲ���*******************/
	switch( SignalType )	
	{
		case POSITIVE_PULSE  :  /************������************/ 
		case POSITIVE_LEVEL  :  /************����ƽ************/                                         
		case RISING          :  /***********������*************/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;		
                                break;	
		case NEGATIVE_PULSE  :  /***********������*************/
		//case NEGATIVE_LEVEL  :  /***********����ƽ*************///////////////by YZ
	  case FALLING         :  /***********�½���*************/ 
								GPIO_SetBits(    GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
		case SWITCH_ON       :  /************������(�պ�)******/
		case NEGATIVE_LEVEL  :  /***********����ƽ*************/		//////////////by YZ				
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_SetBits(    GPIOE, GPIO_Pin_8) ;
                                break;
		case SWITCH_OFF      :  /************������(�Ͽ�)******/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
		default:                /**********�ź����ʹ���************/
								GPIO_ResetBits(  GPIOE, GPIO_Pin_9) ;
								GPIO_ResetBits(  GPIOE, GPIO_Pin_8) ;
		                        break;
	}
	
	
}











/************************************************************************
  * @brief  �ź����ͨ����   
  * @param  ��
  * @retval ��
  * @notice �ڴ���Ҫ�򿪵�ͨ����ͬʱҲ�ر�����һͨ�� ���� Channel = 0 ʱ��
  *         ֻ�ر���һͨ��������ͨ�� 
************************************************************************/
void Signal_OutputChannelSelectOpen( uint16_t Channel )                   //�򿪼̵���ͨ��
{	
	static uint16_t LastChannel = 0 ;                                       //ͨ�������ͨ���ر�
	
	if(( LastChannel != Channel )&&( LastChannel != 0 ))                    //��δ򿪵ĵ�ѹ��������һ�ε�ѹʱ�ر���һ��ͨ�� 
		Signal_OutputChannelClose(  OutputChannel[ LastChannel - 1 ].GPIOx, OutputChannel[ LastChannel - 1 ].GPIO_Pin) ;	//�ر���һ����źŵ�ͨ��
  
	LastChannel = Channel ;                                                 //����ͨ����

	if( Channel  != 0 )                                                     // �����ǹر����е�ͨ��ʱ
		Signal_OutputChannelOpen(  OutputChannel[Channel - 1].GPIOx, OutputChannel[Channel- 1].GPIO_Pin) ;	 // ����Ҫ����źŵ�ͨ��
    else
		Signal_CloseAllOutputChannel();                                       //�ر�����ͨ��
}


/*************************************************************************
  * @brief  �ź����ͨ����     
  * @param  ��
  * @retval ��
  * @notice ͨ�������Ϊ20
************************ ************************************************/
void Signal_OutputChannelSelectClose( uint16_t Channel )                 //�ؼ̵���ͨ��
{	
	if( Channel  > 20 ) return ;                                           // ͨ�������� 
		
	if( Channel  != 0 )                                                    // �����ǹر����е�ͨ��ʱ
	{
		Channel = Channel - 1 ;
		Signal_OutputChannelClose(  OutputChannel[Channel].GPIOx, OutputChannel[Channel].GPIO_Pin) ;	        // ����Ҫ����źŵ�ͨ��
	}

}

/******************************** FILE END ******************************/


