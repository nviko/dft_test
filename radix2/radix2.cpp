#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "tables.h"
#include "radix2.h"

uint32_t Radix2::log2(uint32_t n)
{
	uint32_t log = 0;
	if ( n != 0 )
	{
		uint32_t v = n;
		if (v & 0xFFFF0000) v &= 0xFFFF0000, log = 16;
		if (v & 0xFF00FF00) v &= 0xFF00FF00, log += 8;
		if (v & 0xF0F0F0F0) v &= 0xF0F0F0F0, log += 4;
		if (v & 0xCCCCCCCC) v &= 0xCCCCCCCC, log += 2;
		if (v & 0xAAAAAAAA) log++;
	}
	return log;
}

int32_t Radix2::saturation(int32_t value, uint32_t wide)
{
	int32_t maxValue = 1 << (wide - 1);

	if (value > 0)
	{
		maxValue -= 1;
		return (value > maxValue) ? maxValue : value;
	}
	return (value < -maxValue) ? -maxValue : value;
}

int32_t Radix2::qadd16(int32_t x, int32_t y)
{
	iqr2_t a;
	iqr2_t b;

	a.value = x;
	b.value = y;

	a.iq.re = saturation(a.iq.re + b.iq.re, 16);
	a.iq.im = saturation(a.iq.im + b.iq.im, 16);

	return a.value;
}

int32_t Radix2::qsub16(int32_t x, int32_t y)
{
	iqr2_t a;
	iqr2_t b;

	a.value = x;
	b.value = y;

	a.iq.re = saturation(a.iq.re - b.iq.re, 16);
	a.iq.im = saturation(a.iq.im - b.iq.im, 16);

	return a.value;
}

int32_t Radix2::smusd(int32_t x, int32_t y)
{
	iqr2_t a;
	iqr2_t b;

	a.value = x;
	b.value = y;
	return (int32_t)((a.iq.re * b.iq.re) - (a.iq.im * b.iq.im));
}

int32_t Radix2::smuadx(int32_t x, int32_t y)
{
	iqr2_t a;
	iqr2_t b;

	a.value = x;
	b.value = y;
	return (int32_t) ((a.iq.re * b.iq.im) + (a.iq.im * b.iq.re));
}

Radix2::Radix2()
{
	buffer = new iqs_t[2048];
}

Radix2::~Radix2()
{
	delete[] buffer;
}

iqs_t* Radix2::Run(const int16_t * adc_samples, uint32_t N)
{
	uint32_t p = log2(N);
	int32_t N2 = N >> 1;

	if (p > 11)
	{
		p = 11;
	}
	else if (p < 6)
	{
		p = 6;
	}

	iqr2_t* out = (iqr2_t*) buffer;

	/* make IQ */
	for (uint32_t i = 0; i < N; i++)
	{
		buffer[i].re = adc_samples[i];
		buffer[i].im = 0;
	}

	/* shuffling */
	const uint32_t* range = &R2_ShuffleTableOffset[p - 6];
	for (uint32_t i = range[0]; i < range[1]; i++)
	{
		int32_t x = R2_ShuffleTable[i] & 0xFFFF;
		int32_t y = R2_ShuffleTable[i] >> 16;

		int32_t v = out[x].value;
		out[x].value = out[y].value;
		out[y].value = v;
	}

	iqr2_t A;
	iqr2_t B;
	iqr2_t C;
	iqr2_t D;
	int32_t shift = 11 - p;


	/* butterflies */
	for ( uint32_t i = 0; i < p; i++ )
	{
		int32_t m = 1 << i;			// elements in butterfly
		int32_t groupSize = m << 1;	// group size
		int32_t groupNum = 0;		// group number
		int32_t k = 0;
		int32_t y = 0;
		int32_t z = 0;
		int32_t step = 1 << (p - i - 1); // step
		int32_t j;

		int32_t twiddleCoef;

		for (j = 0; j < N2; j++)
		{
			if (k >= m)
			{
				// next group
				k = y = 0;
				groupNum += groupSize;
				z = groupNum;
			}
			twiddleCoef = (int32_t) R2_Twiddles[y];  // {re0; im0} twiddle/phase
			D.value = out[z + m].value;				 // D <= { re1; im1 } sample
			D.iq.re >>= 1;
			D.iq.im >>= 1;

			A.value = smusd(D.value, twiddleCoef);
			B.value = smuadx(D.value, twiddleCoef);

			D.value = out[z].value;	// D <= {re2; im2} sample
			D.iq.re >>= 1;
			D.iq.im >>= 1;

			C.iq.re = A.value >> 12;	// 12 bit
			C.iq.im = B.value >> 12;	// 12 bit
			A.value = qadd16(D.value, C.value);  // re2 + re, im2 + im
			B.value = qsub16(D.value, C.value);  // re2 - re, im2 - im

			out[z].value = A.value;	     // out[ z ] <= A
			out[z + m].value = B.value;  // out[ z + m ] <= B

			k += 1;
			z += 1;
			y += ( step << shift );
		}
	}
	return buffer;
}
