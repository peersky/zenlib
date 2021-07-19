/*
 * debug.c
 *
 *  Created on: 12 ìàð. 2018 ã.
 *      Author: Teem
 */

#include "zen_debug.h"

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */


char Debug_getchar(void)
{
	char input;
	scanf(" %c", &input);

	return input;
}

uint32_t Debug_gethex(void)
{
	int input;
	scanf(" %x", &input);

	return input;
}




//void debug_avg_mcu_load(void)
//{
//	static 	uint32_t cnt = 0;
//	int new_val = DWT->CYCCNT;
//	static uint32_t timestamp=0;
//	static uint32_t dump=0;
//	uint32_t cycle_length = DWT->CYCCNT-prev_cycle;
//	prev_cycle = DWT->CYCCNT;
//
//	if(HAL_GetTick()-timestamp>1000)
//	{
//		timestamp=HAL_GetTick();
//		float avg_mcu_load = dump*100.0f/(HAL_RCC_GetSysClockFreq());
//		dump=0;
//
//		debug_print("%s","MCU load = ");
//		debug_print("%.1f [%]\n",avg_mcu_load);
//	}
//	else
//	{
//		dump+=cycle_length;
//	}
//}

#ifdef  _ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
 void assert_failed(uint8_t* file, uint32_t line)
{
#ifdef DEBUG
	printf("Wrong parameters value: file %s on line %d\r\n", file, line);
#else
	while(0);
#endif

}

#endif
