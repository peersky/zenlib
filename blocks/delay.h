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
#include "../dsp/DelayInterpolator.h"





namespace zen
{

enum class delayTypes {
	TAPE = 0,
	DIGITAL,
};


template <typename T>
using feedbackHandler = T(*) (T);

template <typename T, size_t max_delay_samples, size_t numChannels>
class Delay : public zen::Feedback<T, numChannels>
{
public:
	using Feedback = zen::Feedback<T,numChannels>;
	
	
	Delay(AudioInstance &Zen) : instance_(Zen)
	{
		
	}
	Delay(const feedbackHandler<T> handler,
		  AudioInstance &Zen) : instance_(Zen)
	{
		
	}
	
	~Delay()
	{
	}
	


	
	void prepareToPlay()
	{
		
		type_ = delayTypes::TAPE;
		Feedback::prepareToPlay();
		tapeInterpolator_.prepareToPlay(0, 48000, max_delay_samples*1000*instance_.getInvSampleRate());
		for (int ch=0; ch < numChannels; ch++)
		{
			preDelayInterpolators_[ch].prepareToPlay(0, 48000, max_delay_samples*1000*instance_.getInvSampleRate());
			delayLine_[ch].prepareToPlay();
			
		}
		digitalState.old = 0;
		digitalState.counter = 0;
		digitalState.newest  = 0;
		digitalState.size = 512;
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
		return delayTime_ms_[channel]*instance_.getSampleRate()*0.001f;
	}
	
	/**
	 * sets delay for a channel in milliseconds
	 * this call is overflow safe (it will clamp to min max values of delay line buffer size)
	 * @param delay - delay in ms
	 * @param channel - channel numer
	 */
	inline void setChannelDelay(T delay_ms, size_t channel)
	{
		delayTime_ms_[channel] = clamp<T>(delay_ms, 0, max_delay_samples*1000*instance_.getInvSampleRate());
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
		const delayTypes type = type_;
		for(int i = 0; i < size; i++)
		{
			Feedback::setLevel(feedbacks[i],1);
			switch(type)
			{
				case delayTypes::TAPE :
					processBlock_tape_(input, delays, preDelays, i);
					break;
				case delayTypes::DIGITAL:
					processBlock_digital_(input, delays, preDelays, i);
					break;
				default: ZEN_ERROR_HANDLER();
					
			}
			Feedback::tick(sample_, feedBackSample_);
			writeAndOutputAllChannels(output, i, outGain);
		}
	}
	
	inline void setDelayType(delayTypes newType)
	{
		type_ = newType;
	}
	
	
private:
	enum digitalDelayStates {
		DIGITAL_DELAY_IDLE = 0,
		DIGITAL_DELAY_BUSY,
	};
	using digitalStateType_ = struct {
		
		digitalDelayStates state;
		T counter;
		T old;
		T newest;
		size_t size;
	};
	
	
	inline void writeAndOutputAllChannels(T **output, size_t index, T outGain)
	{
		for (int ch=0; ch<numChannels; ch++)
		{
			delayLine_[ch].Write(sample_[ch]);
			output[ch][index] = outGain * readOutSample_[ch];
		}
	}
	
	/**
	 * sets delay for a channel in milliseconds
	 * this call is overflow safe (it will clamp to min max values of delay line buffer size)
	 * @param time_ms - value in milliseconds
	 * @return samples
	 */
	inline T ms_to_samples_(T time_ms)
	{
		return clamp<T>(time_ms*instance_.getSampleRate()*0.001f, 0, max_delay_samples);
	}
	
	
	inline void processBlock_tape_(const T **input, float * delays, float ** preDelays, size_t i) {
		
		
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
		
		
	}
	
	inline void processBlock_digital_(const T **input, float * delays, float ** preDelays, size_t i) {
		
		float balance;
		if(digitalState.state == DIGITAL_DELAY_IDLE)
		{
			if(digitalState.old != delays[i])
			{
				digitalState.state = DIGITAL_DELAY_BUSY;
				digitalState.newest = delays[i];
			}
		}
		if(digitalState.state == DIGITAL_DELAY_BUSY)
		{
			if(digitalState.counter<digitalState.size)
			{
				digitalState.counter++;
				balance = (float)digitalState.counter/digitalState.size;
			}
			else
			{
				digitalState.counter = 0;
				digitalState.old = digitalState.newest;
				digitalState.state = DIGITAL_DELAY_IDLE;
			}
		}
		
		float oldDelay = digitalState.old;
		float newDelay = digitalState.newest;
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
		
	}
	
	AudioInstance &instance_;
	digitalStateType_ digitalState;
	
	T sample_[numChannels];
	T feedBackSample_[numChannels];
	T readOutSample_[numChannels];
	T delayTime_ms_[numChannels];
	
	
	
	zen::TapeInterpolator tapeInterpolator_;
	zen::TapeInterpolator preDelayInterpolators_[numChannels];
	
	delayTypes type_;
	zen::DelayLine<T, max_delay_samples> delayLine_[numChannels];
	
};

}//namespace zen

