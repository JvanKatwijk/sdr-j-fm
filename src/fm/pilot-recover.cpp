
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
	this	-> pilot_OscillatorPhase	= 0;
	this	-> SampleLockStableCnt	= 0;
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
constexpr float alpha = 1.0f / 3000.0f;

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




PerfectStereoSeparation::PerfectStereoSeparation(int32_t iRate, float iAlpha, SinCos * ipSinCos)
{
	lockAlpha = 1.0f / iRate;

	rate = iRate;
	mySinCos	= ipSinCos;
	alpha = iAlpha;

	reset	();

	lpFilter = new fftFilter (1024, 295);  // TODO: check sizes
	//lpFilterCos = new fftFilter (1024, 295);
	lpFilter -> setLowPass (15000, rate);
	//lpFilterCos -> setLowPass (15000, rate);
}

PerfectStereoSeparation::~PerfectStereoSeparation()
{
	delete lpFilter;
}


void PerfectStereoSeparation::reset	()
{
	// keep this part fast as it could be called each sample
	curMixResult = { 0, 0 };
	accPhaseShift = 0;
	error_minimized = false;
	mean_error = 0;
	sampleLockStableCnt = 0;
	sampleUnlockStableCnt = 0;
}

float	PerfectStereoSeparation::process_sample(DSPFLOAT iMuxSignal, DSPFLOAT iCurMixPhase)
{
	const DSPCOMPLEX sinCosPath = mySinCos -> getComplex	(iCurMixPhase) * iMuxSignal;
	curMixResult = lpFilter -> Pass(sinCosPath);
	DSPFLOAT error = real(curMixResult) * imag(curMixResult);
	//DSPFLOAT error = (real(curMixResult) > 0 ? +1 : -1) * (imag(curMixResult) > 0 ? +1 : -1) / 100.0f;

	// make swing-in faster when error is not minimized yet
	if (!error_minimized)
		error *= 10.0f;

	accPhaseShift += alpha * error;

	mean_error = lockAlpha * error + mean_error * (1.0f - lockAlpha);
	const bool error_minimized_temp = (abs(mean_error) < 0.001f);

	if (error_minimized_temp) {
		if (error_minimized || ++sampleLockStableCnt > 3 * rate) {
			error_minimized = true;
		}
		sampleUnlockStableCnt = 0;
	}
	else  { // not locked -> reset stable counter
		if (!error_minimized || ++sampleUnlockStableCnt > 3 * rate) {
			error_minimized = false;
		}
		sampleLockStableCnt = 0;
	}

	// do some limitation
	if (accPhaseShift < -M_PI_4)
		accPhaseShift = -M_PI_4;
	else if (accPhaseShift > M_PI_4)
		accPhaseShift = M_PI_4;

	return accPhaseShift;
}


