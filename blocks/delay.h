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
#include "../utils/debug.h"
#include "../dsp/TapeInterpolator.h"


enum delayTypes {
	DELAY_TYPE_TAPE = 0,
	DELAY_TYPE_DIGITAL,
	DELAY_TYPE_REVERB,
	DELAY_TYPE_NUM_ENUM
};

enum digitalDelayStates {
	DIGITAL_DELAY_IDLE = 0,
	DIGITAL_DELAY_BUSY,
};

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
		
		type_ = DELAY_TYPE_TAPE;
		Feedback::prepareToPlay();
		tapeInterpolator_.prepareToPlay(0, 48000, max_delay_samples*1000*Instance_.getInvSampleRate());
		for (int ch=0; ch < numChannels; ch++)
		{
			preDelayInterpolators_[ch].prepareToPlay(0, 48000, max_delay_samples*1000*Instance_.getInvSampleRate());
			delayLine_[ch].prepareToPlay();
		}
		last_delay_ = 0;
		digitalCounter_ = 0;
		newDigital_delay_  = 0;
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
		delayTime_ms_[channel] = clamp<T>(delay_ms, 0, max_delay_samples*1000*Instance_.getInvSampleRate());
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
	 * @param preDelays - pointer to channel buffer of delay offset values (this will not add delay to feedback loop)
	 * @param size - size of a block to process (and size of all of arrays above)
	 * @param outGain - output gain to apply to output array
	 * @return none.
	 */
	inline void processBlock(const T **input, T **output, float * delays, float ** preDelays,  size_t size, float * feedbacks, float outGain) {
		switch(type_)
		{
			case DELAY_TYPE_TAPE:
				processBlock_tape_(input, output, delays, preDelays, size, feedbacks, outGain);
				break;
			case DELAY_TYPE_DIGITAL:
				processBlock_digital_(input, output, delays, preDelays, size, feedbacks, outGain);
				break;
			default: ZEN_ERROR_HANDLER();
				
		}
	}
	
	inline void setDelayType(delayTypes newType)
	{
		type_ = newType;
	}
	
	
private:
	
	
	/**
	 * sets delay for a channel in milliseconds
	 * this call is overflow safe (it will clamp to min max values of delay line buffer size)
	 * @param time_ms - value in milliseconds
	 * @return samples
	 */
	inline T ms_to_samples_(T time_ms)
	{
		return clamp<T>(time_ms*Instance_.getSampleRate()*0.001f, 0, max_delay_samples);
	}
	
	inline void processBlock_tape_(const T **input, T **output, float * delays, float ** preDelays,  size_t size, float * feedbacks, float outGain) {
		
		
		for(int i = 0; i < size; i++)
		{
			float newDelay = tapeInterpolator_.tick(delays[i]);
			for (int ch=0; ch<numChannels; ch++)
			{
				float preDelay = preDelayInterpolators_[ch].tick(preDelays[ch][i]);
				float writeDelay =  newDelay + preDelay;
				setChannelDelay(writeDelay, ch);
				sample_[ch] = input[ch][i];
				
				readOutSample_[ch] = delayLine_[ch].Read(getChannelDelay_samples(ch));
				feedBackSample_[ch] = delayLine_[ch].Read(ms_to_samples_(newDelay));
			}
			Feedback::setLevel(feedbacks[i],1);
			Feedback::tick(sample_, feedBackSample_);
			
			for (int ch=0; ch<numChannels; ch++)
			{
				delayLine_[ch].Write(sample_[ch]);
				output[ch][i] = outGain * readOutSample_[ch];
			}
			
		}
		
	}
	
	inline void processBlock_digital_(const T **input, T **output, float * delays, float ** preDelays,  size_t size, float * feedbacks, float outGain) {
		
		float balance;
		static size_t counter;
		for(int i = 0; i < size; i++)
		{
			if(digital_delay_state_ == DIGITAL_DELAY_IDLE)
			{
				if(last_delay_ != delays[i])
				{
					digital_delay_state_ = DIGITAL_DELAY_BUSY;
					newDigital_delay_ = delays[i];
				}
			}
			if(digital_delay_state_ == DIGITAL_DELAY_BUSY)
			{
				if(counter<size)
				{
					counter++;
					balance = (float)counter/size;
				}
				else
				{
					counter = 0;
					last_delay_ = newDigital_delay_;
					digital_delay_state_ = DIGITAL_DELAY_IDLE;
				}
			}
			
			float oldDelay = last_delay_;
			float newDelay = newDigital_delay_;
			for (int ch=0; ch<numChannels; ch++)
			{
				float preDelay = preDelayInterpolators_[ch].tick(preDelays[ch][i]);
				float oldTotalDelay =  oldDelay + preDelay;
				float newTotalDelay =  newDelay + preDelay;
				setChannelDelay(oldTotalDelay, ch);
				sample_[ch] = input[ch][i];
				
				T oldFeedBackSample = delayLine_[ch].Read(ms_to_samples_(oldDelay));
				T newFeedBackSample = delayLine_[ch].Read(ms_to_samples_(newDelay));
				
				T oldReadOutSample = delayLine_[ch].Read(ms_to_samples_(oldTotalDelay));
				T newReadOutSample = delayLine_[ch].Read(ms_to_samples_(newTotalDelay));
				
				feedBackSample_[ch] = mixer(oldFeedBackSample, newFeedBackSample, balance);
				readOutSample_[ch] = mixer(oldReadOutSample, newReadOutSample, balance);
			}
			Feedback::setLevel(feedbacks[i],1);
			Feedback::tick(sample_, feedBackSample_);
			
			for (int ch=0; ch<numChannels; ch++)
			{
				delayLine_[ch].Write(sample_[ch]);
				output[ch][i] = outGain * readOutSample_[ch];
			}
			
		}
		
	}

	digitalDelayStates digital_delay_state_;
	T digitalCounter_;
	T last_delay_;
	T newDigital_delay_;
	T sample_[numChannels];
	T delayTime_ms_[numChannels];
	AudioInstance &Instance_;
	T readOutSample_[numChannels];
	T feedBackSample_[numChannels];
	zen::DelayLine<T, max_delay_samples> delayLine_[numChannels];
	delayTypes type_;
	zen::TapeInterpolator tapeInterpolator_;
	zen::TapeInterpolator preDelayInterpolators_[numChannels];
	
};

}//namespace zen

