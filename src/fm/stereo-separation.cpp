#
/*
 *    Copyright (C) 2023
 *    Thomas Neder
 *
 *    This file is part of the SDR-J-FM program.
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
 *
 */
#include	"stereo-separation.h"
#ifndef	M_PI_4
#define M_PI_4 0.78539816339744830962 
#endif
	PerfectStereoSeparation::
	            PerfectStereoSeparation (int32_t	iRate,
	                                     float	iAlpha,
	                                     SinCos	*ipSinCos):
	                                             lpFilter (2048, 295) {
	lockAlpha	= 1.0f / iRate;
	this	-> rate = iRate;
	mySinCos	= ipSinCos;
	this	-> alpha = iAlpha;

	reset	();

	lpFilter. setLowPass (15000, rate);
}

	PerfectStereoSeparation::~PerfectStereoSeparation () {
}


void	PerfectStereoSeparation::reset	() {
//	keep this part fast as it could be called each sample

	accPhaseShift		= 0;
	error_minimized		= false;
	mean_error		= 0;
	sampleLockStableCnt	= 0;
	sampleUnlockStableCnt	= 0;
}
//
//	MuxSignal is the demodulated signal,
//	CurMixPhase derived from the pilot, i.e. the pilot phase
//	(corrected for the delay)
//
float	PerfectStereoSeparation::
                     process_sample (DSPFLOAT iMuxSignal,
	                                  DSPFLOAT iCurMixPhase) {
//	complex oscillator signal * real iMuxSignal can be seen as
//	a separated mix of
//	cos (iCurMixPhase) * iMuxSignal and sin (iCurMixPhase) * iMuxSignal
	std::complex<float> sinCosPath =
	          mySinCos -> getComplex (iCurMixPhase) * iMuxSignal;

//	filter out only the L-R part (real (cos()) and imag (sin())
//	are filtered implicit separately),
//	keeping also the negative side is essential as the input
//	phase (iCurMixPhase) in the loop is changed
//	until left and right side is (almost) (conjugate complex) symmetrical
//	(mixing with cos() would give the maximum level,
//	with sin() the minimum level)
//	this is the principle of a costas loop
	sinCosPath	= lpFilter. Pass (sinCosPath);
//	real is cos() path, imag is sin() path
	DSPFLOAT error	= real (sinCosPath) * imag (sinCosPath);

//	make swing-in faster when error is not minimized yet
	if (!error_minimized)
	   error *= 10.0f;

	accPhaseShift += alpha * error;

	mean_error = lockAlpha * error + mean_error * (1.0f - lockAlpha);
	const bool error_minimized_temp = (abs(mean_error) < 0.001f);

	if (error_minimized_temp) {
	   if (error_minimized || (++sampleLockStableCnt > 3 * rate)) {
	      error_minimized = true;
	   }
	   sampleUnlockStableCnt = 0;
	}
	else  { // not locked -> reset stable counter
	   if (!error_minimized || (++sampleUnlockStableCnt > 3 * rate)) {
	      error_minimized = false;
	   }
	   sampleLockStableCnt = 0;
	}

	// do some limitation (should never happen)
	if (accPhaseShift < -M_PI_4)
	   accPhaseShift = -M_PI_4;
	else if (accPhaseShift > M_PI_4)
	   accPhaseShift = M_PI_4;

	return accPhaseShift;
}


