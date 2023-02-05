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
#include	"rds-decoder.h"
#include	"radio.h"
#include	"sdr/shaping_filter.h"

#include <vector>
#include <fstream>

constexpr uint32_t TAPS_MF_RRC = 45; // should be odd
constexpr float RDS_BITCLK_HZ = 1187.5;

/*
 *	RDS is a bpsk-like signal, with a baudrate 1187.5
 *	on a carrier of  3 * 19 k.
 *	48 cycles per bit, 1187.5 bits per second.
 *	With a reduced sample rate of 19k this would mean
 *	19000 / 1187.5 samples per bit, i.e. 16
 *	samples per bit.
 *	Notice that complex mixing to zero IF has been done
 */
	rdsDecoder::rdsDecoder (RadioInterface	*myRadio,
	                        int32_t		rate):
	                            my_rdsBlockSync (myRadio),
	                            my_rdsGroupDecoder (myRadio),
	                            mySinCos  (rate), 
	                            sharpFilter (7, RDS_BITCLK_HZ - 6,
	                                            RDS_BITCLK_HZ + 6,
	                                            rate,
	                                            S_BUTTERWORTH),
	                            rdsFilter (21, RDS_WIDTH, rate),
	                            my_AGC (2e-3f, 0.38f, 9.0f),
	                            my_timeSync (ceil ((float)rate / (float)RDS_BITCLK_HZ) /*== 16.0*/, 0.01f),
	                            my_Costas (rate, 1.0f / 16.0f, 0.02f / 16.0f, 10.0f)
	{

float	synchronizerSamples;

	this	-> myRadioInterface	= myRadio;
	this	-> sampleRate		= rate;
        synchronizerSamples		= sampleRate / (float)RDS_BITCLK_HZ;
        symbolCeiling			= ceil (synchronizerSamples); 
        symbolFloor			= floor (synchronizerSamples);
//

//      The matched filter is a borrowed from the cuteRDS, who in turn
//      borrowed it from course material
//      http://courses.engr.illinois.edu/ece463/Projects/RBDS/RBDS_project.doc
//      Note that the formula down has a discontinuity for
//      two values of x, we better make the symbollength odd

	int length		= (symbolCeiling & ~01) + 1;
	rdsfilterSize		= 2 * length + 1;
	rdsBuffer. resize (rdsfilterSize);
	for (int i = 0; i < rdsfilterSize; i ++)
	   rdsBuffer [i] = 0;
	ip			= 0;
	rdsKernel. resize (rdsfilterSize);
	rdsKernel [length]	= 0;
	for (int i = 1; i <= length; i ++) {
	   float x = ((float)i) / rate * RDS_BITCLK_HZ;
	   rdsKernel [length + i] =  0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64 * x)) -
					    ((1.0 / (9.0 / x - 64 * x))) );
	   rdsKernel [length - i] = - 0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64 *  x)) -
					    ((1.0 / (9.0 / x - 64 *  x))) );
	}
	for (int i = 0; i < rdsfilterSize; i ++)
	   if (isinf (rdsKernel [i]))
	         rdsKernel [i] = 0;
//
//	Matched with this filter is followed by a pretty sharp filter
//	to eliminate noise
	rdsLastSyncSlope	= 0;
	rdsLastSync		= 0;
	rdsLastData		= 0;
	previousBit		= false;

//	Tomneda: a double inverted pulse (manchester code)
//	as matched filter is not working in my opinion
//	(got also no good results)
//	so I use a matched RRC filter design with
//	Ts = 1/(2 * 1187.5Hz), one side of the bi-phase puls
	my_matchedFltKernelVec =
	            ShapingFilter ().
	                root_raised_cosine (1.0, sampleRate,
	                                    2 * RDS_BITCLK_HZ,
	                                    1.0, TAPS_MF_RRC);

	my_matchedFltBufSize	= my_matchedFltKernelVec. size ();
	assert (my_matchedFltBufSize & 1); // is it odd?
	my_matchedFltBuf. resize (my_matchedFltBufSize);
	memset ((void*)my_matchedFltBuf. data (), 0,
	                my_matchedFltBufSize * sizeof (DSPCOMPLEX));
	my_matchedFltBufIdx	= 0;

	my_rdsGroup. clear ();
	my_rdsBlockSync. setFecEnabled (true);

	omegaRDS                = (2 * M_PI * RDS_BITCLK_HZ) / (float)rate;
//
//      for the decoder a la FMStack we need:
        syncBuffer. resize (symbolCeiling);
	memset ((void*)syncBuffer. data (), 0,
	                   symbolCeiling * sizeof (float));

        p                       = 0;
        bitIntegrator           = 0;
        bitClkPhase             = 0;
        prev_clkState           = 0;
	prevBit                 = false;
        Resync                  = true;
//
//	end 

	connect (this, SIGNAL (setCRCErrors (int)),
	         myRadioInterface, SLOT (setCRCErrors (int)));
	connect (this, SIGNAL (setSyncErrors (int)),
	         myRadioInterface, SLOT (setSyncErrors(int)));
}

	rdsDecoder::~rdsDecoder () {
}

void	rdsDecoder::reset () {
	my_rdsGroupDecoder. reset ();
}

DSPCOMPLEX	rdsDecoder::doMatchFiltering (DSPCOMPLEX v) {
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

float	rdsDecoder::Match	(float v) {
int16_t		i;
float	tmp = 0;

	rdsBuffer [ip] = v;
	for (i = 0; i < rdsfilterSize; i ++) {
	   int16_t index = (ip - i);
	   if (index < 0)
	      index += rdsfilterSize;
	   tmp += rdsBuffer [index] * rdsKernel [i];
	}

	ip = (ip + 1) % rdsfilterSize;
	return tmp;
}

bool	rdsDecoder::doDecode (DSPCOMPLEX v,
	                      DSPCOMPLEX *m,
	                      ERdsMode  mode, int ptyLocale) {
// this is called typ. 19000 1/s
DSPCOMPLEX r;
//
	if (mode == rdsDecoder::ERdsMode::RDS_1) {
	   *m = v;
	   bool b = doDecode1 (real (v), ptyLocale);
	   return b;
	}
	else
	if (mode == rdsDecoder::ERdsMode::RDS_2) {
	   bool b = doDecode_tmn (v, m,  ptyLocale);
	   return b;
	}
	else
	if (mode == rdsDecoder::ERdsMode::RDS_3) {
	   *m = v;
	   bool b = doDecode2 (real (v), ptyLocale);
	   return b;
	}
	return false;
}

void	rdsDecoder::processBit (bool bit, int ptyLocale) {
	switch (my_rdsBlockSync. pushBit (bit, &my_rdsGroup)) {
	   case rdsBlockSynchronizer::RDS_WAITING_FOR_BLOCK_A:
	      break;   // still waiting in block A

	   case rdsBlockSynchronizer::RDS_BUFFERING:
	      break;   // just buffer

	   case rdsBlockSynchronizer::RDS_NO_SYNC:
//	      resync if the last sync failed
	      setSyncErrors (my_rdsBlockSync. getNumSyncErrors ());
	      my_rdsBlockSync. resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_NO_CRC:
	      setCRCErrors (my_rdsBlockSync. getNumCRCErrors ());
	      my_rdsBlockSync. resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_COMPLETE_GROUP:
	      if (!my_rdsGroupDecoder. decode (&my_rdsGroup, ptyLocale)) {
	         ;   // error decoding the rds group
	      }
//	      my_rdsGroup. clear ();
	      break;
	}
}

//
//	Decode 1 is the "original" rds decoder, based on the
//	info from cuteSDR. 
bool	rdsDecoder::doDecode1	(float v, int ptyLocale) {
float	rdsSlope;
	v			= rdsFilter. Pass (v);
	v			= Match (v);
	float rdsMag		= sharpFilter. Pass (v * v);
	rdsSlope		= rdsMag - rdsLastSync;
	rdsLastSync		= rdsMag;
	if ((rdsSlope < 0.0) && (rdsLastSyncSlope >= 0.0)) {
//	top of the sine wave: get the data
	   bool bit	= rdsLastData >= 0;
	   processBit (bit ^ previousBit, ptyLocale);
	   previousBit = bit;
	}
	rdsLastData		= v;
	rdsLastSyncSlope	= rdsSlope;
	my_rdsBlockSync. resetResyncErrorCounter ();
	return true;
}
//
//	Decode2 is based on the FMstack software of Michael Feilen
//
bool	rdsDecoder::doDecode2	(float v, int ptyLocale) {
float clkState;
std::complex<float> tt;

	syncBuffer [p]	= rdsFilter. Pass (v);
	p		= (p + 1) % symbolCeiling;

	if (Resync || (my_rdsBlockSync. getNumSyncErrors () > 3)) {
	   synchronizeOnBitClk (syncBuffer, p);
	   my_rdsBlockSync. resync ();
	   my_rdsBlockSync. resetResyncErrorCounter ();
	   Resync = false;
	}

	clkState	= mySinCos. getSin (bitClkPhase);
	bitIntegrator	+= clkState * v;
//
//	rising edge -> look at integrator
	if (prev_clkState <= 0 && clkState > 0) {
	   bool currentBit	= bitIntegrator >= 0;
	   processBit (currentBit ^ previousBit, ptyLocale);
	   bitIntegrator	= 0;		// we start all over
	   previousBit		= currentBit;
	}

	prev_clkState	= clkState;
	bitClkPhase	= fmod (bitClkPhase + omegaRDS, 2 * M_PI);
	return true;
}

bool	rdsDecoder::doDecode_tmn (std::complex<float> v,
	                          std::complex<float> *m, int ptyLocale) {
std::complex<float> r;
	v = doMatchFiltering (v);
	v = my_AGC. process_sample (v);
#ifndef DO_STEREO_SEPARATION_TEST
	v = my_Costas. process_sample (v);
#endif
	if (my_timeSync. process_sample (v, r)) {
//	this runs 19000/16 = 1187.5 1/s times
	   bool bit	= (real (r) >= 0);
	   processBit	(bit ^ previousBit, ptyLocale);
	   previousBit	= bit;
	   *m		= r;
	   return true; // tell caller a changed m value
	}
	return false;
}

void	rdsDecoder::synchronizeOnBitClk (const std::vector<float> & v,
                                         int16_t first) {
bool isHigh	= false;
int32_t	k	= 0;
float phase;
float correlationVector [symbolCeiling];

	memset (correlationVector, 0, symbolCeiling * sizeof (float));

//	synchronizerSamples	= sampleRate / (float)RDS_BITCLK_HZ;
	for (int i = 0; i < symbolCeiling; i ++) {
	   phase = fmod (i * (omegaRDS / 2), 2 * M_PI);
//	reset index on phase change
	   if (mySinCos. getSin (phase) > 0 && !isHigh) {
	      isHigh = true;
	      k = 0;
	   }
	   else
	   if (mySinCos. getSin (phase) < 0 && isHigh) {
	      isHigh = false;
	      k = 0;
	   }

	   correlationVector [k ++] += v [(first + i) % symbolCeiling];
	}

//	detect rising edge in correlation window
	int32_t iMin	= 0;
	while (iMin < symbolFloor && correlationVector [iMin ++] > 0);
	while (iMin < symbolFloor && correlationVector [iMin ++] < 0);

//	set the phase, previous sample (iMin - 1) is obviously the one
	bitClkPhase = fmod (-omegaRDS * (iMin - 1), 2 * M_PI);
	while (bitClkPhase < 0)
	   bitClkPhase += 2 * M_PI;
}
