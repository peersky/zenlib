//
//  dsp.h
//  ZenPlugin
//
//  Created by Tim on 26/06/2021.
//  Copyright © 2021 Teknologic. All rights reserved.
//

#pragma once
#include <algorithm>

#define MAKE_INTEGRAL_FRACTIONAL(x) \
int32_t x ## _integral = static_cast<int32_t>(x); \
float x ## _fractional = x - static_cast<float>(x ## _integral);


inline float lerp(float a, float b, float t) {
		// Exact, monotonic, bounded, determinate, and (for a=b=0) consistent:
	if((a<=0 && b>=0) || (a>=0 && b<=0)) return t*b + (1-t)*a;
	
	if(t==1) return b;                        // exact
											  // Exact at t=0, monotonic except near t=1,
											  // bounded, determinate, and consistent:
	const float x = a + t*(b-a);
	return t>1 == b>a ? std::max(b,x) : std::min(b,x);  // monotonic near t=1
}


/**
 * Takes value with ranges and returns value limited to the range
 *
 * @param x - input value
 * @param min - minimum value
 * @param max - maximum value
 * @return value in the range.
 */
template <typename T>
inline T clamp(T x, T min, T max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}


/**
 * takes softly clips so 1.0f (0dBFs) is max
 * this function will have around 22% distortion
 * at input of 0dBFs
 *  3% distortion at -6 dBFs
 *  0.4% distortion at  -12dBFs
 * @param x - input value
 * @return soft lmited value
 */
inline float softLimit_0dbfs(float x) {
	return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}
