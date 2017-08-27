#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include "stm32f10x.h"                           // STM32头文件 
#include "string.h"                              // 字符串头文件
#include "stdlib.h" 
#include "math.h"
#include "TIM4_PWM.h"                            // PWM调节电压输出 ， PWM输出频率  ：1.2KHz     ,这个时候 72M/1.2K = 60000 对应于 100% 占空比
#include "MODBUS.h"                              // MODBUS通信协议 ，实现与上位机的通信
#include "Communication.h"                       // MODBUS 地址分配
#include "Delay.h"                               // 延时函数 ，软件延时 
#include "TIME7.h"                               // 主函数延时查询函数定时，基准时间：2ms        
#include "TIM5_PulseWidth.h"                     // Signal输出脉宽调节  ， 步进为   ：1ms        ,由于FET的关断时间比较长 ，实际输出比设置的多450us 
#include "SignalChannelSwitch.h"                 // 输出通道切换 和输出开关 ，输出信号类型的单片机I/O口定义
#include "VoltageMeasure.h"                      // 输入输出电压的测量
#include "ToggleSwitchAndAlarm.h"                // 拨码开关和报警器有关头文件






#endif



