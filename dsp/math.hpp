/*
 MIT License

 Copyright (c) 2021 Tim Pechersky

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once
#include <algorithm>
#include <math.h>

#define ZEN_MAKE_INTEGRAL_FRACTIONAL(x)             \
	int32_t x##_integral = static_cast<int32_t>(x); \
	float x##_fractional = x - static_cast<float>(x##_integral);

#define ZEN_PI (3.14159265358979f)
#define ZEN_TWO_PI (6.28318530717958f)
#define ZEN_HALF_PI (1.570796326794897f)

/**
 *
 *   Get volume levels for equal-power crossfade
 * @param fade - fade state (0 -> both volumes are at 0.707 level)
 * @param volumes - pointer to two channel array of volumes
 */
inline void crossfade(float fade, float *volumes)
{
	volumes[0] = sqrtf(0.5f * (1.0f + fade));
	volumes[1] = sqrtf(0.5f * (1.0f - fade));
}

/**
 *
 *  	Mix two channels in equal power state
 * @param fade - fade state (0 -> both volumes are at 0.707 level)
 * @param sample0 - sample for channel0
 * @param sample1 - sample for channel1
 * @return crossfaded result of two samples at given fade state
 */
inline float crossfade_tick(float sample0, float sample1, float fade)
{

	float volume0 = sqrtf(0.5f * (1.0f + fade));
	float volume1 = sqrtf(0.5f * (1.0f - fade));
	return volume0 * sample0 + volume1 * sample1;
}

inline float lerp(float a, float b, float t)
{
	// Exact, monotonic, bounded, determinate, and (for a=b=0) consistent:
	if ((a <= 0 && b >= 0) || (a >= 0 && b <= 0))
		return t * b + (1 - t) * a;

	if (t == 1)
		return b; // exact
				  // Exact at t=0, monotonic except near t=1,
				  // bounded, determinate, and consistent:
	const float x = a + t * (b - a);
	return (t > 1) == (b > a) ? std::max(b, x) : std::min(b, x); // monotonic near t=1
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
	if (x < min)
		x = min;
	if (x > max)
		x = max;
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
inline float softLimit_0dbfs(float x)
{
	return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

/**
 * Linear mix of two samples
 *
 * @param sample0 - input value
 * @param sample1 - input value
 * @param balance - balance (0 -> sample0 100%, 1-> sample1 100%)
 * @return mixed sample
 */
inline float mixer(float sample0, float sample1, float balance)
{
	float RetVal;
	sample1 = sample1 * balance;
	sample0 = sample0 * (1.0f - balance);
	RetVal = sample1 + sample0;
	return RetVal;
}
