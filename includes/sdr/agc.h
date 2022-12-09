#pragma once

#include "fm-constants.h"

class AGC
{
public:
  AGC(float iSR, float iRefLevel, float iStartGain)
  : mSmplRate(iSR)
    , mRefLevel(iRefLevel)
    , mCurGain(iStartGain)
  {
  }
  AGC() = delete;
  ~AGC() = default;

  inline DSPCOMPLEX process_sample(DSPCOMPLEX input)
  {
    DSPCOMPLEX output = input * mCurGain;
    mCurGain += mSmplRate * (mRefLevel - std::abs(output));
    return output;
  }

protected:
  float mSmplRate;     // adjustment rate
  float mRefLevel;// reference value
  float mCurGain;     // current gain
};

