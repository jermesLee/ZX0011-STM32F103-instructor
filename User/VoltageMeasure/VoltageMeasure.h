#ifndef _VOLTAGEMEASURE_H_
#define _VOLTAGEMEASURE_H_

#include "includes.h"
#include "ADC3.h"

extern uint16_t ADC3_SampleOutputVoltageValue(void);                             //ADC3读取实际输出的的电压值 （ 包括3路ADC的比较 ）
extern uint16_t ADC3_SampleInputVoltageValue(void);                              //ADC3读取输入电压的电压值 


#endif










