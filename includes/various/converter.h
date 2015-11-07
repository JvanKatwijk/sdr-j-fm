#
/*
 *    Copyright (C) 2010, 2011, 2012
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
 */

#ifndef	__CONVERTER
#define	__CONVERTER
//
//	Very straightforward fractional resampler
//	What needs to be done is precompute the
//	sine and cosine functions with sufficient precision
#include	<stdint.h>
#include	<stdio.h>
#include	<math.h>
#include	"sincos.h"
template <class elementtype>
class	converter {
private:
	int32_t		rateIn;	
	int32_t		rateOut;
	int32_t		blockLength;
	int32_t		width;
	elementtype	*buffer;
	long double	floatTime;
	long double	currentTime;
	long double	inPeriod;
	long double	outPeriod;
	long double	blockDuration;
	int32_t		bufferP;
	int32_t		bufferSize;
	uint32_t	bufferMask;

static inline
double sincPI (double a) {
	if (a == 0)
	   return 1.0;
	if (a < 0)
	   return - sin (- M_PI * a) / (M_PI * a);
	return sin (M_PI * a) / (M_PI * a);
}

static inline
double HannCoeff (double a, int16_t width) {
double x = 2 * M_PI * (0.5 + a / width);

	if (x < 0)
	   return 0.5 - 0.5 * cos (- x);
	return 0.5 - 0.5 * cos (x);
}
//
//	Shannon applied to floatTime
//	We determine the buffer entry in the table acting as 'zero'
//	and the offset of the time 
elementtype	getValueforTime (double floatTime) {
double	localTime = fmod (floatTime, inPeriod);
int32_t	index	= floor (fmod (floatTime, blockDuration) / inPeriod);
int32_t	i;
elementtype	res	= 0;

	for (i = - width / 2; i < width / 2; i ++) {
	   double ag	= (localTime + i * inPeriod) / inPeriod;
	   float factor = HannCoeff (ag, width) * sincPI (ag);
	   res	=  res + buffer [(index + i) & (bufferSize - 1)] * factor;
	}
	return res;
}

public:
		converter (int32_t	rateIn,
	                   int32_t	rateOut,
	                   int32_t	blockLength,
	                   int16_t	width) {
	this	-> rateIn	= rateIn;
	this	-> rateOut	= rateOut;
	this	-> blockLength	= blockLength;
	this	-> width	= width;
	bufferSize		= 8192;
	bufferMask		= bufferSize - 1;
	buffer			= new elementtype [bufferSize];
	bufferP			= 0;	
	inPeriod		= 1.0 / rateIn;
	outPeriod		= 1.0 / rateOut;
	blockDuration		= bufferSize * inPeriod;
	currentTime		= 0;
	fprintf (stderr, "converter from %d to %d\n", rateIn, rateOut);
//	the first width / 2 samples are only used in the interpolation
//	furthermore, they are neglected. So, we start with:
	floatTime		= width / 2 * inPeriod;
}

		~converter (void) {
	delete [] buffer;
}
//
//	The buffer consists of three parts
//	width / 2 "old" values
//	blockLength values that will be processed
//	width / 2 "future" values
//
//	Whenever the buffer filling reaches blockLength + width,
//	we map the blockLength samples in the middle
bool	convert	(elementtype in, elementtype *out, int16_t *nOut) {
int	outP		= 0;
long double	endTime		= floatTime + blockLength * inPeriod;

	buffer [bufferP] = in;
	bufferP		= (bufferP + 1) % bufferSize;
	currentTime	+= inPeriod;
	if (currentTime < endTime)
	   return false;

//	so, here, currentTime >= endTime and we can process
//	from the current value of floatTime to the value near to endTime
	while (floatTime < endTime ) {
	   out [outP++] = getValueforTime (floatTime);
	   floatTime	+= outPeriod;
	}

	*nOut = outP;
	return true;
}

int32_t	getOutputSize	(void) {
	return rateOut * blockLength / rateIn + 1;
}

int32_t	getOutputsize	(void) {
	return rateOut * blockLength / rateIn + 1;
}
};

#endif

