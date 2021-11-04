/*
 * zero_crossing.h
 *
 *  Created on: 17 ???. 2019 ?.
 *      Author: Tim
 */

#ifndef AUDIO_ZERO_CROSSING_H_
#define AUDIO_ZERO_CROSSING_H_

#include "stdint.h"

typedef struct _Zero_Monitor_StructTypeDef
{
	int prev_val;
	float prev_val_f;
}Zero_Monitor_StructTypeDef;

uint8_t Zero_crossing_check_f(Zero_Monitor_StructTypeDef * Handle, float value);
void Zero_crossing_init_f(Zero_Monitor_StructTypeDef * Handle, float value);

#endif /* AUDIO_ZERO_CROSSING_H_ */
