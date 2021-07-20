	//
	//  zen_interpolator.h
	//  ZenPlugin - Shared Code
	//
	//  Created by Tim on 24/05/2021.
	//  Copyright © 2021 Teknologic. All rights reserved.
	//

#include <iostream>

#include "math.h"
#include "../utils/debug.h"
#pragma once


namespace zen {



class Interpolator
{
public:
	Interpolator()
	{
		
	}
	Interpolator(float x0)
	{
		prepareToPlay(x0, 1);
	}
	Interpolator(float x0, float outputGain)
	{
		prepareToPlay(x0, outputGain);
	}
	~Interpolator()
	{
		
	}
	
	void prepareToPlay(float x0, float inputGain)
	{
		v0_ = x0;
		if_ = inputGain;
	}
	
	void prepareToPlay(float x0)
	{
		prepareToPlay(x0, 1.0f);
	}
	
	
	
	inline void processBlock(float endValue, float* output, size_t size, float gain)
	{
		endValue = endValue*if_;
		for (int i = 0; i< size; i++ )
		{
				//process each step to find multiple points for interpolation
			float retval = lerp(v0_, endValue, (float) i/size);
			output[i] =   retval;
		}
		v0_ = endValue;
	}
	
	
	float a_;
	
	
private:
	float v0_;
	float if_;
};
} //namespace zen
