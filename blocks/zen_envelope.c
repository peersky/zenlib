/*
 * envelope.c
 *
 *  Created on: 23 ???. 2019 ?.
 *      Author: Tim
 */

#include "zen_envelope.h"

float envelope (float Signal, float Envelope, float evnc)
{
	float retval;
	static float oldval;
	retval=fabs((float)fabs(Envelope));
	oldval=LPF_f32(oldval,retval,0.01f);
	retval=oldval*Signal;
	return evnc*retval;
}
