
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
 *	recoding and taking parts for the airspyHandler interface
 *	for the SDR-J-FM receiver
 *	jan van Katwijk
 *	Lazy Chair Computing
 */

#include "airspy.h"

static
const	int	EXTIO_NS= 8192;
static
const	int	EXTIO_BASE_TYPE_SIZE = sizeof (float);
//
//	Note: if/when the "mini" version is selected, we
//	have to inhibit the dynamic selection of rates.
//	The rate is fixed, either by specifying it in the ".ini"
//	file or by default.
//
//	That is why we remove all selectable options from the
//	rateSelector
	airspyHandler::airspyHandler (QSettings *s, bool full, bool *success) {
int	result;
QString	h;
int	k;

	this	-> airspySettings	= s;
	*success			= false;

	this	-> myFrame		= new QFrame (NULL);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();
//
//	Note: on the airspy rate selector, we have 2500 and 10000,
//	the actual rate of the latter is slightly different.
	airspySettings	-> beginGroup ("airspyHandler");
	h	= airspySettings -> value ("airspyRate", 2500). toString ();
	k	=  rateSelector -> findText (h);
	if (k != -1)
	   rateSelector	-> setCurrentIndex (k);
	inputRate	=  (rateSelector -> currentText (). toInt () == 10000) ?
	                                  Khz (10035) : Khz (2500);

	if (!full) {
	   while (rateSelector -> count () > 0)
	      rateSelector -> removeItem (rateSelector -> currentIndex ());
	   rateSelector	-> addItem (QString::number (inputRate / Khz (1)));
	}
	vgaGain			= airspySettings -> value ("vga", 5).toInt ();
	vgaSlider		-> setValue (vgaGain);
	mixerGain		= airspySettings -> value ("mixer", 10). toInt ();
	mixerSlider		-> setValue (mixerGain);
	mixer_agc		= false;
	lnaGain			= airspySettings -> value ("lna", 5). toInt ();
	lnaSlider		-> setValue (lnaGain);
	lna_agc			= false;
	rf_bias			= false;
	airspySettings	-> endGroup ();
	device				= 0;
	serialNumber			= 0;
	theBuffer			= NULL;
	strcpy (serial,"");
	result = airspy_init ();
	if (result != AIRSPY_SUCCESS) {
	   printf("airspy_init() failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
        return;
	}
	
	result = airspy_open (&device);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airpsy_open () failed: %s (%d)\n",
	             airspy_error_name ((airspy_error)result), result);
	   return;
	}
	theBuffer			= new RingBuffer<DSPCOMPLEX>(1024 *1024);
	connect (rateSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (set_rateSelector (const QString &)));
	connect (lnaSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_lna_gain (int)));
	connect (vgaSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_vga_gain (int)));
	connect (mixerSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_mixer_gain (int)));
	connect (lnaButton, SIGNAL (clicked (void)),
	         this, SLOT (set_lna_agc (void)));
	connect (mixerButton, SIGNAL (clicked (void)),
	         this, SLOT (set_mixer_agc (void)));
	connect (biasButton, SIGNAL (clicked (void)),
	         this, SLOT (set_rf_bias (void)));
	displaySerial	-> setText (getSerial ());
	running				= false;
	*success			= true;
}

	airspyHandler::~airspyHandler (void) {
	airspySettings	-> beginGroup ("airspyHandler");
	airspySettings -> setValue ("vga", vgaGain);
	airspySettings -> setValue ("mixer", mixerGain);
	airspySettings -> setValue ("lna", lnaGain);
	airspySettings -> setValue ("airspyRate",
	                           rateSelector -> currentText (). toLatin1 (). data ());
	airspySettings	-> endGroup ();
	myFrame	-> hide ();
	if (device) {
	   int result = airspy_stop_rx (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_stop_rx() failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
	   }
	   if (rf_bias)
	      set_rf_bias ();
	   result = airspy_close(device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_close() failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
	   }
	}
	airspy_exit();
	delete	myFrame;
	if (theBuffer != NULL)
	   delete theBuffer;
}

void	airspyHandler::setVFOFrequency (int32_t nf) {
int result = airspy_set_freq(device, lastFrequency = nf);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_freq() failed: %s (%d)\n",
	         airspy_error_name((airspy_error)result), result);
	}
}

int32_t	airspyHandler::getVFOFrequency (void) {
	return lastFrequency;
}

bool	airspyHandler::legalFrequency (int32_t f) {
	return Khz (5000) <= f && f <= Khz (2000000);
}

int32_t	airspyHandler::defaultFrequency (void) {
	return Khz (94700);
}

bool	airspyHandler::restartReader	(void) {
int	result;
int32_t	bufSize	= EXTIO_NS * EXTIO_BASE_TYPE_SIZE * 2;
	if (running)
	   return true;

	theBuffer	-> FlushRingBuffer ();
	result = ::airspy_set_sample_type(device, AIRSPY_SAMPLE_FLOAT32_IQ);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_sample_type() failed: %s (%d)\n",
	          airspy_error_name((airspy_error)result), result);
	return false;
	}
	
	setExternalRate (inputRate);
	set_vga_gain	(vgaGain);
	set_mixer_gain	(mixerGain);
	set_lna_gain	(lnaGain);
	
	result = ::airspy_start_rx (device, callback, this);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_start_rx() failed: %s (%d)\n",
	         airspy_error_name((airspy_error)result), result);
	   return false;
	}
//
//	finally:
	buffer = new uint8_t [bufSize];
	bs_ = bufSize;
	bl_ = 0;
	return true;
}

void	airspyHandler::stopReader (void) {
	if (!running)
	   return;
int result = airspy_stop_rx (device);

	if (result != AIRSPY_SUCCESS ) {
	   printf ("airspy_stop_rx() failed: %s (%d)\n",
	          airspy_error_name ((airspy_error)result), result);
	} else {
	   delete [] buffer;
	   bs_ = bl_ = 0 ;
	}
}

int airspyHandler::callback(airspy_transfer_t* transfer) {
airspyHandler *p;

	if (!transfer)
	   return 0;		// should not happen
	p = static_cast<airspyHandler *> (transfer->ctx);

// AIRSPY_SAMPLE_FLOAT32_IQ:
	uint32_t bytes_to_write = transfer -> sample_count * 4 * 2; 
	uint8_t *pt_rx_buffer   = (uint8_t *)transfer->samples;
	
	while (bytes_to_write > 0) {
	   int spaceleft = p -> bs_ - p -> bl_ ;
	   int to_copy = std::min ((int)spaceleft, (int)bytes_to_write);
	   ::memcpy (p -> buffer + p -> bl_, pt_rx_buffer, to_copy);
	   bytes_to_write -= to_copy;
	   pt_rx_buffer   += to_copy;
//
//	   bs (i.e. buffersize) in bytes
	   if (p -> bl_ == p -> bs_) {
	      p -> data_available ((void *)p -> buffer, p -> bl_);
	      p->bl_ = 0;
	   }
	   p -> bl_ += to_copy;
	}
	return 0;
}

//	called from AIRSPY data callback
//	this method is declared in airspyHandler class
//	The buffer received from hardware contains
//	32-bit floating point IQ samples (8 bytes per sample)
//
//	recoded for the sdr-j framework
//	4*2 = 8 bytes for sample, as per AirSpy USB data stream format
//	we simply move the floats into the complexes
int 	airspyHandler::data_available (void *buf, int buf_size) {	
float	*sbuf	= (float *)buf;
int nSamples	= buf_size / (sizeof (float) * 2);
DSPCOMPLEX lBuf [nSamples];
int32_t	i;
	for (i = 0; i < nSamples; i ++)
	   lBuf [i] = DSPCOMPLEX (sbuf [2 * i], sbuf [2 * i + 1]);
	theBuffer	-> putDataIntoBuffer (lBuf, nSamples);
	return 0;
}

bool	airspyHandler::status (void) {
	return true;
}

const char *airspyHandler::getSerial (void) {
airspy_read_partid_serialno_t read_partid_serialno;
int result = ::airspy_board_partid_serialno_read (device,
	                                          &read_partid_serialno);
	if (result != AIRSPY_SUCCESS) {
	   printf ("failed: %s (%d)\n",
	         airspy_error_name ((airspy_error)result), result);
	   return "UNKNOWN";
	} else {
	   snprintf (serial, sizeof(serial), "%08X%08X", 
	             read_partid_serialno. serial_no [2],
	             read_partid_serialno. serial_no [3]);
	}
	return serial;
}

int	airspyHandler::open (void) {
int result = airspy_open (&device);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_open() failed: %s (%d)\n",
	          airspy_error_name((airspy_error)result), result);
	   return -1;
	} else {
	   return 0;
	}
}

int	airspyHandler::setExternalRate (int nsr) {
airspy_samplerate_t as_nsr;

	switch (nsr) {
	   case 10000000:
	      as_nsr = AIRSPY_SAMPLERATE_10MSPS;
	      inputRate = nsr;
	      break;
	   case 2500000:
	      as_nsr = AIRSPY_SAMPLERATE_2_5MSPS;
	      inputRate = nsr;
	      break;
	   default:
	      return -1;
	}

	int result = airspy_set_samplerate (device, as_nsr);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_samplerate() failed: %s (%d)\n",
	          airspy_error_name((airspy_error)result), result);
	   return -1;
	} else
	   return 0;
}

void	airspyHandler::resetBuffer (void) {
	theBuffer	-> FlushRingBuffer ();
}

int16_t	airspyHandler::bitDepth (void) {
	return 16;
}

int32_t	airspyHandler::getRate (void) {
	return inputRate;
}

void	airspyHandler::set_rateSelector (const QString &s) {
int32_t v	= s. toInt ();

	setExternalRate (Khz (v));
	set_changeRate (Khz (v));
}

int32_t	airspyHandler::getSamples (DSPCOMPLEX *v, int32_t size) {
	return theBuffer	-> getDataFromBuffer (v, size);
}

int32_t	airspyHandler::getSamples	(DSPCOMPLEX  *V,
	                         int32_t size, uint8_t M) {
	(void)M;
	return getSamples (V, size);
}

int32_t	airspyHandler::Samples	(void) {
	return theBuffer	-> GetRingBufferReadAvailable ();
}
//
uint8_t	airspyHandler::myIdentity		(void) {
	return AIRSPY;
}

/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_lna_gain (int value) {
int result = ::airspy_set_lna_gain (device, lnaGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_lna_gain () failed: %s (%d)\n",
	          airspy_error_name((airspy_error)result), result);
	}
	else
	   lnaDisplay	-> display (value);
}

/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_mixer_gain (int value) {
int result = ::airspy_set_mixer_gain(device, mixerGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf("airspy_set_mixer_gain() failed: %s (%d)\n",
	         airspy_error_name((airspy_error)result), result);
	}
	else
	   mixerDisplay	-> display (value);
}

/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_vga_gain (int value) {
int result = ::airspy_set_vga_gain(device, vgaGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_vga_gain () failed: %s (%d)\n",
	          airspy_error_name ((airspy_error)result), result);
	}
	else
	   vgaDisplay	-> display (value);
}
//
//	agc's
/* Parameter value:
	0=Disable LNA Automatic Gain Control
	1=Enable LNA Automatic Gain Control
*/
void	airspyHandler::set_lna_agc (void) {
	lna_agc	= !lna_agc;
int result = ::airspy_set_lna_agc (device, lna_agc ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_lna_agc() failed: %s (%d)\n",
	          airspy_error_name ((airspy_error)result), result);
	}
}

/* Parameter value:
	0=Disable MIXER Automatic Gain Control
	1=Enable MIXER Automatic Gain Control
*/
void	airspyHandler::set_mixer_agc (void) {
	mixer_agc	= !mixer_agc;

int result = ::airspy_set_mixer_agc (device, mixer_agc ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_mixer_agc () failed: %s (%d)\n",
	            airspy_error_name ((airspy_error)result), result);
	}
}


/* Parameter value shall be 0=Disable BiasT or 1=Enable BiasT */
void	airspyHandler::set_rf_bias (void) {
	rf_bias	= !rf_bias;
int result = ::airspy_set_rf_bias (device, rf_bias ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf("airspy_set_rf_bias() failed: %s (%d)\n",
	         airspy_error_name ((airspy_error)result), result);
	}
}


const char* airspyHandler::board_id_name (void) {
uint8_t bid;

	if (::airspy_board_id_read (device, &bid) == AIRSPY_SUCCESS)
	   return ::airspy_board_id_name ((airspy_board_id)bid);
	else
	   return "UNKNOWN";
}



