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



namespace zen
{

enum FeedbackType
{
	FEEDBACK_TYPE_CROSSOVER=0,
	FEEDBACK_TYPE_PING_PONG,
	FEEDBACK_TYPE_DIRECT,
	FEEDBACK_TYPE_NUM_ENUM
};

template <typename T>
using handler_t = T(*) (T);


template <typename T, size_t numChannels, size_t numLines>
class Feedback {
public:
	
//	enum channelMatrixTypes {
//		DIRECT = 0,
//		PING_PONG,
//		CROSSOVER,
//	};
//
//	enum channelMatrixTypes {
//		DIRECT = 0,
//		PING_PONG,
//		CROSSOVER,
//	};
	
	
	Feedback()
	{
		for(int ch=0; ch<numChannels; ch++ )
		{
			for(int line=0; line<numLines; line++ )
			{
				DCBlockers_[ch][line].init(1.0f - 20.0f / 44100);
			}
		}
		setRoutes_(0);
		
	}
	Feedback(const handler_t<T> handler)
	{
		for(int ch=0; ch<numChannels; ch++ )
		{
			DCBlockers_[ch].init(1.0f - 20.0f / 44100);
		}
		setRoutes_(0);
	}
	
	~Feedback()
	{
	}
	
	
	/**
	 * Initializes feedback instance. Must be called before using
	 * @return none.
	 */
	void prepareToPlay()
	{
		setLevel(0, 1);
		type_ = FEEDBACK_TYPE_DIRECT;
		for(int ch=0; ch<numChannels; ch++)
		{
			for(int line=0; line<numLines; line++ )
			{
				newValue_[ch][line]=0;
				setChainProcessor(&this->shortCircuit_, ch, line);
			}
		}
		numChanelsNormalize_ = 1.0f/(T)numChannels;
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
	inline void setChainProcessor(handler_t<T> handler, size_t ch, size_t line)
	{
		handler_[ch][line]=handler;
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
		handler_[ch][0]=handler;
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
		
		setRoutes_(feedbackLevel);
//		switch(type_)
//		{
//			case FEEDBACK_TYPE_DIRECT:
//				setDirectFeedback_(feedbackLevel, inputGain);
//				break;
//			case FEEDBACK_TYPE_PING_PONG:
//				setPingPongFeedback_(feedbackLevel, inputGain);
//				break;
//			case FEEDBACK_TYPE_CROSSOVER:
//				setCrossoverFeedback_(feedbackLevel, inputGain);
//				break;
//			default: ZEN_ERROR_HANDLER();
//
//		}
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
		for(int toChannel = 0; toChannel<numChannels; toChannel++)
		{
			for(int toLine = 0; toLine<numLines; toLine++)
			{
				newValue_[toChannel][toLine] = 0;
				for(int fromChannel = 0; fromChannel<numChannels; fromChannel++)
				{
					for(int fromLine = 0; fromLine<numLines; fromLine++)
					{
						T returnValue = returns[fromChannel][fromLine];
						T returnMulti = returnsRoutingMatrix_[fromChannel][toChannel][fromLine][toLine];
						T inputValue = io[fromChannel][fromLine];
						T inputMulti = inputsRoutingMatrix_[fromChannel][toChannel][fromLine][toLine];
						
						
						newValue_[toChannel][toLine] += returnValue * returnMulti + inputValue*inputMulti;
					}
					
				}
				newValue_[toChannel][toLine] = handler_[toChannel][toLine](newValue_[toChannel][toLine]);
				newValue_[toChannel][toLine] = DCBlockers_[toChannel][toLine].tick(newValue_[toChannel][toLine]);
				newValue_[toChannel][toLine] = clamp<T>(newValue_[toChannel][toLine], -2.0f, 2.0f);
			}
		}
		memcpy(io, newValue_, numChannels*numLines*sizeof(T));
	}
	
	inline void setInputsRoutingMatrix (T (&newRoutingMatrix)[numChannels][numChannels][numLines][numLines])
	{
		memcpy(inputsRoutingMatrix_,newRoutingMatrix, sizeof(T)*(numChannels*numChannels*numLines*numLines));
	}
	
	inline void setReturnsRoutingMatrix (T (&newRoutingMatrix)[numChannels][numChannels][numLines][numLines])
	{
		memcpy(returnsRoutingMatrix_,newRoutingMatrix, sizeof(T)*(numChannels*numChannels*numLines*numLines));
	}
	
	
	
private:
	
//	inline void setDirectFeedback_(T level, T inputGain)
//	{
//		for(int to = 0; to<numChannels; to++)
//		{
//			for(int from = 0; from<numChannels; from++)
//			{
//				if(to==from)
//				{
//					returnsRoutingMatrix[from][to] = &returnsRoutingMatrix[from][0];
//					routingMatrix[numChannels+from][to] = inputGain;
//				}
//				else
//				{
//					routingMatrix[from][to] = 0;
//					routingMatrix[numChannels+from][to] = 0;
//				}
//
//			}
//		}
//	}
//	inline void setPingPongFeedback_(T level, T inputGain)
//	{
//
//		for(int from=0; from<numChannels; from++)
//		{
//			routingMatrix[from+numChannels][0] = inputGain*numChanelsNormalize_;
//			routingMatrix[from][0] = from == numChannels-1 ? level : 0;
//		}
//
//		for(int to = 1; to<numChannels; to++)
//		{
//			for(int from = 0; from<numChannels; from++)
//			{
//				int inversedToChannel = numChannels-1-to;
//				routingMatrix[from][to] = inversedToChannel==from ? level : 0;
//			}
//		}
//		routingMatrix[0][numChannels-1] = 1.0f;
//	}
//	inline void setCrossoverFeedback_(T level, T inputGain)
//	{
//		for(int to = 0; to<numChannels; to++)
//		{
//			for(int from = 0; from<numChannels; from++)
//			{
//				int inversedToChannel = numChannels-to;
//				routingMatrix[from][to] = inversedToChannel==from ? level : 0;
//			}
//		}
//	}
	
	
	static inline T shortCircuit_(T input)
	{
		return input;
	}
	
	inline void setRoutes_(T value)
	{
		for(int x=0; x<numChannels; x++)
		{
			for(int y=0; y<numChannels; y++)
			{
				for(int z=0; y<numLines; y++)
				{

					for(int v=0; y<numLines; y++)
					{

						returnsRoutingMatrix_[x][y][z][v]=value;
						inputsRoutingMatrix_[x][y][z][v]=value;
					}
				}
			}
		}
	}
	
	zen::DCBlocker DCBlockers_[numChannels][numLines];
	T numChanelsNormalize_;
	T newValue_[numChannels][numLines];
	handler_t<T> handler_[numChannels][numLines];
	FeedbackType type_;
	// holds multiplier cofficients [from][to]
	// on x axis its numChannels*2 so it can hold both inputs and outputs weights
//	T routingMatrix[numChannels*2*numLines][numChannels];
	T returnsRoutingMatrix_[numChannels][numChannels][numLines][numLines];
	T inputsRoutingMatrix_[numChannels][numChannels][numLines][numLines];
};

}//namespace zen

