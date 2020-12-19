#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm softwware is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software. If not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__COLIBRI_HANDLER_H
#define	__COLIBRI_HANDLER_H
#include	<QSettings>
#include	<QTime>
#include	<QDate>
#include	<QLabel>
#include	<QDebug>
#include	"ui_colibri-widget.h"
#include	"ringbuffer.h"
#include	"device-handler.h"


#ifdef __MINGW32__
#  define COLIBRI_NANO_API __stdcall
#else
#  define COLIBRI_NANO_API
#endif
typedef void* Descriptor;

typedef enum {
    Sr_48kHz = 0,
    Sr_96kHz,
    Sr_192kHz,
    Sr_384kHz,
    Sr_768kHz,
    Sr_1536kHz,
    Sr_1920kHz,
    Sr_2560kHz,
    Sr_3072kHz,
} SampleRateIndex;

typedef bool (*pCallbackRx)(std::complex<float> *, uint32_t, bool, void *);


typedef void (COLIBRI_NANO_API *pVersion)(uint32_t&, uint32_t&, uint32_t&);
typedef void (COLIBRI_NANO_API *pInformation)(char**);
typedef void (COLIBRI_NANO_API *pDevices)(uint32_t&);
typedef void (COLIBRI_NANO_API *pFunc1)(void);
typedef bool (COLIBRI_NANO_API *pOpen)(Descriptor*, const uint32_t);
typedef void (COLIBRI_NANO_API *pClose)(Descriptor);
typedef bool (COLIBRI_NANO_API *pStart)(Descriptor, SampleRateIndex, pCallbackRx, void*);
typedef bool (COLIBRI_NANO_API *pStop)(Descriptor);
typedef bool (COLIBRI_NANO_API *pSetPreamp)(Descriptor, float);
typedef bool (COLIBRI_NANO_API *pSetFrequency)(Descriptor, uint32_t);

class	colibriHandler: public deviceHandler, public Ui_colibriWidget {
Q_OBJECT
public:

			colibriHandler		(QSettings *);
			~colibriHandler		(void);
	void		setVFOFrequency		(int32_t);
	int32_t		getVFOFrequency		(void);
	int32_t		defaultFrequency	();
	bool		legalFrequency		(int32_t);
	uint8_t		myIdentity		();
	bool		restartReader		();
	void		stopReader		();
	int32_t		getSamples		(std::complex<float> *, int32_t);
	int32_t		getSamples		(std::complex<float> *, int32_t, uint8_t);
	int32_t		Samples			();
	void		resetBuffer		(void);
	int16_t		bitDepth		(void);
	int32_t		getRate			();
	QString		deviceName		();
	RingBuffer<std::complex<float>>	_I_Buffer;
	int16_t		convBufferSize;
	int16_t		convIndex;
	std::vector <complex<float> >   convBuffer;
	int16_t		mapTable_int   [2048];
	float		mapTable_float [2048];
private:
	QFrame			myFrame;
	QSettings		*colibriSettings;
	int			sampleRate		(int);
	bool			loadFunctions		();
	HINSTANCE		Handle;
	Descriptor		m_deskriptor;
	std::atomic<bool>	running;
	bool			iqSwitcher;
	pVersion		m_version;
	pInformation		m_information;
	pDevices		m_devices;
	pFunc1			m_initialize;
	pFunc1			m_finalize;
	pOpen			m_open;
	pClose			m_close;
	pStart			m_start;
	pStop			m_stop;
	pSetPreamp		m_setPreamp;
	pSetFrequency		m_setFrequency;
//
//	and the wrappers
	void			colibri_initialize	();
	void			colibri_finalize	();
	void			colibri_version		(uint32_t &major,
	                                         uint32_t &minor,
	                                         uint32_t &patch);
	string			colibri_information	();
	uint32_t		colibri_devices		();
	bool			colibri_open		(Descriptor *pDev,
	                                         const uint32_t devIndex);
	void			colibri_close		(Descriptor dev);
	bool			colibri_start		(Descriptor dev,
	                                         SampleRateIndex sr,
	                                         pCallbackRx p,
	                                         void *pUserData);
	bool			colibri_stop		(Descriptor dev);
	bool			colibri_setPream	(Descriptor dev, float value);
	bool			colibri_setFrequency	(Descriptor dev,
	                                                uint32_t value);

private slots:
	void			set_gainControl	(int);
	void			handle_iqSwitcher	();
};
#endif
