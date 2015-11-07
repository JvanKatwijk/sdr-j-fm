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
 */

#ifndef	__RESAMPLER
#define	__RESAMPLER

#include	"fm-constants.h"
#include	"newconverter.h"
//
//	simple resampler.
//	in case the decimation constant is an integer, we
//	use the simple "delete" mechanism. In case the
//	decimation constant is a fractional, we split up
//	into an integer trabsform, followed by a fractional
//	transform.

//
//	Decent upconversions still to be done


class	virtualDecimator {
public:
			virtualDecimator	(void);
	virtual		~virtualDecimator	(void);
	virtual	bool	doResample		(DSPCOMPLEX,
	                                         DSPCOMPLEX *, int32_t *);
	virtual	bool	doResample		(DSPFLOAT,
	                                         DSPFLOAT *, int32_t *);
	virtual	int32_t	getOutputsize		(void);
};

class	reSampler	{
public:
			reSampler		(int32_t, int32_t, int32_t);
			~reSampler		(void);
	bool		doResample		(DSPCOMPLEX,
	                                         DSPCOMPLEX *, int32_t *);
	bool		doResample		(DSPFLOAT,
	                                         DSPFLOAT *, int32_t *);
	int32_t		getOutputsize		(void);
private:
	virtualDecimator *theWorker;
};

class	IDecimator : public virtualDecimator {
public:
			IDecimator		(int32_t, int32_t, int32_t);
			~IDecimator		(void);
	bool		doResample		(DSPCOMPLEX,
	                                         DSPCOMPLEX *, int32_t *);
	bool		doResample		(DSPFLOAT,
	                                         DSPFLOAT *, int32_t *);
	int32_t		getOutputsize		(void);

private:
	int32_t		intRatio;
	int32_t		inCount;
	int32_t		bufCounter;
	int32_t		outSize;
	DSPCOMPLEX	*tempBuffer;
};


class	FDecimator : public virtualDecimator {
public:
			FDecimator		(int32_t, int32_t, int32_t);
			~FDecimator		(void);
	bool		doResample		(DSPCOMPLEX,
	                                         DSPCOMPLEX *, int32_t *);
	bool		doResample		(DSPFLOAT,
	                                         DSPFLOAT *, int32_t *);
	int32_t		getOutputsize		(void);

private:
	int32_t		intRatio;
	int32_t		inCount;
	newConverter	*fracDecimator_C;
};
#endif

