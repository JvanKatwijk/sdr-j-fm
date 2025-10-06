#
/*
 *    Copyright (C) 2010, 2011, 2012, 2013
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 	This particular driver is a very simple wrapper around the
 * 	librtlsdr.  In order to keep things simple, we dynamically
 * 	load the dll (or .so). The librtlsdr is osmocom software and all rights
 * 	are greatly acknowledged
 */


#include	<QThread>
#include	"rtl-sdr.h"
#include	"rtlsdr-handler.h"

#define	READLEN_DEFAULT	8192
//
//	For the callback, we do need some environment which
//	is passed through the ctx parameter
//
//	This is the user-side call back function
//	ctx is the calling task
static
void	RTLSDRCallBack (uint8_t *buf, uint32_t len, void *ctx) {
rtlsdrHandler	*theStick	= (rtlsdrHandler *)ctx;
int32_t	tmp;

	if ((theStick == nullptr) || (len != READLEN_DEFAULT))
	   return;

	tmp = theStick -> _I_Buffer. putDataIntoBuffer (buf, len);
	if ((len - tmp) > 0)
	   theStick	-> sampleCounter += len - tmp;
}
//
//	for handling the events in libusb, we need a controlthread
//	whose sole purpose is to process the rtlsdr_read_async function
//	from the lib.
class	dll_driver : public QThread {
private:
	rtlsdrHandler	*theStick;
public:

	dll_driver (rtlsdrHandler *d) {
	theStick	= d;
	start ();
	}

	~dll_driver () {
	}

private:
virtual void	run (void) {
	(theStick -> rtlsdr_read_async) (theStick -> device,
	                          (rtlsdr_read_async_cb_t)&RTLSDRCallBack,
	                          (void *)theStick,
	                          0,
	                          READLEN_DEFAULT);
	}
};
//
//	Our wrapper is a simple classs
	rtlsdrHandler::rtlsdrHandler (QSettings *s):
	                                 _I_Buffer (32 * 32768),
	                                 myFrame (nullptr) {
int16_t	deviceCount;
int32_t	r;
int16_t	deviceIndex;
QString	h;

	dabSettings		= s;
	setupUi (& myFrame);
	this	-> myFrame. show ();

	inputRate	=  Khz (2304);
//
	this	-> sampleCounter	= 0;
	this	-> vfoOffset		= 0;

#ifdef	__MINGW32__
	const char *libraryString = "rtlsdr.dll";
#else
	const char *libraryString = "librtlsdr.so";
#endif
	
        library_p	= new QLibrary (libraryString);
        library_p	-> load ();
        if (!library_p -> isLoaded ()) {
           throw (device_exception ("failed to open " +
                                        std::string (libraryString)));
        }

        if (!load_rtlFunctions ()) {
           delete library_p;
           throw (device_exception ("could not find one or more library functions"));
        }

	if (!load_rtlFunctions ()) {
	   delete library_p;
	   throw (22);
	}
//
//	Ok, from here we have the library functions accessible
	deviceCount 		= this -> rtlsdr_get_device_count ();
	if (deviceCount == 0) {
	   fprintf (stderr, "No supported RTLSDR devices found\n");
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (24);
	}

	deviceIndex = 0;	// default
	if (deviceCount > 1) {
	   dongleSelector	= new dongleSelect ();
	   for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex ++) {
	      dongleSelector ->
	           addtoDongleList (rtlsdr_get_device_name (deviceIndex));
	   }
	   deviceIndex = dongleSelector -> QDialog::exec ();
	   delete dongleSelector;
	}
//
//	OK, now open the hardware
	r			= this -> rtlsdr_open (&device, deviceIndex);
	if (r < 0) {
	   fprintf (stderr, "Opening dabstick failed\n");
	   this -> rtlsdr_close (device);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (25);
	}
	r			= this -> rtlsdr_set_sample_rate (device,
	                                                          inputRate);
	if (r < 0) {
	   fprintf (stderr, "Setting samplerate failed\n");
	   throw (26);
	}

	r			= this -> rtlsdr_get_sample_rate (device);
	fprintf (stderr, "samplerate set to %d\n", r);

	gainsCount = rtlsdr_get_tuner_gains (device, nullptr);
	{  int ll [gainsCount];
	   gains. resize (gainsCount);
	   (void)rtlsdr_get_tuner_gains (device, ll);
	   gainSlider	-> setMaximum (gainsCount);
	   for (int i = gainsCount; i > 0; i--) {
	      fprintf (stderr, "%.1f ", ll [i - 1] / 10.0);
	      gains [i - 1] = ll [i - 1];
	   }
	}
	rtlsdr_set_tuner_gain_mode (device, 1);
	rtlsdr_set_tuner_gain (device, gains [gainsCount / 2]);

	workerHandle		= nullptr;
	connect (gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainSlider (int)));
	connect (agcChecker, SIGNAL (stateChanged (int)),
	         this, SLOT (set_Agc (int)));
	connect (f_correction, SIGNAL (valueChanged (int)),
	         this, SLOT (freqCorrection  (int)));
	connect (KhzOffset, SIGNAL (valueChanged (int)),
	         this, SLOT (setKhzOffset (int)));
	connect (HzOffset, SIGNAL (valueChanged (int)),
	         this, SLOT (setHzOffset (int)));
	connect (biasT_Control, SIGNAL (stateChanged (int)),
	         this, SLOT (set_biasControl (int)));

	dabSettings	-> beginGroup ("dabstick");
	gainSlider -> setValue (dabSettings -> value ("gainSlider", 10). toInt ());
	f_correction -> setValue (dabSettings -> value ("f_correction", 0). toInt ());
	KhzOffset	-> setValue (dabSettings -> value ("KhzOffset", 0). toInt ());
	HzOffset	-> setValue (dabSettings -> value ("HzOffset", 0). toInt ());
	dabSettings	-> endGroup ();
}

	rtlsdrHandler::~rtlsdrHandler	() {
	this -> rtlsdr_close (device);
	dabSettings	-> beginGroup ("dabstick");
	dabSettings	-> setValue ("gainSlider", gainSlider -> value ());
	dabSettings	-> setValue ("f_correction", f_correction -> value ());
	dabSettings	-> setValue ("KhzOffset", KhzOffset -> value ());
	dabSettings	-> setValue ("HzOffset", HzOffset -> value ());
	dabSettings	-> endGroup ();
	myFrame. hide ();
	delete library_p;
}

void	rtlsdrHandler::setVFOFrequency	(int32_t f) {
	(void)(this -> rtlsdr_set_center_freq (device, f + vfoOffset));
}

int32_t	rtlsdrHandler::getVFOFrequency	() {
	return (int32_t)(this -> rtlsdr_get_center_freq (device)) - vfoOffset;
}

bool	rtlsdrHandler::legalFrequency (int32_t f) {
	return  Mhz (24) <= f && f <= Mhz (1800);
}

int32_t	rtlsdrHandler::defaultFrequency	() {
	return Khz (94700);
}
//
bool	rtlsdrHandler::restartReader	() {
int32_t	r;

	if (workerHandle != nullptr)
	   return true;

	_I_Buffer. FlushRingBuffer ();
	r = this -> rtlsdr_reset_buffer (device);
	if (r < 0)
	   return false;

	this -> rtlsdr_set_center_freq (device, 
	                   (int32_t)(this -> rtlsdr_get_center_freq (device)) +
	                                 vfoOffset);
	workerHandle	= new dll_driver (this);
	return true;
}

void	rtlsdrHandler::stopReader	() {
	if (workerHandle == nullptr)
	   return;

	this -> rtlsdr_cancel_async (device);
	if (workerHandle != nullptr) {
	   while (!workerHandle -> isFinished ()) 
	      usleep (100);

	   delete	workerHandle;
	}

	workerHandle	= nullptr;
}
//

void	rtlsdrHandler::set_gainSlider	(int gain) {
static int	oldGain	= 0;

	if (gain == oldGain)
	   return;
	if ((gain < 0) || (gain >= gainsCount))
	   return;

	oldGain	= gain;
	rtlsdr_set_tuner_gain (device, gains [gainsCount - gain]);
	showGain	-> display (gainsCount - gain);
}
//
//	correction is in Hz
void	rtlsdrHandler::freqCorrection	(int32_t ppm) {
	this -> rtlsdr_set_freq_correction (device, ppm);
}

//
//	The brave old getSamples. For the dab stick, we get
//	size: still in I/Q pairs, but we have to convert the data from
//	uint8_t to DSPCOMPLEX *
int32_t	rtlsdrHandler::getSamples (DSPCOMPLEX *V, int32_t size) { 
int32_t	amount, i;
uint8_t	*tempBuffer = (uint8_t *)alloca (2 * size * sizeof (uint8_t));
//
	amount = _I_Buffer. getDataFromBuffer (tempBuffer, 2 * size);
	for (i = 0; i < amount / 2; i ++)
	    V [i] = DSPCOMPLEX ((float (tempBuffer [2 * i] - 127)) / 128.0,
	                        (float (tempBuffer [2 * i + 1] - 127)) / 128.0);
	return amount / 2;
}

int32_t rtlsdrHandler::getSamples (std::complex<float> *V, int32_t size, uint8_t M) {
	(void)M;
	return getSamples (V, size);
}

int32_t	rtlsdrHandler::Samples	() {
	return _I_Buffer. GetRingBufferReadAvailable () / 2;
}
//
uint8_t	rtlsdrHandler::myIdentity	() {
	return DAB_STICK;
}

//	vfoOffset is in Hz, we have two spinboxes influencing the
//	settings
void	rtlsdrHandler::setKhzOffset	(int k) {
	vfoOffset	= vfoOffset % Khz (1) + Khz (k);
}

void	rtlsdrHandler::setHzOffset	(int h) {
	vfoOffset	= KHz (1) * vfoOffset / KHz (1) + h;
}

bool	rtlsdrHandler::load_rtlFunctions () {
//
//	link the required procedures
	rtlsdr_open	= (pfnrtlsdr_open)
	                       library_p -> resolve ("rtlsdr_open");
	if (rtlsdr_open == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_open\n");
	   return false;
	}
	rtlsdr_close	= (pfnrtlsdr_close)
	                     library_p -> resolve ("rtlsdr_close");
	if (rtlsdr_close == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_close\n");
	   return false;
	}

	rtlsdr_set_sample_rate =
	    (pfnrtlsdr_set_sample_rate)
	                     library_p -> resolve ("rtlsdr_set_sample_rate");
	if (rtlsdr_set_sample_rate == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_sample_rate\n");
	   return false;
	}

	rtlsdr_get_sample_rate	=
	    (pfnrtlsdr_get_sample_rate)
	                      library_p -> resolve ("rtlsdr_get_sample_rate");
	if (rtlsdr_get_sample_rate == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_sample_rate\n");
	   return false;
	}

	rtlsdr_set_agc_mode =
	    (pfnrtlsdr_set_agc_mode)
	                      library_p -> resolve ("rtlsdr_set_agc_mode");
	if (rtlsdr_set_agc_mode == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_agc_mode\n");
	   return false;
	}

	rtlsdr_get_tuner_gains		= (pfnrtlsdr_get_tuner_gains)
	                      library_p -> resolve ("rtlsdr_get_tuner_gains");
	if (rtlsdr_get_tuner_gains == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_tuner_gains\n");
	   return false;
	}

	rtlsdr_set_tuner_gain_mode	= (pfnrtlsdr_set_tuner_gain_mode)
	                      library_p -> resolve ("rtlsdr_set_tuner_gain_mode");
	if (rtlsdr_set_tuner_gain_mode == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_tuner_gain_mode\n");
	   return false;
	}

	rtlsdr_set_tuner_gain	= (pfnrtlsdr_set_tuner_gain)
	                     library_p -> resolve ("rtlsdr_set_tuner_gain");
	if (rtlsdr_set_tuner_gain == nullptr) {
	   fprintf (stderr, "Cound not find rtlsdr_set_tuner_gain\n");
	   return false;
	}

	rtlsdr_get_tuner_gain	= (pfnrtlsdr_get_tuner_gain)
	                     library_p -> resolve ("rtlsdr_get_tuner_gain");
	if (rtlsdr_get_tuner_gain == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_tuner_gain\n");
	   return false;
	}
	rtlsdr_set_center_freq	= (pfnrtlsdr_set_center_freq)
	                     library_p -> resolve ("rtlsdr_set_center_freq");
	if (rtlsdr_set_center_freq == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_center_freq\n");
	   return false;
	}

	rtlsdr_get_center_freq	= (pfnrtlsdr_get_center_freq)
	                     library_p -> resolve ("rtlsdr_get_center_freq");
	if (rtlsdr_get_center_freq == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_center_freq\n");
	   return false;
	}

	rtlsdr_reset_buffer	= (pfnrtlsdr_reset_buffer)
	                     library_p -> resolve ("rtlsdr_reset_buffer");
	if (rtlsdr_reset_buffer == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_reset_buffer\n");
	   return false;
	}

	rtlsdr_read_async	= (pfnrtlsdr_read_async)
	                     library_p -> resolve ("rtlsdr_read_async");
	if (rtlsdr_read_async == nullptr) {
	   fprintf (stderr, "Cound not find rtlsdr_read_async\n");
	   return false;
	}

	rtlsdr_get_device_count	= (pfnrtlsdr_get_device_count)
	                     library_p -> resolve ("rtlsdr_get_device_count");
	if (rtlsdr_get_device_count == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_device_count\n");
	   return false;
	}

	rtlsdr_cancel_async	= (pfnrtlsdr_cancel_async)
	                     library_p -> resolve ("rtlsdr_cancel_async");
	if (rtlsdr_cancel_async == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_cancel_async\n");
	   return false;
	}

	rtlsdr_set_direct_sampling = (pfnrtlsdr_set_direct_sampling)
	                     library_p -> resolve ("rtlsdr_set_direct_sampling");
	if (rtlsdr_set_direct_sampling == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_direct_sampling\n");
	   return false;
	}

	rtlsdr_set_freq_correction = (pfnrtlsdr_set_freq_correction)
	                    library_p -> resolve ("rtlsdr_set_freq_correction");
	if (rtlsdr_set_freq_correction == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_set_freq_correction\n");
	   return false;
	}
	
	rtlsdr_get_device_name = (pfnrtlsdr_get_device_name)
	                  library_p -> resolve ("rtlsdr_get_device_name");
	if (rtlsdr_get_device_name == nullptr) {
	   fprintf (stderr, "Could not find rtlsdr_get_device_name\n");
	   return false;
	}

	rtlsdr_set_bias_tee =
                   (pfnrtlsdr_set_bias_tee)
	                   library_p -> resolve ("rtlsdr_set_bias_tee");
        if (rtlsdr_set_bias_tee == nullptr) {
	   fprintf (stderr, "Could not resolve rtlsdr_set_bias_tee\n");
	}

	fprintf (stderr, "OK, functions seem to be loaded\n");
	return true;
}

void	rtlsdrHandler::resetBuffer () {
	_I_Buffer. FlushRingBuffer ();
}

int16_t	rtlsdrHandler::bitDepth	() {
	return 8;
}

int32_t	rtlsdrHandler::getRate	() {
	return inputRate;
}

void	rtlsdrHandler::set_Agc	(int state) {
	(void)state;
	if (agcChecker -> isChecked ())
	   (void)rtlsdr_set_agc_mode (device, 1);
	else
	   (void)rtlsdr_set_agc_mode (device, 0);
}

void    rtlsdrHandler::set_biasControl  (int dummy) {
        (void)dummy;
        if (rtlsdr_set_bias_tee != nullptr)
           rtlsdr_set_bias_tee (device, biasT_Control -> isChecked () ? 1 : 0);
}       

