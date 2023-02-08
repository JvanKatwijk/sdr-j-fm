

#include "squelchClass.h"

constexpr float SQUELCH_HYSTERESIS_NSQ = 0.001;
constexpr float SQUELCH_HYSTERESIS_LSQ = 0.000;
constexpr float LEVELREDUCTIONFACTOR = 0.000; // no output level while squelch active

		squelch::squelch (int32_t squelchThreshold,
	                          int32_t keyFrequency,
	                          int32_t bufsize,
	                          int32_t sampleRate):
	                                 squelchHighpass (20,
	                                          keyFrequency - 100, 
	                                          sampleRate,
	                                          S_CHEBYSHEV),
	                                 squelchLowpass (20, 
	                                          keyFrequency,
	                                          sampleRate,
	                                          S_CHEBYSHEV) {

	setSquelchLevel (squelchThreshold); // convert 0..100 to 1.0..0.0
	this	-> keyFrequency	= keyFrequency;
	this	-> holdPeriod	= bufsize;
	this	-> sampleRate	= sampleRate;

	this	-> squelchSuppress = false;
	this	-> squelchSuppressLast = !squelchSuppress;
	this	-> squelchCount	= 0;
	this	-> average_High = 0;
	this	-> average_Low	= 0;
}

void	squelch::setSquelchLevel (int n) {
// n = 0..100 (from SQ slider)
	levelSquelchThreshold = pow (10.0f, (n - 80) / 30.0f);
	noiseSquelchThreshold = 1.0f - n / 100.0f;
}

static inline
float decayingAverage (float old, float input, float weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}

float squelch::do_noise_squelch (const float soundSample) {
float val_1 = abs (squelchHighpass.Pass (soundSample));
float val_2 = abs (squelchLowpass. Pass (soundSample));

	average_High = decayingAverage (average_High,
	                                val_1, sampleRate / 100);
	average_Low  = decayingAverage (average_Low,
	                                val_2, sampleRate / 100);

	if (++squelchCount >= holdPeriod) {
	   squelchCount = 0;
//	o.k., looking for a new squelch state
	   if (noiseSquelchThreshold < SQUELCH_HYSTERESIS_NSQ)
	      squelchSuppress = true;
	   else 	// recompute
	   if (average_High < average_Low * noiseSquelchThreshold - SQUELCH_HYSTERESIS_NSQ)
	      squelchSuppress = false;
	   else
	   if (average_High >= average_Low * noiseSquelchThreshold + SQUELCH_HYSTERESIS_NSQ)
	      squelchSuppress = true;
//	   else just keep old squelchSuppress value
	}

	if (squelchSuppress != squelchSuppressLast) {
	   squelchSuppressLast = squelchSuppress;
	   emit setSquelchIsActive (squelchSuppress);
	}

	return squelchSuppress ?
	             soundSample * LEVELREDUCTIONFACTOR :
	             soundSample;
}

float squelch::do_level_squelch (float soundSample, float carrierLevel) {

	if (++squelchCount >= holdPeriod) {
	   squelchCount = 0;

//	looking for a new squelch state
	   if (carrierLevel < levelSquelchThreshold - SQUELCH_HYSTERESIS_LSQ) {
	      squelchSuppress = true;
	   }
	   else
	   if (carrierLevel >= levelSquelchThreshold + SQUELCH_HYSTERESIS_LSQ) {
	      squelchSuppress = false;
	   }
//	   else just keep old squelchSuppress value
	}

	if (squelchSuppress != squelchSuppressLast) {
	   squelchSuppressLast = squelchSuppress;
	   emit setSquelchIsActive (squelchSuppress);
	}

	return squelchSuppress ?
	              soundSample * LEVELREDUCTIONFACTOR :
	              soundSample;
}
