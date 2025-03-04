#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of sdr-j-FM
 *
 *    sdr-j-FM is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    sdr-j-FM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-FM if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	<QObject>
#include	<stdint.h>
#include	<stdio.h>
#include	<sdrplay_api.h>

class	sdrplayHandler_v3;

class Rsp_device : public QObject {
Q_OBJECT
protected:
	sdrplay_api_DeviceT *chosenDevice;
	int	sampleRate;
	int	freq;
	bool	agcMode;
	int	lnaState;
	int	GRdB;
	sdrplay_api_RxChannelParamsT	*chParams;
	sdrplay_api_DeviceParamsT	*deviceParams;
	sdrplayHandler_v3	*parent;
	int	lna_upperBound;
	QString	deviceModel;
	bool	antennaSelect;
	int	nrBits;
	bool	biasT;
public:
		Rsp_device 	(sdrplayHandler_v3 *parent,
	                         sdrplay_api_DeviceT *chosenDevice,
	                         int sampleRate,
	                         int startFrequency,
	                         bool agcMode,
	                         int lnaState,
	                         int GRdB,
	                         bool biasT,
	                         double ppmValue);
	virtual	~Rsp_device	();
virtual int	lnaStates	(int frequency);

virtual	bool	restart		(int freq);
	bool	set_agc		(int setPoint, bool on);
	bool	set_GRdB	(int GRdBValue);
	bool	set_ppm		(double ppm);
virtual	bool	set_lna		(int lnaState);
virtual	bool	set_antenna	(int antenna);
virtual	bool	set_amPort 	(int amPort);
virtual	bool	set_biasT 	(bool biasT);
virtual	bool	set_tuner	(int tuner);
signals:
	void	set_lnabounds_signal	(int, int);
	void	show_lnaGain		(int);
};


