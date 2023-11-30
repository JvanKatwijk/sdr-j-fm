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

#pragma once

#include	"fft-filters.h"
#include	"fm-constants.h"
#include	"sincos.h"

class	pilotRecovery {
public:
		pilotRecovery	(int32_t Rate_in, DSPFLOAT omega,
	                         DSPFLOAT gain, SinCos * mySinCos);
		~pilotRecovery	();
	bool	isLocked	() const;
	float	getLockedStrength() const;
	DSPFLOAT getPilotPhase	(const DSPFLOAT pilot);
private:
	int32_t		Rate_in;
	int32_t		SampleLockStableCnt;
	DSPFLOAT	pilot_OscillatorPhase;
	DSPFLOAT	pilot_oldValue;
	DSPFLOAT	omega;
	DSPFLOAT	gain;
	SinCos		*mySinCos;
	DSPFLOAT	pilot_Lock;
	DSPFLOAT	quadRef;
	bool		pll_isLocked;
	bool		LockStable;
};

