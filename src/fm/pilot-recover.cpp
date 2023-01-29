#
/*
 *    Copyright (C) 2010, 2011, 2012
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
 *
 */
#include	"pilot-recover.h"
#ifndef	M_PI_4
#define M_PI_4 0.78539816339744830962 
#endif
	pilotRecovery::pilotRecovery	(int32_t Rate_in,
	                                 DSPFLOAT omega,
	                                 DSPFLOAT gain,
	                                 SinCos * mySinCos) {
	this	-> Rate_in		= Rate_in;
	this	-> omega		= omega;
	this	-> gain			= gain;
	this	-> mySinCos		= mySinCos;
	this	-> pll_isLocked		= false;
	this	-> LockStable		= false;
	this	-> pilot_Lock		= 0;
	this	-> pilot_oldValue 	= 0;
	this	-> pilot_OscillatorPhase	= 0;
	this	-> SampleLockStableCnt	= 0;
}

	pilotRecovery::~pilotRecovery () {}

bool	pilotRecovery::isLocked	() const {
	return pll_isLocked;
}

float	pilotRecovery::getLockedStrength () const {
	return pilot_Lock;
}

DSPFLOAT pilotRecovery::getPilotPhase (const DSPFLOAT pilot) {
DSPFLOAT OscillatorValue	= mySinCos -> getSin (pilot_OscillatorPhase);
DSPFLOAT PhaseError		= pilot * OscillatorValue;
constexpr float alpha		= 1.0f / 3000.0f;

	pilot_OscillatorPhase += PhaseError * gain;
	const DSPFLOAT currentPhase = PI_Constrain (pilot_OscillatorPhase);
	pilot_OscillatorPhase = PI_Constrain (pilot_OscillatorPhase + omega);
	quadRef	= (OscillatorValue - pilot_oldValue) / omega;

	pilot_oldValue = OscillatorValue;
	pilot_Lock = alpha * (-quadRef * pilot) + pilot_Lock * (1.0 - alpha);
	bool pll_isLocked_temp = (pilot_Lock > 0.07f);
//	Check if the PLL lock is stable for a while.
//	This is important in very noisy receive condition
//	to maintain a stable mono mode.

	if (pll_isLocked_temp) {
//	for 500ms the PLL lock has to be stable
	   if (pll_isLocked || ++SampleLockStableCnt > (Rate_in >> 1)) {
	      pll_isLocked = true;
	   }
	}
	else  { // not locked -> reset stable counter
	   pll_isLocked = false;
	   SampleLockStableCnt = 0;
	}

	return currentPhase;
}

