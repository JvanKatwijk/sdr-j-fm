/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of SDR-J-FM
 *
 *    SDR-J-FM is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J-FM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J-FM; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FM_DEMODULATOR_H
#define __FM_DEMODULATOR_H

#include	<QString>
#include	<QStringList>
#include 	<vector>
#include 	"fm-constants.h"
#include	"Xtan2.h"
#include	"pllC.h"
#include 	"sincos.h"
#include 	"iir-filters.h"

#define PLL_PILOT_GAIN 3000

class	fm_Demodulator {
public:
		fm_Demodulator (int32_t Rate_in);
		~fm_Demodulator ();

	void		setDecoder	(const QString &);
	QStringList	listNameofDecoder	() const;
	DSPFLOAT	demodulate	(DSPCOMPLEX);
	DSPFLOAT	get_DcComponent	();
	DSPFLOAT	get_carrier_ampl () { return am_carr_ampl; }

private:
	SinCos		mySinCos;
	int16_t		selectedDecoder;
	DSPFLOAT	max_freq_deviation;
	int32_t 	rateIn;
	DSPFLOAT	fm_afc;
	DSPFLOAT	fm_cvt;
	DSPFLOAT        K_FM;
	pllC		*myfm_pll;
	int32_t		arcSineSize;
	std::vector<DSPFLOAT>	Arcsine;
	compAtan	myAtan;
	DSPFLOAT	Imin1;
	DSPFLOAT	Qmin1;
	DSPFLOAT	Imin2;
	DSPFLOAT	Qmin2;
	DSPFLOAT	am_carr_ampl;

};
#endif
