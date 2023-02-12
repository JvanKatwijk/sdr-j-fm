/*
 *    Copyright (C)  2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J-FM
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
 */

#ifndef __FFT_SCOPE_H
#define __FFT_SCOPE_H

#include "fft.h"
#include "fm-constants.h"
#include "scope.h"
#include <QObject>
#include <iostream>
#include <stdexcept>
#

class fft_scope : public Scope {
Q_OBJECT
public:
			fft_scope (QwtPlot *, // the canvas
		                   int16_t,   // displayWidth
	                           int32_t,   // scale
	                           int16_t,   // raster size
	                           int32_t,   // samplerate
	                           int16_t);  // repeat frequency
			~fft_scope	();

	void		addElements		(std::complex<float> *,
	                                                       int16_t);
	void		setAmplification	(int16_t);
	void		setZero			(int64_t);
	void		setNeedle		(int32_t);
	void		clearAverage		();
	void		SelectView		(int8_t);
//	void		setAverager		(bool);

private:
	void		addElement		(std::complex<float>);
	void		doAverage		();

	int32_t		sampleRate;
	int16_t		displaySize;
	int32_t		scale;
	int16_t		rasterSize;
	int32_t		spectrumFillpoint;
	int32_t		fillPointer;
	float		*Window;
	std::complex<float>	*inputBuffer;
	std::complex<float>	*spectrumBuffer;
	common_fft	*spectrum_fft;
	int32_t		sampleCounter;
	int32_t		SampleRate;
	int32_t		segmentSize;

	int16_t		freq;
	int32_t		needle;
	int16_t		amplification;
	int64_t		vfo;

	int32_t		spectrumSize;
	double		*X_axis;
	double		*displayBuffer;
	double		*averageBuffer;
	uint32_t	averageCount;
};

#endif
