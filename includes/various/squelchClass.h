#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SQUELCHCLASS_H
#define __SQUELCHCLASS_H

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
	float	noiseSquelchThreshold;
// value between 0 (audio open) and 1 (audio suppressed)
	float		levelSquelchThreshold;
	int32_t		keyFrequency;
	int32_t		holdPeriod;
	int32_t		sampleRate;
	bool		squelchSuppress;
	bool		squelchSuppressLast;
	int32_t		squelchCount;
	float		average_High;
	float		average_Low;
	HighPassIIR	squelchHighpass;
	LowPassIIR	squelchLowpass;

public:
		squelch	(int32_t squelchThreshold,
	                 int32_t keyFrequency,
	                 int32_t bufsize, int32_t sampleRate);
		~squelch () = default;

	void	setSquelchLevel		(int n);
	bool	getSquelchActive	() {
	   return squelchSuppress; }
	float do_noise_squelch	(float soundSample);
	float do_level_squelch	(float soundSample, float iCarrierLevel);

signals:
	void	setSquelchIsActive	(bool);
};

#endif
