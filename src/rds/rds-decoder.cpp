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
//
	rdsDecoder::rdsDecoder (RadioInterface	*myRadio,
	                        int32_t		rate):
	                            my_rdsBlockSync (myRadio),
	                            my_rdsGroupDecoder (myRadio),
	                            my_costas (rate, 1.0f / 16.0f,
	                                            0.02f/16.0f, 10.0f) {
	decoder_1	= new rdsDecoder_1 (myRadio, rate,
	                                    &my_rdsBlockSync,
	                                    &my_rdsGroup,
	                                    &my_rdsGroupDecoder);
	decoder_2 	= new rdsDecoder_2 (myRadio, rate,
	                                    &my_rdsBlockSync,
	                                    &my_rdsGroup,
	                                    &my_rdsGroupDecoder);
	decoder_3 	= new rdsDecoder_3 (myRadio, rate,
	                                    &my_rdsBlockSync,
	                                    &my_rdsGroup,
	                                    &my_rdsGroupDecoder);
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
// this is called typ. 19000 1/s
DSPCOMPLEX r;
bool	b;
	v	= my_costas. process_sample (v);
	switch (mode) {
	   case rdsDecoder::ERdsMode::RDS_1:
	      *m =  v * 4.0f;
	      b = decoder_1 -> doDecode (real (v), ptyLocale);
	      return b;

	   case rdsDecoder::ERdsMode::RDS_2:
	      b = decoder_2 -> doDecode (v, m,  ptyLocale);
	      return b;

	   case rdsDecoder::ERdsMode::RDS_3:
	      *m = v * 4.0f;
	      b = decoder_3 -> doDecode (real (v), ptyLocale);
	      return b;

	   default:
	      return false;
	}
}

