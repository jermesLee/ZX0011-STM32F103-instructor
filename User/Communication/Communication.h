#ifndef __COMMUNICATION_H_
#define	__COMMUNICATION_H_

#include "includes.h"
#include "MODBUS.h"

/*****************MODBUS地址类型4 （ 16bit访问 可读可写 ）***************/
#define  MODBUS_SetSelectChannelAdderss       0X40001                    // 上电初始化为 0 
#define  MODBUS_SetOutputVoltageAdderss       0X40002                    // 上电初始化为 50
#define  MODBUS_SetPulseWidthAdderss          0X40003                    // 上电初始化为 10
#define  MODBUS_SetSignalTypesAdderss         0X40004                    // 上电初始化为 1
#define  MODBUS_SetSwitchOutputStatusAddress  0x40005                    // 上电初始化为 0


/*****************MODBUS地址类型3 （ 16bit访问 只读 ）********************/
#define  MODBUS_WorkStatusAdderss             0X30001
#define  MODBUS_ActualOutputVoltageAdderss    0X30002


typedef enum WorkReadyStatus { NoReadyStatus       = 0 , ReadyStatus         = 1 } WorkReadyStatus ;
typedef enum OutputStatusType{ NoStartOutputStatus = 0 , NowRunOutputStatus = 1 , EndOutputStatus = 2 }OutputStatusType ;


FlagStatus MODBUS_ReadSwitchOutputStatus(void);
uint16_t MODBUS_ReadSetSelectChannel(void);
uint16_t MODBUS_ReadSetOutputVoltage(void);
uint16_t MODBUS_ReadSetPulseWidth(void);
uint16_t MODBUS_ReadSetSignalTypes(void);
void     MODBUS_WriteActualOutputVoltage (uint16_t ActualOutputVoltage);

void MODBUS_WriteOutputStatus(OutputStatusType  WorkStatus);
void MODBUS_WriteChannelStatus( WorkReadyStatus WorkStatus);
void MODBUS_WriteVoltageStatus( WorkReadyStatus WorkStatus);
void MODBUS_WriteSwitchOutputStatus(FlagStatus Status  );
void MODBUS_AddrInit(void);


#endif /* LED.H*/

