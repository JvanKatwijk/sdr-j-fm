
/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 *	recoding and taking parts for the airspyRadio interface
 *	for the SDR-J-FM receiver
 *	jan van Katwijk
 *	Lazy Chair Computing
 */
#ifndef __AIRSPY_RADIO__
#define	__AIRSPY_RADIO__

#include	<QObject>
#include	<QSettings>
#include	<QFrame>
#include	"fm-constants.h"
#include	"ringbuffer.h"
#include	"virtual-input.h"
#include	"ui_airspy-widget.h"
#include	"airspy_lib.h"

class airspyHandler: public virtualInput, public Ui_airspyWidget {
Q_OBJECT
public:
			airspyHandler (QSettings *, bool, bool *);
			~airspyHandler (void);
	void		setVFOFrequency (int32_t nf);
	int32_t		getVFOFrequency (void);
	bool		legalFrequency (int32_t f);
	int32_t		defaultFrequency (void);
	bool		restartReader	(void);
	void		stopReader (void);
	bool		status (void);
	int		setExternalRate (int nsr);
	void		resetBuffer (void);
	int16_t		bitDepth (void);
	int32_t		getRate (void);

	int32_t		getSamples (DSPCOMPLEX *v, int32_t size);
	int32_t		getSamples (DSPCOMPLEX  *V,
	                         int32_t size, uint8_t M);
	int32_t		Samples	(void);
	uint8_t		myIdentity (void);
private slots:
	void		set_rateSelector (const QString &s);
	void		set_lna_gain (int value);
	void		set_mixer_gain (int value);
	void		set_vga_gain (int value);
	void		set_lna_agc (void);
	void		set_mixer_agc (void);
	void		set_rf_bias (void);
private:
	QFrame		*myFrame;
	bool		success;
	bool		running;
	bool		lna_agc;
	bool		mixer_agc;
	bool		rf_bias;
const	char*		board_id_name (void);
	int16_t		vgaGain;
	int16_t		mixerGain;
	int16_t		lnaGain;
	
	QSettings	*airspySettings;
	RingBuffer<DSPCOMPLEX> *theBuffer;
	int32_t		inputRate;
	struct airspy_device* device;
	uint64_t 	serialNumber;
	char		serial[128];
    // callback buffer	
	int 		bs_;
	uint8_t 	*buffer;
	int		bl_;
static
	int		callback(airspy_transfer_t *);
	int		data_available (void *buf, int buf_size);
const	char *		getSerial (void);
	int	open (void);
};

#endif
