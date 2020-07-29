#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
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

#ifndef __PLUTO_HANDLER__
#define	__PLUTO_HANDLER__

#include	<QtNetwork>
#include        <QMessageBox>
#include        <QByteArray>
#include	<QObject>
#include	<QFrame>
#include	<QSettings>
#include	<atomic>
#include	<iio.h>
#include	"fm-constants.h"
#include	"ringbuffer.h"
#include	"device-handler.h"
#include	"ui_pluto-widget.h"


#ifndef	PLUTO_RATE
#define	PLUTO_RATE	2112000
#define	CONV_SIZE	(PLUTO_RATE / DIVIDER)
#endif

class	plutoHandler: public deviceHandler, public Ui_plutoWidget {
Q_OBJECT
public:
			plutoHandler		(QSettings *);
            		~plutoHandler		();
	void		setVFOFrequency		(int32_t);
	int32_t		getVFOFrequency		();
	bool		legalFrequency		(int32_t);
	int32_t		defaultFrequency	();

	bool		restartReader		();
	void		stopReader		();
	int32_t		getSamples		(std::complex<float> *,
	                                                          int32_t);
	int32_t		getSamples		(std::complex<float> *,
	                                                    int32_t, uint8_t);
	int32_t		Samples			();
	int32_t		getRate			();
	uint8_t		myIdentity		();

	void		resetBuffer		();
	int16_t		bitDepth		();

private:
	QFrame			myFrame;
	RingBuffer<std::complex<float>>	_I_Buffer;
	QSettings		*plutoSettings;

	void			run		();
	int32_t			inputRate;
	int32_t			vfoFrequency;
	std::atomic<bool>	running;
	bool			debugFlag;
//      configuration items
        int64_t                 bw_hz; // Analog banwidth in Hz
        int64_t                 fs_hz; // Baseband sample rate in Hz
        int64_t                 lo_hz; // Local oscillator frequency in Hz
	bool			get_ad9361_stream_ch (struct iio_context *ctx,
                                                      struct iio_device *dev,
                                                      int chid,
	                                              struct iio_channel **);

        const char* rfport; // Port name
        struct  iio_channel     *lo_channel;
        struct  iio_channel     *gain_channel;
	struct	iio_device	*rx;
	struct	iio_context	*ctx;
	struct	iio_channel	*rx0_i;
	struct	iio_channel	*rx0_q;
	struct	iio_buffer	*rxbuf;
	bool			connected;
//
private slots:
	void		set_gainControl		(int);
	void		set_agcControl		(int);
	void		toggle_debugButton	();
};
#endif

