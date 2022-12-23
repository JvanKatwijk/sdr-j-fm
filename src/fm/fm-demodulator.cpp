/*
 *    Copyright (C)  2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J-FM program.
 *
 *    SDR-J-FM is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J-FM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J-FM; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include  "fm-demodulator.h"
#include  "Xtan2.h"
#include  <assert.h>

fm_Demodulator::TDecoderListNames fm_Demodulator::sIdx2DecoderName = {
	"AM",
	"FM PLL Decoder",
	"FM Mixed Demod",
	"FM Complex Baseband Delay",
	"FM Real Baseband Delay",
	"FM Difference Based"
};

//	Just to play around a little, I implemented 5 common
//	fm decoders. The main source of inspiration is found in
//	a Diploma Thesis "Implementation of FM demodulator Algorithms
//	on a High Performance Digital Signal Processor", especially
//	chapter 3.
		fm_Demodulator::fm_Demodulator (int32_t rateIn,
	                                        SinCos  *mySinCos,
	                                        DSPFLOAT K_FM) {
int32_t i;

	this	-> rateIn		= rateIn;
	this	-> mySinCos		= mySinCos;
	this	-> K_FM			= 2 * K_FM;

	this	-> selectedDecoder 	= 3;
	this	-> max_freq_deviation	= 0.95 * (0.5 * rateIn);
	myfm_pll	= new pllC (rateIn,
	                            0,
	                            -max_freq_deviation,
	                            +max_freq_deviation,
	                            0.85 * rateIn,
	                            mySinCos);
	arcSineSize			= 4 * 8192;
	Arcsine. resize (arcSineSize + 1);
	for (i = 0; i <= arcSineSize; i ++)
	   Arcsine [i] = asin (2.0 * i / arcSineSize - 1.0) / 2.0;

	Imin1			= 0.2;
	Qmin1			= 0.2;
	Imin2			= 0.2;
	Qmin2			= 0.2;
	fm_afc			= 0;
	fm_cvt			= 1.0;
//	fm_cvt		= 0.50 * (rateIn / (M_PI * 150000));
	am_carr_ampl		= 0;
}

		fm_Demodulator::~fm_Demodulator() {
	delete  myfm_pll;
}

void	fm_Demodulator::setDecoder (const QString &decoder) {
	this -> selectedDecoder = -1;

	for (const auto & dc : listNameofDecoder ()) {
	   this -> selectedDecoder ++;
	   if (decoder == dc)
	      break;
	}
}

fm_Demodulator::TDecoderListNames
	&fm_Demodulator::listNameofDecoder() const {
	return sIdx2DecoderName;
}

DSPFLOAT	fm_Demodulator::demodulate (DSPCOMPLEX z) {
DSPFLOAT res;
DSPFLOAT I, Q;
//	DC filter after FM demodulator (frequency offset)
float carrierAlpha = 0.0010f;
//	DC filter after AM demodulator (carrier level)
float fmDcAlpha    = 0.0001f;
int32_t arcSineIdx;

	DSPFLOAT zAbs = abs (z);
	if (zAbs <= 0.001) {
	   I = Q = 0.001;  // do not make these 0 too often
	}
	else {
	   I = real (z) / zAbs;
	   Q = imag (z) / zAbs;
	}
//
//	added for demodation of AM
//	get DC component or mean carrier power (needed also for level squelch)
	am_carr_ampl	= (1.0f - carrierAlpha) *
	                       am_carr_ampl + carrierAlpha * zAbs;

	if (selectedDecoder == 0) { // AM
//	get carrier offset to have AFC for AM, too
	   myfm_pll	-> do_pll (DSPCOMPLEX (I, Q));
	   res		= myfm_pll -> getPhaseIncr ();
	   fm_afc	= (1 - fmDcAlpha) * fm_afc + fmDcAlpha * res;

//	remove DC component from signal and norm level to carrier power
	   float gainLimit = 0.01f;
	   res = (zAbs - am_carr_ampl) /
	             (am_carr_ampl < gainLimit ? gainLimit : am_carr_ampl);

//	this avoids short spikes which would cause the auto level
//	limitter to reduce audio level too much
	   constexpr float audioLimit = 1.0f;
	   if (res >  audioLimit)
	      res =  audioLimit;
	   else
	   if (res < -audioLimit)
	      res = -audioLimit;

	    return res;
	}
//
//	Now for FM
	z = DSPCOMPLEX (I, Q);
	int	index	= 0;
//	see https://docplayer.net/32191874-Dsp-implementation-of-fm-demodulator-algorithms-on-a-high-performance-digital-signal-processor-diploma-thesis-11-1.html
	float	Scaler	= sqrt (2);
	switch (selectedDecoder) {
	   default:		// fall trhough
	   case 1: // PllDecoder
	      myfm_pll		-> do_pll (z);
	      res		= myfm_pll -> getPhaseIncr ();
	      break;

	   case 2: // MixedDemodulator
	      res = myAtan. atan2 (Q * Imin1 - I * Qmin1,
	                           I * Imin1 + Q * Qmin1);
//	is same as ComplexBasebandDelay (expanded complex multiplication)
	      break;

	   case 3: // ComplexBasebandDelay
	      res = myAtan. argX (z * DSPCOMPLEX (Imin1, -Qmin1));
//	is same as MixedDemodulator
	      break;

	   case 4: // RealBasebandDelay
	      res = (Imin1 * Q - Qmin1 * I + 1) / 2.0;
	      index	= (int)floor (res * arcSineSize);
	      if (index < 0)
	         index = 0;
	      if (index >= arcSineSize)
	         index = arcSineSize;
	      res	= Arcsine [index];
	      break;

	   case 5: // DifferenceBased
	      res	= (Imin1 * (Q - Qmin2) - Qmin1 * (I - Imin2));
	      res	/= (Imin1 * Imin1 + Qmin1 * Qmin1) * Scaler;
	      Imin2	= Imin1;
	      Qmin2	= Qmin1;
	      break;
	}

	fm_afc = (1 - fmDcAlpha) * fm_afc + fmDcAlpha * res;
	res = 20.0f * (res - fm_afc) * fm_cvt / K_FM;

//	and shift ...
	Imin1 = I;
	Qmin1 = Q;

	return res;
}

DSPFLOAT fm_Demodulator::get_DcComponent() {
	return fm_afc;
}

