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
 *
 *	we differentiate between integer decimation and
 *	fractional work. Therefore, the "decimators" are all derived
 *	from a simple "virtualDecimator"
 */

#include	"resampler.h"

	virtualDecimator::virtualDecimator (void) {
}

	virtualDecimator::~virtualDecimator (void) {
}

bool	virtualDecimator::doResample (DSPCOMPLEX Sample,
	                              DSPCOMPLEX *outSample, int32_t *amount) {
	*outSample	= Sample;
	*amount		= 1;
	return true;
}

bool	virtualDecimator::doResample (DSPFLOAT Sample,
	                              DSPFLOAT *outSample, int32_t *amount) {
	*outSample	= Sample;
	*amount		= 1;
	return true;
}

int32_t	virtualDecimator::getOutputsize	(void) {
	return 1;
}
//
//
//	The resampler is merely embodying a selected decimator.
//	For now, we refrain from upsampling.
//
	reSampler::reSampler	(int32_t inRate, int32_t outRate,
	                         int32_t inBlocksize) {
	if (inRate >= outRate) {
	   if (inRate % outRate == 0)	// integer decimation
	      theWorker = new IDecimator (inRate, outRate, inBlocksize);
	   else
	      theWorker = new FDecimator (inRate, outRate, inBlocksize);
	}
	else
	   theWorker = new virtualDecimator ();
}

	reSampler::~reSampler	(void) {
	delete theWorker;
}

bool	reSampler::doResample	(DSPCOMPLEX v,
	                         DSPCOMPLEX *outSamples, int32_t *amount) {
	   return theWorker -> doResample (v, outSamples, amount);
}

bool	reSampler::doResample	(DSPFLOAT v,
	                         DSPFLOAT *outSamples, int32_t *amount) {
	   return theWorker -> doResample (v, outSamples, amount);
}

int32_t	reSampler::getOutputsize	(void) {
	   return theWorker -> getOutputsize ();
}
//
//
//	The IDecimator is used whenever the ratio in/out is integer.
//	It is assumed that the samples in are filtered
//	To keep things simple, we use a single buffer
//	for both the complex and the float (double) samplestreams
	IDecimator::IDecimator (int32_t inRate,
	                        int32_t outRate, int32_t insize) {
	intRatio	= inRate / outRate;
	bufCounter	= 0;
	inCount		= intRatio;
	outSize		= insize / intRatio;
	tempBuffer	= new DSPCOMPLEX [2 * outSize + 1];
}

	IDecimator::~IDecimator (void) {
	delete	tempBuffer;
}

bool	IDecimator::doResample	(DSPCOMPLEX v,
	                         DSPCOMPLEX *out, int32_t *amount) {
int32_t	i;
	if (--inCount > 0) 
	   return false;
	tempBuffer [bufCounter ++] = v;
	inCount = intRatio;
	if (bufCounter >= outSize) {
	   bufCounter = 0;
	   *amount = outSize;
	   for (i = 0; i < outSize; i ++)
	      out [i] = tempBuffer [i];
	   return true;
	}
	return false;
}

bool	IDecimator::doResample	(DSPFLOAT v,
	                         DSPFLOAT *out, int32_t *amount) {
int32_t	i;
	if (--inCount > 0) 
	   return false;
	tempBuffer [bufCounter ++] = DSPCOMPLEX (v, 0);
	inCount = intRatio;
	if (bufCounter >= outSize) {
	   bufCounter = 0;
	   *amount = outSize;
	   for (i = 0; i < outSize; i ++)
	      out [i] = real (tempBuffer [i]);
	   return true;
	}
	return false;
}

int32_t	IDecimator::getOutputsize (void) {
	   return outSize;
}
//
//
//	The FDecimator is used whenever the ratio in/out is not
//	integer. We split into an integer decimation, followed by
//	a fractional part using the sinc-converter defined elsewhere
	FDecimator::FDecimator (int32_t inRate,
	                        int32_t outRate, int32_t insize) {
	intRatio	= inRate / outRate;
	inCount		= inRate / outRate;
int32_t	tempRate	= inRate / (intRatio);
	fracDecimator_C	=
	         new newConverter ((float)inRate, outRate, (float)inRate / 20, 6);
//	         new newConverter (tempRate, outRate, insize / intRatio, 6);
}

	FDecimator::~FDecimator (void) {
	delete fracDecimator_C;
}

bool	FDecimator::doResample	(DSPCOMPLEX v,
	                         DSPCOMPLEX *out, int32_t *amount) {
int16_t	i;
int32_t am;
DSPCOMPLEX	tempBuf    [fracDecimator_C -> getOutputsize () + 10];

//	if (--inCount > 0)
//	   return false;
//	inCount = intRatio;

	if (!fracDecimator_C -> convert (v, tempBuf, &am)) 
	   return false;
	for (i = 0; i < am; i ++)
	   out [i] = tempBuf [i];
	*amount = am;
	return true;
}

bool	FDecimator::doResample	(DSPFLOAT v,
	                         DSPFLOAT *out, int32_t *amount) {
int16_t	i;
int32_t am;
DSPCOMPLEX	tempBuf    [fracDecimator_C -> getOutputsize () + 10];

//	if (--inCount > 0)
//	   return false;
//	inCount = intRatio;

	if (!fracDecimator_C -> convert (DSPCOMPLEX (v, 0), tempBuf, &am)) 
	   return false;
	for (i = 0; i < am; i ++)
	   out [i] = real (tempBuf [i]);
	*amount = am;
	return true;
}

int32_t	FDecimator::getOutputsize	(void) {
	   return fracDecimator_C -> getOutputsize ();
}

