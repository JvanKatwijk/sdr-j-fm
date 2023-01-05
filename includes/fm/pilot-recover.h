
#ifndef	__PILOT_RECOVER_H
#define	__PILOT_RECOVER_H

#include	"fft-filters.h"
#include	"fm-constants.h"
#include	"sincos.h"

class	pilotRecovery {
public:
		pilotRecovery	(int32_t Rate_in, DSPFLOAT omega,
	                         DSPFLOAT gain, SinCos * mySinCos);
		~pilotRecovery	();
	bool	isLocked	() const;
	float	getLockedStrength() const;
	DSPFLOAT getPilotPhase	(const DSPFLOAT pilot);
private:
	int32_t	Rate_in;
	int32_t SampleLockStableCnt;
	DSPFLOAT pilot_OscillatorPhase;
	DSPFLOAT pilot_oldValue;
	DSPFLOAT omega;
	DSPFLOAT gain;
	SinCos	*mySinCos;
	DSPFLOAT pilot_Lock;
	DSPFLOAT quadRef;
	bool	pll_isLocked;
	bool	LockStable;
};


class PerfectStereoSeparation {
public:
	PerfectStereoSeparation() = delete;
	PerfectStereoSeparation(int32_t iRate, float iAlpha, SinCos * ipSinCos);
	~PerfectStereoSeparation();

	bool	is_locked	() const;
	void	reset	();
	DSPCOMPLEX get_cur_mixer_result() const { return curMixResult; }
	DSPFLOAT process_sample(DSPFLOAT iMuxSignal, DSPFLOAT iCurMixPhase);

private:
	fftFilter	*stereoLPImageFilterSin;
	fftFilter	*stereoLPImageFilterCos;

	int32_t	Rate_in;
	int32_t SampleLockStableCnt;
	DSPFLOAT accPhaseShift;
	DSPFLOAT pilot_OscillatorPhase;
	DSPFLOAT pilot_oldValue;
	DSPFLOAT omega;
	DSPFLOAT gain;
	SinCos	*mySinCos;
	DSPFLOAT pilot_Lock;
	DSPFLOAT quadRef;
	bool	pll_isLocked;
	bool	LockStable;
	float alpha;
	DSPCOMPLEX curMixResult;
};

#endif

