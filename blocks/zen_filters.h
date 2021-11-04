/*
 * filters.h
 *
 *  Created on: 13 ????. 2018 ?.
 *      Author: Tim
 */

#ifndef AUDIO_FILTERS_H_
#define AUDIO_FILTERS_H_

#include "stdint.h"
float LPF_f32(float Current, float New, float c);


/**
 * @brief Instance structure for the floating-point FIR filter.
 */
typedef struct
{
	uint16_t numTaps;     /**< number of filter coefficients in the filter. */
	float *pState;    /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
	float *pCoeffs;   /**< points to the coefficient array. The array is of length numTaps. */
} arm_fir_instance_f32;


void arm_fir_init_f32(
					  arm_fir_instance_f32 * S,
					  uint16_t numTaps,
					  float * pCoeffs,
					  float * pState,
					  uint32_t blockSize);

void arm_fir_f32(
				 const arm_fir_instance_f32 * S,
				 float * pSrc,
				 float * pDst,
				 uint32_t blockSize);


#endif /* AUDIO_FILTERS_H_ */
