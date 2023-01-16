/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *	This part of the jsdr is a mixture of code  based on code from
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
 *    This file is part of the SDR-J.
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* massively adapted by tomneda https://github.com/tomneda */

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
		RDS_2
	};

	bool	doDecode	(const DSPCOMPLEX,
	                         DSPCOMPLEX * const,
	                         ERdsMode mode, int ptyLocale);
	void	reset		();

private:
	void			doDecode2	(float v, int);
	void			processBit	(bool, int);

	ERdsMode		mode;
	AGC			my_AGC;
	TimeSync		my_timeSync;
	//Costas			my_Costas1;
	Costas			my_Costas;

	int32_t			sampleRate;
	RadioInterface		*myRadioInterface;
	RDSGroup		*my_rdsGroup;
	rdsBlockSynchronizer	*my_rdsBlockSync;
	rdsGroupDecoder		*my_rdsGroupDecoder;

	std::vector<DSPFLOAT>	my_matchedFltKernelVec;
	std::vector<DSPCOMPLEX> my_matchedFltBuf;
	int16_t			my_matchedFltBufIdx;
	int16_t			my_matchedFltBufSize;

	DSPFLOAT		omegaRDS;
	SinCos			*mySinCos;
	bool			previousBit;
	int			symbolCeiling;
	int			symbolFloor;
	std::vector<float>	syncBuffer;
	bool			prevBit;
	DSPFLOAT		bitIntegrator;
	DSPFLOAT		bitClkPhase;
	DSPFLOAT		prev_clkState;
	bool			Resync;
	int16_t			p;


	DSPFLOAT		doMatchFiltering	(DSPFLOAT);
	DSPCOMPLEX		doMatchFiltering	(DSPCOMPLEX);

	void			synchronizeOnBitClk	(const std::vector<float> &, int16_t);


signals:
	void			setCRCErrors		(int);
	void			setSyncErrors		(int);
	void			setbitErrorRate		(int);
};

#endif
