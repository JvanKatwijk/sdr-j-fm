
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
 *	recoding, taking parts and extending for the airspyHandler interface
 *	for the SDR-J-DAB receiver.
 *	jan van Katwijk
 *	Lazy Chair Computing
 */

#ifdef	__MINGW32__
#define	GETPROCADDRESS	GetProcAddress
#else
#define	GETPROCADDRESS	dlsym
#endif

#include "airspy-handler.h"

static
const	int	EXTIO_NS	=  8192;
static
const	int	EXTIO_BASE_TYPE_SIZE = sizeof (float);

	airspyHandler::airspyHandler (QSettings *s):
	                                myFrame (nullptr),
	                                _I_Buffer (256 * 1024) {
int	result, i;
QString	h;
int	k;
int	distance	= 10000000;
uint32_t myBuffer [20];
uint32_t samplerate_count;

	this	-> airspySettings	= s;

	setupUi (&myFrame);
	myFrame. show ();

	inputRate	= 2500000;

	airspySettings	-> beginGroup ("airspyHandler");
	int16_t temp 		= airspySettings -> value ("linearity", 10).
	                                                          toInt ();
	linearitySlider		-> setValue (temp);
	linearityDisplay	-> display  (temp);
	temp			= airspySettings -> value ("sensitivity", 10).
	                                                          toInt ();
	sensitivitySlider	-> setValue (temp);
	sensitivityDisplay	-> display (temp);
	vgaGain			= airspySettings -> value ("vga", 5).toInt ();
	vgaSlider		-> setValue (vgaGain);
	vgaDisplay		-> display (vgaGain);
	mixerGain		= airspySettings -> value ("mixer", 10). toInt ();
	mixerSlider		-> setValue (mixerGain);
	mixerDisplay		-> display (mixerGain);
	mixer_agc		= false;
	lnaGain			= airspySettings -> value ("lna", 5). toInt ();
	lnaSlider		-> setValue (lnaGain);
	lnaDisplay		-> display  (lnaGain);
	mixer_agc		= false;
	lna_agc			= false;
	rf_bias			= false;
	airspySettings		-> endGroup ();
//
	device			= 0;
	serialNumber		= 0;
#ifdef	__MINGW32__
	const char *libraryString = "airspy.dll";
	Handle		= LoadLibrary ((wchar_t *)L"airspy.dll");
#else
	const char *libraryString = "libairspy.so";
	Handle		= dlopen ("libairspy.so", RTLD_LAZY);
	if (Handle == nullptr)
	   Handle	= dlopen ("libairspy.so.0", RTLD_LAZY);
#endif

	if (Handle == nullptr) {
	   fprintf (stderr, "failed to open %s\n", libraryString);
#ifndef	__MINGW32__
	   fprintf (stderr, "Error = %s\n", dlerror ());
#endif
	   throw (23);
	}

	if (!load_airspyFunctions ()) {
	   fprintf (stderr, "problem in loading functions\n");
	
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (24);
	}
//
	strcpy (serial,"");
	result = this -> my_airspy_init ();
	if (result != AIRSPY_SUCCESS) {
	   fprintf (stderr, "my_airspy_init () failed: %s (%d)\n",
	             my_airspy_error_name((airspy_error)result), result);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (25);
	}

	result = my_airspy_open (&device);
	if (result != AIRSPY_SUCCESS) {
	   fprintf (stderr, "my_airpsy_open () failed: %s (%d)\n",
	             my_airspy_error_name ((airspy_error)result), result);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (26);
	}
//
	(void) my_airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	(void) my_airspy_get_samplerates (device, &samplerate_count, 0);
        fprintf (stderr, "%d samplerates are supported\n", samplerate_count);
        my_airspy_get_samplerates (device,
	                              myBuffer, samplerate_count);

        inputRate    = 0;
        for (i = 0; i < samplerate_count; i ++) {
           fprintf (stderr, "%d \n", myBuffer [i]);
           if (abs ((int)(myBuffer [i]) - 2000000) < distance) {
              distance  = abs ((int)(myBuffer [i]) - 2000000);
              inputRate = myBuffer [i];
           }
        }

	result = my_airspy_set_samplerate (device, inputRate);
//	The sizes of the mapTables follow from the input and output rate
//	(selectedRate / 1000) vs (2304 / 1000)
//	so we end up with buffers with 1 msec content
	convBufferSize		= inputRate / 1000;
	for (i = 0; i < 2304; i ++) {
	   float inVal	= float (inputRate / 1000);
	   mapTable_int [i]	= int (floor (i * (inVal / 2304.0)));
	   mapTable_float [i]	= i * (inVal / 2304.0) - mapTable_int [i];
	}
	convIndex	= 0;
	convBuffer. resize (convBufferSize + 1);
//
	connect (linearitySlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_linearity (int)));
	connect (sensitivitySlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_sensitivity (int)));
	connect (lnaSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_lna_gain (int)));
        connect (vgaSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_vga_gain (int)));
        connect (mixerSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_mixer_gain (int)));

	connect (lnaButton, SIGNAL (clicked ()),
	         this, SLOT (set_lna_agc ()));
	connect (mixerButton, SIGNAL (clicked ()),
	         this, SLOT (set_mixer_agc ()));
	connect (biasButton, SIGNAL (clicked ()),
	         this, SLOT (set_rf_bias ()));
	connect (tabWidget, SIGNAL (currentChanged (int)),
                 this, SLOT (show_tab (int)));

	displaySerial	-> setText (getSerial ());
	show_tab (0);
	running		= false;
}

	airspyHandler::~airspyHandler () {
	airspySettings	-> beginGroup ("airspyHandler");
	airspySettings -> setValue ("linearity", linearitySlider -> value ());
	airspySettings -> setValue ("sensitivity", sensitivitySlider -> value ());
	airspySettings -> setValue ("vga", vgaGain);
	airspySettings -> setValue ("mixer", mixerGain);
	airspySettings -> setValue ("lna", lnaGain);
	airspySettings	-> endGroup ();
	myFrame. hide ();
	if (device) {
	   int result = my_airspy_stop_rx (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("my_airspy_stop_rx () failed: %s (%d)\n",
	             my_airspy_error_name((airspy_error)result), result);
	   }

	   if (rf_bias)
	      set_rf_bias ();
	   result = my_airspy_close (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_close () failed: %s (%d)\n",
	             my_airspy_error_name((airspy_error)result), result);
	   }
	}
	my_airspy_exit ();
	if (Handle != nullptr) 
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
}

void	airspyHandler::setVFOFrequency (int32_t nf) {
int result = my_airspy_set_freq (device, lastFrequency = nf);

	fprintf (stderr, "set frequency %d\n", nf);
	if (result != AIRSPY_SUCCESS) {
	   printf ("my_airspy_set_freq() failed: %s (%d)\n",
	            my_airspy_error_name((airspy_error)result), result);
	}
}

int32_t	airspyHandler::getVFOFrequency () {
	return lastFrequency;
}

bool	airspyHandler::legalFrequency (int32_t f) {
	return Khz (24000) <= f && f <= Khz (1750000);
}

int32_t	airspyHandler::defaultFrequency () {
	return Khz (94700);
}

bool	airspyHandler::restartReader	() {
int	result;
int32_t	bufSize	= EXTIO_NS * EXTIO_BASE_TYPE_SIZE * 2;
	if (running)
	   return true;

	_I_Buffer. FlushRingBuffer ();
	if (currentTab == 0)
           set_sensitivity      (sensitivitySlider -> value ());
        else
        if (currentTab == 1)
           set_linearity        (linearitySlider -> value ());
        else {
           set_vga_gain         (vgaGain);
           set_mixer_gain       (mixerGain);
           set_lna_gain         (lnaGain);
        }

	result = my_airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	if (result != AIRSPY_SUCCESS) {
	   printf ("my_airspy_set_sample_type () failed: %s (%d)\n",
	            my_airspy_error_name ((airspy_error)result), result);
	   return false;
	}
	
	set_linearity	(linearitySlider -> value ());
	set_sensitivity	(sensitivitySlider -> value ());
	
	result = my_airspy_start_rx (device,
	            (airspy_sample_block_cb_fn)callback, this);
	if (result != AIRSPY_SUCCESS) {
	   printf ("my_airspy_start_rx () failed: %s (%d)\n",
	         my_airspy_error_name((airspy_error)result), result);
	   return false;
	}
//
//	finally:
	buffer = new uint8_t [bufSize];
	bs_ = bufSize;
	bl_ = 0;
	running	= true;
	return true;
}

void	airspyHandler::stopReader () {
	if (!running)
	   return;
int result = my_airspy_stop_rx (device);

	if (result != AIRSPY_SUCCESS ) {
	   printf ("my_airspy_stop_rx() failed: %s (%d)\n",
	          my_airspy_error_name ((airspy_error)result), result);
	} else {
	   delete [] buffer;
	   bs_ = bl_ = 0 ;
	}
	running	= false;
}
//
//	Directly copied from the airspy extio dll from Andrea Montefusco
int airspyHandler::callback (airspy_transfer* transfer) {
airspyHandler *p;

	if (!transfer)
	   return 0;		// should not happen
	p = static_cast<airspyHandler *> (transfer -> ctx);

//	we read	AIRSPY_SAMPLE_INT16_IQ
	uint32_t bytes_to_write = transfer -> sample_count * sizeof (int16_t) * 2; 
	uint8_t *pt_rx_buffer   = (uint8_t *)transfer->samples;
	p -> data_available (pt_rx_buffer, bytes_to_write);
	return 0;
}
//
//	called from AIRSPY data callback
//	2*2 = 4 bytes for sample, as per AirSpy USB data stream format
//	we do the rate conversion here, read in groups of 2 * xxx samples
//	and transform them into groups of 2 * 2304 samples
int 	airspyHandler::data_available (void *buf, int buf_size) {	
int16_t	*sbuf	= (int16_t *)buf;
int nSamples	= buf_size / (sizeof (int16_t) * 2);
std::complex<float> temp [2304];

	for (int i = 0; i < nSamples; i ++) {
	   convBuffer [convIndex ++] = std::complex<float> (
	                                     sbuf [2 * i] / (float)2048,
	                                     sbuf [2 * i + 1] / (float)2048);
	   if (convIndex > convBufferSize) {
	      for (int j = 0; j < 2304; j ++) {
	         int16_t  inpBase	= mapTable_int [j];
	         float    inpRatio	= mapTable_float [j];
	         temp [j]	= cmul (convBuffer [inpBase + 1], inpRatio) + 
	                          cmul (convBuffer [inpBase], 1 - inpRatio);
	      }

	      _I_Buffer. putDataIntoBuffer (temp, 2304);
//
//	shift the sample at the end to the beginning, it is needed
//	as the starting sample for the next time
	      convBuffer [0] = convBuffer [convBufferSize];
	      convIndex = 1;
	   }
	}
	return 0;
}
//	Not used here
bool	airspyHandler::status () {
	return true;
}

const char *airspyHandler::getSerial () {
airspy_read_partid_serialno_t read_partid_serialno;
int result = my_airspy_board_partid_serialno_read (device,
	                                          &read_partid_serialno);
	if (result != AIRSPY_SUCCESS) {
	   printf ("failed: %s (%d)\n",
	         my_airspy_error_name ((airspy_error)result), result);
	   return "UNKNOWN";
	} else {
	   snprintf (serial, sizeof(serial), "%08X%08X", 
	             read_partid_serialno. serial_no [2],
	             read_partid_serialno. serial_no [3]);
	}
	return serial;
}
//
//	not used here
int	airspyHandler::open () {
int result = my_airspy_open (&device);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_open() failed: %s (%d)\n",
	          my_airspy_error_name((airspy_error)result), result);
	   return -1;
	} else {
	   return 0;
	}
}

int	airspyHandler::setExternalRate (int nsr) {
	return 0;
}

void	airspyHandler::resetBuffer () {
	_I_Buffer. FlushRingBuffer ();
}

int16_t	airspyHandler::bitDepth () {
	return 12;
}

int32_t	airspyHandler::getRate () {
	return 12 * 192000;
}

int32_t	airspyHandler::getSamples (DSPCOMPLEX *v, int32_t size) {
	return _I_Buffer. getDataFromBuffer (v, size);
}

int32_t	airspyHandler::getSamples	(DSPCOMPLEX  *V,
	                         int32_t size, uint8_t M) {
	(void)M;
	return getSamples (V, size);
}

int32_t	airspyHandler::Samples	() {
	return _I_Buffer. GetRingBufferReadAvailable ();
}
//
uint8_t	airspyHandler::myIdentity		() {
	return AIRSPY;
}
//
//	agc's
/* Parameter value:
	0=Disable LNA Automatic Gain Control
	1=Enable LNA Automatic Gain Control
*/
void	airspyHandler::set_lna_agc () {
	lna_agc	= !lna_agc;
int result = my_airspy_set_lna_agc (device, lna_agc ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_lna_agc() failed: %s (%d)\n",
	            my_airspy_error_name ((airspy_error)result), result);
	}
}

/* Parameter value:
	0=Disable MIXER Automatic Gain Control
	1=Enable MIXER Automatic Gain Control
*/
void	airspyHandler::set_mixer_agc () {
	mixer_agc	= !mixer_agc;

int result = my_airspy_set_mixer_agc (device, mixer_agc ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_mixer_agc () failed: %s (%d)\n",
	            my_airspy_error_name ((airspy_error)result), result);
	}
}

/* Parameter value shall be 0=Disable BiasT or 1=Enable BiasT */
void	airspyHandler::set_rf_bias () {
	rf_bias	= !rf_bias;
int result = my_airspy_set_rf_bias (device, rf_bias ? 1 : 0);

	if (result != AIRSPY_SUCCESS) {
	   printf("airspy_set_rf_bias() failed: %s (%d)\n",
	           my_airspy_error_name ((airspy_error)result), result);
	}
}

const char* airspyHandler::board_id_name () {
uint8_t bid;

	if (my_airspy_board_id_read (device, &bid) == AIRSPY_SUCCESS)
	   return my_airspy_board_id_name ((airspy_board_id)bid);
	else
	   return "UNKNOWN";
}
//
//
bool	airspyHandler::load_airspyFunctions () {
//
//	link the required procedures
	my_airspy_init	= (pfn_airspy_init)
	                       GETPROCADDRESS (Handle, "airspy_init");
	if (my_airspy_init == nullptr) {
	   fprintf (stderr, "Could not find airspy_init\n");
	   return false;
	}

	my_airspy_exit	= (pfn_airspy_exit)
	                       GETPROCADDRESS (Handle, "airspy_exit");
	if (my_airspy_exit == nullptr) {
	   fprintf (stderr, "Could not find airspy_exit\n");
	   return false;
	}

	my_airspy_open	= (pfn_airspy_open)
	                       GETPROCADDRESS (Handle, "airspy_open");
	if (my_airspy_open == nullptr) {
	   fprintf (stderr, "Could not find airspy_open\n");
	   return false;
	}

	my_airspy_close	= (pfn_airspy_close)
	                       GETPROCADDRESS (Handle, "airspy_close");
	if (my_airspy_close == nullptr) {
	   fprintf (stderr, "Could not find airspy_close\n");
	   return false;
	}

	my_airspy_get_samplerates	= (pfn_airspy_get_samplerates)
	                       GETPROCADDRESS (Handle, "airspy_get_samplerates");
	if (my_airspy_get_samplerates == nullptr) {
	   fprintf (stderr, "Could not find airspy_get_samplerates\n");
	   return false;
	}

	my_airspy_set_samplerate	= (pfn_airspy_set_samplerate)
	                       GETPROCADDRESS (Handle, "airspy_set_samplerate");
	if (my_airspy_set_samplerate == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_samplerate\n");
	   return false;
	}

	my_airspy_start_rx	= (pfn_airspy_start_rx)
	                       GETPROCADDRESS (Handle, "airspy_start_rx");
	if (my_airspy_start_rx == nullptr) {
	   fprintf (stderr, "Could not find airspy_start_rx\n");
	   return false;
	}

	my_airspy_stop_rx	= (pfn_airspy_stop_rx)
	                       GETPROCADDRESS (Handle, "airspy_stop_rx");
	if (my_airspy_stop_rx == nullptr) {
	   fprintf (stderr, "Could not find airspy_stop_rx\n");
	   return false;
	}

	my_airspy_set_sample_type	= (pfn_airspy_set_sample_type)
	                       GETPROCADDRESS (Handle, "airspy_set_sample_type");
	if (my_airspy_set_sample_type == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_sample_type\n");
	   return false;
	}

	my_airspy_set_freq	= (pfn_airspy_set_freq)
	                       GETPROCADDRESS (Handle, "airspy_set_freq");
	if (my_airspy_set_freq == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_freq\n");
	   return false;
	}

	my_airspy_set_lna_gain	= (pfn_airspy_set_lna_gain)
	                       GETPROCADDRESS (Handle, "airspy_set_lna_gain");
	if (my_airspy_set_lna_gain == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_lna_gain\n");
	   return false;
	}

	my_airspy_set_mixer_gain	= (pfn_airspy_set_mixer_gain)
	                       GETPROCADDRESS (Handle, "airspy_set_mixer_gain");
	if (my_airspy_set_mixer_gain == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_mixer_gain\n");
	   return false;
	}

	my_airspy_set_vga_gain	= (pfn_airspy_set_vga_gain)
	                       GETPROCADDRESS (Handle, "airspy_set_vga_gain");
	if (my_airspy_set_vga_gain == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_vga_gain\n");
	   return false;
	}

	my_airspy_set_linearity_gain = (pfn_airspy_set_linearity_gain)
	                       GETPROCADDRESS (Handle, "airspy_set_linearity_gain");
	if (my_airspy_set_linearity_gain == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_linearity_gain\n");
	   fprintf (stderr, "You probably did not install 1.0.7 yet\n");
	   return false;
	}

	my_airspy_set_sensitivity_gain = (pfn_airspy_set_sensitivity_gain)
	                       GETPROCADDRESS (Handle, "airspy_set_sensitivity_gain");
	if (my_airspy_set_sensitivity_gain == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_sensitivity_gain\n");
	   fprintf (stderr, "You probably did not install 1.0.7 yet\n");
	   return false;
	}


	my_airspy_set_lna_agc	= (pfn_airspy_set_lna_agc)
	                       GETPROCADDRESS (Handle, "airspy_set_lna_agc");
	if (my_airspy_set_lna_agc == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_lna_agc\n");
	   return false;
	}

	my_airspy_set_mixer_agc	= (pfn_airspy_set_mixer_agc)
	                       GETPROCADDRESS (Handle, "airspy_set_mixer_agc");
	if (my_airspy_set_mixer_agc == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_mixer_agc\n");
	   return false;
	}

	my_airspy_set_rf_bias	= (pfn_airspy_set_rf_bias)
	                       GETPROCADDRESS (Handle, "airspy_set_rf_bias");
	if (my_airspy_set_rf_bias == nullptr) {
	   fprintf (stderr, "Could not find airspy_set_rf_bias\n");
	   return false;
	}

	my_airspy_error_name	= (pfn_airspy_error_name)
	                       GETPROCADDRESS (Handle, "airspy_error_name");
	if (my_airspy_error_name == nullptr) {
	   fprintf (stderr, "Could not find airspy_error_name\n");
	   return false;
	}

	my_airspy_board_id_read	= (pfn_airspy_board_id_read)
	                       GETPROCADDRESS (Handle, "airspy_board_id_read");
	if (my_airspy_board_id_read == nullptr) {
	   fprintf (stderr, "Could not find airspy_board_id_read\n");
	   return false;
	}

	my_airspy_board_id_name	= (pfn_airspy_board_id_name)
	                       GETPROCADDRESS (Handle, "airspy_board_id_name");
	if (my_airspy_board_id_name == nullptr) {
	   fprintf (stderr, "Could not find airspy_board_id_name\n");
	   return false;
	}

	my_airspy_board_partid_serialno_read	=
	                (pfn_airspy_board_partid_serialno_read)
	                       GETPROCADDRESS (Handle, "airspy_board_partid_serialno_read");
	if (my_airspy_board_partid_serialno_read == nullptr) {
	   fprintf (stderr, "Could not find airspy_board_partid_serialno_read\n");
	   return false;
	}

	return true;
}


/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_lna_gain (int value) {
int result = my_airspy_set_lna_gain (device, lnaGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_lna_gain () failed: %s (%d)\n",
	          my_airspy_error_name((airspy_error)result), result);
	}
	else
	   lnaDisplay	-> display (value);
}

/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_mixer_gain (int value) {
int result = my_airspy_set_mixer_gain(device, mixerGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf("airspy_set_mixer_gain() failed: %s (%d)\n",
	         my_airspy_error_name((airspy_error)result), result);
	}
	else
	   mixerDisplay	-> display (value);
}

/* Parameter value shall be between 0 and 15 */
void	airspyHandler::set_vga_gain (int value) {
int result = my_airspy_set_vga_gain(device, vgaGain = value);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_vga_gain () failed: %s (%d)\n",
	          my_airspy_error_name ((airspy_error)result), result);
	}
	else
	   vgaDisplay	-> display (value);
}

#define GAIN_COUNT (22)

uint8_t airspy_linearity_vga_gains[GAIN_COUNT] = { 13, 12, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4 };
uint8_t airspy_linearity_mixer_gains[GAIN_COUNT] = { 12, 12, 11, 9, 8, 7, 6, 6, 5, 0, 0, 1, 0, 0, 2, 2, 1, 1, 1, 1, 0, 0 };
uint8_t airspy_linearity_lna_gains[GAIN_COUNT] = { 14, 14, 14, 13, 12, 10, 9, 9, 8, 9, 8, 6, 5, 3, 1, 0, 0, 0, 0, 0, 0, 0 };
uint8_t airspy_sensitivity_vga_gains[GAIN_COUNT] = { 13, 12, 11, 10, 9, 8, 7, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
uint8_t airspy_sensitivity_mixer_gains[GAIN_COUNT] = { 12, 12, 12, 12, 11, 10, 10, 9, 9, 8, 7, 4, 4, 4, 3, 2, 2, 1, 0, 0, 0, 0 };
uint8_t airspy_sensitivity_lna_gains[GAIN_COUNT] = { 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 12, 12, 9, 9, 8, 7, 6, 5, 3, 2, 1, 0 };

void	airspyHandler::set_linearity (int value) {
int	result = my_airspy_set_linearity_gain (device, value);
int	temp;
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_lna_gain () failed: %s (%d)\n",
	            my_airspy_error_name ((airspy_error)result), result);
	   return;
	}
	linearityDisplay	-> display (value);
	temp	= airspy_linearity_lna_gains [GAIN_COUNT - 1 - value];
	linearity_lnaDisplay	-> display (temp);
	temp	= airspy_linearity_mixer_gains [GAIN_COUNT - 1 - value];
	linearity_mixerDisplay	-> display (temp);
	temp	= airspy_linearity_vga_gains [GAIN_COUNT - 1 - value];
	linearity_vgaDisplay	-> display (temp);
}

void	airspyHandler::set_sensitivity (int value) {
int	result = my_airspy_set_sensitivity_gain (device, value);
int	temp;
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_mixer_gain() failed: %s (%d)\n",
	            my_airspy_error_name ((airspy_error)result), result);
	   return;
	}
	sensitivityDisplay	-> display (value);
	temp	= airspy_sensitivity_lna_gains [GAIN_COUNT - 1 - value];
	sensitivity_lnaDisplay	-> display (temp);
	temp	= airspy_sensitivity_mixer_gains [GAIN_COUNT - 1 - value];
	sensitivity_mixerDisplay	-> display (temp);
	temp	= airspy_sensitivity_vga_gains [GAIN_COUNT - 1 - value];
	sensitivity_vgaDisplay	-> display (temp);
}

void	airspyHandler::show_tab (int t) {
	if (t == 0)		// sensitivity
	   set_sensitivity	(sensitivitySlider -> value ());
	else
	if (t == 1)		// linearity
	   set_linearity	(linearitySlider -> value ());
	else {			// classic view
	   set_vga_gain		(vgaGain);
	   set_mixer_gain	(mixerGain);
	   set_lna_gain		(lnaGain);
	}
	currentTab	= t;
}

