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
#include	"costas.h"
#include	"agc.h"

class RadioInterface;
class	rdsDecoder_1;
class	rdsDecoder_2;
class	rdsDecoder_3;

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
	RDSGroup		my_rdsGroup;
	rdsGroupDecoder		my_rdsGroupDecoder;
	rdsBlockSynchronizer	my_rdsBlockSync;
	AGC				my_AGC;
	Costas			my_costas;
	rdsDecoder_1		*decoder_1;
	rdsDecoder_2		*decoder_2;
	rdsDecoder_3		*decoder_3;
	void                    processBit      (bool, int);

signals:
        void			setCRCErrors	(int);
        void			setSyncErrors	(int);
        void			setbitErrorRate	(int);

};

#endif
