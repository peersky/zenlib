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

#include "stdint.h"
#include "stdlib.h"
#include "debug.h"

namespace zen
{

	class RingBuffer
	{
	public:
		RingBuffer()
		{
		}

		RingBuffer(float *ptr, size_t BufferSize)
		{
			assert_param(BufferSize > 0);
			assert_param(ptr != 0);

			pBuf_ = ptr;
			size_ = BufferSize;
			Head_ = pBuf_ + 1;
			Tail_ = pBuf_;
		}

		~RingBuffer()
		{
		}

		void prepareToPlay(float *ptr, size_t BufferSize)
		{
			assert_param(BufferSize > 0);
			assert_param(ptr != 0);

			pBuf_ = ptr;
			size_ = BufferSize;
			Head_ = pBuf_ + 1;
			Tail_ = pBuf_;
		}

		inline void swap()
		{
			float *temp = Tail_;
			Tail_ = Head_;
			Head_ = temp;
		}

		inline float *point_index(float *FromAddr, signed int offset)
		{
			assert_param(abs(offset) < size_);
			float *retval;
			uint32_t index;

			//	if(pRing->hParent!=NULL)
			//	{
			//		if(FromAddr<pRing->pBuf)
			//		{
			//			float * temp_index;
			//			temp_index = FromAddr - pRing->hParent->pBuf;
			//			temp_index = pRing->hParent->size+temp_index;
			//			FromAddr = &pRing->hParent->pBuf[temp_index];
			//		}
			//	}

			//1. ensure offset is bounded by buffer size -13 % 9 = -4 (offset -13 on buffer size 9 means to move 4 addresses backwards)
			offset = offset % size_;
			//2. convert negative offsets to positive. Eg. -4 addresses from buffer size 9 -> 9+(-4) = 5 forward
			uint32_t offset_u = offset < 0 ? (int)size_ + offset : offset;

			//3. We have pointer somewhere within ring buffer, instead we will find it's index
			index = (uint32_t)(FromAddr - pBuf_); //head index

			//4. And will calculate offset from it's index, so we can ensure boundary check with index % size
			index = (index + offset_u) % size_;

			//5. Value to return is a pointer to new index we just found
			retval = &pBuf_[index];

			//	if(pRing->hParent!=NULL)
			//	{
			//		retval = point_index(pRing->hParent,pRing->pBuf,index);
			//	}

			return retval;
		}

		inline float *Ring_Set_Tail(signed int Length)
		{
			float *retval;
			//	assert_param(abs(Length)<=pRing->size);
			Length = Length % size_;
			retval = point_index(Head_, -1 * Length);

			Tail_ = retval;
			//	set_youngest_tail(pRing,retval);
			return retval;
		}

		inline float *Ring_Set_Tail_From_Ptr(signed int Length, float *Ptr)
		{
			float *retval;
			assert_param(abs(Length) <= size_);
			retval = point_index(Ptr, -1 * Length);

			Tail_ = retval;
			//	set_youngest_tail(pRing,retval);
			return retval;
		}

		inline float *Ring_Point_Offset_From_Tail(int offset)
		{
			float *retval = point_index(Tail_, offset);
			return retval;
		}

		inline float *Ring_Point_Offset_From_Head(int offset)
		{
			float *retval = point_index(Head_, offset);
			return retval;
		}

		inline float *Ring_Ptr_Move(signed int Positions, float *Ptr)
		{
			//	assert_param(abs(Positions)<pRing->size);
			//	assert_param(pRing->size>0);

			//	uint32_t * retval = point_index(pRing,Ptr,Positions);
			Ptr = point_index(Ptr, Positions);
			//	set_youngest_head(pRing,retval);
			return Ptr;
		}

		inline float *Ring_Move_Tail(signed int Positions)
		{
			float *retval;
			retval = point_index(Tail_, Positions);
			Tail_ = retval;
			return retval;
		}

		inline float *Ring_Head_Move(signed int Positions)
		{
			Head_ = point_index(Head_, Positions);
			return Head_;
		}

		inline void Ring_Move(signed int Positions)
		{
			assert_param(abs(Positions) < size_);

			Ring_Head_Move(Positions);
			Ring_Move_Tail(Positions);
		}

		inline float *Ring_Point_Offset_Ptr(int offset, float *Ptr)
		{
			return point_index(Ptr, offset);
		}

		inline void resetHead()
		{
			Head_ = pBuf_;
		}

		inline void resetTail()
		{
			Tail_ = pBuf_;
		}

		inline void push(float value)
		{
			*Head_ = value;
			Ring_Move(1);
		}

		inline size_t getSize()
		{
			return size_;
		}

	private:
		float *pBuf_;
		size_t size_;
		float *Head_;
		float *Tail_;
	};

} //namespace zen

//#ifdef __cplusplus
//extern "C" {
//#endif
//
//
//
//
//typedef struct Ring_Handle_TypeDef
//{
//	float *pBuf;
//	size_t  size;
//	float  *Head;
//	float  *Tail;
//	struct Ring_Handle_TypeDef * hParent;
//
//}Ring_Handle_TypeDef;
//
//typedef struct RingSub_Handle
//{
//	uint32_t *pBuf;
//	uint32_t  size;
//	uint32_t  *Head;
//	uint32_t  *Tail;
//	Ring_Handle_TypeDef *pRing;
//
//}RingSub_Handle_TypeDef;
//
//
//
//
//void ring_buffer_init(Ring_Handle_TypeDef * pRing, float * ptr, size_t size);
//void Ring_Swap(Ring_Handle_TypeDef * pRing);
//void Ring_Sub_Init(Ring_Handle_TypeDef * pRing, Ring_Handle_TypeDef * pParent, signed int TailLength);
//
//
//
////static inline void set_youngest_head(Ring_Handle_TypeDef * pRing, float * Ptr);
////static inline void set_youngest_tail(Ring_Handle_TypeDef * pRing, float * Ptr);
////static inline void set_youngest_pBuf(Ring_Handle_TypeDef * pRing, float * Ptr);
//
//
//
//
//#ifdef __cplusplus
//}
//#endif
//
//#endif /* BSP_BDSP_REVA_CIRCULAR_BUFFER_H_ */
