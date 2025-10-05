#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	We have to create a simple virtual class here, since we
 *	want the interface with different devices (including  filehandling)
 *	to be transparent
 */

#pragma once

#include	<stdint.h>
#include	"fm-constants.h"
#include	<QObject>
#include	<QThread>
#include	<QDialog>

#define	NIX		0100
#define	SDRPLAY		0101
#define	DAB_STICK	0102
#define	EXTIO		0104
#define	AIRSPY		0110
#define	PMSDR		0111
#define	HACKRF		0114
#define	LIME		0120
#define	SDRPLAY_V3	0121
#define	PLUTO		0122
#define	COLIBRI		0124
#define	SPYSERVER	0125
//
//	in some cases we only want to differentiate between sticks
//	and non-sticks

#define	someStick(x)	((x) & 03)

/**
  *	\class virtualInput
  *	base class for devices for the fm software
  *	The class is not completely virtual, since it is
  *	used as a default in case selecting a "real" class did not work out
  */
class	deviceHandler: public QThread {
Q_OBJECT
public:
				deviceHandler 	();
virtual				~deviceHandler 	();
virtual		int32_t		getRate		();
virtual		void		setVFOFrequency	(int32_t);
virtual		int32_t		getVFOFrequency	();
virtual		int32_t		defaultFrequency ();
virtual		bool		legalFrequency	(int);
virtual		bool		restartReader	();
virtual		void		stopReader	();
virtual		int32_t		getSamples	(std::complex<float> *,
	                                                     int32_t);
virtual		int32_t		getSamples	(std::complex<float> *,
	                                                     int32_t, uint8_t);
virtual		int32_t		Samples		();
virtual		void		resetBuffer	();
virtual		int16_t		bitDepth	();
	        int32_t		vfoOffset;
//
protected:
		int32_t	lastFrequency;
signals:
		void	set_changeRate	(int);
};


