#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the FM software
 *
 *    FM software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    FM software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with FM software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	<QObject>
#include	<QFrame>
#include	<QFileDialog>
#include	"fm-constants.h"
#include	"device-handler.h"
#include	"ringbuffer.h"
#include	"ui_elad_widget.h"
#include	<libusb-1.0/libusb.h>

class	QSettings;
class	eladWorker;
class	eladLoader;
typedef	DSPCOMPLEX(*makeSampleP)(uint8_t *);

class	eladHandler: public deviceHandler, public Ui_elad_widget {
Q_OBJECT
public:
		eladHandler		(QSettings *, bool, bool *);
		~eladHandler		();
	void	setVFOFrequency		(int32_t);
	int32_t	getVFOFrequency		();
	bool	legalFrequency		(int32_t);
	int32_t	defaultFrequency	();

	bool	restartReader		();
	void	stopReader		();
	int32_t	getSamples		(DSPCOMPLEX *, int32_t, uint8_t);
	int32_t	Samples			();
	int32_t	getRate			();
	int16_t	bitDepth		();
private	slots:
	void	setGainReduction	();
	void	setMHzOffset		(int);
	void	setFilter		();
	void	setAttenuation		(int);
private:
	QSettings	*eladSettings;
	bool		deviceOK;
	eladLoader	*theLoader;
	eladWorker	*theWorker;
	RingBuffer<uint8_t>	*_I_Buffer;
	int32_t		theRate;
	QFrame		*myFrame;
	int32_t		vfoFrequency;
	int32_t		vfoOffset;
	int		gainReduced;
	int		localFilter;
	uint8_t		conversionNumber;
	int16_t		iqSize;
	int16_t		attenuation;
};

