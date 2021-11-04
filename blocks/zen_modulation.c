/*
 * modulation.c
 *
 *  Created on: 18 Mar 2020
 *      Author: Tim
 */

#include "zen_modulation.h"
#include "../zen_debug.h"


static float modulation_getval(Modulation_Struct_TypeDef *pMod)
{
	switch(pMod->direction)
	{
	case MODULATION_BIDIR:
		return pMod->amplitude*wave_read(pMod->pWave);
		break;
	case MODULATION_UNIN:
		return (pMod->amplitude*wave_read(pMod->pWave))-pMod->amplitude;
		break;
	case MODULATION_UNIP:
		return (pMod->amplitude*wave_read(pMod->pWave))+pMod->amplitude;
		break;
	}
	return 0.0f;
}

void modulation_set_freq(Modulation_Struct_TypeDef *pMod, float freq)
{
	pMod->pWave->f = freq;
}

void modulation_set_Am(Modulation_Struct_TypeDef *pMod, float Am)
{
	pMod->amplitude = Am;
}


float modulate(float base, Modulation_Struct_TypeDef *pMod)
{
	float modulation_float = modulation_getval(pMod);
	float result = base+modulation_float;
	result = result<0?0:result;
	return result;
}


void delay_update_modulation_params(Modulation_Struct_TypeDef *pMod, float amplitude, float freq, float phase)
{
    
    assert_param(amplitude>0.0f);
    assert_param(freq>0.0f);
    assert_param(phase>0.0f);

    modulation_set_freq(pMod,freq);
    modulation_set_Am(pMod,amplitude);
}
