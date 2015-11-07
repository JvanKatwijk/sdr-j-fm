#
/*
 *    Copyright (C)  2014
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

#include	"fm-levels.h"

	fmLevels::fmLevels	(int16_t size,
	                         int32_t rate, int16_t freq) {
int16_t	i;

float	binSize;	// size of bin in Hz

	if ((size & (size - 1)) != 0)
	   size = 256;

	this	-> size		= size;
	this	-> Rate_in	= rate;
	this	-> freq		= freq;
	binSize			= (float)Rate_in / size;
	bufferPointer		= 0;
	inputBuffer		= new DSPFLOAT [size];
	compute			= new common_fft (size);
	buffer			= compute	-> getVector ();
	Window			= new DSPFLOAT [size];
	for (i = 0; i < size; i ++)
	   Window [i] = 0.42 - 0.5 * cos ((2.0 * M_PI * i) / (size - 1)) +
	                      0.08 * cos ((4.0 * M_PI * i) / (size - 1));

	rdsPoller_a		= (int)((57000 - 1450 / 2) / binSize);
	rdsPoller_b		= (int)((57000 + 1450 / 2) / binSize);
	rdsNoisePol_a		= (int)(54000 / binSize);
	rdsNoisePol_b		= (int)(60500 / binSize);
	pilotPoller		= (int)(19000 / binSize);
	pilotNoisePol_a		= (int)(17000 / binSize);
	pilotNoisePol_b		= (int)(21000 / binSize);

	pilotLevel		= 0;
	rdsLevel		= 0;
	pilotNoiseLevel		= 0;
	rdsNoiseLevel		= 0;
	signalLevel		= 0;
	counter			= 0;
}

	fmLevels::~fmLevels	(void) {
	delete	compute;
	delete	inputBuffer;
	delete	Window;
}

void	fmLevels::addItem		(DSPFLOAT v) {
int16_t		i;
DSPFLOAT	p0	= 0;
DSPFLOAT	p1	= 0;
DSPFLOAT	p2	= 0;
DSPFLOAT	p3	= 0;
DSPFLOAT	p4	= 0;

	inputBuffer [bufferPointer] = v * Window [bufferPointer];
	if (++ bufferPointer >= size)
	   bufferPointer = 0;

	counter ++;
	if (counter <= Rate_in / freq) 
	   return;

	counter		= 0;
	for (i = 0; i < size; i ++) {
	   buffer [i] =
	         DSPCOMPLEX (inputBuffer [(bufferPointer + i) % size], 0);
	}

	compute	-> do_FFT ();

	p0	= (abs (buffer [2]) + abs (buffer [size - 2])) / 2; 
	p1	= (0.5 * abs (buffer [pilotPoller - 1]) +
	                 abs (buffer [pilotPoller]) +
	           0.5 * abs (buffer [pilotPoller + 1])) / 2;
	p2	= (0.5 * abs (buffer [rdsPoller_a - 1]) +
	                 abs (buffer [rdsPoller_a]) +
	           0.5 * abs (buffer [rdsPoller_a + 1])  +
	           0.5 * abs (buffer [rdsPoller_b - 1]) +
	                 abs (buffer [rdsPoller_b]) + 
	           0.5 * abs (buffer [rdsPoller_b + 1])) / 4;
	p3	= (0.5 * abs (buffer [pilotNoisePol_a - 1]) +
	                 abs (buffer [pilotNoisePol_a]) +
	           0.5 * abs (buffer [pilotNoisePol_a + 1]) +
      	           0.5 * abs (buffer [pilotNoisePol_b - 1]) +
	                 abs (buffer [pilotNoisePol_b]) +
	           0.5 * abs (buffer [pilotNoisePol_b + 1])) / 4;
	p4	= (0.5 * abs (buffer [rdsNoisePol_a - 1]) +
	                 abs (buffer [rdsNoisePol_a]) +
	           0.5 * abs (buffer [rdsNoisePol_a + 1]) +
	           0.5 * abs (buffer [rdsNoisePol_b - 1]) +
	                 abs (buffer [rdsNoisePol_b]) +
	           0.5 * abs (buffer [rdsNoisePol_b + 1])) / 4;
	
	signalLevel	= 0.5 * p0 + 0.5 * signalLevel;
	pilotLevel	= 0.3 * p1 + 0.7 * pilotLevel;
	rdsLevel	= 0.3 * p2 + 0.7 * rdsLevel;
	pilotNoiseLevel	= 0.3 * p3 + 0.7 * pilotNoiseLevel;
	rdsNoiseLevel	= 0.3 * p4 + 0.7 * rdsNoiseLevel;
}

DSPFLOAT	fmLevels::getSignalStrength (void) {
	return get_db (signalLevel, 256) -
	         get_db (rdsNoiseLevel, 256);
}

DSPFLOAT	fmLevels::getPilotStrength (void) {
	return	get_db (pilotLevel, 256) -
	            get_db (pilotNoiseLevel, 256);
}

DSPFLOAT	fmLevels::getRdsStrength (void) {
	return	get_db (rdsLevel, 256) - get_db (rdsNoiseLevel, 256);
}

DSPFLOAT	fmLevels::getNoiseStrength (void) {
	return	get_db ((pilotNoiseLevel + rdsNoiseLevel) / 2, 256) -
	        get_db (0, 256);
}


