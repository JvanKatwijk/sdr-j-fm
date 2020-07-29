#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of sdr-j-fm
 *
 *    sdr-j-fm is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    sdr-j-fm is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-fm if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QThread>
#include	<QSettings>
#include	<QFileDialog>
#include	<QTime>
#include	<QDate>
#include	<QLabel>
#include	<QDebug>
#include	<QFileDialog>
#include	"pluto-handler.h"
#include	"ad9361.h"

/* static scratch mem for strings */
static char tmpstr[64];

/* helper function generating channel names */
static
char*	get_ch_name (const char* type, int id) {
        snprintf (tmpstr, sizeof(tmpstr), "%s%d", type, id);
        return tmpstr;
}

static 
QString get_ch_name (QString type, int id) {
QString result = type;
	result. append (QString::number (id));
	return result;
}

/* finds AD9361 streaming IIO channels */
bool	plutoHandler::
	      get_ad9361_stream_ch (struct iio_context *ctx,
	                            struct iio_device *dev,
	                            int chid, struct iio_channel **chn) {
	(void)ctx;
        *chn = iio_device_find_channel (dev, get_ch_name ("voltage", chid),
	                                                       false);
        if (*chn == nullptr)
	   *chn = iio_device_find_channel (dev,
	                                   get_ch_name ("altvoltage", chid),
	                                   false);
        return *chn != nullptr;
}

	plutoHandler::plutoHandler  (QSettings *s):
	                                  myFrame (nullptr),
	                                  _I_Buffer (4 * 1024 * 1024) {
struct iio_channel *chn		= nullptr;
struct iio_device *phys_dev	= nullptr;

	plutoSettings			= s;
	setupUi (&myFrame);
	myFrame. show	();

	this	-> ctx			= nullptr;
	this	-> rxbuf		= nullptr;
	this	-> rx0_i		= nullptr;
	this	-> rx0_q		= nullptr;

	this	-> bw_hz		= PLUTO_RATE;
	this	-> fs_hz		= PLUTO_RATE;
	this	-> lo_hz		= 94700000;
	this	-> rfport		= "A_BALANCED";

	plutoSettings	-> beginGroup ("plutoSettings");
	bool agcMode	=
	             plutoSettings -> value ("pluto-agc", 0). toInt () == 1;
	int  gainValue	=
	             plutoSettings -> value ("pluto-gain", 50). toInt ();
	debugFlag	=
	             plutoSettings -> value ("pluto-debug", 0). toInt () == 1;
	plutoSettings	-> endGroup ();

	if (debugFlag)
	   debugButton	-> setText ("debug on");
	if (agcMode) {
	   agcControl	-> setChecked (true);	
	   gainControl	-> hide ();
	}
	gainControl	-> setValue (gainValue);
	   
	state	-> setText ("Looking for context");
	ctx	= iio_create_default_context ();
	if (ctx == nullptr) {
	   if (debugFlag)
	      fprintf (stderr, "default context failed\n");
	   ctx = iio_create_local_context ();
	}

	if (ctx == nullptr) {
	   if (debugFlag)
	      fprintf (stderr, "creating local context failed\n");
	   ctx = iio_create_network_context ("pluto.local");
	}

	if (ctx == nullptr) {
	   if (debugFlag)
	      fprintf (stderr, "creating network context with pluto.local failed\n");
	   ctx = iio_create_network_context ("192.168.2.1");
	}

	if (ctx == nullptr) {
	   fprintf (stderr, "No pluto found, fatal\n");
	   throw (24);
	}

	if (debugFlag)
	   fprintf (stderr, "context name found %s\n",
	                            iio_context_get_name (ctx));

	state	-> setText ("Context created,\n looking for devices");
	if (iio_context_get_devices_count (ctx) <= 0) {
	   state -> setText ("no devices found, fatal");
	   iio_context_destroy	(ctx);
	   throw (25);
	}

	rx = iio_context_find_device (ctx, "cf-ad9361-lpc");
	if (rx == nullptr) {
	   state -> setText ("No device found");
	   iio_context_destroy (ctx);
	   throw (26);
	}
	contextName     -> setText (iio_context_get_name (ctx));
	name_of_device  -> setText (iio_device_get_name (rx));

        state -> setText ("* Configuring AD9361 for streaming\n");

// Configure phy and lo channels
	if (debugFlag)
	   fprintf (stderr, "* Acquiring AD9361 phy channel %d\n", 0);
	phys_dev = iio_context_find_device (ctx, "ad9361-phy");
	if (phys_dev == nullptr) {
	   if (debugFlag) 
	      fprintf (stderr, "no ad9361 found\n");
	   iio_context_destroy (ctx);
	   throw (27);
	}

	chn = iio_device_find_channel (phys_dev,
                                       get_ch_name (QString ("voltage"), 0).
	                                                  toLatin1 (). data (),
                                       false);
	if (chn == nullptr) {
	   if (debugFlag)
	      fprintf (stderr, "cannot acquire phy channel %d\n", 0);
	   iio_context_destroy (ctx);
	   throw (27);
	}

	int res = iio_channel_attr_write (chn, "rf_port_select",
	                                               this -> rfport);
	if (res < 0) {
	   if (debugFlag) {
	      char error [255];
	      iio_strerror (res, error, 255); 
	      fprintf (stderr, "error in port selection %s\n", error);
	   }
	   iio_context_destroy (ctx);
	   throw (28);
	}

	res = iio_channel_attr_write_longlong (chn,
	                                       "rf_bandwidth",
	                                       this -> bw_hz);
	if (res < 0) {
	   if (debugFlag) {
	      char errorText [255];
	      iio_strerror (res, errorText, 255); 
	      fprintf (stderr, "cannot select bandwidth %s\n", errorText);
	   }
	   iio_context_destroy (ctx);
	   throw (29);
	}

	res = iio_channel_attr_write_longlong (chn, "sampling_frequency",
	                                              this -> fs_hz);
	if (res < 0) {
	   if (debugFlag) {
	      char errorText [255];
	      iio_strerror (res, errorText, 255); 
	      fprintf (stderr, "cannot set sampling frequency %s\n", errorText);
	   }
	   iio_context_destroy (ctx);
	   throw (30);
	}

	this	-> gain_channel = chn;

// Configure LO channel
	if (debugFlag)
	   fprintf (stderr, "* Acquiring AD9361 %s lo channel\n", "RX");
	phys_dev = iio_context_find_device (ctx, "ad9361-phy");
//
	this -> lo_channel =
	             iio_device_find_channel (phys_dev,
                                              get_ch_name ("altvoltage", 0),
                                              true);
	if (this -> lo_channel == nullptr) {
	   if (debugFlag)
	      fprintf (stderr, "cannot find lo for channel\n");
	   iio_context_destroy (ctx);
	   throw (31);
	}

	res = iio_channel_attr_write_longlong (this -> lo_channel,
	                                               "frequency",
	                                               this -> lo_hz);
	if (res < 0 ) {
	   if (debugFlag) {
	      char error [255];
	      iio_strerror (res, error, 255); 
	      fprintf (stderr, "cannot set local oscillator frequency %s\n",
	                                                           error);
	   }
	   iio_context_destroy (ctx);
	   throw (32);
	}

	state -> setText ("* Initializing AD9361 IIO streaming channels\n");
        if (!get_ad9361_stream_ch (ctx, rx, 0, &rx0_i)) {
	   if (debugFlag)
	      fprintf (stderr, "Rx chan i not found\n");
	   iio_context_destroy (ctx);
	   throw (33);
	}

        if (!get_ad9361_stream_ch (ctx, rx, 1, &rx0_q)) {
	   if (debugFlag)
              fprintf (stderr, "Rx chan i not found\n");
           iio_context_destroy (ctx);
           throw (34);
	}

        state -> setText ("* Enabling IIO streaming channels");
        iio_channel_enable (rx0_i);
        iio_channel_enable (rx0_q);

        state -> setText ("* Creating non-cyclic IIO buffers with 1 MiS\n");
        rxbuf	= iio_device_create_buffer (rx, 256*1024, false);
	if (rxbuf == nullptr) {
	   if (debugFlag) 
	      fprintf (stderr, "could not create RX buffer, fatal\n");
	   iio_context_destroy (ctx);
	   throw (35);
	}
//
	iio_buffer_set_blocking_mode (rxbuf, true);
	if (!agcMode) {
	   int ret = iio_channel_attr_write (this -> gain_channel,
	                                             "gain_control_mode",
	                                             "manual");
	   ret = iio_channel_attr_write_longlong (this -> gain_channel,
	                                                  "hardwaregain",
	                                                  gainValue);
	   if (ret < 0) 
	      state -> setText ("error in initial gain setting");
	}
	else {
	   int ret = iio_channel_attr_write (this -> gain_channel,
	                                             "gain_control_mode",
	                                             "slow_attack");
	   if (ret < 0)
	      state -> setText ("error in initial gain setting");
	}
	
//	and be prepared for future changes in the settings
	connect (gainControl, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainControl (int)));
	connect (agcControl, SIGNAL (stateChanged (int)),
	         this, SLOT (set_agcControl (int)));
	connect (debugButton, SIGNAL (clicked ()),
	         this, SLOT (toggle_debugButton ()));

	running. store (false);

        float Fpass     = 1950000 / 2;
        float Fstop     = Fpass * 1.5;
        float wnomTX    = 1.0 * Fstop;  // dummy here
        float wnomRX    = PLUTO_RATE;      // RF bandwidth of analog filter
        int ret = ad9361_set_bb_rate_custom_filter_manual (phys_dev, PLUTO_RATE,
                                                           Fpass, Fstop,
                                                           wnomTX, wnomRX);
        if (ret < 0)
           fprintf (stderr, "mislukt, error code %d\n", ret);

	connected	= true;
	state -> setText ("ready to go");
}

	plutoHandler::~plutoHandler() {
	myFrame. hide ();
	plutoSettings	-> beginGroup ("plutoSettings");
	plutoSettings	-> setValue ("pluto-agcMode",
	                              agcControl -> isChecked () ? 1 : 0);
	plutoSettings	-> setValue ("pluto-gain", 
	                              gainControl -> value ());
	plutoSettings	-> setValue ("pluto-debug", debugFlag ? 1 : 0);
	plutoSettings	-> endGroup ();
	if (!connected)		// should not happen
	   return;
	stopReader();
	iio_buffer_destroy (rxbuf);
	iio_context_destroy (ctx);
}
//

void	plutoHandler::setVFOFrequency	(int32_t newFrequency) {
int	ret;
	this	-> lo_hz = newFrequency;
	ret	= iio_channel_attr_write_longlong (this -> lo_channel,
	                                           "frequency",
	                                           this -> lo_hz);
	if (ret < 0) {
	   fprintf (stderr, "cannot set local oscillator frequency\n");
	}
	if (debugFlag)
	   fprintf (stderr, "frequency set to %d\n",
	                                 (int)(this -> lo_hz));
}

int32_t	plutoHandler::getVFOFrequency () {
	return this -> lo_hz;
}
//
//	If the agc is set, but someone touches the gain button
//	the agc is switched off. Btw, this is hypothetically
//	since the gain control is made invisible when the
//	agc is set
void	plutoHandler::set_gainControl	(int newGain) {
int ret;

	if (agcControl -> isChecked ()) {
	   ret = iio_channel_attr_write (this -> gain_channel,
	                                         "gain_control_mode",
	                                         "manual");
	   if (ret < 0) {
	      state -> setText ("error in gain setting");
	      if (debugFlag)
	         fprintf (stderr, "could not change gain control to manual");
	      return;
	   }
	   
	   disconnect (agcControl, SIGNAL (stateChanged (int)),
	         this, SLOT (set_agcControl (int)));
	   agcControl -> setChecked (false);
	   connect (agcControl, SIGNAL (stateChanged (int)),
	         this, SLOT (set_agcControl (int)));
	}

	ret = iio_channel_attr_write_longlong (this -> gain_channel,
	                                       "hardwaregain",
	                                       newGain);
	if (ret < 0) {
	   state -> setText ("error in gain setting");
	   if (debugFlag) 
	      fprintf (stderr,
	               "could not set hardware gain to %d\n", newGain);
	}
}

void	plutoHandler::set_agcControl	(int dummy) {
int ret;

	(void)dummy;
	if (agcControl -> isChecked ()) {
	   ret = iio_channel_attr_write (this -> gain_channel,
	                                         "gain_control_mode",
	                                         "slow_attack");
	   if (ret < 0) {
	      if (debugFlag)
	         fprintf (stderr, "error in setting agc\n");
	      return;
	   }
	   else
	      state -> setText ("agc set");
	   gainControl -> hide ();
	}
	else {	// switch agc off
	   ret = iio_channel_attr_write (this -> gain_channel,
	                                         "gain_control_mode",
	                                         "manual");
	   if (ret < 0) {
	      state -> setText ("error in gain setting");
	      if (debugFlag)
	         fprintf (stderr, "error in gain setting\n");
	      return;
	   }
	   gainControl	-> show ();

	   ret = iio_channel_attr_write_longlong (this -> gain_channel,
	                                          "hardwaregain", 
	                                          gainControl -> value ());
	   if (ret < 0) {
	      state -> setText ("error in gain setting");
	      if (debugFlag)
	         fprintf (stderr,
	                  "could not set hardware gain to %d\n",
	                                          gainControl -> value ());
	   }
	}
}

bool	plutoHandler::restartReader	() {
int ret;
	if (debugFlag)
	   fprintf (stderr, "restart called");
	if (!connected)		// should not happen
	   return false;
	if (running. load())
	   return true;		// should not happen

	ret = iio_channel_attr_write_longlong (this -> lo_channel,
	                                       "frequency",
	                                       this -> lo_hz);
	if (ret < 0) {
	   if (debugFlag)
	      fprintf (stderr, "cannot set local oscillator frequency\n");
	   return false;
	}
	else
	   start ();
	return true;
}

void	plutoHandler::stopReader() {
	if (!running. load())
	   return;
	running. store (false);
	while (isRunning())
	   usleep (500);
}

void	plutoHandler::run	() {
char	*p_end, *p_dat;
int	p_inc;
int	nbytes_rx;
std::complex<float> localBuf [4 * 2048];
int	index	= 0;

	state -> setText ("running");
	running. store (true);
	while (running. load ()) {
	   nbytes_rx	= iio_buffer_refill	(rxbuf);
	   p_inc	= iio_buffer_step	(rxbuf);
	   p_end	= (char *) iio_buffer_end  (rxbuf);

	   for (p_dat = (char *)iio_buffer_first (rxbuf, rx0_i);
	        p_dat < p_end; p_dat += p_inc) {
	      const int16_t i_p = ((int16_t *)p_dat) [0];
	      const int16_t q_p = ((int16_t *)p_dat) [1];
	      std::complex<float>sample = std::complex<float> (i_p / 2048.0,
	                                                       q_p / 2048.0);
	      localBuf [index ++] = sample;
	      if (index >= 4 * 2048) {
	         _I_Buffer. putDataIntoBuffer (localBuf, 4 * 2048);
	         index = 0;
	      }
	   }
	}
}

int32_t	plutoHandler::getSamples (std::complex<float> *V,
	                                     int32_t size, uint8_t M) { 
	(void)M;
	return getSamples (V, size);
}

int32_t	plutoHandler::getSamples (std::complex<float> *V, int32_t size) { 
	if (!isRunning ())
	   return 0;
	return _I_Buffer. getDataFromBuffer (V, size);
}

int32_t	plutoHandler::Samples () {
	return _I_Buffer. GetRingBufferReadAvailable ();
}

int32_t	plutoHandler::getRate	() {
	return 2112000;
}

void	plutoHandler::resetBuffer() {
	_I_Buffer. FlushRingBuffer();
}

int16_t	plutoHandler::bitDepth () {
	return 12;
}

void	plutoHandler::toggle_debugButton	() {
	debugFlag	= !debugFlag;
	debugButton -> setText (debugFlag ? "debug on" : "debug off");
}

bool	plutoHandler::legalFrequency	(int f) {
	(void)f;
	return true;
}

int32_t	plutoHandler::defaultFrequency	() {
	return 94700000;
}

uint8_t	plutoHandler::myIdentity	() {
	return PLUTO;
}

