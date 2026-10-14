#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr decoder
 *
 *    qt-wspr is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-wspr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once
#include <iostream>
#include <vector>
#include <complex>
#include <numeric>
#include <cmath> 

class	new_downConverter {
public:
//	M: Decimation factor, nrTaps, fc, fs
	new_downConverter	(size_t M, int	nrTaps, int fc, int fs);
	~new_downConverter	();
//	Process a single incoming sample, wait until at least M
//	samples are collected
bool	process (std::complex<float> input, std::complex<float> &output);

private:
	size_t decimationFactor;
	size_t numSubfilters;
	std::vector<std::vector<float>> subfilters;
	std::vector<std::vector<std::complex<float>>> stateBuffers;
	std::vector<std::complex<float>> inBuffer;
	int	inp;
};

