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
#ifndef	__STEREO_SEPARATION_H
#define	__STEREO_SEPARATION_H

#include	"fft-filters.h"
#include	"fm-constants.h"
#include	"sincos.h"

class PerfectStereoSeparation {
public:
	PerfectStereoSeparation () = delete;
	PerfectStereoSeparation (int32_t iRate, float iAlpha,
	                                           SinCos * ipSinCos);
	~PerfectStereoSeparation	();

	void		reset		();
	bool		is_error_minimized () const { return error_minimized; };
	float		get_mean_error	() const { return mean_error; }

	DSPCOMPLEX	get_cur_mixer_result () const { return curMixResult; }
	DSPFLOAT	process_sample	(DSPFLOAT iMuxSignal,
	                                             DSPFLOAT iCurMixPhase);

private:
	fftFilter	lpFilter;
	int32_t		rate;
	int32_t		sampleLockStableCnt;
	int32_t		sampleUnlockStableCnt;
	DSPFLOAT	accPhaseShift;
	SinCos		*mySinCos;
	DSPFLOAT	mean_error;
	bool		error_minimized;
	float 		alpha;
	float 		lockAlpha;
	DSPCOMPLEX	curMixResult;
};

#endif

