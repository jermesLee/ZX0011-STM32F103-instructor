#ifndef _TOGGLESWITCHANDALARM_H_
#define _TOGGLESWITCHANDALARM_H_

#include "includes.h"     // 工程总的头文件
#include "Delay.h"        // 延时函数 ，软件延时 

#define Alarm_Open()      GPIO_SetBits(  GPIOB, GPIO_Pin_2)               // 高电平报警器报警
#define Alarm_Close()     GPIO_ResetBits(GPIOB, GPIO_Pin_2)               // 低电平关闭报警器

extern void ToggleSwitchAndAlarm_Init(void);                              // 拨码开关和报警器gpio初始化总函数   
extern uint8_t ToggleSwitch_ReadKeyValue(void);                           // 读取拨码开关按键值


#endif










