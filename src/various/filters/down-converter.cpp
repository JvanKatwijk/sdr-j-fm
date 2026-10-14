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

#include	"down-converter.h"

	new_downConverter::new_downConverter (size_t M,
	                             int nrTaps, int fc, int fs):
	                                   decimationFactor (M),
	                                   numSubfilters(M) {
float	sum = 0;

	int firSize = ((nrTaps + M - 1) / M) * M;
	std::vector<float> kernel (firSize);
	std::vector<float> temp   (firSize);
	float	 frequency	= (float)fc / fs;

	for (int i = 0; i < firSize; i ++)
	   kernel [i] = 0;

	for (int i = 0; i < firSize; i ++) {
	   if (i == firSize / 2)
	      temp [i] = 2 * M_PI * frequency;
	   else
	      temp [i] =
	        sin (2 * M_PI * frequency * (i - firSize / 2)) / (i - firSize / 2);
	   //      Blackman window
	   temp [i]  *= (0.42 -
	                0.5 * cos (2 * M_PI * (float)i / firSize) +
	                0.08 * cos (4 * M_PI * (float)i / firSize));
	   sum += temp [i];
        }
	
//	Pad taps to be a multiple of M
	std::vector<float> paddedTaps;
	for (int i = 0; i < firSize; i ++)
	   paddedTaps. push_back (temp [i] / sum);
	while (paddedTaps. size () % numSubfilters != 0) {
	   paddedTaps. push_back (0.0);
        }
        
	size_t tapsPerSubfilter = paddedTaps. size () / numSubfilters;
	subfilters. resize (numSubfilters,
	                    std::vector<float> (tapsPerSubfilter));
	stateBuffers. resize (numSubfilters,
	                      std::vector<std::complex<float>>(tapsPerSubfilter, 0.0));

//	Decompose the prototype filter into M polyphase branches
	for (size_t i = 0; i < paddedTaps. size (); i++) {
	   size_t subfilterIdx	= i % numSubfilters;
	   size_t tapIdx	= i / numSubfilters;
	   subfilters [subfilterIdx][tapIdx] = paddedTaps [i];
	}
	inBuffer. resize (M);
	inp	= 0;
}

	new_downConverter::~new_downConverter	() {}

//	Process a block of data downsampling by factor M
bool	new_downConverter::process (std::complex<float> input,
	                                   std::complex<float> &output) {
	inBuffer [inp] = input;
	inp ++;
	if (inp < decimationFactor)
	   return false;
	inp	= 0;
	std::complex<float> accumulator (0.0, 0.0);

//	Compute output sample by evaluating the M polyphase subfilters
	for (size_t subIdx = 0; subIdx < numSubfilters; subIdx ++) {
//	Determine matching input sample from the stream commutation
	   size_t inIdx = (decimationFactor - 1 - subIdx);
                
//	Update the state buffer for the specific subfilter branch
	   auto& state	= stateBuffers [subIdx];
	   state. insert (state. begin (), inBuffer [inIdx]);
	   state. pop_back ();

//	Perform the inner multiply-accumulate operation
	   for (size_t tapIdx = 0;
	        tapIdx < subfilters [subIdx].size(); tapIdx ++) {
	      accumulator += state[tapIdx] * subfilters[subIdx][tapIdx];
	   }
	}
	output = accumulator;
	return true;
}
