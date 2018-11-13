#include "fftw_lib.h"
#include "../misc/iq.h"

FftwLib::FftwLib()
{
	input = output = nullptr;
	plan = nullptr;
}

FftwLib::~FftwLib()
{
	fftwf_destroy_plan(plan);
	delete [] input;
	delete [] output;
}

void FftwLib::Init(uint32_t N)
{
	if (input == nullptr)
	{
		input = new iqf_t[N];
		output = new iqf_t[N];
	}
	else
	{
		fftwf_destroy_plan(plan);
		plan = fftwf_plan_dft_1d(N, (fftwf_complex*)input, (fftwf_complex*)output, FFTW_FORWARD, FFTW_ESTIMATE);
	}	
}

iqf_t* FftwLib::Run(const int16_t * adc_samples, uint32_t n_samples)
{
	for (uint32_t i = 0; i < n_samples; i++)
	{
		input[ i ].re = (float) adc_samples[ i ] / (float) 0x0FFF;
		input[ i ].im = 0;
	}	
	fftwf_execute(plan);
	return output;
}
