//
//  patchRouter.h
//  Zen Delay Plugin
//
//  Created by Tim on 30/08/2021.
//  Copyright © 2021 Tim Pechersky aka peersky. All rights reserved.
//



#pragma once
#include "../dsp/Matrix.h"
namespace zen
{


template <typename T>
using handler_t = T(*) (T);


/**
 * MatrixRoute is a generic class that can handle routing channels from input(s) to output(s)
 * It inherits from Matrix class that allows to assign and apply weight for each route
 * routesHandlers_ stores function callbacks that you can call for each route to do additional processing between input and output
 * Additionally you can add inputGain to the MatrixRoute that way all input values can be scaled according to ones needs
 *
 * Usage  -  to use
 */
template <typename T, size_t num_x, size_t num_y>
class PatchRouter : public zen::Matrix<T, num_x, num_y>
{
public:
	using ScalarsMatrix = Matrix<T, num_x, num_y>;
	PatchRouter ()
	{
		for(int x=0; x<num_x; x++)
		{
			for(int y=0; y<num_y; y++ )
			{
				setHandler(&this->shortCircuit_, x, y);
			}
		}
	}
	
	/**
	 * Sets a pointer to a function that can be called to process sample at given index
	 */
	inline void setHandler(handler_t<T> handler, size_t x, size_t y)
	{
		routesHandlers_[x][y]=handler;
	}
	
	/**
	 *  Take input column and route it accordingly
	 *  1. Multiplies by inputGain_
	 *  2. gets a product of input column and weights
	 *  3. processes each resulting value with handler callback function
	 *  4. Summs all resulting values and returns result
	 *  @param input - reference to input values column
	 *  @return resulting value
	 */
	inline T getOutputValue (T (&input)[num_y])
	{
		scaleInput_(input, inputGain_);
		ScalarsMatrix::getProduct1D(input);
		T retval = getHandlersSumm_(input);
		
		return retval;
	}
	
	/**
	 *  Take input column and route it accordingly
	 *  1. Multiplies by inputGain_
	 *  2. get a product of input column and weights
	 *  3. get a product of previous step with results produced by handler callback functions
	 *  @param io - reference to io buffer
	 *  @return none.
	 */
	inline void process (T (&io)[num_y])
	{
		scaleInput_(io, inputGain_);
		ScalarsMatrix::getProduct1D(io);
		processHandlers_(io);
	}
	
	
	/**
	 * Set a common gain that each input signal on each line is muliplied by to
	 *
	 * @param value -  gain
	 * @return none.
	 */
	inline void setCommonGain(T value)
	{
		inputGain_ = value;
	}
	
	/**
	 * Sets all weights to be equal to
	 *
	 * @param weight -  value to set for each element weight
	 * @return none.
	 */
	inline void setLinesToSameWeight(T weight)
	{
		for(int x=0; x<num_x; x++)
		{
			for(int y=0; y<num_y; y++)
			{
				ScalarsMatrix::setElement(weight, x, y);
			}
		}
	}
	
	/**
	 * Copies weights from a reference matrix
	 *
	 * @param newWeights -  reference new matrix of size [num_x][num_y].
	 * @return none.
	 */
	inline void setRoutesWeights(T (&newWeights)[num_x][num_y])
	{
		ScalarsMatrix::setMatrix(newWeights);
	}
	
private:
	
	inline void scaleInput_(T (&input)[num_y], T gain)
	{
		for(int y=0; y<num_y; y++)
		{
			input[y]=input[y]*gain;
		}
	}
	
	inline T getHandlersSumm_(T (&input)[num_y])
	{
		T retval = 0;
		for(int x=0; x<num_x; x++)
		{
			for(int y=0; y<num_y; y++)
			{
				retval +=  routesHandlers_[x][y](input[y]);
			}
		}
		return retval;
	}
	
	inline void processHandlers_(T (&io)[num_y])
	{
		T inputs[num_y];
		memcpy(inputs, io, num_y*sizeof(T));
		
		for(int y=0; y<num_y; y++)
		{
			io[y] = 0;
			for(int x=0; x<num_x; x++)
			{
				io[y] +=  routesHandlers_[x][y](inputs[y]);
			}
		}
	}
	
	
	
	 static inline T shortCircuit_(T input)
	{
		return input;
	}
	
	T inputGain_;
	handler_t<T> routesHandlers_[num_x][num_y];
	
	
	
};
} // namespace zen
