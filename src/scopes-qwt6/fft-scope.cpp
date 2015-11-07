#
/*
 *    Copyright (C)  2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J (JSDR).
 *    Many of the ideas as implemented in JSDR are derived from
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
#include	"fft-scope.h"

static inline
int32_t	nearestTwoPower (int16_t n) {
int32_t	res	= 1;

	if (n < 100)
	   return 128;
	while (n != 0) {
	   n >>= 1;
	   res <<= 1;
	}

	return res;
}

//
//	In this version of the scope, we introduce two additional parameters
//	the spectrumSize and the repetition frequency.
//	If spectrumSize == -1, we create a scope
//	where all samples are made part of the spectrum: pretty heavy
//	otherwise, we take freq times per second spectrumSize / 2 samples
	fft_scope::fft_scope (QwtPlot	*plotfield,
	                      int16_t	displaySize,
	                      int32_t	scale,
	                      int16_t	rasterSize,
	                      int32_t	spectrumSize,
	                      int32_t	SampleRate,
	                      int16_t	freq):Scope (plotfield,
	                                             displaySize,
	                                             rasterSize) {
int16_t	i;
double	temp;

	if ((displaySize & (displaySize - 1 )) != 0)
	   displaySize = 512;
	this	-> displaySize		= displaySize;
	this	-> segmentSize		= SampleRate / freq;
	this	-> scale		= scale;
	this	-> rasterSize		= rasterSize;
	this	-> averageCount		= 0;
	this	-> zoomingLevel		= 0;

	if (spectrumSize < 0) {		// we compute
	   spectrumFillpoint	= segmentSize;
	   spectrumSize		= nearestTwoPower (2 * segmentSize);
	}
	else {
	   if ((spectrumSize & (spectrumSize - 1)) != 0)
	      spectrumSize = 1024;
	   if (displaySize > spectrumSize)
	      spectrumSize = displaySize;

	   if (segmentSize < spectrumSize / 2)
	      spectrumFillpoint = segmentSize;
	   else
	      spectrumFillpoint	= spectrumSize / 2;
	}
	this	-> spectrumSize		= spectrumSize;
	this	-> sampleRate		= SampleRate;
	this	-> MaxFrequency		= SampleRate / 2;
	this	-> freq			= freq;

	this	-> fillPointer		= 0;
	this	-> vfo			= 0;
	this	-> zoomingPoint		= 0;
	this	-> zoomingLevel		= 1;
	this	-> amplification	= 100;
	this	-> needle		= 0;
	this	-> dummyBuffer		= new double [displaySize];
	this	-> dummyCount		= 0;
	this	-> displayBuffer	= new double [displaySize];
	this	-> averageBuffer	= new double [displaySize];
	for (i = 0; i < displaySize; i ++)
	   averageBuffer [i] = 0;
	this	-> X_axis		= new double [displaySize];

	this	-> Window		= new DSPFLOAT [spectrumFillpoint];
	this	-> inputBuffer		= new DSPCOMPLEX [spectrumFillpoint];
	this	-> sampleCounter	= 0;
	this	-> spectrum_fft		= new common_fft (spectrumSize);
	this	-> spectrumBuffer	= spectrum_fft	-> getVector ();
	this	-> binWidth		= sampleRate / spectrumSize;

	for (i = 0; i < spectrumFillpoint; i ++) 
	   Window [i] = 0.42 - 0.5 * cos ((2.0 * M_PI * i) / (spectrumFillpoint - 1)) +
	                      0.08 * cos ((4.0 * M_PI * i) / (spectrumFillpoint - 1));


	temp	= (double)MaxFrequency / displaySize;
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] =
	      ((double)vfo - (double)MaxFrequency
	           +  (double)((i) * (double) 2 * temp)) / ((double)scale);
}
	fft_scope::~fft_scope (void) {
	delete[]	this	-> displayBuffer;
	delete[]	this	-> averageBuffer;
	delete[]	this	-> X_axis;
	delete[]	this	-> dummyBuffer;
	delete[]	this	-> Window;
	delete		this	-> spectrum_fft;
	delete[]	this	-> inputBuffer;
}

//
//	setting the samplerate influences the X-axis
void	fft_scope::setSamplerate	(int32_t s) {
float	temp;
int16_t	i;
	sampleRate	= s;
	MaxFrequency	= s / 2;
	temp	= (double)MaxFrequency / displaySize;
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] =
	      ((double)vfo - (double)MaxFrequency
	           +  (double)((i) * (double) 2 * temp)) / ((double)scale);
}

void	fft_scope::setAmplification (int16_t sliderValue) {
	amplification = (double) sliderValue;
}

void	fft_scope::setZero (int64_t vfo) {
	this	-> vfo = vfo;
	this	-> zoomingPoint	= vfo;
	this	-> zoomingLevel	= 1;
}

void	fft_scope::setZoompoint (int32_t freq) {
	this	-> zoomingPoint = freq;
	if ((zoomingLevel + 1) * displaySize <= spectrumSize)
	   zoomingLevel ++;
}

void	fft_scope::resetZoompoint (void) {
	this	-> zoomingLevel --;
	if (zoomingLevel < 1)
	   zoomingLevel = 1;
}

void	fft_scope::setNeedle (int32_t needle) {
	this -> needle = needle;
}
//
//	This is a kind of hack to allow the spectrumviewer to
//	send in vectors of size n and have them displayed
//	immediately
void	fft_scope::addElementsandShow (DSPCOMPLEX *v, int16_t n) {
int16_t i, j;
int32_t	Incr;
double	temp;

	if (2 * n > spectrumSize) {
	   fprintf (stderr, "Increase n = %d, size = %d\n", n, spectrumSize);
	   return;
	}

	for (i = 0; i < n; i ++)
	   spectrumBuffer [i] = v [i];

	for (i = n; i < spectrumSize; i ++)
	   spectrumBuffer [i] = 0;
	spectrum_fft	-> do_FFT ();

	Incr = spectrumSize / displaySize;
	for (i = 0; i < displaySize / 2; i ++) {
	   displayBuffer [displaySize / 2 + i] = 0;
	   for (j = 0; j < Incr; j ++) 
	      displayBuffer [displaySize / 2 + i] +=
	                    abs (spectrumBuffer [i * Incr + j]) / binWidth;
	   displayBuffer [displaySize / 2 + i] /= Incr;
	}

	for (i = 0; i < displaySize / 2; i ++) {
	   displayBuffer [i] = 0;
	   for (j = 0; j < Incr; j ++) 
	      displayBuffer [i] +=
	                    abs (spectrumBuffer [spectrumSize / 2 + i * Incr + j]) / binWidth;
	   displayBuffer [i] /= Incr;
	}

	temp	= (double)MaxFrequency / displaySize;
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] =
	      ((double)vfo - (double)MaxFrequency
	           +  (double)((i) * (double) 2 * temp)) / ((double)scale);

	doAverage ();

	showSpectrum ();
}

void	fft_scope::addElements (DSPCOMPLEX *v, int16_t n) {
int16_t	i;

	for (i = 0; i < n; i ++)
	   addElement (v [i]);
}

void	fft_scope::addElement (DSPCOMPLEX x) {
int32_t	i;
DSPFLOAT	multiplier	= (DSPFLOAT)segmentSize / displaySize;
//	
	inputBuffer [fillPointer] = x;
	fillPointer	= (fillPointer + 1) % spectrumFillpoint;

	if (++ sampleCounter < segmentSize)
	   return;

	sampleCounter	= 0;
	for (i = 0; i < spectrumFillpoint; i ++) {
	   DSPCOMPLEX tmp = inputBuffer [(fillPointer + i) % spectrumFillpoint];
	   spectrumBuffer [i] = cmul (tmp, multiplier * Window [i]); 
	}
	for (i = spectrumFillpoint; i < spectrumSize; i ++)
	   spectrumBuffer [i] = 0;

	spectrum_fft	-> do_FFT ();

	mapSpectrumtoDisplay (zoomingLevel, zoomingPoint);
	doAverage ();

	showSpectrum ();
}
//
//	fill a vector and show the elements when full
void	fft_scope::addElement (DSPCOMPLEX v, int16_t t) {
static int cnt = 0;
	if (++cnt < t) 
	   return;

	cnt	= 0;
	dummyBuffer [dummyCount] = real (v);
	if (++ dummyCount >= displaySize) {
	   Scope::Display (X_axis,
	                   dummyBuffer,
	                   amplification,
	                   ((int32_t)vfo + needle) / (int32_t)scale);
	   dummyCount = 0;
	}
}

void	fft_scope::addValue (double v, int i) {
	displayBuffer [i] = v;
}

void	fft_scope::showSpectrum (void) {
	Scope::Display (X_axis,
	                displayBuffer,
	                amplification,
	                ((int32_t)vfo + needle) / (int32_t)scale);
}

void	fft_scope::SelectView (int8_t Mode) {
	Scope::SelectView (Mode);
	this	-> zoomingLevel	= 1;
	this	-> zoomingPoint	= vfo;
}

void	fft_scope::setAverager (bool b) {
	averageCount = b ? 1 : 0;
}
	
void	fft_scope::clearAverage (void) {
int16_t	i;

	if (averageCount > 0) {
	   averageCount = 1;
	   for (i = 0; i < displaySize; i ++)
	      averageBuffer [i] = 0;
	}
}
//
//	We map the spectrum onto the display vector. We
//	take into account zooming.
//
#define	realIndex(a)	(a < spectrumSize / 2 ? a + spectrumSize / 2 : a - spectrumSize / 2)
	
void	fft_scope::mapSpectrumtoDisplay (int16_t level, int32_t freq) {
int16_t	i;
double	temp;
int16_t	index_1;	//
int16_t	top_index, bottom_index;
int32_t	min_freq, max_freq;
//
	index_1		= indexOf (freq);
//
//	then compute the top and bottom of the segment in the
//	spectrumBuffer that is to be projected
	top_index	= index_1 + (DSPFLOAT)(spectrumSize - index_1) / level;
	bottom_index	= index_1 - (DSPFLOAT)index_1 / level;
	min_freq	= frequencyFor (bottom_index);
	max_freq	= frequencyFor (top_index);
	temp		= ((DSPFLOAT)max_freq - min_freq) / displaySize;
//
//	Note that top_index and bottom_index are abstract indices,
//	we defined realIndex for the actual mapping
//	Now we have to map the segment bottom-index .. top-index
//	to 0 .. displaySize
//	for the time being, we assume that
//	top_index - bottom_index + 1 >= displaySize
	DSPFLOAT ratio	= ((DSPFLOAT)top_index - bottom_index + 1) / displaySize;
//	we extract for the i-th element of the displayvector
//	those parts of the spectrumvector that would contribute
//	to the value of the displayvector. Assumption is that
//	an accuracy of 10% is sufficient
	for (i = 0; i < displaySize; i ++) {
	   DSPFLOAT sum	= 0;
	   int16_t j;
	   for (j = 0; j < (int16_t)(5 * ratio); j ++) {
	      int32_t a_index =
	             (int32_t)floor ((5 * (bottom_index + i * ratio ) + j) / 5);
	      sum +=  abs (spectrumBuffer [realIndex (a_index)]) / 5;
	   }
	   displayBuffer [i] = sum / temp;
	}
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] =
	      (min_freq + (double)(i * temp)) / ((double)scale);
}

int32_t	fft_scope::indexOf (int64_t freq) {
int64_t	lowFreq		= vfo - MaxFrequency;
int64_t	highFreq	= vfo + MaxFrequency;
double	ratio		= 0;

	if (freq < lowFreq)
	   freq	= lowFreq;
	if (freq > highFreq)
	   freq = highFreq;

	ratio = ((double)(freq - lowFreq)) / (highFreq - lowFreq);
	return ratio * spectrumSize;
}

int64_t	fft_scope::frequencyFor (int64_t index) {
int64_t	lowFreq		= vfo - MaxFrequency;

	if (index < 0)
	   return frequencyFor (0);
	if (index >= spectrumSize)
	   return frequencyFor (spectrumSize - 1);

	return lowFreq + index * (2 * MaxFrequency) / spectrumSize;
}
static inline
float	get_ldb (float x) {
	return 20 * log10 ((x + 1) / (float)(512));
}

void	fft_scope::doAverage (void) {
int32_t	i;

	if (averageCount > 0) {
	   for (i = 0; i < displaySize; i ++) {
	      if (displayBuffer [i] != displayBuffer [i])
	         displayBuffer [i] = 0;
	      if (get_ldb (abs (displayBuffer [i])) > 50)
	         displayBuffer [i] = 0;
	      averageBuffer [i] =
	                   ((double)(averageCount - 1))/averageCount *
	                                                    averageBuffer [i] +
	                   1.0f / averageCount * displayBuffer [i];
	      displayBuffer [i] = averageBuffer [i];
	   }

	   averageCount ++;
	}
	else {
	   for (i = 0; i < displaySize; i ++) {
	      if (displayBuffer [i] != displayBuffer [i])
	         displayBuffer [i] = 0;
	      averageBuffer [i] = 
	               ((double)(freq / 2 - 1)) / (freq / 2) * averageBuffer [i] +
	                1.0f / (freq / 2) * displayBuffer [i];
	      displayBuffer [i] = averageBuffer [i];
	   }
	}
}

