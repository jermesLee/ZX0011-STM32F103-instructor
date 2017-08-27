#include "Communication.h" 

uint16_t  WorkStat = 0  ;


///*****************MODBUS地址类型4 （ 16bit访问 可读可写 ）***************/
//#define  MODBUS_SetSelectChannelAdderss       0X40001                    // 上电初始化为 0 
//#define  MODBUS_SetOutputVoltageAdderss       0X40002                    // 上电初始化为 50
//#define  MODBUS_SetPulseWidthAdderss          0X40003                    // 上电初始化为 10
//#define  MODBUS_SetSignalTypesAdderss         0X40004                    // 上电初始化为 1
//#define  MODBUS_SetSwitchOutputStatusAddress  0x40005                    // 上电初始化为 0
void MODBUS_AddrInit(void)
{
	uint16_t MODBUS_AddrInitData[] = { 0 ,50 , 10 , 1 , 0  } ; 
    MODBUS_AddrMapDataWrite(MODBUS_AddrInitData ,5 , 0X40001); 
}




uint16_t MODBUS_ReadSetSelectChannel(void)
{
	uint16_t SetSelectChannel ;
	MODBUS_AddrMapDataRead(&SetSelectChannel , 1 , MODBUS_SetSelectChannelAdderss ); 
  return 	 SetSelectChannel ;
}


uint16_t MODBUS_ReadSetOutputVoltage(void)
{
	uint16_t SetOutputVoltage ;
	MODBUS_AddrMapDataRead(&SetOutputVoltage , 1 , MODBUS_SetOutputVoltageAdderss); 
  return 	 SetOutputVoltage ;
}


uint16_t MODBUS_ReadSetPulseWidth(void)
{
	uint16_t SetPulseWidth ;
	MODBUS_AddrMapDataRead(&SetPulseWidth   , 1 , MODBUS_SetPulseWidthAdderss ); 
  return 	 SetPulseWidth ;
}


uint16_t MODBUS_ReadSetSignalTypes(void)
{
	uint16_t SetSignalTypes ;
	MODBUS_AddrMapDataRead(&SetSignalTypes   , 1 , MODBUS_SetSignalTypesAdderss ); 
  return 	 SetSignalTypes ;
}



void MODBUS_WriteActualOutputVoltage (uint16_t ActualOutputVoltage)
{
	MODBUS_AddrMapDataWrite(&ActualOutputVoltage   , 1 , MODBUS_ActualOutputVoltageAdderss); 
}


void MODBUS_WriteVoltageStatus( WorkReadyStatus WorkStatus)
{
	WorkStat = ( WorkStat & ( ~( 1 << 3 ) ) ) | ( WorkStatus << 3 )  ; 
	MODBUS_AddrMapDataWrite(&WorkStat      , 1 , MODBUS_WorkStatusAdderss   ); 
}

void MODBUS_WriteChannelStatus( WorkReadyStatus WorkStatus)
{
	WorkStat =( WorkStat & ( ~( 1 << 2 ) ) ) | ( WorkStatus << 2 )  ; 
	MODBUS_AddrMapDataWrite(&WorkStat      , 1 , MODBUS_WorkStatusAdderss   ); 
}

void MODBUS_WriteOutputStatus(OutputStatusType  WorkStatus)
{
	WorkStat = ( WorkStat & ( ~3 ) ) | WorkStatus  ; 
	MODBUS_AddrMapDataWrite(&WorkStat      , 1 , MODBUS_WorkStatusAdderss   ); 
}





FlagStatus MODBUS_ReadSwitchOutputStatus(void)
{
	uint16_t SwitchOutputStatus  ;
	
	MODBUS_AddrMapDataRead(&SwitchOutputStatus  , 1 , MODBUS_SetSwitchOutputStatusAddress );
  if( SwitchOutputStatus != 0 ) return SET ;
  else return RESET ;	  

}


void MODBUS_WriteSwitchOutputStatus(FlagStatus Status  )
{
		uint16_t SwitchOutputStatus = 0  ;
    if( Status == SET ) SwitchOutputStatus = 1 ;
	  else SwitchOutputStatus = 0 ; 
	
		MODBUS_AddrMapDataWrite(&SwitchOutputStatus, 1 , MODBUS_SetSwitchOutputStatusAddress); 

}







/*********************************************END OF FILE**********************/
