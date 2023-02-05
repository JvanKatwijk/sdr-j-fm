/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *	This part of the fm software is a mixture of code  based on code from
 *	various sources. Two in particular:
 *
 *    FMSTACK Copyright (C) 2010 Michael Feilen
 *
 *    Author(s)       : Michael Feilen (michael.feilen@tum.de)
 *    Initial release : 01.09.2009
 *    Last changed    : 09.03.2010
 *
 *	cuteSDR (c) M Wheatly 2011
 *
 *	adapted by tomneda https://github.com/tomneda 
 *
 *    This file is part of the fm software
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm receover is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __RDS_DECODER_H
#define __RDS_DECODER_H

#include	<QObject>
#include	<vector>
#include	"fm-constants.h"
#include	"rds-group.h"
#include	"rds-blocksynchronizer.h"
#include	"rds-groupdecoder.h"
#include	"sdr/agc.h"
#include	"sdr/costas.h"
#include	"sdr/time_sync.h"
#include	"iir-filters.h"
#include	"fir-filters.h"
#include	"sincos.h"

class RadioInterface;

class rdsDecoder : public QObject {
Q_OBJECT
public:
		rdsDecoder	(RadioInterface *, int32_t);
		~rdsDecoder	();

	enum class ERdsMode {
	   RDS_OFF,
		RDS_1,
		RDS_2,
	        RDS_3
	};

	bool	doDecode	(const DSPCOMPLEX,
	                         DSPCOMPLEX * const,
	                         ERdsMode mode, int ptyLocale);
	void	reset		();

private:
	bool			doDecode1	(float v, int);
	bool			doDecode2	(float v, int);
	bool			doDecode_tmn	(std::complex<float> v,
	                                         std::complex<float> *m, int);
	void			processBit	(bool, int);
	RadioInterface		*myRadioInterface;
	rdsBlockSynchronizer	my_rdsBlockSync;
	rdsGroupDecoder		my_rdsGroupDecoder;
	SinCos			mySinCos;
//	
//	for cuteRDS approach we need
	BandPassIIR		sharpFilter;
	LowPassFIR		rdsFilter;
	RDSGroup		my_rdsGroup;
	std::vector<float>	rdsBuffer;
	std::vector<float>	rdsKernel;
	int16_t			ip;
	int16_t			rdsfilterSize;
	float			Match		(float);
	float			rdsLastSyncSlope;
	float			rdsLastSync;
	float			rdsLastData;
	bool			previousBit;
	std::vector<float>	syncBuffer;
	void			synchronizeOnBitClk	(float *, int16_t);
//
//
	ERdsMode		mode;
	AGC			my_AGC;
	TimeSync		my_timeSync;
	Costas			my_Costas;

	int32_t			sampleRate;

	std::vector<float>	my_matchedFltKernelVec;
	std::vector<DSPCOMPLEX> my_matchedFltBuf;
	int16_t			my_matchedFltBufIdx;
	int16_t			my_matchedFltBufSize;

	float		omegaRDS;
	int			symbolCeiling;
	int			symbolFloor;
	bool			prevBit;
	float		bitIntegrator;
	float		bitClkPhase;
	float		prev_clkState;
	bool			Resync;
	int16_t			p;


	float		doMatchFiltering	(float);
	DSPCOMPLEX	doMatchFiltering	(DSPCOMPLEX);

	void		synchronizeOnBitClk	(const std::vector<float> &, int16_t);


signals:
	void		setCRCErrors		(int);
	void		setSyncErrors		(int);
	void		setbitErrorRate		(int);
};

#endif
