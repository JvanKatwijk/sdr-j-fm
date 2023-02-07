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

#ifndef	__RDS_DECODER_2_H
#define	__RDS_DECODER_2_H

#include	<QObject>
#include	<stdlib.h>
#include	<stdio.h>
#include	<cassert>
#include	"rds-decoder.h"
#include        "rds-group.h"
#include        "rds-blocksynchronizer.h"
#include        "rds-groupdecoder.h"
#include        "sdr/agc.h"
#include        "sdr/costas.h"
#include        "sdr/time_sync.h"
#include        "iir-filters.h"
#include        "fir-filters.h"
#include        "sincos.h"
#include	"sdr/shaping_filter.h"
#include	<cmath>
#include	<vector>
#include	<fstream>


class	rdsDecoder_2: public QObject {
Q_OBJECT
public:

		rdsDecoder_2	(RadioInterface	*myRadio,
	                         int32_t		rate,
	                         rdsBlockSynchronizer	*my_rdsBlockSync,
	                         RDSGroup		*my_rdsGroup,
	                         rdsGroupDecoder	*my_rdsGroupDecoder);
		~rdsDecoder_2	();
	bool	doDecode	(std::complex<float>,
	                         std::complex<float> *,  int);

private:

	AGC			my_AGC;
	Costas			my_Costas;
	void			processBit		(bool, int);
	std::complex<float>	doMatchFiltering	(std::complex<float>);
	bool			process_sample		(const DSPCOMPLEX iZ,
	                                                      DSPCOMPLEX & oZ); 
	RadioInterface		*myRadioInterface;
	int			sampleRate;
	RDSGroup		*my_rdsGroup;
	rdsBlockSynchronizer	*my_rdsBlockSync;
	rdsGroupDecoder		*my_rdsGroupDecoder;
	std::vector<float>      my_matchedFltKernelVec;
        std::vector<DSPCOMPLEX> my_matchedFltBuf;
        int16_t                 my_matchedFltBufIdx;
        int16_t                 my_matchedFltBufSize;
	bool			previousBit;

	std::complex<float>	sampleBuffer [3]; // current and last 2 samples
	std::complex<float>	sampleBufferRail [3]; // current and last 2 samp
	float			mMu;		// 0.01;
	float			samplesPerSymbol;
	float			alpha;
	int32_t			skipNrSamples;  // 2 to fill out buffer
	int32_t			sampleCount;
	
signals:
        void			setCRCErrors	(int);
        void			setSyncErrors	(int);
        void			setbitErrorRate	(int);
};
#endif
