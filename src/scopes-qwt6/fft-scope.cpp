#
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

#include "fft-scope.h"


	fft_scope::fft_scope (QwtPlot	*plotfield,
	                      int16_t	displaySize,
	                      int32_t	scale,
	                      int16_t	rasterSize,
	                      int32_t	SampleRate,
	                      int16_t	freq):Scope (plotfield,
	                                             displaySize,
	                                             rasterSize) {
double  temp;

	if ((displaySize & (displaySize - 1)) != 0)
	   displaySize = 1024;
	this	-> displaySize		= displaySize;
	this	-> segmentSize		= SampleRate / freq;
	this	-> scale		= scale;
	this	-> rasterSize		= rasterSize;
	this	-> averageCount		= 0;
	this	-> spectrumSize		= 8 * displaySize;
	this	-> spectrumFillpoint	= this -> spectrumSize;
	this	-> sampleRate		= SampleRate;
	this	-> freq			= freq;

	this	-> fillPointer		= 0;
	this	-> vfo			= 0;
	this	-> amplification	= 100;
	this	-> needle		= 0;
	this	-> displayBuffer	= new double [displaySize];
	this	-> averageBuffer	= new double [displaySize];

	for (int16_t i = 0; i < displaySize; i++)
	   averageBuffer[i] = 0;
	this	-> X_axis		= new double [displaySize];

	this	-> Window		= new float [spectrumFillpoint];
	this	-> inputBuffer		= new std::complex<float> [spectrumFillpoint];
	this	-> sampleCounter	= 0;
	this	-> spectrum_fft		= new common_fft (this -> spectrumSize);
	this	-> spectrumBuffer	= spectrum_fft	-> getVector ();
	for (int16_t i = 0; i < spectrumFillpoint; i++)
	                     + 0.08 * cos((4.0 * M_PI * i) / (spectrumFillpoint - 1));

	temp	= (double)sampleRate / 2 / displaySize;
	for (int16_t i = 0; i < displaySize; i++)
	   X_axis [i] =
	      ((double)vfo - (double)sampleRate / 2 
                  +  (double)((i) * (double)2 * temp)) / ((double)KHz (1));
}

	fft_scope::~fft_scope () {
	delete[]	this	-> displayBuffer;
	delete[]	this	-> averageBuffer;
	delete[]	this	-> X_axis;
	delete[]	this	-> Window;
	delete		this	-> spectrum_fft;
	delete[]	this	-> inputBuffer;
}

void	fft_scope::setAmplification (int16_t sliderValue) {
	amplification = (double) sliderValue;
}

void	fft_scope::setZero (int64_t vfo) {
	this	-> vfo = vfo;
}

void	fft_scope::setNeedle (int32_t needle) {
	this	-> needle = needle;
}
//
void	fft_scope::addElements (std::complex<float> *v, int16_t n) {

	for (int i = 0; i < n; i++)
	   addElement (v [i]);
}

void	fft_scope::addElement (std::complex<float> x) {

	if (fillPointer < spectrumFillpoint)
	   inputBuffer [fillPointer ++] = x;

	sampleCounter ++;
	if (sampleCounter < segmentSize)
	   return;

	fillPointer	= 0;
	sampleCounter	= 0;
	
	for (int i = 0; i < spectrumFillpoint; i++) {
	   std::complex<float> tmp = inputBuffer [i];
	   spectrumBuffer[i] = cmul (tmp, Window [i]);
	}

	for (int i = spectrumFillpoint; i < spectrumSize; i++)
	   spectrumBuffer [i] = std::complex<float> (0, 0);

	spectrum_fft -> do_FFT ();

	int	ratio	= spectrumSize / displaySize;
	for (int i = 0; i < displaySize / 2; i++) {
	   float sum = 0;
	   for (int j = 0; j < ratio; j++) 
	      sum += abs (spectrumBuffer [i * ratio + j]);
	   displayBuffer [displaySize / 2 + i] = sum / ratio;
	   sum = 0;
	   for (int j = 0; j < ratio; j++) 
	      sum += abs (spectrumBuffer [spectrumSize / 2 + i * ratio + j]);
	   displayBuffer [i] = sum / ratio;
	}

	doAverage ();

	double temp	= (double)sampleRate / 2 / displaySize;
	for (int16_t i = 0; i < displaySize; i++)
	   X_axis [i] =
	      ((double)vfo - (double)sampleRate / 2 
                  +  (double)((i) * (double)2 * temp)) / ((double)KHz (1));
	Scope::Display (X_axis,
	                displayBuffer,
	                this -> amplification,
	                ((int32_t)vfo + needle) / (int32_t)KHz (1));
}
//

void	fft_scope::SelectView (int8_t Mode) {
	Scope::SelectView (Mode);
}

//void	fft_scope::setAverager (bool b) {
//	averageCount = (b ? 1 : 0);
//}

void	fft_scope::clearAverage () {

	if (averageCount > 0) {
	   averageCount = 1;
	   for (int i = 0; i < displaySize; i++)
	      averageBuffer [i] = 0;
	}
}
//

static inline
float	get_ldb (float x) {
	return 20 * log10 ((x + 1) / (float)(512));
}

void	fft_scope::doAverage () {

	if (averageCount > 0) {
	   for (int i = 0; i < displaySize; i++) {
	      if (displayBuffer [i] != displayBuffer [i])
	         displayBuffer [i] = 0;
	      if (get_ldb (abs (displayBuffer [i])) > 50)
	         displayBuffer [i] = 0;
	      averageBuffer [i] =
	                ((double)(averageCount - 1)) / averageCount *
	                                                     averageBuffer [i] +
                        1.0f / averageCount * displayBuffer [i];
	      displayBuffer [i] = averageBuffer [i];
	   }

	   averageCount++;
	}
	else {
	   for (int i = 0; i < displaySize; i++) {
	      if (displayBuffer [i] != displayBuffer [i])
	         displayBuffer [i] = 0;
	      averageBuffer [i] =
	               ((double)(freq / 2 - 1)) / (freq / 2) * averageBuffer[i]
                       + 1.0f / (freq / 2) * displayBuffer [i];
	      displayBuffer[i] = averageBuffer[i];
	   }
	}
}

