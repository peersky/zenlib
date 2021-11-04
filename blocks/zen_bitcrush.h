/*
 * bitcrush.h
 *
 *  Created on: Dec 15, 2015
 *      Author: janis_zaharans
 */

#ifndef BITCRUSH_H_
#define BITCRUSH_H_


#include <stdint.h>
#include "math.h"

static const float topval = 2147483648.0f;

typedef struct
{
	float res_in_cpy;
	float 	resolution;
	float		sample_rate;

	float		sh_counter;
	float		sh_value;
} bitcrush_t;


extern const int32_t	bit_resolution_mask[];


static inline void Bitcrush_SetSampleRate(bitcrush_t *bitcrush, float sample_rate)
{
	bitcrush->sample_rate = sample_rate;
}

//Expensive to call!
static inline void Bitcrush_SetResolution(bitcrush_t *bitcrush, float resolution)
{
	//only if resolution has changed it's value
	if(resolution!=bitcrush->res_in_cpy)
	{
		bitcrush->res_in_cpy=resolution;
		bitcrush->resolution = topval/(0.5f*pow(2.0f,(resolution*32.0f)));
	}
}


static inline float Bitcrush_ProcessSample(bitcrush_t *bitcrush, float input)
{
	bitcrush->sh_counter += bitcrush->sample_rate;

	if(bitcrush->sh_counter >= 1.0f) {
		bitcrush->sh_counter -= 1.0f;
		bitcrush->sh_value = floor(input / bitcrush->resolution)*bitcrush->resolution;

	}

	return bitcrush->sh_value;
}


#endif /* BITCRUSH_H_ */
