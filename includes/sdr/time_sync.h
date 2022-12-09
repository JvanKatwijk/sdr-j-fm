#pragma once

#include "fm-constants.h"

class TimeSync
{
public:
  TimeSync(const float iSps, const float iAlpha) : mSmplPerSym(iSps), mAlpha(iAlpha) {}
  TimeSync() = delete;
  ~TimeSync() = default;

  inline bool process_sample(const DSPCOMPLEX iZ, DSPCOMPLEX & oZ)
  {
    mSmplBuff[0] = mSmplBuff[1];
    mSmplBuff[1] = mSmplBuff[2];
    mSmplBuff[2] = iZ;

    if (++mSmplCnt >= mSkipNoSmpl)
    {
      // get hard decision values (rail to rail)
      for (int32_t i = 0; i < 3; ++i)
      {
        mSampBuffRail[i] = DSPCOMPLEX((real(mSmplBuff[i]) > 0.0f ? 1.0f : -1.0f), (imag(mSmplBuff[i]) > 0.0f ? 1.0f : -1.0f));
      }

      // Calculate Mueller & Muller metrics
      //    const DSPCOMPLEX x = (out_rail[2] - out_rail[0]) * conj(out[1]);
      //    const DSPCOMPLEX y = (out[2] - out[0]) * conj(out_rail[1]);
      //    const DSPFLOAT mm_val = real(y - x);

      //    const DSPFLOAT x = real((out_rail[2] - out_rail[0]) * conj(out[1]));
      //    const DSPFLOAT y = real((out[2] - out[0]) * conj(out_rail[1]));
      //    const DSPFLOAT mm_val = y - x;

      const DSPFLOAT x = real(mSampBuffRail[2] - mSampBuffRail[0]) * real(mSmplBuff[1])     + imag(mSampBuffRail[2] - mSampBuffRail[0]) * imag(mSmplBuff[1]);
      const DSPFLOAT y = real(mSmplBuff[2]     - mSmplBuff[0])     * real(mSampBuffRail[1]) + imag(mSmplBuff[2]     - mSmplBuff[0])     * imag(mSampBuffRail[1]);
      const DSPFLOAT mm_val = y - x;

      mMu += mSmplPerSym + mAlpha * mm_val;
      mSkipNoSmpl = (int32_t)(/*round*/(mMu)); // round down to nearest int since we are using it as an index
      mMu -= mSkipNoSmpl; // remove the integer part of mu

      oZ = mSmplBuff[2];
      mSmplCnt = 0;
      return true;
    }

    return false;
  }

private:
  DSPCOMPLEX mSmplBuff[3] {}; // current and last 2 samples
  DSPCOMPLEX mSampBuffRail[3] {}; // current and last 2 samples
  DSPFLOAT mMu = 0.00; // 0.01;
  float mSmplPerSym = 16;
  float mAlpha = 0.01;
  int32_t mSkipNoSmpl = 3;  // 2 to fill out buffer
  int32_t mSmplCnt = 0;
};

