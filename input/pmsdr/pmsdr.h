#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
 *    along with SDR-J; if not, see <http://www.gnu.org/licenses/>.
 *
 * 	Elements of this code are adapted from:
 *	1. mbed SI570 Library, for driving the SI570 programmable VCXO
 *      Copyright (c) 2010, Gerrit Polder, PA3BYA
 *	2. control a PMSDR hardware on Linux
 *	Modified to comply to firmware version 2.1.x
 *	Copyright (C) 2008,2009,2010  Andrea Montefusco IW0HDV,
 *	Martin Pernter IW3AUT
 *	Both available under a (L)GPL
 */

#ifndef	__PMSDR
#define	__PMSDR

#include	<QFrame>
#include	"fm-constants.h"
#include	"pmsdr-comm.h"
#include	"virtual-input.h"
#include	"ui_pmsdr-widget.h"

class	QSettings;
class	paReader;
//
//	Implementing the rig interface
// Global CONSTANTS
//-----------------------------------------------------------------------------

//
const static QsdBiasFilterIfGain qsdBfigDefault = {
    512,  // bias at half scale
    0,    // mute off
    0,    // no filter
    1     // 20 dB gain
};

typedef
struct si570data_ {
        unsigned char	n1;
        unsigned char	hsdiv;
        uint32_t	frac_bits;
	double		currentFreq;
        double		rfreq;
        double		fxtal;
} si570data;

class	pmsdrHandler: public virtualInput, public Ui_pmsdrWidget {
Q_OBJECT

public:
			pmsdrHandler		(QSettings *, bool *);
			~pmsdrHandler		(void);
	void		setVFOFrequency		(int32_t);
	int32_t		getVFOFrequency		(void);
	int32_t		defaultFrequency	(void);
	bool		legalFrequency		(int32_t);
	uint8_t		myIdentity		(void);

	bool		restartReader		(void);
	void		stopReader		(void);
	int32_t		getSamples		(DSPCOMPLEX *, int32_t);
	int32_t		getSamples		(DSPCOMPLEX *, int32_t, uint8_t);
	int32_t		Samples			(void);
	int16_t		bitDepth		(void);
	int32_t		getRate			(void);
private slots:
	void		setMute			(void);
	void		setGain			(int);
	void		setBias			(int);
	void		set_offset_KHz		(int);
	void		set_StreamSelector	(int);
private:
	QSettings	*pmsdrSettings;
	void		setFilter		(int16_t);
	void		setFilterAutomatic	(int32_t);
	bool		automaticFiltering;
	paReader	*myReader;
	int32_t		inputRate;
	void		setup_device	(void);
	int32_t		vfoOffset;
	QFrame		*myFrame;
	QsdBiasFilterIfGain qsdBfig;
	RadioInterface	*myRadioInterface;
	bool		radioOK;
	pmsdr_comm	*pmsdrDevice;
	int16_t		usbFirmwareSub;
	int16_t		usbFirmwareMajor;
	int16_t		usbFirmwareMinor;
	int32_t		LOfreq;
	bool		thirdHarmonic;
	uint8_t		fLcd;
	
	int16_t		currentFilter;
	uint8_t		postdivider;
	uint8_t		si570_reg[8];
	si570data	si570pmsdr;

	bool		ReadandConvertRegisters	(int8_t);
	void		calc_si570registers	(uint8_t *, si570data *);
	uint8_t		SetBits			(uint8_t, uint8_t, uint8_t);
	void		ComputeandSetSi570	(double);
	void		convertFloattoHex	(si570data *, uint16_t *);
	bool		computeDividers		(double, uint8_t *, uint8_t *);
	bool		setFrequencySmallChange	(si570data *,
	                                         double, uint16_t *);
	bool		setFrequencyLargeChange	(si570data *,
	                                         double, uint16_t *);
	bool		store_hwlo		(int32_t);
	bool		read_hwlo		(int32_t *);

};

#endif

