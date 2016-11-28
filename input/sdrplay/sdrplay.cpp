#
/*
 *    Copyright (C) 2014
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
 */

#include	<QThread>
#include	<QSettings>
#include	<QHBoxLayout>
#include	<QLabel>

#include	"sdrplay.h"		// our header
#include	"sdrplay-worker.h"	// the worker
#include	"sdrplay-loader.h"	// funtion loader
//
#define	DEFAULT_GAIN	25

	sdrplay::sdrplay  (QSettings *s, bool full, bool *success) {
int	err;
float	ver;
QString	h;
int	k, i;

	sdrplaySettings		= s;
	this	-> myFrame	= new QFrame (NULL);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();

	sdrplaySettings	-> beginGroup ("sdrplay");
	h	= sdrplaySettings	-> value ("sdrplayRate", 2000). toString ();
	k	=  rateSelector -> findText (h);
	if (k != -1)
	   rateSelector	-> setCurrentIndex (k);
	inputRate	=  Khz (rateSelector -> currentText (). toInt ());
	sdrplaySettings	-> endGroup ();

	if (!full) {
	   while (rateSelector -> count () > 0)
	      rateSelector -> removeItem (rateSelector -> currentIndex ());
	   rateSelector	-> addItem (QString::number (inputRate / Khz (1)));
	}
	this	-> bandWidth	= getBandwidth (inputRate);
	
	*success		= false;
	_I_Buffer	= NULL;
	theLoader	= NULL;

	theLoader	= new sdrplayLoader (success);
	if (!(*success)) {
	   fprintf (stderr, " No success in loading sdrplay lib\n");
	   delete theLoader;
	   theLoader = NULL;
	   return;
	}

	err		= theLoader -> my_mir_sdr_ApiVersion (&ver);
	if (ver != MIR_SDR_API_VERSION) {
	   fprintf (stderr, "Foute API: %f, %f\n", ver, MIR_SDR_API_VERSION);
	   statusLabel	-> setText ("mirics error");
	}

	api_version	-> display (ver);
	_I_Buffer	= new RingBuffer<int16_t>(2 * 1024 * 1024);
	vfoFrequency	= Khz (94700);
	currentGain	= DEFAULT_GAIN;
	vfoOffset	= 0;
	theWorker	= NULL;
	connect (externalGain, SIGNAL (valueChanged (int)),
	         this, SLOT (setExternalGain (int)));
	connect (f_correction, SIGNAL (valueChanged (int)),
	         this, SLOT (freqCorrection  (int)));
	connect (rateSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (set_rateSelector (const QString &)));
	connect (KhzOffset, SIGNAL (valueChanged (int)),
	         this, SLOT (setKhzOffset (int)));
	sdrplaySettings	-> beginGroup ("sdrplay");
	externalGain -> setValue (sdrplaySettings -> value ("externalGain", 10). toInt ());
	f_correction -> setValue (sdrplaySettings -> value ("f_correction", 0). toInt ());
	KhzOffset	-> setValue (sdrplaySettings -> value ("KhzOffset", 0). toInt ());
	sdrplaySettings	-> endGroup ();
	*success	= true;
}

	sdrplay::~sdrplay	(void) {
	stopReader ();
	if (_I_Buffer != NULL)
	   delete _I_Buffer;
	if (theLoader != NULL)
	   delete theLoader;
	if (theWorker != NULL)
	   delete theWorker;
	sdrplaySettings	-> beginGroup ("sdrplay");
	sdrplaySettings	-> setValue ("externalGain", externalGain -> value ());
	sdrplaySettings	-> setValue ("f_correction", f_correction -> value ());
	sdrplaySettings	-> setValue ("KhzOffset", KhzOffset -> value ());
	sdrplaySettings	-> setValue ("sdrplayRate", 
	                              rateSelector -> currentText (). toLatin1 (). data ());
	sdrplaySettings	-> endGroup ();
	delete myFrame;
}
//
//	The filter bank for the dongle is the first
//	one
static inline
int16_t	bankFor_dongle (int32_t freq) {
	if (freq < 60 * MHz (1))
	   return -1;
	if (freq < 120 * MHz (1))
	   return 1;
	if (freq < 245 * MHz (1))
	   return 2;
	if (freq < 420 * MHz (1))
	   return -1;
	if (freq < 1000 * MHz (1))
	   return 3;
	return -1;
}
//
//	But for the sdrplay we use the second one
static inline
int16_t bankFor_sdr (int32_t freq) {
	if (freq < 100 * Khz (1))
	   return -1;
	if (MIR_SDR_API_VERSION > 1.5) {
	   if (freq < 12 * Mhz (1))
	      return 1;
	   if (freq < 30 * Mhz (1))
	      return 2;
	   if (freq < 60 * Mhz (1))
	      return 3;
	   if (freq < 120 * Mhz (1))
	      return 4;
	   if (freq < 250 * Mhz (1))
	      return 5;
	   if (freq < 420 * Mhz (1))
	      return 6;
	   if (freq < 1000 * Mhz (1))
	      return 7;
	   if (freq < 2000 * Mhz (1))
	      return 8;
	   return - 1;
	}
	else {   // old API
	   if (freq < 12 * MHz (1))
	      return 1;
	   if (freq < 30 * MHz (1))
	      return 2;
	   if (freq < 60 * MHz (1))
	      return 3;
	   if (freq < 120 * MHz (1))
	      return 4;
	   if (freq < 250 * MHz (1))
	      return 5;
	   if (freq < 380 * MHz (1))
	      return 6;
	   if (freq < 420 * MHz (1))
	      return -1;
	   if (freq < 1000 * MHz (1))
	      return 7;
	   return -1;
	}
}

bool	sdrplay::legalFrequency (int32_t f) {
	return (bankFor_sdr (f) != -1);
}

int32_t	sdrplay::defaultFrequency	(void) {
	return Khz (94700);
}

void	sdrplay::setVFOFrequency	(int32_t newFrequency) {
int32_t	realFreq = newFrequency + vfoOffset;

	if (bankFor_sdr (realFreq) == -1)
	   return;

	if (theWorker == NULL) {
	   vfoFrequency = newFrequency + vfoOffset;
	   return;
	}

	if (bankFor_sdr (realFreq) != bankFor_sdr (vfoFrequency)) {
	   stopReader ();
	   vfoFrequency	= realFreq;
	   restartReader ();
	   return;
	}
	else
	   theWorker -> setVFOFrequency (realFreq);
	vfoFrequency = realFreq;
}

int32_t	sdrplay::getVFOFrequency	(void) {
	return vfoFrequency - vfoOffset;
}

void	sdrplay::setExternalGain	(int newGain) {
	if (newGain < 0 || newGain > 102)
	   return;

	fprintf (stderr, "gain is nu %d\n", newGain);
	if (theWorker != NULL)
	   theWorker -> setExternalGain (newGain);
	currentGain = newGain;
}

int16_t	sdrplay::maxGain	(void) {
	return 101;
}

int32_t	sdrplay::setExternalRate	(int32_t newRate) {
	if (newRate < bandWidth)
	   return inputRate;

	if (theWorker != NULL)
	   theWorker -> setExternalRate (newRate);
	inputRate	= newRate;
	return inputRate;
}

int32_t	sdrplay::getRate	(void) {
	return inputRate;
}

bool	sdrplay::restartReader	(void) {
bool	success;

	if (theWorker != NULL)
	   return true;
	_I_Buffer	-> FlushRingBuffer ();

	theWorker = new sdrplayWorker (inputRate,
	                               bandWidth,
	                               vfoFrequency,
	                               theLoader,
	                               _I_Buffer,
	                               &success);
	if (success)
	   theWorker -> setExternalGain (currentGain);
	return success;
}

void	sdrplay::stopReader	(void) {
	if (theWorker == NULL)
	   return;
	theWorker	-> stop ();
	while (theWorker -> isRunning ())
	   usleep (100);
	delete theWorker;
	theWorker = NULL;
}
//
//	The brave old getSamples. 
//	we get the size in I/Q pairs
//	Note that the sdrPlay returns 10 bit values
int32_t	sdrplay::getSamples (DSPCOMPLEX *V, int32_t size) { 
int32_t	amount, i;
int16_t	buf [2 * size];
//
	amount = _I_Buffer	-> getDataFromBuffer (buf, 2 * size);
	for (i = 0; i < amount / 2; i ++)  
	   V [i] = DSPCOMPLEX (buf [2 * i] / 1024.0,
	                       buf [2 * i + 1] / 1024.0);
	return amount / 2;
}

int32_t	sdrplay::getSamples (DSPCOMPLEX *V, int32_t size, uint8_t M) {
	(void)M;
	return getSamples (V, size);
}

int32_t	sdrplay::Samples	(void) {
	return _I_Buffer	-> GetRingBufferReadAvailable () / 2;
}

uint8_t	sdrplay::myIdentity	(void) {
	return SDRPLAY;
}

void	sdrplay::resetBuffer	(void) {
	_I_Buffer	-> FlushRingBuffer ();
}

int16_t	sdrplay::bitDepth	(void) {
	return 11;
}

int32_t	sdrplay::getBandwidth	(int32_t rate) {
static int validWidths [] = {200, 300, 600, 1536, 5000, 6000, 7000, 8000, -1};
int16_t	i;
	for (i = 1; validWidths [i] != -1; i ++)
	   if (rate / Khz (1) < validWidths [i] &&
	       rate / Khz (1) >= validWidths [i - 1])
	      return validWidths [i - 1] * Khz (1);
	if (rate / Khz (1) == 8000)
	   return rate;
	return 1536 * Khz (1);	// default
}

void	sdrplay::set_rateSelector (const QString &s) {
	inputRate	= s == "1536" ? Khz (1536) : 
	                  s == "2000" ? Khz (2000) : 
	                  s == "2500" ? Khz (2500) :
	                  s == "3000" ? Khz (3000) : 
	                  s == "5000" ? Khz (5000) :
	                  s == "6000" ? Khz (6000) :
	                  Khz (8000);

	this	-> bandWidth	= getBandwidth (inputRate);
	stopReader ();
	set_changeRate (inputRate);
}

//	vfoOffset is in Hz, we have two spinboxes influencing the
//	settings
void	sdrplay::setKhzOffset	(int k) {
	vfoOffset	= vfoOffset % Khz (1) + Khz (k);
}

void	sdrplay::freqCorrection	(int f) {
	(void)f;
}

