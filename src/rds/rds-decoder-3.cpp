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
#include	"rds-decoder-3.h"
#include	"radio.h"
#include	<cmath>
#include	<vector>
#include	<fstream>

#define RDS_BITCLK_HZ 1187.5

//	decoder_2 is based on the FMstack software of Michael Feilen
/*
 *	RDS is a bpsk-like signal, with a baudrate 1187.5
 *	on a carrier of  3 * 19 k.
 *	48 cycles per bit, 1187.5 bits per second.
 *	With a reduced sample rate of 19k this would mean
 *	19000 / 1187.5 samples per bit, i.e. 16
 *	samples per bit.
 */
	rdsDecoder_3::rdsDecoder_3 (RadioInterface	*myRadio,
	                            int32_t             rate,
                                    rdsBlockSynchronizer  *my_rdsBlockSync,
                                    RDSGroup            *my_rdsGroup,
                                    rdsGroupDecoder     *my_rdsGroupDecoder):
	                               mySinCos  (rate), 
	                               rdsFilter (21, RDS_WIDTH, rate) {

float	synchronizerSamples;

	this	-> myRadioInterface	= myRadio;
	this	-> sampleRate		= rate;
	this	-> my_rdsBlockSync	= my_rdsBlockSync;
	this	-> my_rdsGroup		= my_rdsGroup;
	this	-> my_rdsGroupDecoder	= my_rdsGroupDecoder;

        synchronizerSamples		= sampleRate / (float)RDS_BITCLK_HZ;
        symbolCeiling			= ceil (synchronizerSamples); 
        symbolFloor			= floor (synchronizerSamples);
//

	previousBit		= 0;
	my_rdsGroup	->  clear ();
	my_rdsBlockSync	-> setFecEnabled (true);

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
        Resync                  = true;
}

	rdsDecoder_3::~rdsDecoder_3 () {
}

//
bool	rdsDecoder_3::doDecode	(float v, uint8_t *d) {
float clkState;
std::complex<float> tt;
bool	res	= false;

	syncBuffer [p]	= rdsFilter. Pass (v);
	p		= (p + 1) % symbolCeiling;

	if (Resync || (my_rdsBlockSync -> getNumSyncErrors () > 3)) {
	   synchronizeOnBitClk (syncBuffer, p);
	   my_rdsBlockSync -> resync ();
	   my_rdsBlockSync -> resetResyncErrorCounter ();
	   Resync = false;
	}

	clkState	= mySinCos. getSin (bitClkPhase);
	bitIntegrator	+= clkState * v;
//
//	rising edge -> look at integrator
	if (prev_clkState <= 0 && clkState > 0) {
	   bool theBit	= bitIntegrator >= 0;
	   *d	= theBit ^ previousBit;
	   bitIntegrator	= 0;		// we start all over
	   previousBit		= theBit;
	   res		= true;
	}

	prev_clkState	= clkState;
	bitClkPhase	= fmod (bitClkPhase + omegaRDS, 2 * M_PI);
	return res;
}


void	rdsDecoder_3::synchronizeOnBitClk (std::vector<float> &v,
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
