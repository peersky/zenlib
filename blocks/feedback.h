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
#include "../utils/debug.h"
#include "svf.h"
#include "../dsp/math.h"
#include "../utils/PatchRouter.h"

namespace zen
{

	enum FeedbackType
	{
		FEEDBACK_TYPE_CROSSOVER = 0,
		FEEDBACK_TYPE_PING_PONG,
		FEEDBACK_TYPE_DIRECT,
		FEEDBACK_TYPE_NUM_ENUM
	};

	enum SourceType
	{
		INPUTS = 0,
		RETURNS,
		SOURCE_TYPES_NUM_ENUM,
	};

	template <typename T, size_t numChannels, size_t numLines>
	class Feedback
	{

	public:
		Feedback()
		{
			prepareToPlay();
		}

		//TODO: add common handler to whole matrix
		~Feedback()
		{
		}

		/**
	 * Initializes feedback instance. Must be called before using
	 * @return none.
	 */
		void prepareToPlay()
		{
			for (int ch = 0; ch < numChannels; ch++)
			{
				for (int line = 0; line < numLines; line++)
				{
					DCBlockers_[ch][line].init(1.0f - 20.0f / 44100);
				}
			}
			setAllRouteLinesToSameGain_(1.0f, RETURNS);
			setAllRouteLinesToSameGain_(1.0f, INPUTS);
			type_ = FEEDBACK_TYPE_DIRECT;
			for (int ch = 0; ch < numChannels; ch++)
			{
				for (int line = 0; line < numLines; line++)
				{
					newValue_[ch][line] = 0;
				}
			}
			numChanelsNormalize_ = 1.0f / (T)numChannels;
		}

		/**
	 * Sets a pointer to a function that can be called to process feeback sample
	 *  Pass pointer to any FX processing needed by the feedback loop
	 *  or pass a pointer to shortCircuit_ from this class to not use the feedback loop
	 *   ^^^ see how its done in PrepareToPlay()
	 *
	 * @param handler - pointers to channel buffer input[ch][sample]
	 * @param ch - pointers to channel buffer output[ch][sample]
	 * @return none.
	 */
		inline void setChainProcessor(handler_t<T> handler, size_t ch)
		{
			handler_[ch] = handler;
		}

		/**
	 * Populates a routingMatrix with uniform weights according to selected type_
	 *
	 * @param feedbackLevel - weight for return channels (one for all)
	 * @param inputgain - weight for input channels (one for all)
	 * @return none.
	 */
		inline void setLevel(T feedbackLevel, T inputGain)
		{
			switch (type_)
			{
			case FEEDBACK_TYPE_DIRECT:
				setDirectFeedback_(feedbackLevel, inputGain);
				break;
			case FEEDBACK_TYPE_PING_PONG:
				setPingPongFeedback_(feedbackLevel, inputGain);
				break;
			case FEEDBACK_TYPE_CROSSOVER:
				setCrossoverFeedback_(feedbackLevel, inputGain);
				break;
			default:
				ZEN_ERROR_HANDLER();
			}
		}

		/**
	 * sets a feedback type (in current implementation you still have to rerun
	 * setLevel in order for this to make any audible changes
	 *
	 * @param newType - type of a new feedback
	 * @return none.
	 */
		inline void setType(FeedbackType newType)
		{
			type_ = newType;
		}

		/**
	 * takes a pointer to an array with input value per channel
	 * places newly calculated feedback values in to that array
	 *
	 * @param io - pointer to channels buffer
	 * @param returns - buffer containing values to be mixed
	 * with input values (feedbacks from the delay loop for instance)
	 * @return none.
	 */

		inline void tick(T (&io)[numChannels][numLines], T (&returns)[numChannels][numLines])
		{
			for (int toChannel = 0; toChannel < numChannels; toChannel++)
			{
				for (int toLine = 0; toLine < numLines; toLine++)
				{
					newValue_[toChannel][toLine] = 0;
					for (int fromChannel = 0; fromChannel < numChannels; fromChannel++)
					{
						// For each fromChannel process all routes in corresponding buffer
						//Feedback matrix should take care about levels normalization prior to running this!
						channelRoutes_[INPUTS][fromChannel][toChannel].process(io[fromChannel]);
						channelRoutes_[RETURNS][fromChannel][toChannel].process(returns[fromChannel]);
					}
					newValue_[toChannel][toLine] += io[toChannel][toLine] + returns[toChannel][toLine];
					//TODO: can these handlers move in to RoutingMatrix handlers
					newValue_[toChannel][toLine] = handler_[toChannel](newValue_[toChannel][toLine]);
					newValue_[toChannel][toLine] = DCBlockers_[toChannel][toLine].tick(newValue_[toChannel][toLine]);
					newValue_[toChannel][toLine] = clamp<T>(newValue_[toChannel][toLine], -2.0f, 2.0f);
				}
			}
			memcpy(io, newValue_, numChannels * numLines * sizeof(T));
		}

		/**
	 * replaces lines gains inputs matrix with a new one for a selected feedback route
	 * @param newRoutingMatrix - reference to a new matrix to copy from, matrix size should be equal to [numLines][numLines]
	 * @param fromCh - specifier for feedback from channel
	 * @param toCh - specifier for feedback to channel
	 * @return none.
	 */
		inline void setInputsRoutingMatrix(T (&newRoutingMatrix)[numLines][numLines], size_t fromCh, size_t toCh)
		{
			channelRoutes_[INPUTS][fromCh][toCh].setRoutesWeights(newRoutingMatrix);
		}

		/**
	 * replaces lines gains  return matrix with a new one for a selected feedback route
	 * @param newRoutingMatrix - reference to a new matrix to copy from, matrix size should be equal to [numLines][numLines]
	 * @param fromCh - specifier for feedback from channel
	 * @param toCh - specifier for feedback to channel
	 * @return none.
	 */
		inline void setReturnsRoutingMatrix(T (&newRoutingMatrix)[numLines][numLines], size_t fromCh, size_t toCh)
		{
			channelRoutes_[INPUTS][fromCh][toCh].setRoutesWeights(newRoutingMatrix);
		}

		/**
	 * sets all lines in all feedback channels to same gain
	 * sets both inputs and returns
	 *
	 * @param level - level to be set (0..1)
	 * @return none.
	 */
		inline void setAllLinesToSameGain(T level)
		{
			setAllRouteLinesToSameGain_(level, RETURNS);
			setAllRouteLinesToSameGain_(level, INPUTS);
		}

		/**
	 * sets all lines in all feedback channels to same gain
	 * applies only to one of matricies
	 *
	 * @param level - level to be set (0..1)
	 * @return none.
	 */
		inline void setAllLinesToSameGain(T level, SourceType matrix)
		{
			setAllRouteLinesToSameGain_(level, matrix);
		}

	private:
		inline void setAllRouteLinesToSameGain_(T level, SourceType routeName)
		{
			for (int to = 0; to < numChannels; to++)
			{
				for (int from = 0; from < numChannels; from++)
				{
					channelRoutes_[routeName][from][to].setLinesToSameWeight(level);
				}
			}
		}

		inline void setDirectFeedback_(T level, T inputGain)
		{
			for (int to = 0; to < numChannels; to++)
			{
				for (int from = 0; from < numChannels; from++)
				{
					channelRoutes_[INPUTS][from][to].setCommonGain(from == to ? inputGain : 0);
					channelRoutes_[RETURNS][from][to].setCommonGain(from == to ? level : 0);
				}
			}
		}

		// Each input contribute its noramlized value to 0 channel
		// Each return is being passed to inverse channel input.
		inline void setPingPongFeedback_(T level, T inputGain)
		{
			T tval = inputGain * numChanelsNormalize_;
			for (int to = 0; to < numChannels; to++)
			{
				for (int from = 0; from < numChannels; from++)
				{
					int inversedToChannel = numChannels - 1 - to;
					channelRoutes_[INPUTS][from][to].setCommonGain(to == 0 ? tval : 0);
					channelRoutes_[RETURNS][from][to].setCommonGain(from == inversedToChannel ? level : 0);
				}
			}
		}
		//Each input is being routed directly
		//Each return is routed inverse
		inline void setCrossoverFeedback_(T level, T inputGain)
		{
			for (int to = 0; to < numChannels; to++)
			{
				for (int from = 0; from < numChannels; from++)
				{

					int inversedToChannel = numChannels - 1 - to;
					channelRoutes_[INPUTS][from][to].setCommonGain(from == to ? inputGain : 0);
					channelRoutes_[RETURNS][from][to].setCommonGain(from == inversedToChannel ? level : 0);
				}
			}
		}

		static inline T shortCircuit_(T input)
		{
			return input;
		}

		zen::DCBlocker DCBlockers_[numChannels][numLines];
		T numChanelsNormalize_;
		T newValue_[numChannels][numLines];
		handler_t<T> handler_[numChannels];
		FeedbackType type_;
		zen::PatchRouter<T, numLines, numLines> channelRoutes_[2][numChannels][numChannels];
	};

} //namespace zen
