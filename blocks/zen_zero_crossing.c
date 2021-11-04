/*
 * zero_crossing.c
 *
 *  Created on: 17 ???. 2019 ?.
 *      Author: Tim
 */

#include "zen_zero_crossing.h"


void Zero_crossing_init_f(Zero_Monitor_StructTypeDef * Handle, float value)
{
	Handle->prev_val_f = value;
}

uint8_t Zero_crossing_check_f(Zero_Monitor_StructTypeDef * Handle, float value)
{
	uint8_t retval = value > 0.0f ? Handle->prev_val_f > 0.0f ? 0:1 : Handle->prev_val_f<0? 0:1;
	Handle->prev_val_f = value;
	return retval;
}
