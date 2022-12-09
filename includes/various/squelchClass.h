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
#define __SQUELCHCLASS

#include "fm-constants.h"
#include "iir-filters.h"
#include <QObject>

//
//	just a simple class to include elementary squelch handling
//	The basic idea is that when there is no signal, the noise
//	in the upper bands will be roughly as high as in the lowerbands.
//	Measurement shows that the average amplitude of the noise in the
//	upper band is roughly 0.6 times that of the lower part.
//	If the average signal value of the upper part is larger
//	than factor times the average signal value of the lower part,
//	where factor is a value between 0 .. 1, set by the user.

class RadioInterface;

class squelch : public QObject {
Q_OBJECT

private:
// value between 0 (audio suppressed) and 1 (audio open)
	DSPFLOAT	mNoiseSquelchThreshold;
// value between 0 (audio open) and 1 (audio suppressed)
	DSPFLOAT	mLevelSquelchThreshold;
	int32_t		mKeyFrequency;
	int32_t		mHoldPeriod;
	int32_t		mSampleRate;
	bool		mSquelchSuppress;
	bool		mSquelchSuppressLast;
	int32_t		mSquelchCount;
	DSPFLOAT	mAverage_High;
	DSPFLOAT	mAverage_Low;
	HighPassIIR	mSquelchHighpass;
	LowPassIIR	mSquelchLowpass;

public:
		squelch	(const int32_t iSquelchThreshold,
	                 const int32_t iKeyFrequency,
	                 const int32_t iBufsize, const int32_t iSampleRate);
		~squelch (void) = default;

	void	setSquelchLevel		(int n);
	bool	getSquelchActive	() const {
	   return mSquelchSuppress; }
	DSPFLOAT do_noise_squelch	(const DSPFLOAT soundSample);
	DSPFLOAT do_level_squelch	(const DSPFLOAT soundSample,
	                                 const DSPFLOAT iCarrierLevel);

signals:
	void	setSquelchIsActive	(bool);
};

#endif
