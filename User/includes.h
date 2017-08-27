#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include "stm32f10x.h"                           // STM32ͷ�ļ� 
#include "string.h"                              // �ַ���ͷ�ļ�
#include "stdlib.h" 
#include "math.h"
#include "TIM4_PWM.h"                            // PWM���ڵ�ѹ��� �� PWM���Ƶ��  ��1.2KHz     ,���ʱ�� 72M/1.2K = 60000 ��Ӧ�� 100% ռ�ձ�
#include "MODBUS.h"                              // MODBUSͨ��Э�� ��ʵ������λ����ͨ��
#include "Communication.h"                       // MODBUS ��ַ����
#include "Delay.h"                               // ��ʱ���� �������ʱ 
#include "TIME7.h"                               // ��������ʱ��ѯ������ʱ����׼ʱ�䣺2ms        
#include "TIM5_PulseWidth.h"                     // Signal����������  �� ����Ϊ   ��1ms        ,����FET�Ĺض�ʱ��Ƚϳ� ��ʵ����������õĶ�450us 
#include "SignalChannelSwitch.h"                 // ���ͨ���л� ��������� ������ź����͵ĵ�Ƭ��I/O�ڶ���
#include "VoltageMeasure.h"                      // ���������ѹ�Ĳ���
#include "ToggleSwitchAndAlarm.h"                // ���뿪�غͱ������й�ͷ�ļ�






#endif



