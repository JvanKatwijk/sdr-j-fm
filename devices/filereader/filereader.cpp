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
#include	<QLabel>
#include	<QFileDialog>
#include	"fm-constants.h"
#include	"filehulp.h"
#include	"filereader.h"
#include	"ui_filereader-widget.h"

	fileReader::fileReader	(QSettings *s) {
	(void)s;
	this	-> myFrame	= new QFrame (NULL);
	setupUi		(this -> myFrame);
	this	-> myFrame	-> show ();
	inputRate	= 192000;		// it is a dummy
	nameofFile	-> setText (QString ("no file"));
	setup_Device	();
	this	-> lastFrequency	= Khz (94700);	
	this	-> attenuationLevel	= 100;
	attenuationSlider	-> setValue (attenuationLevel);
	attenuationDisplay	-> display (attenuationLevel);
	connect (attenuationSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_attenuation (int)));
}
//

	fileReader::~fileReader	(void) {
	if (myReader != NULL)
	   delete myReader;
	myReader	= NULL;
	delete	myFrame;
}

void	fileReader::setup_Device	(void) {
bool	success;
	QString	replayFile
	              = QFileDialog::
	                 getOpenFileName (myFrame,
	                                  tr ("load file .."),
	                                  QDir::homePath (),
	                                  tr ("sound (*.wav)"));
	replayFile	= QDir::toNativeSeparators (replayFile);
	myReader	= new fileHulp (replayFile, &success);
	if (success)
	   nameofFile	-> setText (replayFile);
	else
	   nameofFile	-> setText (QString ("ERROR"));
}

void	fileReader::setVFOFrequency		(int32_t f) {
	lastFrequency	= f;
}

int32_t	fileReader::getVFOFrequency		(void) {
	return lastFrequency;
}

bool	fileReader::legalFrequency		(int32_t f) {
	return f > 0;
}

int32_t	fileReader::defaultFrequency		(void) {
	return Khz (94700);
}
//
bool	fileReader::restartReader		(void) {
	return myReader -> restartReader ();
}

void	fileReader::stopReader			(void) {
	myReader	-> stopReader ();
}

int32_t	fileReader::Samples			(void) {
	return myReader	-> Samples ();
}

int32_t	fileReader::getSamples			(DSPCOMPLEX *v,
	                                         int32_t a, uint8_t m) {
	return	myReader -> getSamples (v, a, m, (float)attenuationLevel / 100.0);
}

int32_t	fileReader::getRate			(void) {
	return myReader -> getRate	();
}

int16_t	fileReader::bitDepth		(void) {
	return 8;
}

void	fileReader::set_attenuation	(int l) {
	attenuationLevel	= l;
	attenuationDisplay	-> display (l);
}
