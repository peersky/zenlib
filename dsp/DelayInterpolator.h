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

/**
 A collection of structs to pass as the template argument when setting the
 interpolation type for the DelayLine class.
 */
namespace DelayLineInterpolationTypes
{
/**
 No interpolation between successive samples in the delay line will be
 performed. This is useful when the delay is a constant integer or to
 create lo-fi audio effects.
 
 @tags{DSP}
 */
struct None {};

/**
 Successive samples in the delay line will be linearly interpolated. This
 type of interpolation has a low compuational cost where the delay can be
 modulated in real time, but it also introduces a low-pass filtering effect
 into your audio signal.
 
 @tags{DSP}
 */
struct Linear {};

/**
 Successive samples in the delay line will be interpolated using a 3rd order
 Lagrange interpolator. This method incurs more computational overhead than
 linear interpolation but reduces the low-pass filtering effect whilst
 remaining amenable to real time delay modulation.
 
 @tags{DSP}
 */
struct Lagrange3rd {};

/**
 Successive samples in the delay line will be interpolated using 1st order
 Thiran interpolation. This method is very efficient, and features a flat
 amplitude frequency response in exchange for less accuracy in the phase
 response. This interpolation method is stateful so is unsuitable for
 applications requiring fast delay modulation.
 
 @tags{DSP}
 */
struct Thiran {};
}



class TapeInterpolator
{
public:
	TapeInterpolator()
	{
		
	}
	TapeInterpolator(float x0, uint32_t N, float max_value)
	{
		prepareToPlay(x0, N, max_value, 1);
	}
	TapeInterpolator(float x0, uint32_t N, float max_value, float outputGain)
	{
		prepareToPlay(x0, N, max_value, outputGain);
	}
	~TapeInterpolator()
	{
		
	}
	
	void prepareToPlay(float x0, uint32_t N, float max_value, float inputGain)
	{
		float N_flt = (float)N;
		N_ = N_flt;
		y_=x0;
		nf_ = 1/(max_value*inputGain);
		if_ = inputGain;
		counter=0;
		x_prev = x0;
		c2_ = 0;
	}
	
	void prepareToPlay(float x0, uint32_t N, float max_value)
	{
		prepareToPlay(x0, N, max_value, 1.0f);
		monitor.startAverageMonitor();
	}
	
	
	inline float iir (float input)
	{
		float delta = fabs(input - y_)*nf_;
		
		float a = delta*100/N_;
		float y = y_ * (1-a) + input  * a;
		return y;
	}
	
	inline float tick(float newValue)
	{
		
		float delta = fabs(newValue - y_)*nf_;
		
			//		a_ = 5.5/N_;
			//		if(delta>0.008f)
		if(0!=delta && delta<0.008f)
		{
			c2_ += 0.00001f* nf_;
		}
		else
		{
			c2_=0;
		}
		
		a_ = delta>0 ? delta*100/N_ + c2_ : 1.0f;
		a_ = a_>1.0f ? 1.0f : a_;
		y_ = y_ * (1-a_) + newValue  * a_;
		return y_;
	}
	
	inline void processBlock(float* io, size_t size, float gain)
	{
		for (int i = 0; i< size; i++ )
		{
			float retval =  gain * tick(io[i]);
			io[i] =   retval;
		}
	}
	
	
	float a_;
	
	
private:
	
	inline void find_a (float input)
	{
		float delta = fabs(input - y_);
		if(delta > delta_th_)
		{
			a_ = (( delta ) * 100/N_);
			a_ = a_ >1.0f ? 1.0f : a_;
		}
		else
		{
			a_ += delta_th_/100.0f;
		}
		a_ = a_ >1.0f ? 1.0f : a_;
	}
	
	inline float find_gravitational_a (float input)
	{
		float delta = fabs(input - y_)*nf_;
		
		float a_ = delta*100/N_;
		
		
		
		return a_;
	}
	
	zen::LoadMonitor monitor;
	float c2_;
	float x_prev;
	int counter;
	float y_;
	float N_;
	float if_;
	float delta_th_;
	float nf_;
};
} //namespace zen
