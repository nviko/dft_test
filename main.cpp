#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "fftw/fftw_lib.h"
#include "radix2/radix2.h"

const uint64_t Fs = 12000000;

const float Amplitude = 1900;
const float DC = 2040.0f;

static int16_t* test_sample = nullptr;
static uint32_t sample_length = 0;

static int16_t* gen_signal(uint64_t fs_Hz, uint64_t fc_Hz, uint32_t time)
{
	if (time > sample_length)
	{
		sample_length = time;
		delete[] test_sample;
		test_sample = new int16_t[sample_length];
	}

	double lambda = 2.0f * M_PI * (double) fc_Hz / (double) fs_Hz;

	for (uint32_t i = 0; i < sample_length; i++)
	{
		test_sample[i] = (int16_t)((Amplitude * sin((double)i * lambda)) + DC);
	}

	return test_sample;
}


template<typename A, typename B> void check_result(B* input, uint32_t lenFFT, uint32_t fc, const char* who)
{
	uint16_t maxPosition = 0xFFFF;
	A maxModule = 0;

	for (uint16_t j = 1; j < (lenFFT >> 1); j++)
	{
		A m = (A)sqrt((double)input[j].re * input[j].re + (double)input[j].im * input[j].im);
		if (m > maxModule)
		{
			maxModule = m;
			maxPosition = j;
		}
	}

	int32_t delta = (int32_t)fc - maxPosition;
	if (delta < 0) delta = -delta;
	// expecting max amp on +/- 1 carrier freq position
	if (delta > 2)
	{
		fprintf(stderr, "%s Error: expect %u, got %u\n", who, fc, maxPosition);
	}
}

int main( void )
{
	FftwLib* fftwlib = new FftwLib();
	Radix2* radix2 = new Radix2();

	static const uint32_t N[] = { 256, 512, 1024, 2048 }; // radix-2 is only supports max 2048 points FFT
	const uint32_t nTests = sizeof(N) / sizeof(N[0]);

	fftwlib->Init(N[nTests - 1]);

	for (uint32_t test = 0; test < nTests; test++)
	{
		uint32_t lenFFT = N[test];
		uint64_t resolution = Fs / lenFFT;
		uint32_t fc = 1; // carrier frequency index (0 freq is skipped)

		fftwlib->Init(lenFFT);
		
		for (uint64_t freq = resolution; freq < Fs / 4; freq += resolution)
		{
		        // gen sin wave with "freq" carrier frequency
			int16_t* adc_sim_signal = gen_signal(Fs, freq, lenFFT);

			// do both ffts
			iqs_t* r2_res = radix2->Run(adc_sim_signal, lenFFT);
			iqf_t* fftwf_res = fftwlib->Run(adc_sim_signal, lenFFT);

			// check frequency response. Position with max amplitude should be close to "fc" value
			check_result<int16_t, iqs_t>(r2_res, lenFFT, fc, "radix-2");
			check_result<double, iqf_t>(fftwf_res, lenFFT, fc, "fftwf");
			fc++;
		}
	}

	delete fftwlib;
	delete radix2;
	delete[] test_sample;

	fprintf(stderr, "Done\n");
	int32_t c = getc(stdin);
	return 0;
}

