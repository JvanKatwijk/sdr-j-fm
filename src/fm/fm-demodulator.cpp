/*
 *    Copyright (C)  2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fmreceiver
 *
 *    fmreceiver is free software; you can redistribute it and/or modify
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

#define	AM_DECODER		1
#define	PLL_DECODER		2
#define	MIXED_DECODER		3
#define	COMPLEX_BB_DECODER	4
#define	REAL_BB_DECODER		5
#define	DIFF_DECODER		6
struct {
	int code;
	QString name;
} nameTable [] = { 
	{AM_DECODER, 		"AM"},
	{PLL_DECODER,		"FM PLL Decoder"},
	{MIXED_DECODER,		"FM Mixed Demod"},
	{COMPLEX_BB_DECODER,	"FM Complex Baseband Delay"},
	{REAL_BB_DECODER,	"FM Real Baseband Delay"},
	{DIFF_DECODER,		"FM Difference Based"},
	{0,		""}
};

//	Just to play around a little, I implemented 5 common
//	fm decoders. The main source of inspiration is found in
//	a Diploma Thesis "Implementation of FM demodulator Algorithms
//	on a High Performance Digital Signal Processor", especially
//	chapter 3.
//	see https://docplayer.net/32191874-Dsp-implementation-of-fm-demodulator-algorithms-on-a-high-performance-digital-signal-processor-diploma-thesis-11-1.html
		fm_Demodulator::fm_Demodulator (int32_t rateIn):
	                                          mySinCos (rateIn) {
int32_t i;

	this	-> rateIn		= rateIn;

// highest freq in message
	float F_G			= 0.65 * rateIn / 2;
	float Delta_F			= 0.95 * rateIn / 2;    //
	float B_FM			= 2 * (Delta_F + F_G);
//
//	K_FM depends on fmRate which is known, it turns out that
//	K_FM	is about 15
	K_FM				=  2 * B_FM * M_PI / F_G;

	this	-> selectedDecoder 	= 3;
	this	-> max_freq_deviation	= 0.95 * (0.5 * rateIn);
	myfm_pll	= new pllC (rateIn,
	                            0,
	                            -max_freq_deviation,
	                            +max_freq_deviation,
	                            0.85 * rateIn,
	                            &mySinCos);
	arcSineSize			= 4 * 8192;
	Arcsine. resize (arcSineSize + 1);
	for (i = 0; i <= arcSineSize; i ++)
	   Arcsine [i] = asin (2.0 * i / arcSineSize - 1.0) / 2.0;

	Imin1			= 0.01;
	Qmin1			= 0.01;
	Imin2			= 0.01;
	Qmin2			= 0.01;
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

	for (int i = 0; nameTable [i]. code != 0; i ++) {
	   if (nameTable [i]. name == decoder) {
	      selectedDecoder = nameTable [i]. code;
	      break;
	   }
	}
}

QStringList fm_Demodulator::listNameofDecoder() const {
QStringList res;
	for (int i = 0; nameTable [i]. code != 0; i ++)
	   res = res << nameTable [i]. name;
	return res;
}

DSPFLOAT	fm_Demodulator::demodulate (DSPCOMPLEX z) {
DSPFLOAT res;
DSPFLOAT I, Q;
//	DC filter after FM demodulator (frequency offset)
float carrierAlpha = 0.0010f;
//	DC filter after AM demodulator (carrier level)
float fmDcAlpha    = 0.0001f;

	DSPFLOAT zAbs = abs (z);
	if (zAbs <= 0.001) {
	   I = Q = 0.001;  // do not make these 0 too often
	}
	else {
	   I = real (z) / zAbs;
	   Q = imag (z) / zAbs;
	}
//
//	added for demodulation of AM
//	get DC component or mean carrier power (needed also for level squelch)
	am_carr_ampl	= (1.0f - carrierAlpha) *
	                       am_carr_ampl + carrierAlpha * zAbs;

	if (selectedDecoder == AM_DECODER) { // AM
	   return decodeAM (z);
////	get carrier offset to have AFC for AM, too
//	   myfm_pll	-> do_pll (DSPCOMPLEX (I, Q));
//	   res		= myfm_pll -> getPhaseIncr ();
//	   fm_afc	= (1 - fmDcAlpha) * fm_afc + fmDcAlpha * res;
//
////	remove DC component from signal and norm level to carrier power
//	   float gainLimit = 0.01f;
//	   res = (zAbs - am_carr_ampl) /
//	             (am_carr_ampl < gainLimit ? gainLimit : am_carr_ampl);
//
////	this avoids short spikes which would cause the auto level
////	limitter to reduce audio level too much
//	   constexpr float audioLimit = 1.0f;
//	   if (res >  audioLimit)
//	      res =  audioLimit;
//	   else
//	   if (res < -audioLimit)
//	      res = -audioLimit;
//
//	    return res;
	}
//
//	Now for FM
	z = DSPCOMPLEX (I, Q);
	int	index	= 0;
	float	Scaler	= sqrt (2);
	switch (selectedDecoder) {
	   default:		// fall trhough
	   case PLL_DECODER: // PllDecoder
	      myfm_pll		-> do_pll (z);
	      res		= myfm_pll -> getPhaseIncr ();
	      break;

	   case MIXED_DECODER: // MixedDemodulator
	      res = myAtan. atan2 (Q * Imin1 - I * Qmin1,
	                           I * Imin1 + Q * Qmin1);
//	is same as ComplexBasebandDelay (expanded complex multiplication)
	      break;

	   case COMPLEX_BB_DECODER: // ComplexBasebandDelay
	      res = myAtan. argX (z * DSPCOMPLEX (Imin1, -Qmin1));
//	is same as MixedDemodulator
	      break;

	   case REAL_BB_DECODER: // RealBasebandDelay
	      res = (Imin1 * Q - Qmin1 * I + 1) / 2.0;
	      index	= (int)floor (res * arcSineSize);
	      if (index < 0)
	         index = 0;
	      if (index >= arcSineSize)
	         index = arcSineSize;
	      res	= Arcsine [index];
	      break;

	   case DIFF_DECODER: // DifferenceBased
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

float	fm_Demodulator::get_carrier_ampl 	() {
	return am_carr_ampl;
}

float	fm_Demodulator::decodeAM (std::complex<float> z) {
float fmDcAlpha    = 0.0001f;
float	res;
//
//	am_carr_ampl was already computed

//	get carrier offset to have AFC for AM, too
	myfm_pll	-> do_pll (z);
	res		= myfm_pll -> getPhaseIncr ();
	fm_afc		= (1 - fmDcAlpha) * fm_afc + fmDcAlpha * res;

//	remove DC component from signal and norm level to carrier power
	float gainLimit = 0.01f;
	res = (abs (z) - am_carr_ampl) /
	             (am_carr_ampl < gainLimit ? gainLimit : am_carr_ampl);

//	this avoids short spikes which would cause the auto level
//	limiter to reduce audio level too much
	float audioLimit = 1.0f;
	if (res >  audioLimit)
	   res =  audioLimit;
	else
	if (res < -audioLimit)
	   res = -audioLimit;

	return res;
}
