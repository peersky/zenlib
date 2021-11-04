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

#include "../utils/RingBuffer.h"
#include <cstdint>
#include "../zen.h"
#include "../dsp/math.h"
#include "../utils/debug.h"



namespace zen
{
enum WaveForm
{
	WAVETABLE_SINE=0,
	WAVETABLE_SINE2,
	WAVETABLE_SINE3,
	WAVETABLE_SAW,
	WAVETABLE_SQUARE,
	WAVETABLE_NUM_ENUM
};

class Wave
{
public:
	Wave(AudioInstance &Zen) : instance_(Zen)
	{
	}
	Wave(float *PtrTable, uint32_t size, WaveForm Form,
		 AudioInstance &Zen) : instance_(Zen)
	{
		prepareToPlay(PtrTable, size, Form);
	}

	~Wave()
	{
	}

	/**
	 * Call this on initialization
	 *
	 * @param PtrTable - pointer to wavetable buffer
	 * @param size - size of wavetable
	 * @param Form -  wave form
	 * @return none.
	 */
	void prepareToPlay(float *PtrTable, uint32_t size, WaveForm Form)
	{
		buffer_.prepareToPlay(PtrTable, size);

		switch(Form)
		{
			case WAVETABLE_SINE:
				generate_sine();
				break;
			default: ZEN_ERROR_HANDLER();
		}

	}

	/**
	 * sets frequency
	 *
	 * @param freq - new frequency
	 * @return none.
	 */
	inline void setFreq(float freq)
	{
		f_=freq;
	}

	/**
	 * sets phase
	 *
	 * @param phase - phase (0...1) where 1 equals to phase of  2*PI
	 * @return none.
	 */
	inline void set_phase (float phase)
	{
		float new_offset = buffer_.getSize() * phase;
		float offset = phase * buffer_.getSize();
		ptr_ = buffer_.point_index(ptr_,((int)offset-(int)new_offset));
		phase = phase;
	}

	/**
	 * Process one step of wavetable generation
	 * @return new sample.
	 */
	inline float tick()
	{
		float fltpart;
		float intpart;


		size_t size = buffer_.getSize();
		float norm_freq = f_*size/instance_.getSampleRate();
		fltpart = modff(norm_freq,&intpart);
		float new_balance = fltpart + balance_;
		if(new_balance>1.0f)
		{
			intpart++;
			new_balance-=1.0f;
		}
		balance_=new_balance;
		ptr_ = buffer_.point_index(ptr_,(int)intpart);

		float v1 = ptr_[0];
		float v2 = buffer_.point_index(ptr_,1)[0];
		float retval=mixer(v1,v2,balance_);
		return retval;

	}
	/**
	 * Process a block of wavetable samples with a frequency array
	 * Call this only when you have an array of frequencies that are changing
	 * @param io - pointer to output buffer
	 * @param freqs - pointer to frequencies array for each sample
	 * @param blockSize - size of block to process
	 * @return none.
	 */
	inline void processBlock(float *io, float *freqs, size_t BlockSize)
	{
		for(size_t i=0; i<BlockSize; i++)
		{
			size_t size = buffer_.getSize();
			float stride = freqs[i]*size/instance_.getSampleRate();
			ZEN_MAKE_INTEGRAL_FRACTIONAL(stride);
			float newBalance = stride_fractional + balance_;
			if(newBalance>1.0f)
			{
				stride_integral++;
				newBalance-=1.0f;
			}
			balance_=newBalance;
			ptr_ = buffer_.point_index(ptr_,stride_integral);
			float v1 = ptr_[0];
			float v2 = buffer_.point_index(ptr_,1)[0];
			float retval=mixer(v1,v2,balance_);
			f_ = freqs[i];
			io[i] = retval;

		}
	}

	/**
	 * Process a block of wavetable samples with a fixed frequency
	 * Call this if for the whole block frequency is constant
	 * @param io - pointer to output buffer
	 * @param blockSize - size of block to process
	 * @return none.
	 */
	inline void processBlock(float *io, size_t BlockSize)
	{
		for(size_t i=0; i<BlockSize; i++)
		{
			size_t size = buffer_.getSize();
			float stride = f_*size/instance_.getSampleRate();
			ZEN_MAKE_INTEGRAL_FRACTIONAL(stride);
			float new_balance = stride_fractional + balance_;
			if(new_balance>1.0f)
			{
				stride_integral++;
				new_balance-=1.0f;
			}
			balance_=new_balance;
			ptr_ = buffer_.point_index(ptr_,stride_integral);
			float v1 = ptr_[0];
			float v2 = buffer_.point_index(ptr_,1)[0];
			float retval=mixer(v1,v2,balance_);
			io[i] = retval;

		}
	}

private:
	float balance_;
	float phase_balance_;
	float * ptr_;
	float f_;
	float phase_;
	RingBuffer buffer_;
	AudioInstance &instance_;


	//Period is table size
	inline void generate_sine()
	{
		size_t size = buffer_.getSize();
		buffer_.resetHead();
		float val;
		for(size_t y = 0; y<size; y++)
		{
			val=sin(2.0f*ZEN_PI*(double)y/(double)size);
			buffer_.push(val);
		}
	}
};

} //namespace zen
