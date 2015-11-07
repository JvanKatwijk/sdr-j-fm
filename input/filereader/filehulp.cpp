#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair programming
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

#include	<QFrame>
#include	<samplerate.h>
#include	<sys/time.h>
#include	<time.h>
#include	"fm-constants.h"
#include	"filehulp.h"

#define	__BUFFERSIZE	32768 * 8

static inline
int64_t		getMyTime	(void) {
struct timeval	tv;

	gettimeofday (&tv, NULL);
	return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}

	fileHulp::fileHulp	(QString f, bool *success) {
SF_INFO	*sf_info;

	*success		= false;
	fileName		= f;
	readerOK		= false;
	inputRate		= Khz (192);
	_I_Buffer		= new RingBuffer<float>(__BUFFERSIZE);

	sf_info			= (SF_INFO *)alloca (sizeof (SF_INFO));
	sf_info	-> format	= 0;
	filePointer		= sf_open (fileName. toLatin1 (). data (),
	                                   SFM_READ, sf_info);
	if (filePointer == NULL) {
	   fprintf (stderr, "file %s no legitimate sound file\n",
	                    f. toLatin1 (). data ());
	   return;
	}

	samplesinFile	= sf_info	-> frames;
	inputRate	= sf_info	-> samplerate;
	numofChannels	= sf_info	-> channels;
	readerOK	= true;
	*success	= true;
	readerPausing	= true;
	
	currPos		= 0;
	fprintf (stderr, "file %s geopend\n", f. toLatin1 (). data ());
	start ();
}

	fileHulp::~fileHulp (void) {
	ExitCondition = true;
	if (readerOK) {
	   while (isRunning ())
	      usleep (100);
	   sf_close (filePointer);
	}
	delete _I_Buffer;
}

bool	fileHulp::restartReader	(void) {
	if (readerOK)
	   readerPausing = false;
	return readerOK;
}

void	fileHulp::stopReader	(void) {
	if (readerOK)
	   readerPausing = true;
}

int32_t	fileHulp::Samples	(void) {
	if (ExitCondition)
	   return 0;
	return _I_Buffer -> GetRingBufferReadAvailable () / 2;
}

int32_t	fileHulp::getSamples	(DSPCOMPLEX *V, int32_t n, uint8_t Mode) {
float	*buf = (float *)alloca (2 * n * sizeof (float));
int32_t	i;

	while (!ExitCondition &&
	       (_I_Buffer -> GetRingBufferReadAvailable () < (uint32_t)(2 * n))) {
	   usleep (100);
	}

	if (ExitCondition)
	   return 0;

	_I_Buffer	-> getDataFromBuffer (buf, 2 * n);
	for (i = 0; i < n; i ++) {
	   switch (Mode) {
	      default:
	      case IandQ:
	         V [i] = DSPCOMPLEX (buf [2 * i], buf [2 * i + 1]);
	         break;

	      case QandI:
	         V [i] = DSPCOMPLEX (buf [2 * i + 1], buf [2 * i]);
	         break;

	      case I_Only:
	         V [i]= DSPCOMPLEX (buf [2 * i], 0.0);
	         break;

	      case Q_Only:
	         V [i]	= DSPCOMPLEX (buf [2 * i + 1], 0.0);
	         break;
	   }
	}

	return n;
}

/*	length is number of floats that we read.
 *	In case of mono, we add zeros to the right channel
 *	and therefore only read length / 2 floats
 *	Notice that sf_readf_xxx reads frames (samples * numofchannels)
 */
int32_t	fileHulp::readBuffer (float *data, int32_t length) {
int32_t	n, i;
float	*tempBuffer;

	if (numofChannels == 2) {
	   n = sf_readf_float (filePointer, data, length / 2);
	}
	else {
//	apparently numofChannels == 1
	   tempBuffer	= (float *)alloca (length / 2 * sizeof (float));
	   n		= sf_readf_float (filePointer, tempBuffer, length / 2);
	   for (i = 0; i < n; i ++) {
	      data [2 * i] = tempBuffer [i];
	      data [2 * i + 1] = 0;
	   }
	}

	currPos		+= n;
	if (n < length / 2)
	   sf_seek (filePointer, 0, SEEK_SET);
	return	2 * n;
}

bool	fileHulp::isWorking	(void) {
	return readerOK;
}

int32_t	fileHulp::getRate	(void) {
	return inputRate;
}
//
//	for processing the file-io we start a separate thread
//
void	fileHulp::run () {
int32_t	t, i;
float	*bi;
int32_t	bufferSize;
int32_t	period;
int64_t	nextStop;

	if (!readerOK)
	   return;

	ExitCondition	= false;
	ThreadFinished	= false;

	period		= 10000; // microseconds
	bufferSize	= 2 * inputRate / 100;
	bi		= new float [bufferSize];
	nextStop	= getMyTime ();
	fprintf (stderr, "We are running\n");
	while (!ExitCondition) {
	   if (readerPausing) {
	      usleep (1000);
	      nextStop = getMyTime ();
	      continue;
	   }
	   while (_I_Buffer -> WriteSpace () < bufferSize + 10) {
	      if (ExitCondition)
	         break;
	      usleep (1000);
	   }

	   nextStop += period;
	   t = readBuffer (bi, bufferSize);
	   if (t <= 0) {
	      for (i = 0; i < bufferSize; i ++)
	          bi [i] = 0;
	      t = bufferSize;
	   }
	   _I_Buffer -> putDataIntoBuffer (bi, t);
	   if (nextStop - getMyTime () > 0)
	      usleep (nextStop - getMyTime ());
	}
	fprintf (stderr, "taak voor replay eindigt hier\n");
	ThreadFinished	= false;
}


