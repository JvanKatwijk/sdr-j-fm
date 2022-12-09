#include "squelchClass.h"

constexpr DSPFLOAT SQUELCH_HYSTERESIS_NSQ = 0.001;
constexpr DSPFLOAT SQUELCH_HYSTERESIS_LSQ = 0.000;
constexpr DSPFLOAT LEVELREDUCTIONFACTOR = 0.000; // no output level while squelch active
//constexpr DSPFLOAT LEVELREDUCTIONFACTOR = 0.025; // 32dB lower output level

squelch::squelch(const int32_t iSquelchThreshold, const int32_t iKeyFrequency, const int32_t iBufsize, const int32_t iSampleRate) :
  mSquelchHighpass(20, iKeyFrequency - 100, iSampleRate, S_CHEBYSHEV),
  mSquelchLowpass(20, iKeyFrequency, iSampleRate, S_CHEBYSHEV)
{
  setSquelchLevel(iSquelchThreshold); // convert 0..100 to 1.0..0.0
  mKeyFrequency = iKeyFrequency;
  mHoldPeriod = iBufsize;
  mSampleRate = iSampleRate;

  mSquelchSuppress = false;
  mSquelchSuppressLast = !mSquelchSuppress;
  mSquelchCount = 0;
  mAverage_High = 0;
  mAverage_Low = 0;
}

void squelch::setSquelchLevel(int n)
{
  // n = 0..100 (from SQ slider)
  //mLevelSquelchThreshold = n / 100.0f;
  mLevelSquelchThreshold = pow(10.0f, (n - 80) / 30.0f);
  mNoiseSquelchThreshold = 1.0f - n / 100.0f;
}

static inline DSPFLOAT decayingAverage(DSPFLOAT old, DSPFLOAT input, DSPFLOAT weight)
{
  if (weight <= 1)
  {
    return input;
  }
  return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}

DSPFLOAT squelch::do_noise_squelch(const DSPFLOAT soundSample)
{
  const DSPFLOAT val_1 = abs(mSquelchHighpass.Pass(soundSample));
  const DSPFLOAT val_2 = abs(mSquelchLowpass.Pass(soundSample));

  mAverage_High = decayingAverage(mAverage_High, val_1, mSampleRate / 100);
  mAverage_Low  = decayingAverage(mAverage_Low,  val_2, mSampleRate / 100);

  if (++mSquelchCount >= mHoldPeriod)
  {
    mSquelchCount = 0;

    //	looking for a new squelch state
    if (mNoiseSquelchThreshold < SQUELCH_HYSTERESIS_NSQ)   // force squelch if zero
    {
      mSquelchSuppress = true;
    }
    else if (mAverage_High < mAverage_Low * mNoiseSquelchThreshold - SQUELCH_HYSTERESIS_NSQ)
    {
      mSquelchSuppress = false;
    }
    else if (mAverage_High >= mAverage_Low * mNoiseSquelchThreshold + SQUELCH_HYSTERESIS_NSQ)
    {
      mSquelchSuppress = true;
    }
    //	else just keep old squelchSuppress value
  }

  if (mSquelchSuppress != mSquelchSuppressLast)
  {
    mSquelchSuppressLast = mSquelchSuppress;
    emit setSquelchIsActive(mSquelchSuppress);
  }

  return mSquelchSuppress ? soundSample * LEVELREDUCTIONFACTOR : soundSample;
}

DSPFLOAT squelch::do_level_squelch(const DSPFLOAT soundSample, const DSPFLOAT iCarrierLevel)
{
  if (++mSquelchCount >= mHoldPeriod)
  {
    mSquelchCount = 0;

    //	looking for a new squelch state
    if (iCarrierLevel < mLevelSquelchThreshold - SQUELCH_HYSTERESIS_LSQ)
    {
      mSquelchSuppress = true;
    }
    else if (iCarrierLevel >= mLevelSquelchThreshold + SQUELCH_HYSTERESIS_LSQ)
    {
      mSquelchSuppress = false;
    }
    //	else just keep old squelchSuppress value
  }

  if (mSquelchSuppress != mSquelchSuppressLast)
  {
    mSquelchSuppressLast = mSquelchSuppress;
    emit setSquelchIsActive(mSquelchSuppress);
  }

  return mSquelchSuppress ? soundSample * LEVELREDUCTIONFACTOR : soundSample;
}
