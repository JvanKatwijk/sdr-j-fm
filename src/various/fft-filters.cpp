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
#include <cstring>

	fftFilter::fftFilter (int32_t size, int degree) {

	fftSize		= size;
	filterDegree	= degree;
	OverlapSize	= filterDegree;
	NumofSamples	= fftSize - OverlapSize;

	MyFFT		= new common_fft	(fftSize);
	FFT_A		= MyFFT		->	getVector ();
	MyIFFT		= new common_ifft	(fftSize);
	FFT_C		= MyIFFT	->	getVector ();

	FilterFFT	= new common_fft	(fftSize);
	filterVector	= FilterFFT	->	getVector ();
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
	delete		MyFFT;
	delete		MyIFFT;
	delete		FilterFFT;
	delete[]	RfilterVector;
	delete[]	Overloop;
}

void	fftFilter::setSimple (int32_t low, int32_t high, int32_t rate) {
BasicBandPass *BandPass		= new BasicBandPass (filterDegree,
	                                             low, high, rate);

	for (int i = 0; i < filterDegree; i++)
	   filterVector [i] = (BandPass -> getKernel ()) [i];

        memset (&filterVector [filterDegree], 0,
                        (fftSize - filterDegree) * sizeof (DSPCOMPLEX));
        FilterFFT       -> do_FFT ();
        inp             = 0;
        delete  BandPass;
}

void	fftFilter::setBand (int32_t low, int32_t high, int32_t rate) {
BandPassFIR	*BandPass	= new BandPassFIR ((int)filterDegree,
	                                           low, high,
	                                           rate);

      for (int i = 0; i < filterDegree; i ++)
           filterVector [i] = (BandPass -> getKernel ()) [i];
//         filterVector [i] = conj ((BandPass -> getKernel ()) [i]);
        memset (&filterVector [filterDegree], 0,
                        (fftSize - filterDegree) * sizeof (DSPCOMPLEX));
        FilterFFT       -> do_FFT ();
        inp             = 0;
        delete  BandPass;
}

void	fftFilter::setLowPass (int32_t low, int32_t rate) {
int32_t	i;
LowPassFIR	*LowPass	= new LowPassFIR (filterDegree,
	                                          low,
	                                          rate);

	for (i = 0; i < filterDegree; i ++)
	   filterVector [i] = (LowPass -> getKernel ()) [i];
	memset (&filterVector [filterDegree], 0,
	                (fftSize - filterDegree) * sizeof (DSPCOMPLEX));
	FilterFFT	-> do_FFT ();
	inp	= 0;
	delete LowPass;
}

DSPFLOAT	fftFilter::Pass (DSPFLOAT x) {
DSPFLOAT	sample;

	sample	= real (FFT_C [inp]);
	FFT_A [inp] = x;

	if (++inp >= NumofSamples) {
	   inp = 0;
	   memset (&FFT_A [NumofSamples], 0,
	               (fftSize - NumofSamples) * sizeof (DSPCOMPLEX));
	   MyFFT	-> do_FFT ();

	   for (int j = 0; j < fftSize; j ++) {
	      FFT_C [j] = FFT_A [j] * filterVector [j];
              FFT_C [j] = DSPCOMPLEX (real (FFT_C [j]) * 3,
                                      imag (FFT_C [j]) * 3);
	   }

	   MyIFFT	-> do_IFFT ();
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
	   memset (&FFT_A [NumofSamples], 0,
	               (fftSize - NumofSamples) * sizeof (DSPCOMPLEX));
	   MyFFT	-> do_FFT ();

	   for (j = 0; j < fftSize; j ++) 
	      FFT_C [j] = FFT_A [j] * filterVector [j];

	   MyIFFT	-> do_IFFT ();
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

DSPCOMPLEX	fftFilterHilbert::Pass (DSPFLOAT x) {
	return fftFilter::Pass (DSPCOMPLEX (x, 0));
}

void	fftFilterHilbert::setHilbert () {
// set the frequency coefficients directly (set negative spectrum to zero)

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

