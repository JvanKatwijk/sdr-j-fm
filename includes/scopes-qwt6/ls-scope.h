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

#ifndef __LS_SCOPE_H
#define __LS_SCOPE_H

#include "fm-constants.h"
#include "scope.h"
#include <QObject>
#include <iostream>
#include <stdexcept>
#

class ls_scope : public Scope {
Q_OBJECT
public:
			ls_scope (QwtPlot *, // the canvas
	                          int,		// averageCounter
		                  int,		// displayWidth
	                          int,		// spectrumSize
	                          int);		// samplerate
			~ls_scope	();

	void		processLFSpectrum	(std::vector<std::complex<float>> &,
	                                         int,
	                                         bool,
	                                         bool,
	                                         int);

#ifdef USE_EXTRACT_LEVELS
	void		extractLevels (double * const in,
                                       int32_t rangea, int zoomfactor);
	void		extractLevelsHalfSpectrum (double * in,
                                             int32_t range, int zoomfactor);
	float		get_pilotStrength ();
	float		get_rdsStrength ();
	float		get_noiseStrength ();
#endif
	
private:
	void		add_to_average	(double *, bool, double *);
	void		mapSpectrum (std::complex<float> *in,
                                     bool     showFull,
                                     double *const out,
                                     int32_t ioZoomFactor);

	int		averageCount;
	int		displaySize;
	int		spectrumSize;
	int		sampleRate;

	double          *displayBuffer;
        double          *averageBuffer;
	double		*X_axis;
	float		*Window;
	std::complex<float>	*ftBuffer;

#ifdef	USE_EXTRACT_LEVELS
	float		noiseLevel;
	float		pilotLevel;
	float		rdsLevel;
#endif

};

#endif
