#
/*
 *    Copyright (C) 2009, 2010, 2011
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the fm receiver
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PA_READER__
#define	__PA_READER__

#include	<QObject>
#include	<portaudio.h>
#include	"virtual-reader.h"


class	QComboBox;

//class  cardReader: public QObject, public virtualReader {
//Q_OBJECT
class	cardReader: public virtualReader {
public:
	 		cardReader	(RingBuffer<std::complex<float>> *,
	                                 QComboBox *);
			~cardReader	(void);
	void		restartReader	(int32_t);
	void		stopReader	(void);
	int16_t		bitDepth	(void);
//
//	particular to the cardreader
	bool		worksFine		(void);
	bool		set_streamSelector	(int);
private:
//	for setting up
	int16_t		numberofDevices		(void);
	int16_t		invalidDevice		(void);
	bool		isValidDevice		(int16_t);
//	operational control
	bool		selectDevice		(int16_t);
	bool		setupChannels		(QComboBox *);
	bool		InputrateIsSupported	(int16_t, int32_t);
const	char		*inputChannelwithRate	(int16_t, int32_t);

	int32_t		inputRate;
	int32_t		portAudio;
	int16_t		numofDevices;
	int		paCallbackReturn;
	int16_t		bufSize;
	PaStream	*istream;

	PaStreamParameters	inputParameters;
	bool		readerRunning;
	int16_t		*inTable;
	QComboBox	*streamSelector;
protected:
static	int	paCallback_i	(const void *input,
	                         void	    *output,
	                         unsigned long framesperBuffer,
	                         const PaStreamCallbackTimeInfo *timeInfo,
	                         PaStreamCallbackFlags statusFlags,
	                         void	    *userData);
};

#endif
