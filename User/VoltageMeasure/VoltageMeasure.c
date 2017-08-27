#include "VoltageMeasure.h"

uint16_t ADC3_SampleOutputVoltageValue(void);                             //ADC3读取实际输出的的电压值 （ 包括3路ADC的比较 ）
uint16_t ADC3_SampleInputVoltageValue(void);                              //ADC3读取输入电压的电压值 



/*************************************************************************
  * @brief  ADC3 读取实际输出的的电压值 （ 包括3路ADC的比较 ）
  * @param  无
  * @retval 无 
  * @notice 比较的时候和ADC的先后采样次序有关（ 在ADC的初始化里设置 ），现在的采样次序为 ：
  *         Channel 13 -> Channel 12  -> Channel 11 ( 和硬件有关 ，目的先采集大电压再采集小电压)
*************************************************************************/
uint16_t ADC3_SampleOutputVoltageValue(void)
{
	uint32_t k,Channel;                                                     //三路ADC值使用哪路通道 
    uint16_t ChannelData[3];                                                //读取三路ADC值的临时保存区
	uint32_t VoltageValue,VoltageValue1 ,VoltageValue2;                     //保存采集到的电压值 ，（单位 ：0.01V）
	
	/********在ADC的配置里配置先采集的大电压通道电压（硬件有关）************/
	ChannelData[0] =	ADC3_SampleAverageValue( 1 );                         //采集到的最大电压（ Channel 13） ，硬件决定 
	ChannelData[1] =	ADC3_SampleAverageValue( 2 );                         //采集到的中间电压（ Channel 12） ，硬件决定 
	ChannelData[2] =	ADC3_SampleAverageValue( 3 );                         //采集到的最小电压（ Channel 11） ，硬件决定 

  /************比较使用哪路ADC的值*************/	
	for( Channel = 0 ; Channel < 3 ; Channel++ )                            // 3路ADC计数
	{
		if( ChannelData[Channel] < 4090 )   break ;                           // 大于4090的采样表示溢出 ，转到下一路ADC值的比较 
	}
  if( Channel >= 3 )	 Channel = 2 ;                                      // 3路adc都溢出 ，使用量程最大的一路ADC
 
	/**不同ADC通道的采样电压和实际输出电压的比例**/		
	switch( Channel  )                                                      //使用不同ADC通道的计算输出电压比例 ，比例已放大100倍
	{
		case 0 : k = 32400/100 + 100 ;break ; //324  + 100                    //实际为 ： 4.23
		case 1 : k = 32400/51  + 100 ;break ; //635  + 100                    //实际为 ： 7.35
		case 2 : k = 32400/20  + 100 ;break ; //1620 + 100                    //实际为 ：17.20
	}

  /********实际输出电压的计算方法**************/	
	VoltageValue1 = ChannelData[Channel] * k *3 / 4096;                     //参考电压为 3.083，把参考电压进行拆解计算 （ 参考电压整数部分）
  VoltageValue2 = ChannelData[Channel] * k *83/ 4096000;                  // （参考电压小数部分） 
	VoltageValue  = VoltageValue1 + VoltageValue2 ;                         // 和   

  return (uint16_t)VoltageValue ; 	                                      //返回实际的输出电压值 （ 电压被放大100倍 ）
}




/*************************************************************************
  * @brief  ADC3d读取输入的的电压值 
  * @param  无
  * @retval 无 
  * @notice 用于输入断电判断使用
*************************************************************************/
uint16_t ADC3_SampleInputVoltageValue(void)
{
	uint32_t InputChannelData ;
	uint32_t VoltageValue,VoltageValue1 ,VoltageValue2;                     //保存采集到的电压值 ，（单位 ：0.01V）
	uint32_t k  = 5100 / 2 + 100 ;                                          //实际为26.5
	
	/************************************/
	InputChannelData =	ADC3_SampleAverageValue( 4 );                       //采集到输入电压（ Channel 10） ，硬件决定 
 
	VoltageValue1 = InputChannelData * k *3 / 4096;                         //参考电压为 3.085，把参考电压进行拆解计算 （ 参考电压整数部分）
  VoltageValue2 = InputChannelData * k *83/ 4096000;                      // （参考电压小数部分） 
	VoltageValue  = VoltageValue1 + VoltageValue2 + 70 ;                    // 和  ,70为二极管的压降 

	return ((uint16_t)VoltageValue);                                        //返回实际的输出电压值 （ 电压被放大100倍 ）              
}



