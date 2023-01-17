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
//#include	"iir-filters.h"
#include "sdr/shaping_filter.h"

#include <vector>
#include <fstream>

constexpr uint32_t TAPS_MF_RRC = 45; // should be odd
constexpr DSPFLOAT RDS_BITCLK_HZ = 1187.5;

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
	                            my_AGC (2e-3f, 0.38f, 9.0f),
	                            my_timeSync (ceil ((float)rate / (float)RDS_BITCLK_HZ) /*== 16.0*/, 0.01f),
	                            //my_Costas1 (RDS_BITCLK_HZ, 1.0f, 0.02f, 20.0f) ,
	                            my_Costas (rate, 1.0f / 16.0f, 0.02f / 16.0f, 10.0f)
	{

DSPFLOAT	synchronizerSamples;

	this	-> myRadioInterface	= myRadio;
	this	-> sampleRate		= rate;

//	Tomneda: a double inverted pulse (manchester code)
//	as matched filter is not working in my opinion
//	(got also no good results)
//	so I use a matched RRC filter design with
//	Ts = 1/(2*1187.5Hz), one side of the bi-phase puls
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
	previousBit		= false;

	my_rdsGroup		= new RDSGroup ();
	my_rdsGroup		-> clear ();
	my_rdsBlockSync		= new rdsBlockSynchronizer (myRadioInterface);
	my_rdsBlockSync		-> setFecEnabled (true);
	my_rdsGroupDecoder	= new rdsGroupDecoder (myRadioInterface);

	this    -> mySinCos     = new SinCos (rate); 
	omegaRDS                = (2 * M_PI * RDS_BITCLK_HZ) / (DSPFLOAT)rate;
//
//      for the decoder a la FMStack we need:
        synchronizerSamples     = sampleRate / (DSPFLOAT)RDS_BITCLK_HZ;
        symbolCeiling           = ceil (synchronizerSamples); 
        symbolFloor             = floor (synchronizerSamples);
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
	delete		my_rdsGroupDecoder;
	delete		my_rdsGroup;
	delete		my_rdsBlockSync;
	delete		mySinCos;
}

void	rdsDecoder::reset () {
	my_rdsGroupDecoder -> reset ();
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

bool	rdsDecoder::doDecode (DSPCOMPLEX v,
	                      DSPCOMPLEX * const m,
	                      ERdsMode  mode, int ptyLocale) {
// this is called typ. 19000 1/s
DSPCOMPLEX r;

   v = doMatchFiltering (v);
	v = my_AGC. process_sample (v);
/*
 *  Excluding the Costas Loop when DO_STEREO_SEPARATION_TEST is set is
 *  to be able to see the phase shift which remains on the RDS signal.
*/
#ifndef DO_STEREO_SEPARATION_TEST
	v = my_Costas. process_sample (v);
#endif
	if (mode	==  rdsDecoder::ERdsMode::RDS_2) {
		*m = v;
		doDecode2 (real (v), ptyLocale);
	   return true;
	}

	if (my_timeSync. process_sample (v, r)) {
//	this runs 19000/16 = 1187.5 1/s times
		//r = my_Costas1. process_sample (r); // the Costas is now made above
	   bool bit	= (real (r) >= 0);
	   processBit	(bit ^ previousBit, ptyLocale);
	   previousBit	= bit;
	   *m		= r;

	   return true; // tell caller a changed m value
	}

	return false;
}

void	rdsDecoder::processBit (bool bit, int ptyLocale) {

	switch (my_rdsBlockSync -> pushBit (bit, my_rdsGroup)) {
	   case rdsBlockSynchronizer::RDS_WAITING_FOR_BLOCK_A:
	      break;   // still waiting in block A

	   case rdsBlockSynchronizer::RDS_BUFFERING:
	      break;   // just buffer

	   case rdsBlockSynchronizer::RDS_NO_SYNC:
//	      resync if the last sync failed
	      setSyncErrors (my_rdsBlockSync -> getNumSyncErrors ());
	      my_rdsBlockSync -> resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_NO_CRC:
	      setCRCErrors (my_rdsBlockSync -> getNumCRCErrors ());
	      my_rdsBlockSync -> resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_COMPLETE_GROUP:
	      if (!my_rdsGroupDecoder -> decode (my_rdsGroup, ptyLocale)) {
	         ;   // error decoding the rds group
	      }
//	      my_rdsGroup -> clear ();
	      break;
	}
}

void	rdsDecoder::doDecode2	(float v, int ptyLocale) {
DSPFLOAT clkState;
std::complex<float> tt;

	syncBuffer [p]	= v;
	p		= (p + 1) % symbolCeiling;

	if (Resync || (my_rdsBlockSync -> getNumSyncErrors () > 3)) {
		synchronizeOnBitClk (syncBuffer, p);
	   my_rdsBlockSync -> resync ();
	   my_rdsBlockSync -> resetResyncErrorCounter ();
	   Resync = false;
	}

	clkState	= mySinCos -> getSin (bitClkPhase);
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
}

void	rdsDecoder::synchronizeOnBitClk (const std::vector<float> & v,
                                       int16_t first) {
bool	isHigh	= false;
int32_t	k = 0;
DSPFLOAT	phase;
DSPFLOAT correlationVector [symbolCeiling];

	memset (correlationVector, 0, symbolCeiling * sizeof (DSPFLOAT));

//	synchronizerSamples	= sampleRate / (DSPFLOAT)RDS_BITCLK_HZ;
	for (int i = 0; i < symbolCeiling; i ++) {
	   phase = fmod (i * (omegaRDS / 2), 2 * M_PI);
//	reset index on phase change
	   if (mySinCos -> getSin (phase) > 0 && !isHigh) {
	      isHigh = true;
	      k = 0;
	   }
	   else
	   if (mySinCos -> getSin (phase) < 0 && isHigh) {
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
