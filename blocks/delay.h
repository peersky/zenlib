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

#include <cstdint>
#include "delay_line.h"
#include "feedback.h"
#include "compressor.h"
#include "../dsp/math.h"




namespace zen
{


template <typename T>
using feedbackHandler = T(*) (T);

template <typename T, size_t max_delay_samples, size_t numChannels>
class Delay : public zen::Feedback<T, numChannels>
{
public:
	using Feedback = zen::Feedback<T,numChannels>;
	
	Delay(AudioInstance &Zen) : Instance_(Zen)
	{
		
	}
	Delay(const feedbackHandler<T> handler,
			 AudioInstance &Zen) : Instance_(Zen)
	{
		
	}
	
	~Delay()
	{
	}
	
	void prepareToPlay()
	{
		Feedback::prepareToPlay();
		for (int ch=0; ch < numChannels; ch++)
		{
			delayLine_[ch].prepareToPlay();
		}
	}
	
	/**
	 * returns  delay in  with respect to offset of a particular channel
	 * @return delay in milliseconds
	 */
	inline T getChannelDelay_ms(size_t channel)
	{
		return delayTime_ms_[channel];
	}
	
	/**
	 * returns  delay in  with respect to offset of a particular channel
	 * @return delay in samples
	 */
	inline T getChannelDelay_samples(size_t channel)
	{
		return delayTime_ms_[channel]*Instance_.getSampleRate()*0.001f;
	}
	
	/**
	 * sets delay for a channel in milliseconds
	 * this call is overflow safe (it will clamp to min max values of delay line buffer size)
	 * @param delay - delay in ms
	 * @param channel - channel numer
	 */
	inline void setChannelDelay(T delay_ms, size_t channel)
	{
//		channel_delay_ms[channel] = clamp<T>(delay_ms, 0, max_delay_samples*1000*Instance_.getInvSampleRate());
//		delay_lines_[channel].setDelay(channel_delay_ms[channel]*Instance_.getSampleRate()*0.001f);
		delayTime_ms_[channel] = clamp<T>(delay_ms, 0, max_delay_samples*1000*Instance_.getInvSampleRate());
//		delay_lines_[channel].setDelay(channel_delay_ms[channel]);
	}
	
	/**
	 * Takes input buffers and outputs them in to output buffers
	 *
	 * @param input - pointers to channel buffer input[ch][sample]
	 * @param output - pointers to channel buffer output[ch][sample]
	 * @param delays -  pointer to array of delay values for each sample
	 * @param offsets - pointer to channel buffer of delay offset values
	 * @param size - size of a block to process (and size of all of arrays above)
	 * @return none.
	 */
	inline void processBlock(const T **input, T **output, float * delays, float ** offsets,  size_t size, float * feedbacks) {
		processBlock_(input, output, delays, offsets, size, feedbacks, 1.0f);
	}
	
	/**
	 * Takes input buffers and outputs them in to output buffers
	 *
	 * @param input - pointers to channel buffer input[ch][sample]
	 * @param output - pointers to channel buffer output[ch][sample]
	 * @param delays -  pointer to array of delay values for each sample
	 * @param offsets - pointer to channel buffer of delay offset values
	 * @param size - size of a block to process (and size of all of arrays above)
	 * @param outGain - output gain to apply to output array
	 * @return none.
	 */
	inline void processBlock(const T **input, T **output, float * delays, float ** offsets,  size_t size, float * feedbacks, float outGain) {
		processBlock_(input, output, delays, offsets, size, feedbacks, outGain);
	}
	
	

	
private:
	
	inline void processBlock_(const T **input, T **output, float * delays, float ** offsets,  size_t size, float * feedbacks, float outGain) {
		
		for(int i = 0; i < size; i++)
		{
			for (int ch=0; ch<numChannels; ch++)
			{
				sample_[ch] = input[ch][i];
				setChannelDelay(delays[i] + offsets[ch][i], ch);
				readOutSample_[ch] = delayLine_[ch].Read(getChannelDelay_samples(ch));
			}
			Feedback::setLevel(feedbacks[i],1);
			Feedback::tick(sample_, readOutSample_);

			for (int ch=0; ch<numChannels; ch++)
			{
				delayLine_[ch].Write(sample_[ch]);
				output[ch][i] = outGain * readOutSample_[ch];
			}
			
		}
		
	}
	
	T sample_[numChannels];
	T delayTime_ms_[numChannels];
	AudioInstance &Instance_;
	T readOutSample_[numChannels];
	zen::DelayLine<T, max_delay_samples> delayLine_[numChannels];
	
};

}//namespace zen

