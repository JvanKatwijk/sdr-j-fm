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

#ifndef	__RDS_DECODER_3_H
#define	__RDS_DECODER_3_H

#include	<QObject>
#include	<stdlib.h>
#include	<stdio.h>
#include	<cassert>
#include	<cmath>
#include	<vector>
#include	<fstream>
#include	"rds-group.h"
#include	"rds-blocksynchronizer.h"
#include	"rds-groupdecoder.h"
#include	"fir-filters.h"
#include	"sincos.h"

class	rdsDecoder_3: public QObject {
Q_OBJECT
public:
		rdsDecoder_3	(RadioInterface	*myRadio,
	                         int32_t	rate,
	                         rdsBlockSynchronizer  *my_rdsBlockSync,
                                 RDSGroup            *my_rdsGroup,
                                 rdsGroupDecoder     *my_rdsGroupDecoder);
		~rdsDecoder_3	();
	bool	doDecode	(float, uint8_t *);
private:
	SinCos			mySinCos;
	LowPassFIR		rdsFilter;

	RDSGroup		*my_rdsGroup;
	rdsBlockSynchronizer	*my_rdsBlockSync;
        rdsGroupDecoder		*my_rdsGroupDecoder;
        std::vector<float>      syncBuffer;
        void                    synchronizeOnBitClk (std::vector<float> &,
	                                                          int16_t);
//
        RadioInterface          *myRadioInterface;
        int                     sampleRate;

	float			omegaRDS;
	int			symbolCeiling;
	int			symbolFloor;
	float			bitIntegrator;
	float			bitClkPhase;
        bool                    previousBit;
	float			prev_clkState;
	bool			Resync;
	int16_t			p;
};
#endif


