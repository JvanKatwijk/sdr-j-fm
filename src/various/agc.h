#pragma once

#include "fm-constants.h"

class AGC {
public:
		AGC (float iSR, float iRefLevel, float iStartGain):
	                                           sampleRate (iSR)
	                                         , refLevel (iRefLevel)
	                                         , curGain (iStartGain) { }
		~AGC () {}

	inline
	DSPCOMPLEX process_sample (DSPCOMPLEX input) {
	   DSPCOMPLEX output	= input * curGain;
	   curGain		+= sampleRate * (refLevel - std::abs (output));
	   return output;
	}

protected:
	float sampleRate;     // adjustment rate
	float refLevel;		// reference value
	float curGain;     // current gain
};

