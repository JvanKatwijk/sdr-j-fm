#
/*
 *    Copyright (C)  2014
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

#include	<stdlib.h>
#include	<stdio.h>
#include	<cassert>
#include	"radio.h"
#include	"rds-decoder-2.h"
#include	"shaping_filter.h"
#include	<cmath>
#include	<vector>
#include	<fstream>

constexpr uint32_t TAPS_MF_RRC = 45; // should be odd
constexpr float RDS_BITCLK_HZ = 1187.5;

/*
 *	RDS is a bpsk-like signal, with a baudrate 1187.5
 *	on a carrier of  3 * 19 k.
 *	48 cycles per bit, 1187.5 bits per second.
 *	with a samplerate of 24000 this leads to just over 20
 *	samples per bit.
 */
	rdsDecoder_2::rdsDecoder_2 (RadioInterface	*myRadio,
	                            int32_t		rate):
	                               my_AGC (2e-3f, 0.38f, 9.0f),
	                               my_Costas (rate, 1.0f, 0.02f, 10.0f) {

	this	-> myRadioInterface	= myRadio;
	this	-> sampleRate		= rate;

	this	-> samplesPerSymbol 	= rate / (float)RDS_BITCLK_HZ;
	for (int i = 0; i < 3; i ++) {
	   sampleBuffer [i] = std::complex<float> (0, 0);
	   sampleBufferRail [i] = std::complex<float> (0, 0);
	}
	this	-> sampleCount		= 0;
	this	-> mMu			= 0;
	this	-> alpha		= 0.01;
	this	-> skipNrSamples	= 3;
	   
//	Tomneda: a double inverted pulse (manchester code)
//	I used a matched RRC filter design with Ts = 1 / (2 * 1187.5 Hz)
//	one side of the bi phase filter
	my_matchedFltKernelVec =
	            ShapingFilter ().
	                root_raised_cosine (1.0, sampleRate,
	                                    2 * RDS_BITCLK_HZ,
	                                    1.0, TAPS_MF_RRC);

	my_matchedFltBufSize	= my_matchedFltKernelVec. size ();
//	assert (my_matchedFltBufSize & 1); // is it odd?
	my_matchedFltBuf. resize (my_matchedFltBufSize);
	memset ((void*)my_matchedFltBuf. data (), 0,
	                my_matchedFltBufSize * sizeof (DSPCOMPLEX));
	my_matchedFltBufIdx	= 0;
}

	rdsDecoder_2::~rdsDecoder_2 () {
//	symsync_crf_destroy (q);
}

DSPCOMPLEX	rdsDecoder_2::doMatchFiltering (DSPCOMPLEX v) {
DSPCOMPLEX tmp = 0;

	my_matchedFltBuf [my_matchedFltBufIdx] = v;

	for (int16_t i = 0; i < my_matchedFltBufSize; i++) {
	   int16_t index = (my_matchedFltBufIdx - i);
	   if (index < 0) {
	      index += my_matchedFltBufSize; // wrap around index
	   }
	   tmp += my_matchedFltBuf [index] * my_matchedFltKernelVec [i];
	}

	my_matchedFltBufIdx = (my_matchedFltBufIdx + 1) % my_matchedFltBufSize;
	return tmp;
}

//
bool	rdsDecoder_2::doDecode (std::complex<float> v,
	                        std::complex<float> *m, uint8_t *d) {
std::complex<float> r;

	v = doMatchFiltering (v);
	v	= my_AGC. process_sample (v);
	if (process_sample (v, r)) {
#ifdef	DO_STEREO_SEPARATION_TEST
	   r = my_Costas. process_sample (r);
#endif
	   bool theBit	= (real (r) >= 0);
	   *d	= theBit ^ previousBit;
	   previousBit	= theBit;
	   *m		= r;
	   return true; // tell caller a changed m value
	}
	return false;
}

bool	rdsDecoder_2::process_sample (const DSPCOMPLEX iZ, DSPCOMPLEX &oZ) {

	sampleBuffer [0] = sampleBuffer [1];
	sampleBuffer [1] = sampleBuffer [2];
	sampleBuffer [2] = iZ;

	if (++sampleCount >= skipNrSamples) {
//	get hard decision values (rail to rail)
	   for (int32_t i = 0; i < 3; ++i) {
	      sampleBufferRail [i] =
	          DSPCOMPLEX ((real (sampleBuffer [i]) > 0.0f ? 1.0f : -1.0f),
	                      (imag (sampleBuffer [i]) > 0.0f ? 1.0f : -1.0f));
	   }

	   const DSPFLOAT x =
	                real (sampleBufferRail [2] - sampleBufferRail [0]) *
	                            real (sampleBuffer [1])  +
	                imag (sampleBufferRail [2] - sampleBufferRail[0]) *
	                           imag (sampleBuffer [1]);
	   const DSPFLOAT y =
	                real (sampleBuffer [2] - sampleBuffer [0]) *
	                           real (sampleBufferRail [1]) +
	                imag (sampleBuffer [2] - sampleBuffer [0]) *
	                            imag (sampleBufferRail [1]);

	   const DSPFLOAT mm_val = y - x;

	   mMu	+= samplesPerSymbol + alpha * mm_val;
//	round down to nearest int since we are using it as an index:
	   skipNrSamples = (int32_t)(mMu);
	   mMu	-= skipNrSamples; // remove the integer part of mu

	   oZ = sampleBuffer [2];
	   sampleCount	 = 0;
	   return true;
	}
	return false;
}

