/*
 * modulation.h
 *
 *  Created on: 18 Mar 2020
 *      Author: Tim
 */

#ifndef AUDIO_MODULATION_H_
#define AUDIO_MODULATION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "zen_wave.h"

typedef enum
{
	MODULATION_BIDIR=0,
	MODULATION_UNIP,
	MODULATION_UNIN,
	MODULATION_NUM_ENUM
} Modulation_Type_EnumTypeDef;


typedef struct _Modulation_Struct_TypeDef
{
	float amplitude;
	Wave_Handle_TypeDef * pWave;
	Modulation_Type_EnumTypeDef direction;

}Modulation_Struct_TypeDef;

void modulation_set_freq(Modulation_Struct_TypeDef *pMod, float freq);
void modulation_set_Am(Modulation_Struct_TypeDef *pMod, float Am);
static float modulation_getval(Modulation_Struct_TypeDef *pMod);
void modulation_set_freq(Modulation_Struct_TypeDef *pMod, float freq);
float modulate(float base, Modulation_Struct_TypeDef *pMod);

#ifdef __cplusplus
}
#endif
#endif /* AUDIO_MODULATION_H_ */
