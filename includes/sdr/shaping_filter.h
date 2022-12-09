#pragma once

#include "fm-constants.h"
#include <vector>

class ShapingFilter
{
public:
  ShapingFilter() = default;
  ~ShapingFilter() = default;

  std::vector<float> root_raised_cosine(double gain, double sampling_freq, double symbol_rate, double alpha, int ntaps);
};
