#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J.
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

#ifndef __FILEHULP
#define	__FILEHULP

#include	<QThread>
#include	<QString>
#include	<sndfile.h>
#include	"fm-constants.h"
#include	"ringbuffer.h"
//
//
class	fileHulp: public QThread {
Q_OBJECT
public:
		fileHulp	(QString, bool *);
		~fileHulp	();
//	
//	functions really for this rig
	bool		restartReader	();
	void		stopReader	();
	int32_t		Samples		();
	int32_t		getSamples	(DSPCOMPLEX *, int32_t, float);
	int32_t		getRate		();

	bool		isWorking	();
protected:
virtual void		run		();
	QString		f;
	RingBuffer<float>	*_I_Buffer;
	QString		fileName;
	int32_t		readBuffer	(float *data, int32_t length);
	SNDFILE		*filePointer;
	bool		readerOK;
	int32_t		sampleRate;
	int32_t		currPos;
	int16_t		numofChannels;
	bool		ExitCondition;
	bool		ThreadFinished;
	bool		readerPausing;
	int32_t		inputRate;
	int32_t		samplesinFile;
};
#endif

