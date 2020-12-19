#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software. if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QSettings>
#include	<QMessageBox>
#include	<QTime>
#include	<QDate>
#include	<QLabel>
#include	<QDebug>
#include	"colibri-handler.h"

#ifdef __MINGW32__
#define GETPROCADDRESS  GetProcAddress
#else
#define GETPROCADDRESS  dlsym
#endif

	colibriHandler::colibriHandler  (QSettings *s):
	                                  _I_Buffer (4 * 1024 * 1024),
	                                  myFrame (nullptr) {
	colibriSettings		= s;
	setupUi (&myFrame);


#ifdef  __MINGW32__
        const char *libraryString = "colibrinano_lib.dll";
        Handle          = LoadLibrary ((wchar_t *)L"colibrinano_lib.dll");
#else
        const char *libraryString = "libcolibrinano_lib.so";
        Handle          = dlopen (libraryString, RTLD_NOW);
#endif

        if (Handle == nullptr) {
           QMessageBox::critical (nullptr, "colibri",
	                          tr("Failed to load colibrinano_lib.so"));
	   throw (21);
	}

	fprintf (stderr, "library is loaded\n");
	if (!loadFunctions ()) {
#ifdef __MINGW32__
           FreeLibrary (Handle);
#else
           dlclose (Handle);
#endif
           throw (21);
        }

	colibri_initialize ();

	fprintf (stderr, "... and initialized\n");
	uint32_t t_devices	= colibri_devices ();
	if (t_devices == 0) {
	   QMessageBox::critical (nullptr, "colibri",
	                          tr ("No device available\n"));
	   throw (22);
	}

	fprintf (stderr, "we found %d device(s)\n", t_devices);
//	set some defaults
	if (!colibri_open (&m_deskriptor, 0)) {
	   QMessageBox::warning (nullptr, "colibri",
	                         tr("Failed to open ColibriNANO!"));
	   throw (23);
        }

	fprintf (stderr, "and opening device 0 was ok\n");
        colibri_setFrequency (m_deskriptor, defaultFrequency ());

	this		-> lastFrequency	= defaultFrequency ();
	fprintf (stderr, "set on %d\n", lastFrequency);
	colibriSettings -> beginGroup ("colibriSettings");
	int gainSetting = colibriSettings -> value ("colibri-gain", 20). toInt ();
	gainSelector	-> setValue (gainSetting);
	colibriSettings -> endGroup ();
        colibri_setPream (m_deskriptor, gainSelector -> value () * 0.5 + -31.5);
	actualGain	-> display (gainSelector -> value () * 0.5 + -31.5);

//	and be prepared for future changes in the settings
	connect (gainSelector, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainControl (int)));
	connect (iqSwitchButton, SIGNAL (clicked ()),
	         this, SLOT (handle_iqSwitcher ()));

	iqSwitcher	= false;
	switchLabel	-> setText ("I/Q");
	nameLabel	-> setText (deviceName ());
	running. store (false);
}

	colibriHandler::~colibriHandler () {
	myFrame. hide ();
	stopReader	();
	colibri_finalize ();
	colibriSettings	-> beginGroup ("colibriSettings");
	colibriSettings	-> setValue ("colibri-gain", 
	                              gainSelector -> value ());
	colibriSettings	-> endGroup ();
}
//

void	colibriHandler::setVFOFrequency	(int32_t newFrequency) {
        colibri_setFrequency (m_deskriptor, newFrequency);
	this	-> lastFrequency	= newFrequency;
}

int32_t	colibriHandler::getVFOFrequency () {
	return this -> lastFrequency;
}

void	colibriHandler::set_gainControl	(int newGain) {
float	gainValue	= -31.5 + newGain * 0.5;
	if (gainValue <= 6) {
           colibri_setPream (m_deskriptor, newGain);
	   actualGain	-> display (gainSelector -> value () * 0.5 + -31.5);
	   fprintf (stderr, "gain set to %f\n", 
	                            gainSelector -> value () * 0.5 + -31.5);
	}
}

void	colibriHandler::handle_iqSwitcher	()  {
	iqSwitcher	= !iqSwitcher;
	if (iqSwitcher)
	   switchLabel	-> setText ("Q/I");
	else
	   switchLabel -> setText ("I/Q");
}

//static int cnt	= 0;
static
bool	the_callBackRx (std::complex<float> *buffer, uint32_t len,
	                               bool overload, void *ctx) {
colibriHandler *p = static_cast<colibriHandler *>(ctx);

	(void)overload;
	p -> _I_Buffer. putDataIntoBuffer (buffer, len);
//	cnt += len;
//	if (cnt > 1920000) {
//	   fprintf (stderr, "x");
//	   cnt = 0;
//	}
	return true;
}

bool	colibriHandler::restartReader	() {
	if (running. load())
	   return true;		// should not happen

	fprintf (stderr, "reader is going to start\n");
	colibri_start (m_deskriptor, (SampleRateIndex)Sr_1920kHz,
                        the_callBackRx,
	                this);
	running. store (true);
	return true;
}

void	colibriHandler::stopReader() {
	if (!running. load())
	   return;
	fprintf (stderr, "reader is going to stop\n");
	colibri_stop (m_deskriptor);
	running. store (false);
}

int32_t	colibriHandler::getSamples (std::complex<float> *V, int32_t size) { 
	if (iqSwitcher) {
	   std::complex<float> xx [size];
	   _I_Buffer. getDataFromBuffer (xx, size);
	   for (int i = 0; i < size; i ++)
	      V[i] = std::complex<float> (imag (xx [i]), real (xx [i]));
	   return size;
	}
	else
	   return _I_Buffer. getDataFromBuffer (V, size);
}

int32_t colibriHandler::getSamples	(std::complex<float> *V,
	                                  int32_t size, uint8_t M) {
	(void)M;
	return getSamples (V, size);
}

int32_t	colibriHandler::Samples () {
	return _I_Buffer. GetRingBufferReadAvailable ();
}

void	colibriHandler::resetBuffer() {
	_I_Buffer. FlushRingBuffer();
}

int16_t	colibriHandler::bitDepth () {
	return 24;
}

QString	colibriHandler::deviceName	() {
std::string s = colibri_information ();
	return QString (s. c_str ());
}

int colibriHandler::sampleRate (int index) {
    switch (index) {
        case Sr_48kHz: return 48000;
        case Sr_96kHz: return 96000;
        case Sr_192kHz: return 192000;
        case Sr_384kHz: return 384000;
        case Sr_768kHz: return 768000;
        case Sr_1536kHz: return 1536000;
        case Sr_1920kHz: return 1920000;
        case Sr_2560kHz: return 2560000;
        case Sr_3072kHz: return 3072000;

        default: break;
    }

    return 48000;
}

int32_t	colibriHandler::getRate	() {
	return 1920000;
}

uint8_t	colibriHandler::myIdentity	() {
	return COLIBRI;
}

bool	colibriHandler::legalFrequency	(int32_t freq) {
	(void) freq;
	return true;
}

int32_t	colibriHandler::defaultFrequency	() {
	return 94700000;
}

bool	colibriHandler::loadFunctions		() {
	m_initialize =
	     reinterpret_cast<pFunc1> (GETPROCADDRESS (Handle, "initialize"));
	if (m_initialize == nullptr)
	   return false;

	m_finalize =
	     reinterpret_cast<pFunc1> (GETPROCADDRESS (Handle, "finalize"));
	if (m_finalize == nullptr)
	   return false;

	m_version =
	     reinterpret_cast<pVersion> (GETPROCADDRESS (Handle, "version"));
	if (m_version == nullptr)
	   return false;

	m_information =
	    reinterpret_cast<pInformation>(GETPROCADDRESS (Handle, "information"));
	if (m_information == nullptr)
	   return false;

	m_devices =
	    reinterpret_cast<pDevices>(GETPROCADDRESS (Handle, "devices"));
	if (m_devices == nullptr)
	   return false;

	m_open = reinterpret_cast<pOpen>(GETPROCADDRESS (Handle, "open"));
	if (m_open == nullptr)
	   return false;

	m_close =
	       reinterpret_cast<pClose>(GETPROCADDRESS (Handle, "close"));
	if (m_close == nullptr)
	   return false;

	m_start =
	       reinterpret_cast<pStart>(GETPROCADDRESS (Handle, "start"));
	if (m_start == nullptr)
	   return false;

	m_stop = reinterpret_cast<pStop>(GETPROCADDRESS (Handle, "stop"));
	if (m_stop == nullptr)
	   return false;

	m_setPreamp =
	      reinterpret_cast<pSetPreamp>(GETPROCADDRESS (Handle, "setPream"));
	if (m_setPreamp == nullptr)
	   return false;

	m_setFrequency =
	      reinterpret_cast<pSetFrequency>(GETPROCADDRESS (Handle, "setFrequency"));
	if (m_setFrequency == nullptr)
	   return false;

	return true;
}

void colibriHandler::colibri_initialize () {
    if (m_initialize)
        m_initialize();
}

void colibriHandler::colibri_finalize () {
    if (m_finalize)
        m_finalize();
}

void colibriHandler::colibri_version (uint32_t &major,
	                              uint32_t &minor, uint32_t &patch) {
    if (m_version) {
        m_version (major, minor, patch);
    }
    else {
        major = 0;
        minor = 0;
        patch = 0;
    }
}

string colibriHandler::colibri_information () {
string t_str;

    if (m_information) {
        char *pStr = nullptr;
        m_information (&pStr);
        if (pStr)
            t_str = string (pStr);
    }

    return t_str;
}

uint32_t colibriHandler::colibri_devices () {
uint32_t t_count = 0;
    if (m_devices)
        m_devices(t_count);
    return t_count;
}

bool colibriHandler::colibri_open(Descriptor *pDev, const uint32_t devIndex) {
	if (m_open)
        return m_open(pDev, devIndex);
    return false;
}

void colibriHandler::colibri_close (Descriptor dev) {
	if (m_close)
	   m_close(dev);
}

bool colibriHandler::colibri_start (Descriptor dev,
	                            SampleRateIndex sr,
	                            pCallbackRx p, void *pUserData) {
	if (m_start)
	   return m_start(dev, sr, p, pUserData);
	return false;
}

bool colibriHandler::colibri_stop (Descriptor dev) {
	if (m_stop)
	   return m_stop(dev);
	return false;
}

bool colibriHandler::colibri_setPream (Descriptor dev, float value) {
	if (m_setPreamp)
	   return m_setPreamp (dev, value);
	return false;
}

bool colibriHandler::colibri_setFrequency (Descriptor dev, uint32_t value) {
	if (m_setFrequency)
	   return m_setFrequency(dev, value);
	return false;
}


