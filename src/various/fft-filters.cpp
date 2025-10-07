/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdr-j-fm
 *
 *    sdr-j-fm is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdr-j-fm is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fft-filters.h"
#include "fir-filters.h"
#include	"fft-complex.h"

#include <cstring>

	fftFilter::fftFilter (int32_t size, int degree) {

	fftSize		= size;
	filterDegree	= degree;
	OverlapSize	= filterDegree;
	NumofSamples	= fftSize - OverlapSize;

	FFT_A		= new std::complex<float> [fftSize];
	FFT_C		= new std::complex<float> [fftSize];
	filterVector	= new std::complex<float> [fftSize];
	RfilterVector	= new DSPFLOAT [fftSize];
	Overloop	= new DSPCOMPLEX [OverlapSize];
	inp		= 0;
	for (int i = 0; i < fftSize; i ++) {
	   FFT_A [i] = 0;
	   FFT_C [i] = 0;
	   filterVector [i] = 0;
	   RfilterVector [i] = 0;
	}
}

	fftFilter::~fftFilter () {
	delete[]	FFT_A;
	delete[]	FFT_C;
	delete[]	filterVector;
	delete[]	RfilterVector;
	delete[]	Overloop;
}

void	fftFilter::setSimple (int32_t low, int32_t high, int32_t rate) {
BasicBandPass BandPass (filterDegree, low, high, rate);

	for (int i = 0; i < filterDegree; i++)
	   filterVector [i] = (BandPass. getKernel ()) [i];

	for (int i = filterDegree; i < fftSize; i ++)
	   filterVector [i] = DSPCOMPLEX (0, 0);

	Fft_transform (filterVector, fftSize, false);
        inp             = 0;
}

void	fftFilter::setBand (int32_t low, int32_t high, int32_t rate) {
BandPassFIR BandPass ((int)filterDegree, low, high, rate);

	for (int i = 0; i < filterDegree; i ++)
           filterVector [i] = (BandPass. getKernel ()) [i];
	
	for (int i = filterDegree; i < fftSize; i ++)
	   filterVector [i] = DSPCOMPLEX (0, 0);

	Fft_transform (filterVector, fftSize, false);
        inp             = 0;
}

void	fftFilter::setLowPass (int32_t low, int32_t rate) {
LowPassFIR LowPass (filterDegree, low, rate);

	for (int i = 0; i < filterDegree; i ++)
	   filterVector [i] = (LowPass. getKernel ()) [i];

	for (int i = filterDegree; i < fftSize; i ++)
	   filterVector [i] = DSPCOMPLEX (0, 0);

	Fft_transform (filterVector, fftSize, false);
	inp	= 0;
}

DSPFLOAT	fftFilter::Pass (DSPFLOAT x) {
DSPFLOAT	sample;

	sample	= real (FFT_C [inp]);
	FFT_A [inp] = x;

	if (++inp >= NumofSamples) {
	   inp = 0;
	   for (int i = NumofSamples; i < fftSize; i ++)
	      FFT_A [i] = DSPCOMPLEX (0, 0);

	   Fft_transform (FFT_A, fftSize, false);

	   for (int j = 0; j < fftSize; j ++) {
	      FFT_C [j] = FFT_A [j] * filterVector [j];
              FFT_C [j] = DSPCOMPLEX (real (FFT_C [j]) * 3,
                                      imag (FFT_C [j]) * 3);
	   }
//
//	cheating to get the inverse
	   for (int j = 0; j < fftSize; j ++)
	      FFT_C [j] = conj (FFT_C [j]);
	   Fft_transform (FFT_C, fftSize, false);
//	   MyIFFT	-> do_IFFT ();
	   for (int j = 0; j < fftSize; j ++)
	      FFT_C [j] = conj (FFT_C [j]) * (float) (1.0f / fftSize);
	   for (int j = 0; j < OverlapSize; j ++) {
	      FFT_C [j] += Overloop [j];
	      Overloop [j] = FFT_C [NumofSamples + j];
	   }
	}

	return sample;
}

DSPCOMPLEX	fftFilter::Pass (DSPCOMPLEX z) {
DSPCOMPLEX	sample;
int		j;
	sample	= FFT_C [inp];
	FFT_A [inp] = DSPCOMPLEX (real (z), imag (z));

	if (++inp >= NumofSamples) {
	   inp = 0;
	   for (int i = NumofSamples; i < fftSize; i ++)
	      FFT_A [i] = DSPCOMPLEX (0, 0);

	   Fft_transform (FFT_A, fftSize, false);

	   for (j = 0; j < fftSize; j ++) 
	      FFT_C [j] = FFT_A [j] * filterVector [j];
//
//	cheating to get the inverse
//	IFFT (X) = conj (FFT (conj (X)) / N
	  for (int j = 0; j < fftSize; j ++)
	      FFT_C [j] = conj (FFT_C [j]);
	   Fft_transform (FFT_C, fftSize, false);
	   float factor = 1.0 / fftSize;
	   for (int j = 0; j < fftSize; j ++)
	      FFT_C [j] =  conj (FFT_C [j]) * factor;
	   for (j = 0; j < OverlapSize; j ++) {
	      FFT_C [j] += Overloop [j];
	      Overloop [j] = FFT_C [NumofSamples + j];
	   }
	}

	return sample;
}


	fftFilterHilbert::fftFilterHilbert (int32_t size, int degree):
	                                              fftFilter (size, degree) {
	setHilbert ();
}

	fftFilterHilbert::~fftFilterHilbert	() {}

DSPCOMPLEX	fftFilterHilbert::Pass (DSPFLOAT x) {
	return fftFilter::Pass (DSPCOMPLEX (x, 0));
}

void	fftFilterHilbert::setHilbert () {
//	set the frequency coefficients directly (set negative spectrum to zero)

	if ((fftSize & 0x1) == 0) {		// even fftSize
	   filterVector [0] = 1.0f;

	   for (int i = 1; i < fftSize / 2; i++) 
	      filterVector [i] = 2.0f;

	   filterVector [fftSize / 2] = 1.0f;

	   for (int i = fftSize / 2 + 1; i < fftSize; i++)
	      filterVector [i] = 0.0f;
	}
	else {					// odd fftsize
	   filterVector [0] = 1.0f;
	   for (int i = 1; i < (fftSize + 1) / 2; i++)
	      filterVector [i] = 2.0f;

	   for (int i = (fftSize - 1) / 2 + 1; i < fftSize; i++)
	      filterVector[i] = 0.0f;
	}

	inp = 0;
}

