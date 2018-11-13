#pragma once

#include <stdint.h>
#include "lib/fftw3.h"
#include "../misc/iq.h"

class FftwLib
{

private:
	uint32_t N;
	uint64_t Fs;

	fftwf_plan plan;
	iqf_t* input;
	iqf_t* output;

public:
	FftwLib();
	~FftwLib();

	void Init(uint32_t N);
	iqf_t* Run( const int16_t* adc_samples, uint32_t n_samples );
};
