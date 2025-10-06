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
#include	"rds-decoder.h"
#include	"rds-decoder-1.h"
#include	"rds-decoder-2.h"
#include	"rds-decoder-3.h"
#include	"radio.h"
//
//
//	Since there are different decoders for rds,
//	this interface one is merely a dispatcher

	rdsDecoder::rdsDecoder (RadioInterface	*myRadioInterface,
	                        int32_t		rate):
	                            my_rdsGroupDecoder (myRadioInterface),
	                            my_rdsBlockSync (myRadioInterface),
	                            my_costas (rate, 1.0f / 16.0f,
	                                            0.02f/16.0f, 10.0f),
	                            my_AGC (2e-3f, 0.38f, 9.0f) {
	decoder_1	= new rdsDecoder_1 (myRadioInterface, rate);
	decoder_2 	= new rdsDecoder_2 (myRadioInterface, rate);
	decoder_3 	= new rdsDecoder_3 (myRadioInterface, rate,
	                                    &my_rdsBlockSync,
	                                    &my_rdsGroup,
	                                    &my_rdsGroupDecoder);
	my_rdsGroup. clear ();
        my_rdsBlockSync. setFecEnabled (true);
 
        connect (this, SIGNAL (setCRCErrors (int)),
                 myRadioInterface, SLOT (setCRCErrors (int)));
        connect (this, SIGNAL (setSyncErrors (int)),
                 myRadioInterface, SLOT (setSyncErrors(int)));

}

	rdsDecoder::~rdsDecoder () {
	delete decoder_1;
	delete decoder_2;
	delete decoder_3;
}

void	rdsDecoder::reset () {
	my_rdsGroupDecoder. reset ();
}

bool	rdsDecoder::doDecode (DSPCOMPLEX v,
	                      DSPCOMPLEX *m,
	                      ERdsMode  mode, int ptyLocale) {
// this is called typ. 24000 1/s
bool	b;
uint8_t	theBit;

	switch (mode) {
	   case rdsDecoder::ERdsMode::RDS_1:
	      v	= my_costas. process_sample (v);
	      *m = v * 4.0f;
//	      *m	= my_AGC. process_sample (v);
	      b = decoder_1 -> doDecode (real (v), &theBit);
	      if (b)
	         processBit (theBit, ptyLocale);
	      return b;

	   case rdsDecoder::ERdsMode::RDS_2:
	      b = decoder_2 -> doDecode (v, m, &theBit);
	      if (b)
	         processBit (theBit, ptyLocale);
	      return b;

	   case rdsDecoder::ERdsMode::RDS_3:
	      v	= my_costas. process_sample (v);
	      *m = v * 4.0f;
//	      *m	= my_AGC. process_sample (v);
	      b = decoder_3 -> doDecode (real (v), &theBit);
	      if (b)
	         processBit (theBit, ptyLocale);
	      return b;

	   default:
	      return false;
	}
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
	      my_rdsGroup. clear ();
	      break;
	}
}
