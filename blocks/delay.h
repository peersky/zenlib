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

template <typename T, size_t max_delay_samples, size_t numChannels>
class Delay : public zen::Feedback<T, numChannels, 1>
{
public:
	using Feedback = zen::Feedback<T,numChannels, 1>;

	typedef struct  {
		T*  delay;
		T* preDelay;
		T* nextChannelOffset[numChannels];
		T* feedback;
		T* outGain;
		T **io;
		size_t size;

	} delayProps;

	Delay(AudioInstance &Zen) : instance_(Zen)
	{

	}
	Delay(const handler_t<T> handler,
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
		preDelayInterpolator_.prepareToPlay(0, 48000, max_delay_samples*1000*instance_.getInvSampleRate());
		for (int ch=0; ch < numChannels; ch++)
		{
			for (int ch2=0; ch2 < numChannels; ch2++)
			{
				Feedback::setAllLinesToSameGain(1.0f, zen::SourceType::INPUTS);
				Feedback::setAllLinesToSameGain(1.0f, zen::SourceType::RETURNS);
			}
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
	 * @param preDelays - pointer to channel buffer of delay offset values (this will not add delay to feedback loop)
	 * @param size - size of a block to process (and size of all of arrays above)
	 * @param outGain - output gain to apply to output array
	 * @return none.
	 */
	inline void processBlock(const T **input, T **output, float * delays, float ** offsets, float ** preDelays,  size_t size, float * feedbacks, float outGain) {
		processBlock_(input, output, delays, size, feedbacks, outGain);
	}

	inline void setDelayType(delayTypes newType)
	{
		type_ = newType;
	}

	inline void processBlock(delayProps props) {
		processBlock_(props);
	}



private:

	inline void processBlock_(delayProps props) {
		const delayTypes type = type_;
		for(int i = 0; i < props.size; i++)
		{

			Feedback::setLevel(props.feedback[i],1);
			switch(type)
			{
				case delayTypes::TAPE :
					processBlock_tape_(props, i);
					break;
				case delayTypes::DIGITAL:
					processBlock_digital_(props, i);
					break;
				default: ZEN_ERROR_HANDLER();

			}
			Feedback::tick(sample_, feedbackSample_);
			writeAndOutputAllChannels(props, i);
		}
	}



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


	inline void writeAndOutputAllChannels(delayProps props, size_t index)
	{
		for (int ch=0; ch<numChannels; ch++)
		{
			delayLine_[ch].Write(sample_[ch][0]);
			props.io[ch][index] = props.outGain[0] * readOutSample_[ch][0];
			append_to_analyser(0,props.io[ch][index] );
			append_to_analyser(1,props.io[ch][index] );
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


	inline void getSamples_(delayProps props, size_t i, size_t ch, T newDelay)
	{

		sample_[ch][0] = props.io[ch][i];
		readOutSample_[ch][0] = delayLine_[ch].Read(getChannelDelay_samples(ch));
		feedbackSample_[ch][0] = delayLine_[ch].Read(ms_to_samples_(newDelay));
	}

	inline void processBlock_tape_(delayProps props, size_t i) {


		float newDelay = tapeInterpolator_.tick(props.delay[i]);
		float preDelay = preDelayInterpolator_.tick(props.preDelay[i]);
		if(isPreDelayConst)
		{
			for (int ch=0; ch<numChannels; ch++)
			{
				T writeDelay =  newDelay + preDelay;
				setChannelDelay(writeDelay, ch);
				getSamples_(props, i, ch, newDelay);

			}
		}
		else
		{
			for (int ch=0; ch<numChannels; ch++)
			{
				T writeDelay =  newDelay + props.preDelay[0];
				setChannelDelay(writeDelay, ch);
				getSamples_(props, i, ch, newDelay);
			}
		}


	}

	inline void processBlock_digital_(delayProps props, size_t i) {

		float balance;
		if(digitalState.state == DIGITAL_DELAY_IDLE)
		{
			if(digitalState.old != props.delay[i])
			{
				digitalState.state = DIGITAL_DELAY_BUSY;
				digitalState.newest = props.delay[i];
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
		float preDelay = preDelayInterpolator_.tick(props.preDelay[0]);
		for (int ch=0; ch<numChannels; ch++)
		{
			float oldTotalDelay =  oldDelay + preDelay;
			float newTotalDelay =  newDelay + preDelay;
			setChannelDelay(oldTotalDelay, ch);
			sample_[ch][0] = props.io[ch][i];

			T oldFeedBackSample = delayLine_[ch].Read(ms_to_samples_(oldDelay));
			T newFeedBackSample = delayLine_[ch].Read(ms_to_samples_(newDelay));

			T oldReadOutSample = delayLine_[ch].Read(ms_to_samples_(oldTotalDelay));
			T newReadOutSample = delayLine_[ch].Read(ms_to_samples_(newTotalDelay));

			feedbackSample_[ch][0] = mixer(oldFeedBackSample, newFeedBackSample, balance);
			readOutSample_[ch][0] = mixer(oldReadOutSample, newReadOutSample, balance);
		}

	}

	AudioInstance &instance_;
	digitalStateType_ digitalState;

	T sample_[numChannels][1];
	T feedbackSample_[numChannels][1];
	T readOutSample_[numChannels][1];
	T delayTime_ms_[numChannels];

	bool isOutGainConst = true;
	bool isPreDelayConst = true;
	bool isOffsetConst = true;


	T outGain_ = 1.0f;
	T preDelay_ = 0;
	T nextOffset[numChannels] = {0};


	zen::TapeInterpolator tapeInterpolator_;
	zen::TapeInterpolator preDelayInterpolator_;

	delayTypes type_;
	zen::DelayLine<T, max_delay_samples> delayLine_[numChannels];

};

}//namespace zen

