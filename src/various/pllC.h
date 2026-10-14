#
/*
 *    Copyright (C) 2010, 2011
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
 *    fmreceiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#pragma once
/*
 */
#include	"fm-constants.h"
#include	"sincos.h"
#include	"Xtan2.h"

class pllC {
private:
	int32_t		rate;
	int32_t		cf;   // center frequency
	DSPFLOAT	NcoPhase;
	DSPFLOAT	phaseIncr;
	DSPFLOAT	NcoHLimit;
	DSPFLOAT	NcoLLimit;
	DSPFLOAT	Beta;
	DSPCOMPLEX	pll_Delay;
	SinCos		*mySinCos;
	DSPFLOAT	phaseError;
	compAtan	myAtan;
//	DSPCOMPLEX	oldNcoSignal;
//	bool		pll_lock;

public:
			pllC (int32_t	rate,
			       DSPFLOAT freq, DSPFLOAT lofreq, DSPFLOAT hifreq,
	                       DSPFLOAT bandwidth,
	                       SinCos *Table	= nullptr);

			~pllC ();

	void		do_pll		(DSPCOMPLEX signal);
	DSPCOMPLEX	getDelay	();
	DSPFLOAT	getPhaseIncr	();
	DSPCOMPLEX	getNco		();
	DSPFLOAT	getPhaseError	();
//	bool		isLocked	();
};

