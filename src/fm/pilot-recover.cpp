
#include	"pilot-recover.h"
#ifndef	M_PI_4
#define M_PI_4 0.78539816339744830962 
#endif
	pilotRecovery::pilotRecovery	(int32_t Rate_in,
	                                 DSPFLOAT omega,
	                                 DSPFLOAT gain,
	                                 SinCos * mySinCos) {
	this	-> Rate_in		= Rate_in;
	this	-> omega		= omega;
	this	-> gain			= gain;
	this	-> mySinCos		= mySinCos;
	this	-> pll_isLocked		= false;
	this	-> LockStable		= false;
	this	-> pilot_Lock		= 0;
	this	-> pilot_oldValue 	= 0;
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
DSPFLOAT OscillatorValue	= mySinCos -> getSin (pilot_OscillatorPhase);
DSPFLOAT PhaseError		= pilot * OscillatorValue;
constexpr float alpha		= 1.0f / 3000.0f;

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
	return currentPhase + M_PI_4;
}


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
//	lp1Filter. setLowPass (15000, rate);
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


