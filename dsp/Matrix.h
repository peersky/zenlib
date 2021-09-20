	//
	//  matrix.h
	//  Zen Delay Plugin
	//
	//  Created by Tim on 30/08/2021.
	//  Copyright © 2021 Tim Pechersky aka peersky. All rights reserved.
	//


#pragma once
#include "../utils/debug.h"
namespace zen
{


template <typename T, size_t num_x, size_t num_y>
class Matrix {
public:
	Matrix ()
	{
		for(int x = 0; x<num_x; x++)
		{
			for(int y = 0; y<num_y; y++)
			{
				matrix_[x][y]=0;
			}
		}
	}
	
	/**
	 * Sets an element  of a matrix to a value
	 
	 * @param value - value to set
	 * @param x - horizontal index
	 * @param y - vertical index
	 * @return none.
	 */
	inline void setElement(T value, size_t x, size_t y)
	{
		matrix_[x][y]=value;
	}
	
	/**
	 * gets an element  of a matrix
	 
	 * @param x - horizontal index
	 * @param y - vertical index
	 * @return value of the element.
	 */
	inline T getElement(size_t x, size_t y)
	{
		return matrix_[x][y];
	}
	
	/**
	 * multiplies an array with given row
	 
	 * @param rowIO - array of input-output values. Memory bounds are not checked, don't shoot your own leg - !Must be length of num_x!
	 * @param y - row index.  Memory bounds are not checked, don't shoot your own leg - !Must be length of num_y!
	 * @return none.
	 */
	inline void multiplyRow(T* rowIO, size_t y)
	{
		assert_param(y<num_y);
		for (int x=0; x<num_x; x++)
		{
			rowIO[x] = matrix_[x][y]*rowIO[x];
		}
	}
	
	/**
	 * multiplies an array with given column
	 
	 * @param rowIO - array of input-output values. Memory bounds are not checked, don't shoot your own leg - !Must be length of num_y!
	 * @param y - row index.  Memory bounds are not checked, don't shoot your own leg - !Must be length of num_y!
	 * @return none.
	 */
	inline void multiplyColumn(T* columnIO, size_t x)
	{
		for (int y=0; y<num_x; y++)
		{
			columnIO[y] = matrix_[x][y]*columnIO[y];
		}
	}
	
	/**
	 * gets matrix pointer of 0 index.
	 */
	inline T* getMatrix ()
	{
		return matrix_;
	}
	
	/**
	 *  copies matrix from given pointer
	 */
	inline void setMatrix (T (&newMatrix)[num_x][num_y])
	{
		for(int x=0; x<num_x; x++)
		{
			for(int y=0; y<num_y; y++)
			{
				matrix_[x][y] = newMatrix[x][y];
			}
		}
	}
	
	/**
	 *  multiplies IO matrix by stored matrix
	 */
	inline void multiplateMatricies (T** IO)
	{
		for(int x=0; x<num_x; x++)
		{
			for(int y=0; y<num_y; y++)
			{
				IO[x][y] = IO[x][y] * matrix_[x][y];
			}
		}
	}
	
	/**
	 *  multiplies IO 1D vector by stored matrix
	 *  IO vector is column with size num_y
	 */
	inline void getProduct1D (T (&IO)[num_y])
	{
		T inputs[num_y];
		memcpy(inputs, IO, num_y*sizeof(T));
		// for each row
		for(int y=0; y<num_y; y++)
		{
			IO[y]=0;
			//iterate trough columns
			for(int x=0; x<num_x; x++)
			{
				IO[y] += inputs[y] * matrix_[x][y];
			}
		}
	}
	
private:
	
	T matrix_[num_x][num_y];
};
} // namespace zen
