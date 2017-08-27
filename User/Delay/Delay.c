#include "Delay.h" 


void Delay1Us(__IO uint16_t nTime);
void Delay1Ms(__IO uint16_t nTime);



void Delay1Us(__IO uint16_t nTime)
{
	uint32_t Us;
  Us = 12 * nTime - 6 ; 	
	while(--Us);
}




void Delay1Ms(__IO uint16_t nTime)
{
	while(nTime--)
	{
		Delay1Us(1000);
	}
}


/*********************************************END OF FILE**********************/


