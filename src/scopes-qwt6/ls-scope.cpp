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

#include	"ls-scope.h"
#include	"fft-complex.h"

	ls_scope::ls_scope (QwtPlot	*plotfield,
	                    int		averageCount,
	                    int		displaySize,
	                    int		spectrumSize,
	                    int32_t	SampleRate):Scope (plotfield,
	                                             displaySize,
	                                             50) {
double  temp;

	this	-> averageCount		= averageCount;
	this	-> displaySize		= displaySize;
	this	-> spectrumSize		= spectrumSize;
	this	-> sampleRate		= SampleRate;

	this	-> displayBuffer	= new double [displaySize];
	this	-> averageBuffer	= new double [displaySize];

	for (int i = 0; i < displaySize; i++)
	   averageBuffer[i] = 0;
	this	-> X_axis		= new double [displaySize];

	this	-> Window		= new float [spectrumSize];
	this	-> ftBuffer		= new std::complex<float> [spectrumSize];
	for (int16_t i = 0; i < spectrumSize; i++)
	   Window [i] = 0.43 - 0.5 * cos ((2.0 * M_PI * i) / spectrumSize)
	                     + 0.08 * cos((4.0 * M_PI * i) / (spectrumSize - 1));

	temp	= (double)sampleRate / 2 / displaySize;
	for (int16_t i = 0; i < displaySize; i++)
	   X_axis [i] =
	      (- (double)sampleRate / 2 
                  +  (double)((i) * (double)2 * temp)) / ((double)KHz (1));

#ifdef USE_EXTRACT_LEVELS
	this	-> noiseLevel 		= 0;
	this	-> pilotLevel 		= 0;
	this	-> rdsLevel		= 0;
#endif
}

	ls_scope::~ls_scope () {
	delete[]	this	-> displayBuffer;
	delete[]	this	-> averageBuffer;
	delete[]	this	-> X_axis;
	delete[]	this	-> Window;
	delete[]	this	-> ftBuffer;
}

//
void	ls_scope::processLFSpectrum (std::vector<std::complex<float>> &v,
	                             int	zoomFactor,
	                             bool	showFull,
	                             bool	refresh, 	
	                             int	amplification) {
double Y_values [displaySize];
	for (int i = 0; i < spectrumSize; i++) {
	   std::complex<float> tmp = v [i];
	   if (isinf (abs (tmp)) || std::isnan (abs (tmp)))
	      ftBuffer [i] = std::complex<float> (0, 0);
	   else
	      ftBuffer [i] = cmul (tmp, Window [i]);
	}

	Fft_transform (ftBuffer, spectrumSize, false);

	mapSpectrum (ftBuffer, showFull, Y_values, zoomFactor);
	add_to_average (Y_values, refresh, displayBuffer);

	if (refresh)
	   return;
#ifdef USE_EXTRACT_LEVELS
	if (showFull) {
	   extractLevels (displayBuffer, sampleRate, zoomFactor);
	}
	else {
	   extractLevelsHalfSpectrum (displayBuffer, sampleRate, zoomfactor);
	}
#endif

	double	temp	= (double)sampleRate / 2 / displaySize;
//	first X axis labels
	if (showFull) {
	   for (int i = 0; i < displaySize; i++) {
	      X_axis [i] =
	            (-(sampleRate / 2.0) + (2 * i * temp)) /
	                           ((double)Khz(1)) / zoomFactor; // two side spectrum
	   }
	}
	else {
	   for (int i = 0; i < displaySize; i++) {
	      X_axis [i] = (i * temp) / ((double)Khz (1)) / zoomFactor; // one-side spectrum
	   }
	}

//	get the buffer data

	Scope::Display (X_axis,
	                displayBuffer,
	                amplification,
	                0);
}
//

void	ls_scope::mapSpectrum (std::complex<float> *in,
	                       bool	showFull,
	                       double	*out,
	                       int & ioZoomFactor) {
int16_t factor = spectrumSize / displaySize;  // typ factor = 4 (whole divider)

	if (!showFull) 
	   factor /= 2;

	if (factor / ioZoomFactor >= 1) {
	   factor /= ioZoomFactor;
	}
	else {
	   ioZoomFactor = factor;
	   factor = 1;
	}

	if (showFull) {	// full
	   for (int32_t i = 0; i < displaySize / 2; i++) {
	      double f = 0;
//	read 0Hz to rate/2 -> map to mid to end of display
	      for (int32_t j = 0; j < factor; j++) {
	         f += abs (in [i * factor + j]);
	      }
	      out [displaySize / 2 + i] = f / factor;

	      f = 0;
//	read rate / 2 down to 0Hz -> map to begin to mid of display
	      for (int32_t j = 0; j < factor; j++) {
	         f += abs (in [spectrumSize - 1 - (i * factor + j)]);
	      }
	      out [displaySize / 2 - 1 - i] = f / factor;
	   }
	}
	else {		// half
	   for (int32_t i = 0; i < displaySize; i++) {
	      double f = 0;
//	read 0Hz to rate / 2 -> map to mid to end of display
	      for (int32_t j = 0; j < factor; j++) {
	         f += abs (in [i * factor + j]);
	      }
	      out [i] = f / factor;
	   }
	}
}

void	ls_scope::add_to_average (double *in,
	                          bool refresh, double *buffer) {
double alpha	= 1.0 / averageCount;
double beta	= (averageCount - 1.0) / averageCount;

	if (refresh) {
	   for (int32_t i = 0; i < displaySize; i++) 
	      averageBuffer [i] = in [i];
	   return;
	}

	for (int32_t i = 0; i < displaySize; i++) 
	   averageBuffer [i] = alpha * in [i] + beta * averageBuffer [i];

	for (int i = 0; i < displaySize; i ++)
	   displayBuffer [i] = averageBuffer [i];
}

#ifdef USE_EXTRACT_LEVELS
void	ls_scope::extractLevels (double * const in,
	                            int32_t rangea, int zoomfactor) {
int	binWidth	= spectrumSize / zoomfactor/ sampleRate;
int32_t pilotOffset = displaySize / 2 - 19000 / binWidth;
int32_t rdsOffset   = displaySize / 2 - 57000 / binWidth;
int32_t noiseOffset = displaySize / 2 - 70000 / binWidth;
float noiseAvg = 0, pilotAvg = 0, rdsAvg = 0;

	for (int32_t i = 0; i < 7; i++) {
	   noiseAvg += in [noiseOffset - 3 + i];
	   rdsAvg += in [rdsOffset - 3 + i];
	}

	for (int32_t i = 0; i < 3; i++) {
	   pilotAvg += in [pilotOffset - 1 + i];
	}

	noiseLevel = 0.95 * noiseLevel + 0.05 * noiseAvg / 7;
	pilotLevel = 0.95 * pilotLevel + 0.05 * pilotAvg / 3;
	rdsLevel   = 0.95 * rdsLevel   + 0.05 * rdsAvg / 7;
}

void	ls_scope::extractLevelsHalfSpectrum (double * in,
	                                     int32_t range, int zoomfactor) {
const float binWidth	= (float)range / zoomFactor / displaySize / 2;
const int32_t pilotOffset = 19000 / binWidth;
const int32_t rdsOffset   = 57000 / binWidth;
const int32_t noiseOffset = 70000 / binWidth;

constexpr int32_t avgNoiseRdsSize = 1 + 2 * 6; // mid plus two times sidebands
constexpr int32_t avgPilotSize    = 1 + 2 * 2;

float	noiseAvg = 0;
float	pilotAvg = 0;
float	rdsAvg	= 0;

	for (int32_t i = 0; i < avgNoiseRdsSize; i++) {
	   noiseAvg += in [noiseOffset - 3 + i];
	   rdsAvg += in [rdsOffset - 3 + i];
	}

	for (int32_t i = 0; i < avgPilotSize; i++) {
	   pilotAvg += in [pilotOffset - 1 + i];
	}

	constexpr float ALPHA = 0.2f;
	noiseLevel	= (1.0f - ALPHA) * noiseLevel +
	                          ALPHA * noiseAvg / avgNoiseRdsSize;
	pilotLevel	= (1.0f - ALPHA) * pilotLevel +
	                          ALPHA * pilotAvg / avgPilotSize;
	rdsLevel	= (1.0f - ALPHA) * rdsLevel +
	                          ALPHA * rdsAvg / avgNoiseRdsSize;
}


float ls_scope::get_pilotStrength () {
	if (running. load ()) 
	   return get_db (pilotLevel, 128) - get_db (0, 128);
	return 0.0;
}

float ls_scope::get_rdsStrength () {
	if (running. load ()) {
	   return get_db (rdsLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}

float ls_scope::get_noiseStrength () {
	if (running. load ()) {
	   return get_db (noiseLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}
#endif
