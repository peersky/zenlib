/*
 * envelope.h
 *
 *  Created on: 23 ???. 2019 ?.
 *      Author: Tim
 */

#ifndef AUDIO_ENVELOPE_H_
#define AUDIO_ENVELOPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "../utils/debug.h"
#include "math.h"
#include "zen_filters.h"
float envelope (float Signal, float Envelope, float evnc);

#ifdef __cplusplus
}
#endif
#endif /* AUDIO_ENVELOPE_H_ */
