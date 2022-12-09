
#include	"pilot-recover.h"

	pilotRecovery::pilotRecovery	(int32_t Rate_in,
	                                 DSPFLOAT omega,
	                                 DSPFLOAT gain,
	                                 SinCos * mySinCos) {
	this	-> Rate_in	= Rate_in;
	this	-> omega	= omega;
	this	-> gain		= gain;
	this	-> mySinCos	= mySinCos;
	this	-> pll_isLocked = false;
	this	-> LockStable	= false;
	this	-> pilot_Lock	= 0;
	this	-> pilot_oldValue = 0;
	pilot_OscillatorPhase	= 0;
	SampleLockStableCnt	= 0;
}

	pilotRecovery::~pilotRecovery () {}

bool	pilotRecovery::isLocked	() const {
	return pll_isLocked;
}

float	pilotRecovery::getLockedStrength () const {
	return pilot_Lock;
}

DSPFLOAT pilotRecovery::getPilotPhase (const DSPFLOAT pilot) {
DSPFLOAT OscillatorValue = mySinCos -> getCos (pilot_OscillatorPhase);
DSPFLOAT PhaseError	= pilot * OscillatorValue;
float alpha = 1.0f / 3000.0f;

	pilot_OscillatorPhase += PhaseError * gain;
	const DSPFLOAT currentPhase = PI_Constrain (pilot_OscillatorPhase);
	pilot_OscillatorPhase = PI_Constrain (pilot_OscillatorPhase + omega);
	quadRef	= (OscillatorValue - pilot_oldValue) / omega;

	pilot_oldValue = OscillatorValue;
	pilot_Lock = alpha * (-quadRef * pilot) + pilot_Lock * (1.0 - alpha);
	bool pll_isLocked_temp = (pilot_Lock > 0.07f);
//	Check if the PLL lock is stable for a while.
//	This is important in very noisy receive condition
//	to maintain a stable mono mode.

	if (pll_isLocked_temp) {
//	for 500ms the PLL lock has to be stable
	   if (pll_isLocked || ++SampleLockStableCnt > (Rate_in >> 1)) {
	      pll_isLocked = true;
	   }
	}
	else  { // not locked -> reset stable counter
	   pll_isLocked = false;
	   SampleLockStableCnt = 0;
	}

	return currentPhase;
}
