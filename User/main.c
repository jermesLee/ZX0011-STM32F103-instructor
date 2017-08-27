#include "includes.h"

/*****************************�궨��************************************/
#define RELAY_SWITCHOVER_TIMER      20                                    // �̵����л�ʱ�䣨 ��λ �� ms��
#define HIGH_LEVEL_PULSE_WIDTH      ((uint16_t)0XFFFF )                   // ��ʾ��������������ı�־���ߵ�ƽ��
#define EDGE_PULSE_WIDTH            0                                     // ��ʾ�������

/*************״̬��ִ��״̬���壨���� ��������� ������δ��ɣ�************/
typedef enum _MyFlagStatus { FLAG_ERROR = 0,FLAG_SET ,FLAG_RESET  } MyFlagStatus ;


/****************************״̬��״̬����******************************/
typedef enum _StateDiagramStatusType
{	
   AdjustIdleStatus                  = 0 ,                                // ���ڿ���״̬��û����Ҫ���ڵ�����                �� ���ڵ�ѹ�����ʾΪ ����ѹ������� ��
	 NoStartAdjustStatus               = 1 ,                                // δ��ʼ����״̬ ����������Ҫ���ڵ���û�п�ʼ���� �� ���ڵ�ѹ�����ʾΪ ����ѹ��δ��� ��
	 StartAdjustStatus                 = 2 ,                                // ��ʼ����״̬                                    �� ���ڵ�ѹ�����ʾΪ ����ѹ��ʼ��� ��
	 AdjustRunWaitStatus               = 3 ,                                // ���ڵȴ�״̬ ���ӿ�ʼ���ڵ����������Ҫһ��ʱ�� �� ���ڵ�ѹ�����ʾΪ ����ѹ������� ��
	 AdjustFinishStatus                = 4 ,	                              // �������                                        �� ���ڵ�ѹ�����ʾΪ ����ѹ������ ��
}StateDiagramStatusType;


/***********************��״̬�������Ƿ��������*************************/
typedef struct _StateDiagramFlagType
{
  MyFlagStatus  ControlFinishFlag ;                                       // ����������ɱ�־ 
	MyFlagStatus  VoltageAmplitudeFlag ;                                    // ��ѹ��ֵ�����Ƿ���� ��FLAG_SET: ������� ��FLAG_RESET :����δ��� ��FLAG_ERROR�����ڳ���
	MyFlagStatus  ChannelChangeFlag ;                                       // ���ͨ���л��Ƿ���� ��FLAG_SET: ������� ��FLAG_RESET :����δ��� ��FLAG_ERROR�����ڳ���
	MyFlagStatus  SignalTypeFlag    ;                                       // �ź������л��Ƿ���� �FLAG_�SET: ������� ��FLAG_RESET :����δ��� ��FLAG_ERROR�����ڳ���
	MyFlagStatus  OutputVoltageFlag ;                                       // �����ѹ�Ƿ������� �FLAG_�SET: ������ ��FLAG_RESET :���δ��� ��FLAG_ERROR�����ڳ���
}StateDiagramFlagType ;

/***********************����Ʋ�����ʵ�ʲ���ֵ***************************/
typedef struct _SystemParameterType 
{
	uint32_t StartAdjustTime ;                                              // ��ʼ���ڵ�ʱ�� ����������������ʱ��ʹ�� 
	uint16_t ActualInputVoltage ;                                           // ʵ�������ѹ ���Ѿ��Ŵ�100�� ������1LSB = 0.01V
	uint16_t ActualOutputVoltage ;                                          // ʵ�ʲ�����ѹ ���Ѿ��Ŵ�100�� ������1LSB = 0.01V
	uint16_t SetOutputVoltage ;                                             // ���������ѹ ���Ѿ��Ŵ�100�� ������1LSB = 0.01V
	uint16_t SetPulseWidth ;                                                // ����������� , ��Χ�� 10 - 60000 ����λΪ ��ms
	uint16_t SetSelectChannel ;                                             // �������ͨ���� ����Χ 0 - 20 �� 0��ʾ�ر��ϴ�ͨ��
  uint16_t SetSignalTypes ;                                               // �����ź����� �� ������ѹ������ѹ��������3���ź�����
}	SystemParameterType;




/*��ѹ���� ��ͨ������ ���ź����͵��ڡ�������� �����ɱ�־λ��ʼ��ΪSET*/
StateDiagramFlagType StateDiagramFlag ={ FLAG_SET ,FLAG_SET, FLAG_SET , FLAG_SET , FLAG_SET  };// ״̬������������ı�־λ
SystemParameterType  SystemParameter  ;                                   // ���ò�����ʵ�ʲ�������ռ�



/***************************�û��Ӻ�������*******************************/
void MODBUS_SetSlaveAddress(void);                                                                 // MODBUS �ӻ���ַ����                                                               
uint16_t PWM_SetOutputVoltageAmplitude(uint16_t SetVoltage);                                       // ���õ������ѹ �� PWMռ�ձ����� �Ĺ�ϵ ,��������PWM���ռ�ձ�
ErrorStatus AdjustAmendOutputVoltage(int16_t SetOutputVoltage , int16_t ActualOutputVoltage);      // �������������ѹ �� �����Ŀ����㷨 ��

void  StateDiagram_SetVoltageAmplitude(uint16_t SetVoltage, uint16_t ActualVoltage ,StateDiagramFlagType *StateDiagramFlag ) ;//���õ�ѹ��ֵ״̬�� 
void  StateDiagram_SetChannelChange(uint16_t Channel ,StateDiagramFlagType *StateDiagramFlag) ;    //����ͨ��״̬�� 
void  StateDiagram_SetSignalType(uint16_t SignalType ,StateDiagramFlagType *StateDiagramFlag) ;    //�����ź�����״̬�� 
void  StateDiagram_OutputVoltage(uint16_t PulseWidth_IN ,StateDiagramFlagType *StateDiagramFlag) ;    //�����ѹ����״̬�� 
void  ActualOutputAndInputVoltageMeasure(SystemParameterType  *SystemParameter );                  //���������ѹ����
void  MODBUS_ReadSystemParameter(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);             //����λ�����͵����ò�����������Ӧ�ı�����
void  AdjustStatusError(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);//������� z
void  ControlFinishFlag_FinishCondition(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag);




/*************************************************************************
  * @brief  ���ô���USART2����ģʽ
  * @param  Baudrate �� ������ 
  * @retval �� 
  * @notice ��
*************************************************************************/
int main(void)
{  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                         // ����ϵͳ�ж����ȼ�����2
  Signal_OutputChannelGPIO_Init();                                        // �ź����ͨ��GPIO�������ų�ʼ��
  TIM4_PWM_Init();                                                        // ���������ѹ��PWM�����ʱ����ʼ��
  ADC3_Init(1000);                                                        // ADC3��ѹ�ɼ���ʼ�� ������Ϊ������Ƶ�ʣ�1ms ����һ��ɨ�������
  MODBUS_Init(9600);                                                      // MODBUSͨ�Ų����ʳ�ʼ�� 
// MODBUS_SetSlaveAddress();                                               // MODBUS�ӻ���ַ����
  MODBUS_AddrInit();                                                      // MODBUS��ַ�ϵ��ʼ��
  TIM5_PluseWidthAdjustInit();                                            // Signal������嶨ʱ����ʼ�� ���䲽��Ϊ �� 1MS
  TIM7_Init();                                                            // ��ʱ����ʼ�� ��������������ѯ��ʱ 
  while(1)
  {    
    MODBUS_HandleFunction();                                              // 32us ,MODBUSЭ���ܴ�����    �� �ڽ��յ�һ֡���� �������������ݽ��������ݱ�����MODBUS��Ӧ���������	 
    MODBUS_ReadSystemParameter(&SystemParameter , &StateDiagramFlag);     // �ж��Ƿ���ܵ�һ֡���� �����յ�һ֡���ݵ�����½���λ�����͵����ݱ��浽��Ӧ�������������Ӧ״̬��������ɱ�־λ 
    ActualOutputAndInputVoltageMeasure(&SystemParameter);                 // ʵ�����������ѹ���� ��������ֵ���浽 SystemParameter ��              
    AdjustStatusError(&SystemParameter ,&StateDiagramFlag);               // ������ ����ѹ̫�� ��������ڳ�ʱ��
    ControlFinishFlag_FinishCondition(&SystemParameter ,&StateDiagramFlag);//z������������������������Դ�����һ�����ݵ�������
     
    StateDiagram_SetVoltageAmplitude(SystemParameter.SetOutputVoltage ,SystemParameter.ActualOutputVoltage,&StateDiagramFlag ) ; // ���������ѹ��ֵ״̬��
    StateDiagram_SetChannelChange(SystemParameter.SetSelectChannel    ,&StateDiagramFlag) ;                                      // ���������ѹͨ��״̬��
    StateDiagram_SetSignalType(SystemParameter.SetSignalTypes ,&StateDiagramFlag) ;                                              // ���������ѹ����״̬��
    StateDiagram_OutputVoltage(SystemParameter.SetPulseWidth  ,&StateDiagramFlag) ;                                              // ��ѹ������״̬�� 
 }
}




/*************************************************************************
  * @brief  ����ͨ��״̬�� 
  * @param  EnableSet ��ʹ��ͨ���л�
  *         Channel   ����Ҫ�л���ͨ���� 
  * @retval ��
  * @notice ��
*************************************************************************/
StateDiagramStatusType    ChannelStateDiagramStatus   = AdjustIdleStatus ;               //ͨ���л�״̬���������� 
void  StateDiagram_SetChannelChange(uint16_t Channel ,StateDiagramFlagType *StateDiagramFlag ) 
{
  static uint32_t StartAdjustTime ;                                                       //ͨ���л���ʼʱ��
  
  /**************���ֵ��ڴ�������ʱ���Զ��ص�����״̬**************/
  if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //���ڳ��ִ���
  {
    ChannelStateDiagramStatus   = AdjustIdleStatus ;                                      //״̬���ص�����״̬
    StateDiagramFlag -> ChannelChangeFlag   = FLAG_ERROR ;                                //���ڳ���
  } 
  
  /*****************����ͨ��״̬�� *****************/
  switch( ChannelStateDiagramStatus )
  {
    case  AdjustIdleStatus     : //û����Ҫ���ڵ�ͨ��ʱ�Ŀ���״̬
                                 if( StateDiagramFlag -> ChannelChangeFlag == FLAG_RESET )// ��־λ ��ͨ��δ�������
                                 {
                                   ChannelStateDiagramStatus = NoStartAdjustStatus ;      //�л���δ��ʼ����״̬				 
                                 }
                                 break ;                                             

    case  NoStartAdjustStatus  : //����Ҫ���ڵ�ͨ����δ��ʼ��״̬ �� ��ʼ����ǰ��׼������ ��           
                                 MODBUS_WriteChannelStatus( NoReadyStatus );              // ͨ��δ����
                                 if( Channel  > 20   )                                    // ѡ��ͨ�����Ƿ���ȷ ,ͨ�������Ϊ20 
                                 {
                                   ChannelStateDiagramStatus   = AdjustIdleStatus  ;      // ѡ��ͨ������ ���л������ڿ���															 
                                   StateDiagramFlag -> ChannelChangeFlag   = FLAG_ERROR ; // ���ڳ���
                                 }
                                 else
                                 ChannelStateDiagramStatus   = StartAdjustStatus ;    // ͨ������ȷ ���л�����ʼ����״̬     
                                 break ;                                 

  case  StartAdjustStatus    : //��ʼ����ͨ�� 
                                 Signal_OutputChannelSelectOpen( Channel );               // ����ͨ����  
                                 StartAdjustTime  = TIM7_ReadTimeCount();                 // ����ͨ����ʼ���ڵ�ʱ��  ,��λ:ms ,��С����Ϊ TIM7_TimeTick
                                 ChannelStateDiagramStatus       = AdjustRunWaitStatus  ; // �л������ڵȴ�״̬    
                                 break ;                                

  case  AdjustRunWaitStatus  : //���ڵȴ�״̬ ���ӿ�ʼ���ڵ����������Ҫһ��ʱ��
                                 if(TIM7_ReadTimeCount() - StartAdjustTime  >=  RELAY_SWITCHOVER_TIMER  ) // �̵����л���� ����ʱRELAY_SWITCHOVER_TIMER ms�ļ̵����л�ʱ�� ��
                                 {
                                   ChannelStateDiagramStatus    =  AdjustFinishStatus  ; // ͨ��������� ���л���ͨ���������״̬
                                 }
                                 break ;                           

  case  AdjustFinishStatus   : //ͨ��������� 
                                 StateDiagramFlag -> ChannelChangeFlag   = FLAG_SET ;     // ��־λ ��ͨ���������
                                 MODBUS_WriteChannelStatus( ReadyStatus );                // ͨ������
                                 ChannelStateDiagramStatus       =  AdjustIdleStatus   ;  // ͨ��������� ��û����Ҫ���ڵ�ͨ�� ���л������ڿ���
                                 break ;                                                  // �������  
                          
  default                    : ChannelStateDiagramStatus       =  AdjustIdleStatus   ;  // �����ܷ� ��ǿ���л������ڿ���
  }
}



/*************************************************************************
  * @brief  �����ź�����״̬�� 
  * @param  StateDiagramFlag ��״̬����־
  *         SignalType����Ҫ���ź�����
  * @retval ��
  * @notice ��
*************************************************************************/
StateDiagramStatusType  SignalTypeStateDiagramStatus  = AdjustIdleStatus ;               //�ź�����״̬���������� 
void StateDiagram_SetSignalType(uint16_t SignalType ,StateDiagramFlagType *StateDiagramFlag) 
{
  static uint32_t StartAdjustTime ;                                                       //�ź������л���ʼʱ��
  static SignalTypeDef SignalTypeID ;
  
	/**************���ֵ��ڴ�������ʱ���Զ��ص�����״̬**************/
	if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //���ڳ��ִ���
	{
     SignalTypeStateDiagramStatus   = AdjustIdleStatus ;                                  //״̬���ص�����״̬
     StateDiagramFlag -> SignalTypeFlag =   FLAG_ERROR ;
  }	

	/*****************�����ź�����״̬��*****************/	
	switch( SignalTypeStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //û����Ҫ���ź����͵Ŀ���״̬
			                           if(  StateDiagramFlag ->SignalTypeFlag  == FLAG_RESET  ) // �ź�����δ�������
																 {
																   SignalTypeStateDiagramStatus = NoStartAdjustStatus ;   //�л���δ��ʼ����״̬
																 }		
			                           break ;                                             

		case  NoStartAdjustStatus  : //����Ҫ���ڵ��ź����ͻ�δ��ʼ��״̬ �� ��ʼ����ǰ��׼������ ��   
                                 SignalTypeID = Signal_MODBUSValueConvertToOutputType(SignalType) ;                       
			                           if( SignalTypeID  == ERROR_SIGNAL_TYPE  )                // ѡ���ź������Ƿ���ȷ 
                                 {
                                   SignalTypeStateDiagramStatus = AdjustIdleStatus  ;     // ѡ���ź����ʹ��� ���л����ź����͵��ڿ���״̬
                                   StateDiagramFlag ->SignalTypeFlag  = FLAG_ERROR ;      // �������� 
                                 }
                                 else
																		 SignalTypeStateDiagramStatus = StartAdjustStatus ;   // ͨ������ȷ ���л�����ʼ����״̬     
		                             break ;                                 

		case  StartAdjustStatus    : //��ʼ�л��ź�����                                
			                           Signal_SetOutputType( SignalTypeID );
		                             StartAdjustTime  = TIM7_ReadTimeCount();                 // ����ͨ����ʼ���ڵ�ʱ��  ,��λ:ms ,��С����Ϊ TIM7_TimeTick
                                 SignalTypeStateDiagramStatus     = AdjustRunWaitStatus  ;// �л������ڵȴ�״̬    
			                           break ;                                

		case  AdjustRunWaitStatus  : //�л��ź����͵ȴ�״̬ ���ӿ�ʼ���ڵ��л������Ҫһ��ʱ��
				                         if(TIM7_ReadTimeCount() - StartAdjustTime  >=  RELAY_SWITCHOVER_TIMER  ) // �̵����л���� ����ʱRELAY_SWITCHOVER_TIMER ms�ļ̵����л�ʱ�� ��
				                         {
															      SignalTypeStateDiagramStatus  =  AdjustFinishStatus  ;// �ź������������ ���л����ź������������״̬
																 }
			                           break ;                           

		case  AdjustFinishStatus   : //ͨ��������� 
			                           StateDiagramFlag ->SignalTypeFlag  = FLAG_SET ;          // �ź����͵������
															   SignalTypeStateDiagramStatus     =  AdjustIdleStatus   ; // ͨ��������� ��û����Ҫ���ڵ�ͨ�� ���л������ڿ���
			                           break ;                                                  // �������  
 
		default                    : SignalTypeStateDiagramStatus     =  AdjustIdleStatus   ; // �����ܷ� ��ǿ���л������ڿ���
	}
	
}



/*************************************************************************
  * @brief  ���õ�ѹ��ֵ״̬�� 
  * @param  StateDiagramFlag    ��״̬����־
  *         ActualVoltage  ��ʵ�ʵĵ�ѹֵ
  *         SetVoltage     �����õĵ�ѹֵ
  * @retval ���ص�ѹ��ֵ״̬����״̬
  * @notice ��
*************************************************************************/
StateDiagramStatusType    VoltageStateDiagramStatus   = AdjustIdleStatus ;              //��ѹ����״̬���������� 
void  StateDiagram_SetVoltageAmplitude( uint16_t SetVoltage, uint16_t ActualVoltage ,StateDiagramFlagType *StateDiagramFlag ) 
{
  static uint32_t StartAdjustTime ;                                                       //��ѹ���ڶ�ʱ
  static uint16_t VoltageAdjustFinishCount = 0 ;   
	
	/**************���ֵ��ڴ�������ʱ���Զ��ص�����״̬**************/
	if(StateDiagramFlag -> ControlFinishFlag  == FLAG_ERROR )                               //���ڳ��ִ���
	{
     VoltageStateDiagramStatus   = AdjustIdleStatus ;                                     //״̬���ص�����״̬
     StateDiagramFlag ->VoltageAmplitudeFlag  =   FLAG_ERROR ;
  }	

	
	/*****************���÷�ֵ״̬��*****************/		
	switch( VoltageStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //û����Ҫ���ڵ�ѹ�Ŀ���״̬
			                           /**************�ж��Ƿ��������õ�ѹ************/
			                           if(  StateDiagramFlag ->VoltageAmplitudeFlag == FLAG_RESET  )    // ��ѹ��ֵδ�������
																 {
																		VoltageStateDiagramStatus = NoStartAdjustStatus ;     //�л���δ��ʼ����״̬
																 } 
			                           break ;                                             

		case  NoStartAdjustStatus  : //��ѹ��δ��ʼ����
			                           /************��ʼ����ǰ��׼������**************/
	                             	 VoltageAdjustFinishCount  = 0 ; 
		                             MODBUS_WriteVoltageStatus( NoReadyStatus );              // ��ѹδ���� 
 
		                             /*************���õ�ѹ�޷��ж�***************/
			                           if( ( SetVoltage  > 5000 ) || ( SetVoltage  < 500 ))			// ��ѹ�����Ƿ���ȷ   
																 {
                                 	   VoltageStateDiagramStatus   = AdjustIdleStatus  ;    // ��ѹ���ô��� ���л������ڿ���
		                                 StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_ERROR;// �������� 
                                 }
                                 else
                                 {
																	 	 VoltageStateDiagramStatus   = StartAdjustStatus ;    // ͨ������ȷ ���л�����ʼ����״̬     
		                             }
                                 break ;                                 

		case  StartAdjustStatus    : //��ʼ���ڵ�ѹ
																 PWM_SetOutputVoltageAmplitude(SetVoltage) ;              // pwm ���������ѹ  
		                             StartAdjustTime  = TIM7_ReadTimeCount();                 // �����ѹ��ʼ���ڵ�ʱ��  ,��λ:ms ,��С����Ϊ TIM7_TimeTick
 		                             VoltageStateDiagramStatus       = AdjustRunWaitStatus;   // �л������ڵȴ���
			                           break ;                                

		case  AdjustRunWaitStatus  : //���ڵȴ�״̬ ��һ�β�һ�����ڳɹ� �����뷴��
			                           /************200ms����һ�ε�ѹ******************/
                                 if(TIM7_ReadTimeCount() - StartAdjustTime  >=  200 )     // 200ms����һ��
																 {
																	 	 StartAdjustTime  = TIM7_ReadTimeCount();             // ����ʱ��  ,��λ:ms ,��С����Ϊ TIM7_TimeTick
																	   VoltageAdjustFinishCount++ ;
//																		 /***************��������ѹ����*************/
//																		 if(AdjustAmendOutputVoltage(SetVoltage ,ActualVoltage) == SUCCESS )//��������
//																		 {
																			  VoltageStateDiagramStatus = AdjustFinishStatus;   // ��ѹ������� ���л����������
//																		 } else
																			 if( VoltageAdjustFinishCount > 30 )                // 6s��ʱ���
																			 {
																			     VoltageStateDiagramStatus   = AdjustIdleStatus;// ���ڳ�ʱ���л������ڿ���
		                                       StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_ERROR ; // �������� 
																			 }
																 }
																 break ;                           

		case  AdjustFinishStatus   : //��ѹ������� 
		                             StateDiagramFlag ->VoltageAmplitudeFlag = FLAG_SET ;     // ��ѹ��ֵ�������
																 MODBUS_WriteVoltageStatus(ReadyStatus );                 // ��ѹ����  
															   VoltageStateDiagramStatus         = AdjustIdleStatus ;   // �л������ڿ���״̬
			                           break ;                                                          
 
		default                    : VoltageStateDiagramStatus         = AdjustIdleStatus ;   // �����ܷ� ��ǿ���л������ڿ���
	}
	
}




/*************************************************************************
  * @brief  �����ѹ����״̬�� 
  * @param  StateDiagramFlag��״̬����־
  *         ActualVoltage  ��ʵ�ʵĵ�ѹֵ
  * @retval ��
  * @notice  0XFFFF :��ʾ��������޳�� ����������ߵ͵�ƽ
*************************************************************************/
 StateDiagramStatusType  OutputStateDiagramStatus      = AdjustIdleStatus ;               //�����ѹ״̬���������� 
void  StateDiagram_OutputVoltage( uint16_t PulseWidth_IN ,StateDiagramFlagType *StateDiagramFlag) 
{
  static uint16_t  PulseWidth ;

	/**************���ֵ��ڴ�������ʱ���Զ��ص�����״̬**************/
	if(StateDiagramFlag ->ControlFinishFlag  == FLAG_ERROR )                                //���ڳ��ִ���
	{
     OutputStateDiagramStatus  = AdjustIdleStatus ;                                       //״̬���ص�����״̬
     StateDiagramFlag -> OutputVoltageFlag  =   FLAG_ERROR ;
  }	

	switch( OutputStateDiagramStatus )
	{
		case  AdjustIdleStatus     : //û����Ҫ����Ŀ���ʱ��
			                           /**************�жϵ�ѹ�Ƿ��Ѿ����************/
                                 PulseWidth  = PulseWidth_IN ;
			                           if( StateDiagramFlag -> OutputVoltageFlag == FLAG_RESET) // ��ѹ��δ���
																 {
 																	  MODBUS_WriteOutputStatus(NoStartOutputStatus);        // δ��ʼ�����ѹ 
																		OutputStateDiagramStatus = NoStartAdjustStatus ;      // �л���δ��ʼ����״̬ 
																 }
			                           break ;                                             

		case  NoStartAdjustStatus  : //��ѹ��δ��ʼ�����ѹ ����ѹ��δ������� 
		                             /*************���õ�ѹ��ͨ�����ź������Ƿ�������***************/
		                             if(( StateDiagramFlag -> VoltageAmplitudeFlag == FLAG_SET )&&\
																	  ( StateDiagramFlag -> ChannelChangeFlag    == FLAG_SET )&&\
																	  ( StateDiagramFlag -> SignalTypeFlag       == FLAG_SET ))
                                 {
																   if ( PulseWidth    >= 10 && PulseWidth <= 60000 ) 
                                   {                                     
		                                 OutputStateDiagramStatus = StartAdjustStatus  ;      // �л�����ʼ�����ѹ 
                                   }
                                   else if( PulseWidth == HIGH_LEVEL_PULSE_WIDTH )        // ����ߵ͵�ƽ
                                   {
                                      FET_PluseWidthSignalOutputOpen();                   // ��FET��
                                      MODBUS_WriteOutputStatus(NowRunOutputStatus);       // ��λ��־λ �� ���������ѹ״̬  
                                      StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;// ��ѹ������  
                                      OutputStateDiagramStatus  =  AdjustIdleStatus  ;    // ������У����Խ�����һ�����ݵĶ�ȡ
                                   }
																	 else if(PulseWidth == EDGE_PULSE_WIDTH )
																	 {
																	 	  OutputStateDiagramStatus = StartAdjustStatus  ;     // �л�����ʼ�����ѹ 
                                      PulseWidth  = 2 ;                                   // 2ms�����ʾ����
																	 }
                                   else 
                                   {
                                      StateDiagramFlag -> OutputVoltageFlag = FLAG_ERROR ;// ������������
                                      OutputStateDiagramStatus  = AdjustIdleStatus ;      // ״̬���ص�����״̬
                                   }
                                 }
		                             break ;                                 

		case  StartAdjustStatus    : //��ʼ�����ѹ
                                 TIM5_SetOutputPluseWidth(PulseWidth);	                  // ����Signal�������ʱ�� ����λ ��ms ��ʵ��ֵ������ֵ��450us 
			                           MODBUS_WriteOutputStatus(NowRunOutputStatus);            // ��λ��־λ �� ���������ѹ״̬    
			                           OutputStateDiagramStatus = AdjustRunWaitStatus  ;        // �л�����ѹ�������״̬ 
		                             break ;                                

		case  AdjustRunWaitStatus  : //��ѹ����ȴ�״̬
				                         if( TIM5_OutputPluseWidthFinish() == SET)                // ����ѹ�����Ƿ�������
	                                  OutputStateDiagramStatus = AdjustFinishStatus  ;      // ��ѹ������ ���л����ź�������״̬                                       
																 break ;                           

		case  AdjustFinishStatus   : //��ѹ������
																 Delay1Us(500);                                           // FET ���عܵĹض�ʱ��ԼΪ450us  
		                             StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET  ;    // ��ѹ������  
																 MODBUS_WriteOutputStatus(EndOutputStatus);               // ��ѹ������״̬ 
		                             OutputStateDiagramStatus  =  AdjustIdleStatus  ;         // ������У����Խ�����һ�����ݵĶ�ȡ
			                           break ;                                                          
 
		default                    : OutputStateDiagramStatus  =  AdjustIdleStatus  ;         // �����ܷ� ��ǿ���л����������
	}

	
}





/*************************************************************************
  * @brief  ����λ�����͵����ò�����������Ӧ�ı�����
  * @param  SystemParameter   ����Ҫ���õı���
  *         StateDiagramFlag  ����״̬��������ɱ�־
  * @retval ��
  * @notice ���뺯��ǰ�ȼ����һ�������Ƿ������� ���������ɵ�������
  *         �ټ���Ƿ������ʹ�ܱ�־
*************************************************************************/
void MODBUS_ReadSystemParameter(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{
	static uint8_t OutputFlag =  0 ; 
  static SystemParameterType  SystemLastParameter = {0};                                  // ����ÿ40ms��ȡһ��MODBUS������ 

	/********�ڽ��յ�����źź���Ҫ�ȹر��ϴ�������ܲ�����һ��*****/
	if ( ( OutputFlag == 1 ) && ( TIM7_ReadTimeCount() - SystemParameter  -> StartAdjustTime > 2  ) ) //��FET�ܵĹض���������ʱ��Ϊ2ms
	{
     OutputFlag =  0 ;
		 StateDiagramFlag ->ControlFinishFlag   = FLAG_RESET  ;                               // �����δ��� ���൱�ڴ���������أ�                    
	}
  
  /*************���ڿ������ ���������ȡMODBUS����***************/
  if( (StateDiagramFlag ->ControlFinishFlag   == FLAG_RESET  ) || (OutputFlag == 1 ) )		return ;     // �����ɵ������¶�ȡ�Ƿ������״̬
	
  /*************ÿ 20ms ��ȡһ��MODBUS���ݱ��浽������************/
  if( TIM7_20MsFinish() != SET )   return ;
  
  /**********************������һ�ζ�ȡMIDBUS�ڴ�����*************/
  (void)memcpy(&SystemLastParameter ,SystemParameter ,sizeof(SystemParameterType));       // ������һ�ε����� 
 
   /******************��ȡMODBUS�ڴ����ݱ��浽����****************/
  SystemParameter -> SetOutputVoltage = MODBUS_ReadSetOutputVoltage() * 10;               // ��ȡ���õĵ�ѹֵ
  SystemParameter -> SetSignalTypes   = MODBUS_ReadSetSignalTypes()  ;                    // ��ȡ���õ��ź�����
  SystemParameter -> SetPulseWidth    = MODBUS_ReadSetPulseWidth()   ;                    // ��ȡ���õ�����ź�����
  SystemParameter -> SetSelectChannel = MODBUS_ReadSetSelectChannel();                    // ��ȡ���õ����ͨ�� 


  /*********************�ж���Щ������Ҫ���µ���*****************/
  if(StateDiagramFlag -> ControlFinishFlag    == FLAG_ERROR)  
	{
		StateDiagramFlag -> VoltageAmplitudeFlag = FLAG_RESET ;                               // ��ѹ��ֵδ���ں�
		StateDiagramFlag -> ChannelChangeFlag    = FLAG_RESET ;                               // ͨ��δ���ں�
		StateDiagramFlag -> SignalTypeFlag       = FLAG_RESET ;                               // �ź�����δ���ں�
	}
	else
	{
		if( ( SystemParameter-> SetOutputVoltage !=  SystemLastParameter.SetOutputVoltage))// ||\
			 (abs( SystemParameter -> SetOutputVoltage - SystemParameter -> ActualOutputVoltage ) > 5 ))	// �����ѹ�иĶ� �� ʵ�������ѹ�����õ�ѹѹ�����50mV
			StateDiagramFlag -> VoltageAmplitudeFlag = FLAG_RESET ;                                     // ��ѹ��ֵδ���ں�
	 
		if(( SystemParameter -> SetSelectChannel != SystemLastParameter.SetSelectChannel ))//||\
			(  Signal_ReadChannelSwitchStatus(SystemParameter -> SetSelectChannel) != SET ) )		// ͨ���иĶ� �� ͨ��δ�� 
			StateDiagramFlag -> ChannelChangeFlag    = FLAG_RESET ;                             // ͨ��δ���ں�
	 
		if( SystemParameter-> SetSignalTypes  != SystemLastParameter.SetSignalTypes )         // �ź������иĶ�
			StateDiagramFlag -> SignalTypeFlag       = FLAG_RESET ;                             // �ź�����δ���ں�
		
	}
	


  /************************��ȡ���ʹ�ܶ� ��Ϊ1��ʾ���*************************/
  if( MODBUS_ReadSwitchOutputStatus() == SET )                                            // �ж��Ƿ���յ����ʹ��λ��1                              
  {
	
		/****************�����ź����;��������ѹ״̬���Ŀ��� ***********************/    
			switch(Signal_MODBUSValueConvertToOutputType(SystemParameter -> SetSignalTypes))
			{
				case POSITIVE_PULSE  :  /************������************/ 	
				case NEGATIVE_PULSE  :  /***********������*************/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // �źŻ�δ��� 
																if( SystemParameter  -> SetPulseWidth   == HIGH_LEVEL_PULSE_WIDTH  ) 
																	 SystemParameter  -> SetPulseWidth    = HIGH_LEVEL_PULSE_WIDTH - 1 ;//��ֹ�͵�ƽ�����ͻ�������ڴ˼�1       
                                else  if( SystemParameter  -> SetPulseWidth   == EDGE_PULSE_WIDTH  )	
																	 SystemParameter  -> SetPulseWidth    = EDGE_PULSE_WIDTH  + 1 ;     //��ֹ�ͱ��������ͻ�������ڴ˼�1     
																	
																break ;
				case RISING          :  /***********������*************/
				case FALLING         :  /***********�½���*************/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // �źŻ�δ��� 
																SystemParameter  -> SetPulseWidth       = EDGE_PULSE_WIDTH ;         // �������½�������С����������ʾ ���������� 2MS��ʾ
																break ;
				case POSITIVE_LEVEL  :  /************����ƽ************/                                         
				//case NEGATIVE_LEVEL  :  /***********����ƽ*************////////////by YZ
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_RESET ;    // �źŻ�δ���   
																SystemParameter  -> SetPulseWidth       = HIGH_LEVEL_PULSE_WIDTH ;   // ��ʾ�������������� 
																break;      
				case SWITCH_ON       :  /************������(�պ�)******/
				case SWITCH_OFF      :  /************������(�Ͽ�)******/
				case NEGATIVE_LEVEL  :  /***********����ƽ*************///////////////by YZ
				
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;      // ����Ҫ�����ѹ �����õ�ѹ�Ѿ���� 
																break;
				default:                /**********�ź����ʹ���********/
																StateDiagramFlag -> OutputVoltageFlag   = FLAG_SET ;      // ����Ҫ�����ѹ �����õ�ѹ�Ѿ���� 
																break;
			}
			
		  OutputFlag    =  1 ;                                                                // ���յ�����ź�ʱ�����ȹ��ϲ�������ܲ�����һ�� ��OutputFlag ��Ϊ֮������ı�־
      SystemParameter  -> StartAdjustTime   = TIM7_ReadTimeCount();                       // ����ʱ��  ,��λ:ms ,��С����Ϊ TIM7_TimeTick
      FET_PluseWidthSignalOutputClose();                                                  //

		}
}



/*************************************************************************
  * @brief  ���õ������ѹ �� PWMռ�ձ����� �Ĺ�ϵ ,��������PWM���ռ�ձ�
  * @param  SetVoltage �� ���õ������ѹ�� �Ŵ�100�� ��
  * @retval ����PWM��ռ�ձ� �� ��λ�� ���֮һ ��
  * @notice ռ�ձȵ�λ �����֮һ  ����ѹ�ĵ�λ �� 0.01V
*************************************************************************/
uint16_t PWM_SetOutputVoltageAmplitude(uint16_t SetVoltage)
{
	uint16_t PWM_SetDutyRatio;                                                              //PWM��ռ�ձ�
	
	/****************�����ѹ�޷�*****************/
	if( SetVoltage > 5275 ) SetVoltage = 5275 ;                                             //��������ѹΪ52.75V : 5275 = 96741000 /  18337 
	if( SetVoltage < 150  ) SetVoltage = 150  ;                                             //��С�����ѹΪ1.50V
	
	/****���õ������ѹ �� PWMռ�ձ����� �Ĺ�ϵ***/
	PWM_SetDutyRatio = (uint16_t)(( 96741000 -  18337 * SetVoltage ) / 10000 );             //��ѹ��PWMռ�ձȵĹ�ϵ  
	
 	/*****************����PWM���*****************/
  TIM4_PWM_SetDutyRatio(PWM_SetDutyRatio);                                                //����PWMռ�ձ� ����λ�� ���֮һ  
	return PWM_SetDutyRatio ;                                                               // ����ʵ��PWM���ռ�ձ�
}







/*************************************************************************
  * @brief  �������������ѹ
  * @param  SetOutputVoltage    �� ���õ������ѹ �� ��ѹ�Ŵ�100�� ��
  *         ActualOutputVoltage �� ʵ�������ѹ   �� ��ѹ�Ŵ�100�� ��
  * @retval �Ƿ���ڳɹ�, ERROR ��δ���ڳɹ� ���ȴ���һ�εĵ���
                        SUCCESS �����ڳɹ� 
	* @notice ��
*************************************************************************/
ErrorStatus AdjustAmendOutputVoltage(int16_t SetOutputVoltage , int16_t ActualOutputVoltage)
{
	static uint16_t  ActualSetOutputVoltage  ;                                              // ͨ��������ʵ����Ҫ���õ������ѹֵ                               
	static int16_t  SetOutputVoltage2 = 0   ;                                               // �������õ������ѹֵ
	static int16_t  LastOutputVoltageError ;                                                // �ϴ�ʵ�ʵ�ѹ�����õ�ѹ�Ĳ�ֵ 
  int16_t  OutputVoltageError;                                                            // ���ʵ�ʵ�ѹ�����õ�ѹ�Ĳ�ֵ
	
	/***********�������õ�ѹֵ*****************/
	if(SetOutputVoltage2  != SetOutputVoltage  )                                            // �ж��Ƿ���������������ѹ
	{
		SetOutputVoltage2       =  SetOutputVoltage ;                                         // ����������õĵ�ѹֵ
		ActualSetOutputVoltage  =  SetOutputVoltage ;                                         // ʵ�����õ������ѹ�Ȱ�����ֵ���������ٵ���
    LastOutputVoltageError	=  0 	;                                                       // �ϴ�ƫ�����
	}	

	 /***���ڳɹ��ı�־Ϊ���������ε�ѹ��20mv��***/
  OutputVoltageError       =  ActualOutputVoltage    -  SetOutputVoltage ;                // ����ʵ���ѹ�����õ�ѹ�Ĳ�ֵ   
	if( ( OutputVoltageError < 2 ) && ( OutputVoltageError > -2  ) )  
	{
		if( ( LastOutputVoltageError < 2 ) && ( LastOutputVoltageError > -2  ) )  
		 return SUCCESS;                                                                      // ����ɹ�
	}

	/*******ֻ����ѹ����1v��Χ�ڲ��������********/
	if( ( OutputVoltageError > 100 ) ||( OutputVoltageError < -100  ) ) return ERROR;       // ��ѹѹ�����1V���������
  
	/****************��������޷�*****************/
  if( OutputVoltageError > 10 )       OutputVoltageError = 10  ;                          // һ��������100mV��ѹ      
	else if( OutputVoltageError < -10 ) OutputVoltageError = -10 ;                          // һ��������100mV��ѹ   
 
	LastOutputVoltageError     =  OutputVoltageError ;                                      // ������ε�ѹ��ֵ                            
  ActualSetOutputVoltage   =  ActualSetOutputVoltage -  OutputVoltageError ;              // ����ʵ�����õ������ѹֵ
	PWM_SetOutputVoltageAmplitude( ActualSetOutputVoltage) ;                                // PWM��ѹ�������
	return ERROR ;
}




/*************************************************************************
  * @brief  ��ʵ�����������ѹ���в������㣬�õ�ʵ�ʵ����������ѹֵ ����ѹ��λ �� 1LSB = 0.01 V
  * @param  SystemParameter    ��ʵ�ʵ�ѹ�ı���λ��
  * @retval ��
	* @notice ��
*************************************************************************/
void  ActualOutputAndInputVoltageMeasure(SystemParameterType  *SystemParameter )
{
  if(TIM7_50MsFinish() == SET )       //ÿ50ms����һ��ʵ�������ѹ
	{
		SystemParameter -> ActualOutputVoltage   = ADC3_SampleOutputVoltageValue();           //�����ѹ
	  MODBUS_WriteActualOutputVoltage (SystemParameter -> ActualOutputVoltage);
	}
	if(TIM7_200MsFinish() == SET )      //ÿ200Ms�Ӹ���һ�������ѹ��ֵ
	{ 
		SystemParameter -> ActualInputVoltage    =  ADC3_SampleInputVoltageValue();           // �����ѹ
	}
}


/*************************************************************************
  * @brief  ���ֵ��ڳ������� �����ѹһֱ�޷����ڳɹ� �������ѹ̫��
  * @param  SystemParameter    ��ϵͳ����
  *         StateDiagramFlag   ��ϵͳ��־
  * @retval ��
	* @notice ��
*************************************************************************/
void AdjustStatusError(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{	
	/************ �����ѹ̫�� �� ���ڳ�ʱ( 1S ���һ�� ) **********/ 
	if( TIM7_1sFinish() == SET )
	{
		if((SystemParameter -> ActualInputVoltage - SystemParameter -> SetOutputVoltage < 300 )||\
			 (SystemParameter -> ActualInputVoltage < 1000 ))                                   // �����ѹ̫��( (�������ѹ��3V)||(�����ѹС��10V)  )                  
		{
			StateDiagramFlag -> ControlFinishFlag    = FLAG_ERROR ;                             // ���ڳ���                                  
		}
		
	}
	
	/*************����������������������********************/
	if( StateDiagramFlag -> ControlFinishFlag  != FLAG_SET  )
	if(( StateDiagramFlag -> VoltageAmplitudeFlag == FLAG_ERROR  ) || ( StateDiagramFlag -> ChannelChangeFlag == FLAG_ERROR  ) ||\
	  	(StateDiagramFlag -> SignalTypeFlag       == FLAG_ERROR  ) || ( StateDiagramFlag -> OutputVoltageFlag == FLAG_ERROR  ))  
	{
     StateDiagramFlag -> ControlFinishFlag  = FLAG_SET ;                                  // �������� �������κδ��� ���ȴ�������һ������                       {
     MODBUS_WriteSwitchOutputStatus(RESET);                                               // д0 ����ʾ�����ɣ����Դ�����һ������ 
	}	
}







/*************************************************************************
  * @brief  ����ͨ�ŵ������ ������������������������
  * @param  SystemParameter    ��ϵͳ����
  *         StateDiagramFlag   ��ϵͳ��־
  * @retval ��
	* @notice ��
*************************************************************************/
void ControlFinishFlag_FinishCondition(SystemParameterType  *SystemParameter , StateDiagramFlagType *StateDiagramFlag)
{
	/***************ֻ�������ڵ�������ż������Ƿ����****************/
	if( StateDiagramFlag -> ControlFinishFlag  != FLAG_RESET ) return ;

  /***************�����ͬ�ź����ͣ��������������ͬ*******************/	
  switch(Signal_MODBUSValueConvertToOutputType(SystemParameter -> SetSignalTypes))
  {
    case POSITIVE_PULSE  :  /***********������*************/ 	
    case NEGATIVE_PULSE  :  /***********������*************/
		case RISING          :  /***********������*************/
	  case FALLING         :  /***********�½���*************/
    case POSITIVE_LEVEL  :  /************����ƽ************/                                         
    //case NEGATIVE_LEVEL  :  /***********����ƽ*************/////////////////////////by YZ
                            if(StateDiagramFlag -> OutputVoltageFlag == FLAG_SET )        // ��ѹ������ 
                              StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;  
                            break;      
    case SWITCH_ON       :  /************������(�պ�)******/
    case SWITCH_OFF      :  /************������(�Ͽ�)******/
		case NEGATIVE_LEVEL  :  /***********����ƽ*************////////////////////////////////by YZ

		
                          if(( StateDiagramFlag -> SignalTypeFlag == FLAG_SET )&&\
                               ( StateDiagramFlag -> ChannelChangeFlag == FLAG_SET ))     // �ź����� �� ͨ����� 
                               StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;  

                            break;
    default:                /**********�ź����ʹ���********/
                            StateDiagramFlag -> ControlFinishFlag   = FLAG_SET ;          // ����Ҫ�����ѹ �����õ�ѹ�Ѿ���� 
                            break;
  }
  
	/******************�жϿ����Ƿ����****************/
  if( StateDiagramFlag -> ControlFinishFlag  != FLAG_RESET )                              //�ܿ������(����ڳ���)��������дMODBUS���Խ�����һ֡���ݵı�־                        
  {
     MODBUS_WriteSwitchOutputStatus(RESET);                                               // д0 ����ʾ�����ɣ����Դ�����һ������ 
  }

}




/*************************************************************************
  * @brief  MODBUS �ӻ���ַ���� ��ͨ�����뿪�ص�ֵ����MODBUS��ַ 
  * @param  ��
  * @retval ��
	* @notice ��
*************************************************************************/
void MODBUS_SetSlaveAddress(void)
{
	uint8_t KeyValue ;                                                                      // ���뿪�ذ���ֵ��ʱ������
  KeyValue = ToggleSwitch_ReadKeyValue();                                                 // ��ȡ���뿪�ذ���ֵ
  MODBUS_ChangeSalveID(KeyValue);                                                         // ����MODBUS�ӻ���ַ      
}






