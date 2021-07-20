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


#define ZEN_DEBUG 1

#include "stdint.h"
#include "stdio.h"
#include <string>

#ifdef ZEN_PLATFORM_BDSP
#else
#include <chrono>  // for high_resolution_clock
#endif

inline void Error_Handler_(const char *file, int line)
{
	printf("Error: file %s on line %d\r\n", file, line);
}

#define ZEN_PRINT(fmt, ...) do {\
if (ZEN_DEBUG) { printf(fmt, __VA_ARGS__);\
	fflush(stdout);\
}\
} while (0)

#define ZEN_ERROR_HANDLER() \
do { if (ZEN_DEBUG) Error_Handler_(__FILE__, __LINE__); } while(0)


inline void assert_failed(uint8_t* file, uint32_t line)
{
#ifdef DEBUG
	printf("Wrong parameters value: file %s on line %d\r\n", file, line);
#else
	while(0);
#endif
	
}

inline char Debug_getchar(void)
{
	char input;
	scanf(" %c", &input);
	
	return input;
}

inline uint32_t Debug_gethex(void)
{
	int input;
	scanf(" %x", &input);
	
	return input;
}


#define _ASSERT    0U
#ifdef  _ASSERT

#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */



//ToDo: This class has not been tested yet
namespace zen {
class LoadMonitor {
public:
	LoadMonitor()
	{
		
	}
	~LoadMonitor()
	{
		
	}
#ifdef ZEN_PLATFORM_BDSP
	inline void startAverageMonitor()
	{
		start_ = DWT->CYCCNT;
		lap_ = 0;
		laps_=0;
		total_ = 0;
	}
	inline void Lap()
	{
		lap_ = DWT->CYCCNT;
		total_ += lap_;
		laps_++;
		average_ = total_ / laps_;
	}
#else
	inline void startAverageMonitor()
	{
		start_ = std::chrono::high_resolution_clock::now();
		lap_ = 0;
		laps_=0;
		total_ = 0;
	}
	inline void lapStart()
	{
		start_ = std::chrono::high_resolution_clock::now();
		
	}
	inline void lapEnd()
	{
		lap_ = std::chrono::duration<float, std::milli>( std::chrono::high_resolution_clock::now() - start_).count();
		total_ += lap_;
		laps_++;
		average_ = total_ / laps_;
	}
#endif
	
private:
	
	
#ifdef ZEN_PLATFORM_BDSP
	int start_;
	int finish_;
	int laps_;
	int average_;
	int lap_;
	int total_;
#else
	std::chrono::steady_clock::time_point start_;
	std::chrono::steady_clock::time_point finish_;
	float average_;
	float laps_;
	float lap_;
	float total_;
#endif
	
	
};
} //namespace zen


