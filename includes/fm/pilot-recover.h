
#ifndef	__PILOT_RECOVER_H
#define	__PILOT_RECOVER_H

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
#endif

