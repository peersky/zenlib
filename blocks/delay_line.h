	// Delay line code significantly modified by Tim Pechersky
	// Copyright 2021 Tim Pechersky

	// Copyright 2014 Emilie Gillet.
	//
	// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
	//
	// Permission is hereby granted, free of charge, to any person obtaining a copy
	// of this software and associated documentation files (the "Software"), to deal
	// in the Software without restriction, including without limitation the rights
	// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	// copies of the Software, and to permit persons to whom the Software is
	// furnished to do so, subject to the following conditions:
	//
	// The above copyright notice and this permission notice shall be included in
	// all copies or substantial portions of the Software.
	//
	// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	// THE SOFTWARE.
	//
	// See http://creativecommons.org/licenses/MIT/ for more information.
	//
	// -----------------------------------------------------------------------------
	//

#pragma once

#include <algorithm>

namespace zen {

template<typename T, size_t maxDelay>
class DelayLine {
public:
	DelayLine() {
		Reset();
	}
	~DelayLine() { }
	
	void prepareToPlay() {
		Reset();
	}
	
	void Reset() {
		std::fill(&line_[0], &line_[maxDelay], T(0));
		delay_ = 1;
		writeIdx = 0;
	}
	
	inline void setDelay(size_t delay) {
		delay_ = delay;
	}
	
	inline size_t getDelay()
	{
		return delay_;
	}
	
	inline void Write(const T sample) {
		line_[writeIdx] = sample;
		writeIdx = (writeIdx - 1 + maxDelay) % maxDelay;
	}
	
	inline const T Allpass(const T sample, size_t delay, const T coefficient) {
		T read = line_[(writeIdx + delay) % maxDelay];
		T write = sample + coefficient * read;
		Write(write);
		return -write * coefficient + read;
	}
	
	inline const T WriteRead(const T sample, float delay) {
		Write(sample);
		return Read(delay);
	}
	
	/**
	 * delay line processing in block with modulator
	 *
	 * @param input - pointer to input values array
	 * @param output - output array pointer
	 * @param delays - delay (in samples array)
	 * @param modulator - pointer to modulator wavetable
	 * @param modDepth - gain of modulator wavetable
	 * @param size - size of a block to process
	 * @return none.
	 */
	inline void processBlock(const T *input, T *output, float * delays, float * modulator, float * modDepth, size_t size) {
		
		for(int i = 0; i < size; i++)
		{
			float delay = delays[i];// * (1 - modDepth[i] * modulator[i]);
			Write(input[i]);
			output[i] = Read(delay);
			delay_ = delay;
		}
		
	}
	

	
	
	inline const T Read() const {
		return line_[(writeIdx + delay_) % maxDelay];
	}
	
	inline const T Read(size_t delay) const {
		return line_[(writeIdx + delay) % maxDelay];
	}
	
	inline const T Read(float delay) const {
		ZEN_MAKE_INTEGRAL_FRACTIONAL(delay)
		const T a = line_[(writeIdx + delay_integral +1) % maxDelay];
		const T b = line_[(writeIdx + delay_integral + 2) % maxDelay];
		return a + (b - a) * delay_fractional;
	}
	
	inline const T ReadHermite(float delay) const {
		ZEN_MAKE_INTEGRAL_FRACTIONAL(delay)
		int32_t t = (writeIdx + delay_integral + maxDelay);
		const T xm1 = line_[(t - 1) % maxDelay];
		const T x0 = line_[(t) % maxDelay];
		const T x1 = line_[(t + 1) % maxDelay];
		const T x2 = line_[(t + 2) % maxDelay];
		const float c = (x1 - xm1) * 0.5f;
		const float v = x0 - x1;
		const float w = c + v;
		const float a = w + v + (x2 - x0) * 0.5f;
		const float b_neg = w + a;
		const float f = delay_fractional;
		return (((a * f) - b_neg) * f + c) * f + x0;
	}
	
private:
	size_t writeIdx;
	size_t delay_;
	T line_[maxDelay];
	
	ZEN_DISALLOW_COPY_AND_ASSIGN(DelayLine);
};

}  // namespace stmlib



