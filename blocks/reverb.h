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

#include "../zen.h"
#include <cstdint>
#include "delay_line.h"
#include "feedback.h"
#include "compressor.h"
#include "../dsp/math.h"
#include "../utils/debug.h"
#include "../dsp/DelayInterpolator.h"

namespace zen
{

	enum class reverbTypes
	{
		one = 0,
		two,
		three,
	};

	// template <typename T>
	// using feedbackHandler = T (*)(T);

	template <typename T, size_t max_delay_samples, size_t numChannels, size_t reverbLinesInChannel>
	class Reverb : public zen::Feedback<T, numChannels, reverbLinesInChannel>
	{
	public:
		using Feedback = zen::Feedback<T, numChannels, reverbLinesInChannel>;

		Reverb(AudioInstance &Zen) : instance_(Zen)
		{
		}
		Reverb(const handler_t<T> handler,
			   AudioInstance &Zen) : instance_(Zen)
		{
		}

		~Reverb()
		{
		}

		void prepareToPlay()
		{

			type_ = reverbTypes::one;
			Feedback::prepareToPlay();
			tapeInterpolator_.prepareToPlay(0, 48000, max_delay_samples * 1000 * instance_.getInvSampleRate());
			for (int ch = 0; ch < numChannels; ch++)
			{
				preDelayInterpolators_[ch].prepareToPlay(0, 48000, max_delay_samples * 1000 * instance_.getInvSampleRate());
				for (int line = 0; line < reverbLinesInChannel; line++)
				{
					delayLine_[ch][line].prepareToPlay();
					//				size_t index = (ch*reverbLinesInChannel) + line;
					//				std::cout << index << std::endl;
					filters_[ch][line].template set_f<FREQUENCY_EXACT>(ZEN_TWO_PI * 440.0f * instance_.getInvSampleRate());
				}
				delayTimeCoefficients[0] = 1.0f / 1.0f;
				delayTimeCoefficients[1] = 1.0f / 3.0f;
				delayTimeCoefficients[2] = 1.0f / 5.0f;
				delayTimeCoefficients[3] = 1.0f / 7.0f;
			}
		}

		void inline setDecay(T newDecay)
		{
			for (int ch = 0; ch < numChannels; ch++)
			{
				for (int line = 0; line < reverbLinesInChannel; line++)
				{
					//				size_t index = (ch*reverbLinesInChannel) + line;
					filters_[ch][line].template set_f<FREQUENCY_ACCURATE>(ZEN_TWO_PI * newDecay * 10000.0f * instance_.getInvSampleRate());
				}
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
			return delayTime_ms_[channel] * instance_.getSampleRate() * 0.001f;
		}

		/**
	 * sets delay for a channel in milliseconds
	 * this call is overflow safe (it will clamp to min max values of delay line buffer size)
	 * @param delay - delay in ms
	 * @param channel - channel numer
	 */
		inline void setChannelDelay(T delay_ms, size_t channel)
		{
			//		for(int line=0; line>reverbLinesInChannel; line++)
			//		{
			delayTime_ms_[channel] = clamp<T>(delay_ms, 0, max_delay_samples * 1000 * instance_.getInvSampleRate());
			//		}
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
		inline void processBlock(const T **input, T **output, float *delays, float **offsets, size_t size, float *feedbacks)
		{
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
		inline void processBlock(const T **input, T **output, float *delays, float **preDelays, size_t size, float *feedbacks, float outGain)
		{
			for (int i = 0; i < size; i++)
			{
				float newDelay = tapeInterpolator_.tick(delays[i]);
				//			Feedback::setType(FEEDBACK_TYPE_CROSSOVER);
				for (int ch = 0; ch < numChannels; ch++)
				{
					float preDelay = preDelayInterpolators_[ch].tick(preDelays[ch][i]);
					for (int line = 0; line < reverbLinesInChannel; line++)
					{

						Feedback::setLevel(feedbacks[i], 1);
						readOutSample_[ch][line] = delayLine_[ch][line].Read(newDelay * delayTimeCoefficients[line] + preDelay);

						//				//prepare new routing matrixes
						//				for (int fromCh=0; fromCh<numChannels; fromCh++)
						//				{
						//					for (int toCh=0; toCh<numChannels; toCh++)
						//					{
						//						for (int toLine=0; toLine<reverbLinesInChannel; toLine++)
						//						{
						//							for (int fromLine=0; fromLine<reverbLinesInChannel; fromLine++)
						//							{
						//								newInputsRoutingMatrix[fromCh][toCh][fromLine][toLine] = fromCh == toCh ? fromLine == toLine ? 1.0f : 0 : 0;
						//								newReturnsRoutingMatrix[fromCh][toCh][fromLine][toLine] = fromCh == toCh ? fromLine == toLine ? feedbacks[i] : 0 : 0;
						//							}
						//						}
						//					}
						//				}
						//
						//				// Load new routing matrices
						//				Feedback::setInputsRoutingMatrix(newInputsRoutingMatrix);
						//				Feedback::setReturnsRoutingMatrix(newReturnsRoutingMatrix);

						// Take input samples buffer, feedbackSamples buffer and mix them in to get value for writing in the write buffer.
						//					Feedback::tick(sample_, feedBackSample_);

						// Write all samples to their delay lines and output sample_'s to their outputs
						writeAndOutputAllChannels(output, i, outGain);
					}
				}
			}
		}

		inline void setFeedbackType(FeedbackType newType)
		{
			Feedback::setType(newType);
		}

		inline void setDelayType(reverbTypes newType)
		{
			type_ = newType;
		}

	private:
		inline void writeAndOutputAllChannels(T **output, size_t index, T outGain)
		{
			for (int ch = 0; ch < numChannels; ch++)
			{
				float lineMixResultOut = 0;
				for (int line = 0; line < reverbLinesInChannel; line++)
				{
					lineMixResultOut += readOutSample_[ch][line];
					delayLine_[ch][line].Write(sample_[ch][line]);
				}
				lineMixResultOut = lineMixResultOut / reverbLinesInChannel;
				//			lineMixResultOut = readOutSample_[0][0];

				output[ch][index] = clamp<T>(outGain * lineMixResultOut, -2.0f, 2.0f);
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
			return clamp<T>(time_ms * instance_.getSampleRate() * 0.001f, 0, max_delay_samples);
		}

		AudioInstance &instance_;
		T sample_[numChannels][reverbLinesInChannel];
		T feedBackSample_[numChannels][reverbLinesInChannel];
		T readOutSample_[numChannels][reverbLinesInChannel];
		T delayTime_ms_[numChannels];
		T delayTimeCoefficients[reverbLinesInChannel];
		zen::OnePole filters_[numChannels][reverbLinesInChannel];
		zen::DelayLine<T, max_delay_samples> delayLine_[numChannels][reverbLinesInChannel];

		float newInputsRoutingMatrix[numChannels][numChannels][reverbLinesInChannel][reverbLinesInChannel];
		float newReturnsRoutingMatrix[numChannels][numChannels][reverbLinesInChannel][reverbLinesInChannel];

		zen::TapeInterpolator tapeInterpolator_;
		zen::TapeInterpolator preDelayInterpolators_[numChannels];

		reverbTypes type_;
	};

} //namespace zen
