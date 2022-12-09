#include "sdr/shaping_filter.h"

std::vector<float> ShapingFilter::root_raised_cosine(double gain, double sampling_freq, double symbol_rate, double alpha, int ntaps)
{
  ntaps |= 1; // ensure that ntaps is odd

  double spb = sampling_freq / symbol_rate;// samples per bit/symbol
  std::vector<float> taps(ntaps);
  double scale = 0;

  for (int i = 0; i < ntaps; i++)
  {
    double x1, x2, x3, num, den;
    double xindx = i - ntaps / 2;
    x1 = M_PI * xindx / spb;
    x2 = 4 * alpha * xindx / spb;
    x3 = x2 * x2 - 1;

    if (fabs(x3) >= 0.000001) // Avoid Rounding errors...
    {
      if (i != ntaps / 2)
      {
        num = cos((1 + alpha) * x1) + sin((1 - alpha) * x1) / (4 * alpha * xindx / spb);
      }
      else
      {
        num = cos((1 + alpha) * x1) + (1 - alpha) * M_PI / (4 * alpha);
      }
      den = x3 * M_PI;
    }
    else
    {
      if (alpha == 1)
      {
        taps[i] = -1;
        scale += taps[i]; // this was missing in gnuradio, seems to be a bug there
        continue;
      }
      x3  = (1 - alpha) * x1;
      x2  = (1 + alpha) * x1;
      num = (sin(x2) * (1 + alpha) * M_PI
             - cos(x3) * ((1 - alpha) * M_PI * spb) / (4 * alpha * xindx)
             + sin(x3) * spb * spb / (4 * alpha * xindx * xindx));
      den = -32 * M_PI * alpha * alpha * xindx / spb;
    }
    taps[i] = 4 * alpha * num / den;
    scale  += taps[i];
  }

  for (int i = 0; i < ntaps; i++)
  {
    taps[i] = taps[i] * gain / scale;
  }

  return taps;
}

