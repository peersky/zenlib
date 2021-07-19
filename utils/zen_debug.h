/*
 * debug.h
 *
 *  Created on: 12 мар. 2018 г.
 *      Author: Teem
 */


#pragma once


#define ZEN_DEBUG 1

#include "stdint.h"
#include "stdio.h"
#include <string>

inline void Error_Handler_(const char *file, int line)
{
	printf("Error: file %s on line %d\r\n", file, line);
}

#define ZEN_PRINT(fmt, ...) do {\
	if (ZEN_DEBUG) { printf(fmt, __VA_ARGS__);\
	fflush(stdout);\
	}\
            } while (0)

#define zen_Error_Handler() \
	do { if (ZEN_DEBUG) Error_Handler_(__FILE__, __LINE__); } while(0)





void assert_failed(uint8_t* file, uint32_t line);


#define _ASSERT    0U
#ifdef  _ASSERT

  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

char Debug_getchar(void);
uint32_t Debug_gethex(void);


