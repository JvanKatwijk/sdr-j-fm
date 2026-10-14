#
/*
 *    Copyright (C) 2020, 2026
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
#include	"fft-handler.h"

class fftFilter {
public:
			fftFilter	(int32_t, int);
			~fftFilter	();

	void		setBand		(int32_t, int32_t, int32_t);
	void		setSimple	(int32_t, int32_t, int32_t);
	void		setLowPass	(int32_t, int32_t);
	float		Pass		(float);
	Complex		Pass		(Complex);

protected:
	fftHandler	fftForward;
	fftHandler	fftBackwards;
	int32_t		fftSize;
	int		filterDegree;
	int		OverlapSize;
	int		NumofSamples;
	Complex		*FFT_A;
	Complex		*FFT_C;
	Complex		*filterVector;
	float		* RfilterVector;
	Complex		*Overloop;
	int32_t		inp;
};

//	hilbert has real input but complex output
class fftFilterHilbert : public fftFilter {
public:
			fftFilterHilbert	(int32_t, int);
			~fftFilterHilbert	();
	Complex		Pass			(float);

private:
	void		setHilbert		();
};

