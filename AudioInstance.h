/*
 =============================================================================

 ZenInstance.
 Created: 19 Jul 2021 12:52:08p
 Author:  Tim

 =============================================================================
 */

#pragma once

#include "utils/zen_memory.h"
#include "dsp/math.hpp"

namespace zen {


class AudioInstance
{
public:

	AudioInstance()
	{

	}

	AudioInstance(float sr, size_t blockSize, float(*random)(void))
	{

		setSampleRate(sr);
		random = random;
		blockSize=blockSize;
		clearOnAllocation_ = 0;
	}


	AudioInstance(float sr, char* memory, size_t memorysize, float(*random)(void)) : memory_(memory, memorysize)
	{

		setSampleRate(sr);
		random = random;
		clearOnAllocation_ = 0;
	}

	AudioInstance(float sr, char* memory, char* SDRAM_memory, size_t memorysize, size_t SDRAM_memorysize, float(*random)(void)) : memory_(memory, memorysize), SDRAM_(SDRAM_memory, SDRAM_memorysize)
	{

		setSampleRate(sr);
		random = random_;
		clearOnAllocation_ = 0;
	}

	~AudioInstance()
	{
	}

	void prepareToPlay(float sr, size_t bs, float(*rnd)(void))
	{
		setSampleRate(sr);
		random_ = rnd;
		blockSize_=bs;
		clearOnAllocation_ = 0;
	}

	inline float getSampleRate()
	{
		return sampleRate_;
	};



	inline float getInvSampleRate()
	{
		return invSampleRate_;
	}

	inline float getBlockSize()
	{
		return blockSize_;
	}

	void setSampleRate(float sr)
	{
		sampleRate_ = sr;
		invSampleRate_ = 1.0f/sampleRate_;
		twoPiTimesInvSampleRate_ = invSampleRate_ * ZEN_TWO_PI;
	}




private:
	float   sampleRate_; //!< The current audio sample rate. Set with LEAF_setSampleRate().
	float   invSampleRate_; //!< The inverse of the current sample rate.
	size_t     blockSize_; //!< The audio block size.
	float   twoPiTimesInvSampleRate_; //!<  Two-pi times the inverse of the current sample rate.
	float   (*random_)(void); //!< A pointer to the random() function provided on initialization.
	int     clearOnAllocation_; //!< A flag that determines whether memory allocated from the LEAF memory pool will be cleared.
	zen::Mempool memory_; //!< The default  mempool object.
	zen::Mempool SDRAM_; //!< The   mempool object for large memory (typically it will be external SDRAM chip in embedded system) .
	size_t header_size_; //!< The size in bytes of memory region headers within mempools.
};

} //namespace zen
