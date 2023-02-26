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
#include	"rds-decoder-1.h"
#include	"radio.h"
#include	<cmath>
#include	<vector>
#include	<fstream>

#define RDS_BITCLK_HZ  1187.5

/*
 *	RDS is a bpsk-like signal, with a baudrate 1187.5
 *	on a carrier of  3 * 19 k.
 *	48 cycles per bit, 1187.5 bits per second.
 *	With a reduced sample rate of 24k this would mean
 *	24000 / 1187.5 samples per bit, i.e. just over 20
 *	samples per bit.
 */

	rdsDecoder_1::rdsDecoder_1 (RadioInterface	*myRadio,
	                            int32_t		rate):
	                            sharpFilter (7, RDS_BITCLK_HZ - 6,
	                                            RDS_BITCLK_HZ + 6,
	                                            rate,
	                                            S_BUTTERWORTH),
	                            rdsFilter (21, RDS_WIDTH, rate) {

float	synchronizerSamples;

	this	-> myRadioInterface	= myRadio;
        synchronizerSamples		= rate / (float)RDS_BITCLK_HZ;
        int symbolCeiling		= ceil (synchronizerSamples); 
//

//      The matched filter is a borrowed from the cuteRDS, who in turn
//      borrowed it from course material
//      http://courses.engr.illinois.edu/ece463/Projects/RBDS/RBDS_project.doc
//      Note that the formula down has a discontinuity for
//      two values of x, we better make the symbollength odd

	int length		= (symbolCeiling & ~01) + 1;
	rdsBufferSize		= 2 * length + 1;
	rdsBuffer. resize (rdsBufferSize);
	for (int i = 0; i < rdsBufferSize; i ++)
	   rdsBuffer [i] = 0;
	ip			= 0;
	rdsKernel. resize (rdsBufferSize);
	rdsKernel [length]	= 0;
//
//	
//	While the original samplerate (24000) gave a perfect match,
//	the samplerate as changed by Tomneda (19000) gave 4 "inf"
//	values (that is probably why tomneda was not content with
//	the results of the match).
//	To catch the inf values, I at first dded an "isinf" test, 
//	this "isinf" check  was with the "fast math" option
//	in the "pro" file disregarded.
//	Anyway, a minor mod in the formula, changing 64 to 64.01, solved
//	the problem.
//
	for (int i = 1; i <= length; i ++) {
	   float x = ((float)i) / rate * RDS_BITCLK_HZ;
	   rdsKernel [length + i] =  0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64.01 * x)) -
					    ((1.0 / (9.0 / x - 64.01 * x))) );
	   rdsKernel [length - i] = - 0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64.01 * x)) -
					    ((1.0 / (9.0 / x - 64.01 * x))) );
	}
//
//	Matched with this filter is followed by a pretty sharp filter
//	to eliminate noise
	rdsLastSyncSlope	= 0;
	rdsLastSync		= 0;
	rdsLastData		= 0;
	previousBit		= 0;
}

	rdsDecoder_1::~rdsDecoder_1 () {
}

float	rdsDecoder_1::Match	(float v) {
int16_t		i;
float	tmp = 0;

	rdsBuffer [ip] = v;
	for (i = 0; i < rdsBufferSize; i ++) {
	   int16_t index = (ip - i);
	   if (index < 0)
	      index += rdsBufferSize;
	   tmp += rdsBuffer [index] * rdsKernel [i];
	}

	ip = (ip + 1) % rdsBufferSize;
	return tmp;
}

//	Decode 1 is the "original" rds decoder, based on the
//	info from cuteSDR. 
bool	rdsDecoder_1::doDecode	(float v, uint8_t *d) {
float	rdsSlope;
bool	res	= false;
	v			= rdsFilter. Pass (v);
	v			= Match (v);
	float rdsMag		= sharpFilter. Pass (v * v);
	rdsSlope		= rdsMag - rdsLastSync;
	rdsLastSync		= rdsMag;
	if ((rdsSlope < 0.0) && (rdsLastSyncSlope >= 0.0)) {
//	top of the sine wave: get the data
	   uint8_t theBit	= rdsLastData >= 0 ? 1 : 0;
	   *d	= theBit ^ previousBit;
	   previousBit = theBit;
	   res	= true;
	}
	rdsLastData		= v;
	rdsLastSyncSlope	= rdsSlope;
	return res;
}

