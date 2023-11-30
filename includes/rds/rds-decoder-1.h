#
/*
 *    Copyright (C)  2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdr-j-fm
 *
 *    sdr-j-fm is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdr-j-fm is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	<QObject>
#include	<stdlib.h>
#include	<stdio.h>
#include	<cassert>
#include	<cmath>
#include	<vector>
#include	<fstream>
#include	"iir-filters.h"
#include	"fir-filters.h"

class	RadioInterface;

class	rdsDecoder_1 : public QObject {
Q_OBJECT
public:
		rdsDecoder_1 (RadioInterface	*myRadio,
	                      int32_t		rate);
		~rdsDecoder_1	();
	bool	doDecode	(float, uint8_t *);
private:
        RadioInterface          *myRadioInterface;
	BandPassIIR		sharpFilter;
	LowPassFIR		rdsFilter;

	bool			previousBit;
	std::vector<float>	rdsBuffer;
	std::vector<float>	rdsKernel;
	int16_t			ip;
	int16_t			rdsBufferSize;
	float			Match		(float);
	float			rdsLastSyncSlope;
	float			rdsLastSync;
	float			rdsLastData;
};

