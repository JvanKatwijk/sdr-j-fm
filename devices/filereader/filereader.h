#
/*
 *    Copyright (C) 2008, 2009, 2010
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
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __FILEREADER__
#define	__FILEREADER__

#include	"device-handler.h"
#include	<QWidget>
#include	<QFrame>
#include	<QString>
#include	"ui_filereader-widget.h"

class	QLabel;
class	QSettings;
class	fileHulp;
/*
 */
class	fileReader: public deviceHandler, public Ui_filereaderWidget {
Q_OBJECT
public:
		fileReader		(QSettings *);
		~fileReader		();
	void	setVFOFrequency		(int32_t);
	int32_t	getVFOFrequency		();
	bool	legalFrequency		(int32_t);
	int32_t	defaultFrequency	();

	bool	restartReader		();
	void	stopReader		();
	int32_t	Samples			();
	int32_t	getSamples		(DSPCOMPLEX *, int32_t);
	int32_t	getRate			();
	int16_t	bitDepth		();
protected:
	void		setup_Device	();
	QFrame		*myFrame;
	fileHulp	*myReader;
	QLabel		*indicator;
	QLabel		*fileDisplay;
	int32_t		inputRate;
	int32_t		lastFrequency;
	int16_t		attenuationLevel;
private slots:
	void		set_attenuation	(int);
};
#endif

