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

#include "../dsp/math.h"
#pragma once



namespace zen
{

template <typename T>
class SimpleCompressor
{

public:
	SimpleCompressor()
	{
	}
	SimpleCompressor(float maxVal, float thresholdRatio)
	{
		prepareToPlay(maxVal,thresholdRatio);
	}
	
	~SimpleCompressor()
	{
	}
	
	/**
	 * Call this before using compressor
	 * @param maxVal - output value that never should be reached
	 * @param thresholdCoeff - rate at which compression should be applied
	 * @return none
	 */
	void prepareToPlay(float maxVal, float thresholdCoeff)
	{
		maxVal_ = maxVal;
		thCompiled_ = maxVal * maxVal * thresholdCoeff * (1.0 - thresholdCoeff);
		thValue_ = thresholdCoeff*maxVal;
	}
	
	
	/**
	 * process one sample of compression
	 * Compressor will kick in after its over thCompiled_ value
	 * Most important feature is that when input value is moving forward infinity
	 * Return value will never reach over maxVal_
	 * @param newValue - sample
	 * @return compressed sample
	 */
	inline T tick(T newValue)
	{

		
		if(newValue > thValue_)
		{
			T tv = thCompiled_ / (newValue);
			return (maxVal_ - tv);
		}
		else if (newValue < -thValue_)
		{
			T tv = thCompiled_ / (newValue);
			return (-maxVal_ - tv);
		}
		else return newValue;
	}
	
	inline void processBlock(T *io, size_t size)
	{
		for(int i=0; i<size; i++)
		{
			io[i] = tick(io[i]);
		}
	}
	
private:
	T thCompiled_;
	T thValue_;
	T maxVal_;
};

} //namespace zen
