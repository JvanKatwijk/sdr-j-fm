#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J.
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

#ifndef	__FM_LEVELS
#define	__FM_LEVELS

#include	"fm-constants.h"
#include	"fft.h"

class	fmLevels {
public:
			fmLevels	(int16_t size,
	                                 int32_t rate,
	                                 int16_t freq);
	        	~fmLevels	(void);
	void		addItem			(DSPFLOAT);
	DSPFLOAT	getPilotStrength	(void);
	DSPFLOAT	getRdsStrength		(void);
	DSPFLOAT	getNoiseStrength	(void);
	DSPFLOAT	getSignalStrength	(void);
private:
	int16_t		size;
	int32_t		Rate_in;
	int16_t		freq;
	int16_t		fillSize;
	DSPFLOAT	rdsNoiseLevel;
	DSPFLOAT	pilotNoiseLevel;
	DSPFLOAT	rdsLevel;
	DSPFLOAT	pilotLevel;
	DSPFLOAT	signalLevel;

	DSPFLOAT	*Window;
	int16_t		pilotPoller;
	int16_t		rdsPoller_a;
	int16_t		rdsPoller_b;
	int16_t		pilotNoisePol_a;
	int16_t		pilotNoisePol_b;
	int16_t		rdsNoisePol_a;
	int16_t		rdsNoisePol_b;

	common_fft	*compute;
	DSPCOMPLEX	*buffer;
	DSPFLOAT	*inputBuffer;
	int16_t		bufferPointer;
	int32_t		counter;
};

#endif

