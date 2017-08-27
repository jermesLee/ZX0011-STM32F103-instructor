#ifndef _TOGGLESWITCHANDALARM_H_
#define _TOGGLESWITCHANDALARM_H_

#include "includes.h"     // �����ܵ�ͷ�ļ�
#include "Delay.h"        // ��ʱ���� �������ʱ 

#define Alarm_Open()      GPIO_SetBits(  GPIOB, GPIO_Pin_2)               // �ߵ�ƽ����������
#define Alarm_Close()     GPIO_ResetBits(GPIOB, GPIO_Pin_2)               // �͵�ƽ�رձ�����

extern void ToggleSwitchAndAlarm_Init(void);                              // ���뿪�غͱ�����gpio��ʼ���ܺ���   
extern uint8_t ToggleSwitch_ReadKeyValue(void);                           // ��ȡ���뿪�ذ���ֵ


#endif










