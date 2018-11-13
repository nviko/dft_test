#pragma once

#include <stdint.h>

typedef struct
{
	float re;
	float im;
} iqf_t;

typedef struct
{
	int16_t re;
	int16_t im;
} iqs_t;

typedef union
{
	iqs_t   iq;
	int32_t value;
} iqr2_t;
