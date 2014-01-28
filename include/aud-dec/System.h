#pragma once

#include<climits>

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif

#ifdef _DEBUG
#define ASSERT(p) 	if (0 == p) \
                    { \
                        printf("ASSERT failed: file[%d] line[%d]\n", __LINE__, __FILE__); \
                        exit(1); \
                };
#else
#define ASSERT(p);
#endif

#ifndef INT8_MIN
#define INT8_MIN (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (4294967295U)
#endif
