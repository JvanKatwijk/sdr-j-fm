#pragma once

#include "fm-constants.h"

class Costas {
public:
		Costas (float iSR, float iAlpha,
	                    float iBeta, float iFreqLimitHz):
	                            sampleRate (iSR)
	                          , alpha (iAlpha)
	                          , beta (iBeta)
	                          , freqLimit (2 * M_PI * iFreqLimitHz / iSR) {

	freq = 0;
	phase = 0;
}

		~Costas() {}

inline
	DSPCOMPLEX process_sample (const DSPCOMPLEX z) {
	   const DSPCOMPLEX r	= z * std::exp (DSPCOMPLEX (0, -phase));
	   const float error	= real (r) * imag (r);

	   freq += (beta * error);
	   if (abs (freq) > freqLimit) {
	      freq = 0;
	   }

	   phase	+= freq + (alpha * error);
	   phase	= PI_Constrain (phase);
	   return r;
	}

private:
	const float sampleRate;
	const float alpha;
	const float beta;
	const float freqLimit;
	float freq;
	float phase;
};

