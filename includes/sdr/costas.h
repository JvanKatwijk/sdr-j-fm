#pragma once

#include "fm-constants.h"

class Costas {
public:
		Costas (float iSR, float iAlpha, float iBeta, float iFreqLimitHz):
	                                                    mSmplRate (iSR)
	                                                  , mAlpha (iAlpha)
	                                                  , mBeta (iBeta)
	                                                  , mFreqLimit (2 * M_PI * iFreqLimitHz / iSR) {

	mFreq = 0;
	mPhase = 0;
}

		Costas () = delete;
		~Costas() = default;

inline
	DSPCOMPLEX	process_sample (const DSPCOMPLEX z) {
	   const DSPCOMPLEX r	= z * std::exp (DSPCOMPLEX (0, -mPhase));
	   const float error	= real(r) * imag(r);

	   mFreq += (mBeta * error);
	   if (abs (mFreq) > mFreqLimit) {
	      mFreq = 0;
	   }

	   mPhase	+= mFreq + (mAlpha * error);
	   mPhase	= PI_Constrain (mPhase);
	   return r;
	}

//	float get_cur_freq () const { return mFreq * mSampleRate / (2 * M_PI); }

private:
	const float mSmplRate;
	const float mAlpha;
	const float mBeta;
	const float mFreqLimit;
	float mFreq;
	float mPhase;
};

