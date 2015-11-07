#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair programming
 *
 *    This file is part of the SDR-J (JSDR).
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License. 
 *    All copyrights of the original authors are recognized.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __SQUELCHCLASS
#define	__SQUELCHCLASS

#include	"iir-filters.h"
//
//	just a simple class to include elementary squelch handling
//	The basic idea is that when there is no signal, the noise
//	in the upper bands will be roughly as high as in the lowerbands.

#define	SQUELCH_HYSTERESIS	0.02

class	squelch {
private:
	int16_t		squelchThreshold;
	int32_t		keyFrequency;
	int32_t		holdPeriod;
	int32_t		sampleRate;
	bool		squelchSuppress;
	int32_t		squelchCount;
	DSPFLOAT	squelchAverage;
	HighPassIIR	*squelchHighpass;
	LowPassIIR	*squelchLowpass;
public:
	squelch (int32_t	squelchThreshold,
	         int32_t	keyFrequency,
	         int32_t	bufsize,
	         int32_t	sampleRate) {
	this	-> squelchThreshold	= squelchThreshold;
	this	-> keyFrequency		= keyFrequency;
	this	-> holdPeriod		= bufsize;
	this	-> sampleRate		= sampleRate;

	squelchSuppress			= false;
	squelchCount			= 0;
	squelchAverage			= 0;
	squelchHighpass			= new HighPassIIR (20,
	                                                   keyFrequency - 100,
	                                                   sampleRate,
	                                                   S_CHEBYSHEV);
	squelchLowpass			= new LowPassIIR (20,
	                                                  keyFrequency,
	                                                  sampleRate,
	                                                  S_CHEBYSHEV);
	squelchAverage			= 0;
}

	~squelch (void) {
	delete	squelchHighpass;
	delete	squelchLowpass;
}

void		setSquelchLevel (int n) {
	squelchThreshold = n;
}

static inline
DSPFLOAT decayingAverage (DSPFLOAT old, DSPFLOAT input, DSPFLOAT weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}

DSPCOMPLEX	do_squelch (DSPCOMPLEX	v) {
DSPFLOAT	val;
	val	= abs (squelchHighpass	-> Pass (v));
	
	squelchAverage	= decayingAverage (squelchAverage,
	                                   val, sampleRate / 100);

	if (++squelchCount < holdPeriod) {	// use current squelch state
	   if (squelchSuppress)
	      return DSPCOMPLEX (0.001, 0.001);
	   else
	      return squelchLowpass -> Pass (v);
	}

	squelchCount = 0;
//	o.k. looking for a new squelch state
	if (squelchThreshold == 0)  	// force squelch if zero
	   squelchSuppress = true;
	else	// recompute 
	if (squelchAverage <
	          ((float)squelchThreshold / 500 - SQUELCH_HYSTERESIS))
	   squelchSuppress = false;
	else
	if (squelchAverage >=
	               ((float)squelchThreshold / 500 + SQUELCH_HYSTERESIS))
	   squelchSuppress = true;
//	else just keep old squelchSuppress value
	
	return squelchSuppress ?
	           DSPCOMPLEX (0.001, 0.001) :
	           squelchLowpass -> Pass (v);
}
};

#endif

