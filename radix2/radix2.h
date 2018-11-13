#pragma once

#include <stdint.h>
#include "../misc/iq.h"

class Radix2
{
private:
	iqs_t* buffer;
	
	uint32_t log2(uint32_t n);

	int32_t saturation(int32_t value, uint32_t wide);
	int32_t qadd16(int32_t x, int32_t y);
	int32_t qsub16(int32_t x, int32_t y);
	int32_t smusd(int32_t x, int32_t y);
	int32_t smuadx(int32_t x, int32_t y);

public:
	Radix2();
	~Radix2();

	iqs_t* Run(const int16_t * adc_samples, uint32_t n_samples);
};