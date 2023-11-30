#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm receiver
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fmreceover is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#pragma once

#include	"fm-constants.h"

class fftFilter {
public:
			fftFilter	(int32_t, int);
			~fftFilter	();

	void		setBand		(int32_t, int32_t, int32_t);
	void		setSimple	(int32_t, int32_t, int32_t);
	void		setLowPass	(int32_t, int32_t);
	DSPCOMPLEX	Pass		(DSPCOMPLEX);
	DSPFLOAT	Pass		(DSPFLOAT);

protected:
	int32_t		fftSize;
	int		filterDegree;
	int		OverlapSize;
	int		NumofSamples;
	DSPCOMPLEX	*FFT_A;
	DSPCOMPLEX	*FFT_C;
	DSPCOMPLEX	*filterVector;
	DSPFLOAT	* RfilterVector;
	DSPCOMPLEX	*Overloop;
	int32_t		inp;
};

//	hilbert has real input but complex output
class fftFilterHilbert : public fftFilter {
public:
			fftFilterHilbert	(int32_t, int);
			~fftFilterHilbert	();
	DSPCOMPLEX	Pass			(DSPFLOAT);

private:
	void		setHilbert		();
};

