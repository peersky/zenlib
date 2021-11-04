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

 In this header you can write any custom lirary related debug functions.
 debugProvider.h must be defined in target files and must provide folliowing:
	inline void setDebugAnalyser(float *ch1, float *ch2, size_t size)
	inline void send_to_analyser(size_t channel, float *buffer)
	inline void append_to_analyser(size_t channel, float sample)
	inline void Error_Handler_(const char *file, int line)

 */

#pragma once

#include <debugProvider.hpp>

#define ZEN_ERROR_HANDLER() ERROR_HANDLER()
