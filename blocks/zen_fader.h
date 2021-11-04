/*
 * fader.h
 *
 *  Created on: 13 ????. 2018 ?.
 *      Author: Tim
 */

#ifndef FADER_H_
#define FADER_H_

#include "stdint.h"
#include "../utils/debug.h"
#include "math.h"
#include "zen_filters.h"

//typedef struct
//{
//	uint8_t counter;
//	uint8_t activated;
//	uint32_t transition_length;
//	uint32_t subcounter;
//
//	float min_level;
//	float level;
//	uint8_t activated;
//	float step_size;
//	float transition_length_ms;
//	float Transition_Fs;
////	uint32_t subcounter;
//	uint32_t step_len_samples;
//	uint32_t step_num;
//
//}FadeInOut_f_ms_HandleTypeDef;

#define  MAX_ATT_LVL 121


typedef struct
{
	uint32_t counter;
	uint8_t activated;
	uint32_t transition_length;
	uint32_t db_in_a_step_1;
	uint32_t db_in_a_step_2;
	uint32_t level;
	uint32_t samples_per_step;
	uint32_t steps;
	uint32_t leftover;

}Fader_StructTypeDef;


typedef struct
{
	uint8_t activated;
	uint32_t Silence_length;
	Fader_StructTypeDef FadeIn;
	Fader_StructTypeDef FadeOut;
	uint32_t level_cpy;

}FadeInOut_HandleTypeDef;

typedef struct
{
	uint32_t counter;
	float min_level;
	volatile float level;
	uint8_t activated;
	float step_size;
	float transition_length_ms;
	float Transition_Fs;
	uint32_t subcounter;
	uint32_t step_len_samples;
	uint32_t step_num;

}FadeInOut_ms_HandleTypeDef;

void FadeInOut_Init(FadeInOut_HandleTypeDef * hFadeInOut, uint32_t transition_length, uint32_t silence_length);
float FadeInOut(float InVal, FadeInOut_HandleTypeDef * hFadeInOut);
float FadeIn(float InVal, Fader_StructTypeDef * hFadeIn);
float FadeOut(float InVal, Fader_StructTypeDef * hFadeOut);
float FadeInOut(float InVal, FadeInOut_HandleTypeDef * hFadeInOut);
void FadeInOut_Stereo(float * InVal, FadeInOut_HandleTypeDef * hFadeInOut);
float Fader(float Sample, float Divider);
float Fader_f(float Sample, float Level);
float Fader_log(float Sample, uint32_t db_level);

void Fader_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length, uint32_t starting_level);
void FadeIn_Stereo(float * InVal, Fader_StructTypeDef * hFadeIn);
void FadeOut_Stereo(float * InVal, Fader_StructTypeDef * hFadeOut);
void FadeIn_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length);
void FadeOut_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length);
float Fader_log_lpf(float Sample, uint32_t db_level);

float FadeInOut_ms(float InVal, FadeInOut_ms_HandleTypeDef * hFadeInOut);
void FadeInOut_ms_Init(FadeInOut_HandleTypeDef * hFadeInOut, float transition_length_ms, uint32_t Fs);
float FadeInOut_ms_Stereo(float InVal, FadeInOut_ms_HandleTypeDef * hFadeInOut, uint32_t channel);


static float gain_array[121]={1,0.891250938133746,0.794328234724282,0.707945784384138,0.630957344480193,0.562341325190349,0.501187233627272,0.446683592150963,0.398107170553497,0.354813389233575,0.316227766016838,0.281838293126445,0.251188643150958,0.223872113856834,0.199526231496888,0.177827941003892,0.158489319246111,0.141253754462275,0.125892541179417,0.112201845430196,0.100000000000000,0.0891250938133746,0.0794328234724281,0.0707945784384138,0.0630957344480193,0.0562341325190349,0.0501187233627272,0.0446683592150963,0.0398107170553497,0.0354813389233576,0.0316227766016838,0.0281838293126445,0.0251188643150958,0.0223872113856834,0.0199526231496888,0.0177827941003892,0.0158489319246111,0.0141253754462275,0.0125892541179417,0.0112201845430196,0.0100000000000000,0.00891250938133746,0.00794328234724281,0.00707945784384138,0.00630957344480193,0.00562341325190349,0.00501187233627273,0.00446683592150963,0.00398107170553497,0.00354813389233575,0.00316227766016838,0.00281838293126446,0.00251188643150958,0.00223872113856834,0.00199526231496888,0.00177827941003892,0.00158489319246111,0.00141253754462275,0.00125892541179417,0.00112201845430196,0.00100000000000000,0.000891250938133746,0.000794328234724281,0.000707945784384138,0.000630957344480193,0.000562341325190349,0.000501187233627273,0.000446683592150963,0.000398107170553497,0.000354813389233575,0.000316227766016838,0.000281838293126446,0.000251188643150958,0.000223872113856834,0.000199526231496888,0.000177827941003892,0.000158489319246111,0.000141253754462275,0.000125892541179417,0.000112201845430196,0.000100000000000000,8.91250938133746e-05,7.94328234724282e-05,7.07945784384137e-05,6.30957344480193e-05,5.62341325190349e-05,5.01187233627272e-05,4.46683592150964e-05,3.98107170553497e-05,3.54813389233575e-05,3.16227766016838e-05,2.81838293126446e-05,2.51188643150958e-05,2.23872113856834e-05,1.99526231496888e-05,1.77827941003892e-05,1.58489319246111e-05,1.41253754462276e-05,1.25892541179417e-05,1.12201845430196e-05,1.00000000000000e-05,8.91250938133746e-06,7.94328234724282e-06,7.07945784384137e-06,6.30957344480193e-06,5.62341325190349e-06,5.01187233627273e-06,4.46683592150964e-06,3.98107170553497e-06,3.54813389233575e-06,3.16227766016838e-06,2.81838293126445e-06,2.51188643150958e-06,2.23872113856834e-06,1.99526231496888e-06,1.77827941003892e-06,1.58489319246111e-06,1.41253754462276e-06,1.25892541179417e-06,1.12201845430196e-06,1.00000000000000e-06};

#endif /* FADER_H_ */
