#include "VoltageMeasure.h"

uint16_t ADC3_SampleOutputVoltageValue(void);                             //ADC3��ȡʵ������ĵĵ�ѹֵ �� ����3·ADC�ıȽ� ��
uint16_t ADC3_SampleInputVoltageValue(void);                              //ADC3��ȡ�����ѹ�ĵ�ѹֵ 



/*************************************************************************
  * @brief  ADC3 ��ȡʵ������ĵĵ�ѹֵ �� ����3·ADC�ıȽ� ��
  * @param  ��
  * @retval �� 
  * @notice �Ƚϵ�ʱ���ADC���Ⱥ���������йأ� ��ADC�ĳ�ʼ�������� �������ڵĲ�������Ϊ ��
  *         Channel 13 -> Channel 12  -> Channel 11 ( ��Ӳ���й� ��Ŀ���Ȳɼ����ѹ�ٲɼ�С��ѹ)
*************************************************************************/
uint16_t ADC3_SampleOutputVoltageValue(void)
{
	uint32_t k,Channel;                                                     //��·ADCֵʹ����·ͨ�� 
    uint16_t ChannelData[3];                                                //��ȡ��·ADCֵ����ʱ������
	uint32_t VoltageValue,VoltageValue1 ,VoltageValue2;                     //����ɼ����ĵ�ѹֵ ������λ ��0.01V��
	
	/********��ADC�������������Ȳɼ��Ĵ��ѹͨ����ѹ��Ӳ���йأ�************/
	ChannelData[0] =	ADC3_SampleAverageValue( 1 );                         //�ɼ���������ѹ�� Channel 13�� ��Ӳ������ 
	ChannelData[1] =	ADC3_SampleAverageValue( 2 );                         //�ɼ������м��ѹ�� Channel 12�� ��Ӳ������ 
	ChannelData[2] =	ADC3_SampleAverageValue( 3 );                         //�ɼ�������С��ѹ�� Channel 11�� ��Ӳ������ 

  /************�Ƚ�ʹ����·ADC��ֵ*************/	
	for( Channel = 0 ; Channel < 3 ; Channel++ )                            // 3·ADC����
	{
		if( ChannelData[Channel] < 4090 )   break ;                           // ����4090�Ĳ�����ʾ��� ��ת����һ·ADCֵ�ıȽ� 
	}
  if( Channel >= 3 )	 Channel = 2 ;                                      // 3·adc����� ��ʹ����������һ·ADC
 
	/**��ͬADCͨ���Ĳ�����ѹ��ʵ�������ѹ�ı���**/		
	switch( Channel  )                                                      //ʹ�ò�ͬADCͨ���ļ��������ѹ���� �������ѷŴ�100��
	{
		case 0 : k = 32400/100 + 100 ;break ; //324  + 100                    //ʵ��Ϊ �� 4.23
		case 1 : k = 32400/51  + 100 ;break ; //635  + 100                    //ʵ��Ϊ �� 7.35
		case 2 : k = 32400/20  + 100 ;break ; //1620 + 100                    //ʵ��Ϊ ��17.20
	}

  /********ʵ�������ѹ�ļ��㷽��**************/	
	VoltageValue1 = ChannelData[Channel] * k *3 / 4096;                     //�ο���ѹΪ 3.083���Ѳο���ѹ���в����� �� �ο���ѹ�������֣�
  VoltageValue2 = ChannelData[Channel] * k *83/ 4096000;                  // ���ο���ѹС�����֣� 
	VoltageValue  = VoltageValue1 + VoltageValue2 ;                         // ��   

  return (uint16_t)VoltageValue ; 	                                      //����ʵ�ʵ������ѹֵ �� ��ѹ���Ŵ�100�� ��
}




/*************************************************************************
  * @brief  ADC3d��ȡ����ĵĵ�ѹֵ 
  * @param  ��
  * @retval �� 
  * @notice ��������ϵ��ж�ʹ��
*************************************************************************/
uint16_t ADC3_SampleInputVoltageValue(void)
{
	uint32_t InputChannelData ;
	uint32_t VoltageValue,VoltageValue1 ,VoltageValue2;                     //����ɼ����ĵ�ѹֵ ������λ ��0.01V��
	uint32_t k  = 5100 / 2 + 100 ;                                          //ʵ��Ϊ26.5
	
	/************************************/
	InputChannelData =	ADC3_SampleAverageValue( 4 );                       //�ɼ��������ѹ�� Channel 10�� ��Ӳ������ 
 
	VoltageValue1 = InputChannelData * k *3 / 4096;                         //�ο���ѹΪ 3.085���Ѳο���ѹ���в����� �� �ο���ѹ�������֣�
  VoltageValue2 = InputChannelData * k *83/ 4096000;                      // ���ο���ѹС�����֣� 
	VoltageValue  = VoltageValue1 + VoltageValue2 + 70 ;                    // ��  ,70Ϊ�����ܵ�ѹ�� 

	return ((uint16_t)VoltageValue);                                        //����ʵ�ʵ������ѹֵ �� ��ѹ���Ŵ�100�� ��              
}



