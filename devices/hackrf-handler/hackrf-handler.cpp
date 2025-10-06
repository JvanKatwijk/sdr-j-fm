#
/*
 *    Copyright (C) 2014 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of Qt-DAB
 *
 *    Qt-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    Qt-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Qt-DAB if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QThread>
#include	<QSettings>
#include	<QHBoxLayout>
#include	<QLabel>
#include	<QDebug>
#include	"hackrf-handler.h"
#include	"device-exceptions.h"

#define	DEFAULT_GAIN	30

	hackrfHandler::hackrfHandler  (QSettings *s):
	                               _I_Buffer (1024 * 1024) {
int	res;
	hackrfSettings			= s;
	this	-> myFrame		= new QFrame (nullptr);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();
	this	-> inputRate		= Khz (12 * 192);

#ifdef  __MINGW32__
	const char *libraryString = "libhackrf.dll";
#else
	const char *libraryString = "libhackrf.so";
#endif

	library_p	= new QLibrary (libraryString);
	library_p	-> load ();
	if (!library_p -> isLoaded ()) {
           throw (device_exception ("failed to open " +
                                        std::string (libraryString)));
	}

	libraryLoaded   = true;
	if (!load_hackrfFunctions ()) {
	   delete library_p;
	   delete myFrame;
	   throw (21);
	}
//
//	From here we have a library available

	vfoFrequency	= Khz (220000);
//
//	See if there are settings from previous incarnations
	hackrfSettings		-> beginGroup ("hackrfSettings");
	lnagainSlider 		-> setValue (
	            hackrfSettings -> value ("hack_lnaGain", DEFAULT_GAIN). toInt ());
	vgagainSlider 		-> setValue (
	            hackrfSettings -> value ("hack_vgaGain", DEFAULT_GAIN). toInt ());
//      contributed by Fabio
	bool isChecked =
	    hackrfSettings -> value ("hack_AntEnable", false). toBool ();
	AntEnableButton -> setCheckState (isChecked ? Qt::Checked :
	                                              Qt::Unchecked);
	isChecked       =
	   hackrfSettings -> value ("hack_AmpEnable", false). toBool();
	AmpEnableButton -> setCheckState (isChecked ? Qt::Checked :
	                                              Qt::Unchecked);
	ppm_correction      -> setValue (
	          hackrfSettings -> value ("hack_ppmCorrection", 0). toInt ());
//      end

	hackrfSettings	-> endGroup ();

//
	res	= this -> hackrf_init ();
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_init:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete library_p;
	   delete myFrame;
	   throw (device_exception (std::string ("problem with hackrf init ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	res	= this	-> hackrf_open (&theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_open:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with hackrf opem ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	   throw (22);
	}

	res	= this -> hackrf_set_sample_rate (theDevice, 2304000.0);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_samplerate:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with set samplerate ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	
	}

	int bandWidth = this -> hackrf_compute_baseband_filter_bw (256000);
	fprintf (stderr, "computed value %d\n", bandWidth);
	res	= this -> hackrf_set_baseband_filter_bandwidth (theDevice,
	                                                        bandWidth);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_bw:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with set bw ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	res	= this -> hackrf_set_freq (theDevice, 220000000);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: ");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with set freq ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	res = this -> hackrf_set_antenna_enable (theDevice, 1);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_antenna_enable: ");
	   fprintf (stderr, "%s \n",
	                this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with set ntennaa ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	res = this -> hackrf_set_amp_enable (theDevice, 1);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_amp_enable: ");
	   fprintf (stderr, "%s \n",
	                this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with set amp enable ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	uint16_t regValue;
	res = this -> hackrf_si5351c_read (theDevice, 162, &regValue);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_si5351c_read: ");
	   fprintf (stderr, "%s \n",
	             this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with si5351 ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	res = this -> hackrf_si5351c_write (theDevice, 162, regValue);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_si5351c_write: ");
	   fprintf (stderr, "%s \n",
	              this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   delete library_p;
	   throw (device_exception (std::string ("problem with si5351 ") +
	                 this -> hackrf_error_name (hackrf_error (res))));
	}

	setLNAGain	(lnagainSlider	-> value ());
	setVGAGain	(vgagainSlider	-> value ());
	EnableAntenna   (1);            // value is a dummy really
	EnableAmpli     (1);            // value is a dummy, really
	set_ppmCorrection (ppm_correction       -> value ());

//	and be prepared for future changes in the settings
	connect (lnagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setLNAGain (int)));
	connect (vgagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setVGAGain (int)));
	connect (AntEnableButton, SIGNAL (stateChanged (int)),
	         this, SLOT (EnableAntenna (int)));
	connect (AmpEnableButton, SIGNAL (stateChanged (int)),
	         this, SLOT (EnableAmpli (int)));
	connect (ppm_correction, SIGNAL (valueChanged (int)),
	         this, SLOT (set_ppmCorrection  (int)));

	hackrf_device_list_t *deviceList = this -> hackrf_device_list ();
	if (deviceList != NULL) {	// well, it should be
	   char *serial = deviceList -> serial_numbers [0];
	   serial_number_display -> setText (serial);
	   enum hackrf_usb_board_id board_id =
	                 deviceList -> usb_board_ids [0];
	   usb_board_id_display ->
	                setText (this -> hackrf_usb_board_id_name (board_id));
	}

	running. store (false);
}

	hackrfHandler::~hackrfHandler	() {
	stopReader ();
	delete myFrame;
	hackrfSettings	-> beginGroup ("hackrfSettings");
	hackrfSettings	-> setValue ("hack_lnaGain",
	                                 lnagainSlider -> value ());
	hackrfSettings -> setValue ("hack_vgaGain",
	                                 vgagainSlider	-> value ());
	hackrfSettings -> setValue ("hack_AntEnable",
	                              AntEnableButton -> checkState () == Qt::Checked);
	hackrfSettings -> setValue ("hack_AmpEnable",
	                              AmpEnableButton -> checkState () == Qt::Checked);
	hackrfSettings  -> setValue ("hack_ppmCorrection",
	                              ppm_correction -> value ());

	hackrfSettings	-> endGroup ();
	this	-> hackrf_close (theDevice);
	this	-> hackrf_exit ();
}
//
bool    hackrfHandler::legalFrequency (int32_t f) {
	(void)f;
	return true;
}

int32_t hackrfHandler::defaultFrequency	() {
	return Khz (105600);
}

void	hackrfHandler::setVFOFrequency	(int32_t newFrequency) {
int	res;
	res	= this -> hackrf_set_freq (theDevice, newFrequency);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: \n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//
//      It seems that after changing the frequency, the preamp is switched off
	if (AmpEnableButton -> checkState () == Qt::Checked)
	   EnableAmpli (1);

	vfoFrequency = newFrequency;
}

int32_t	hackrfHandler::getVFOFrequency	() {
	return vfoFrequency;
}

void	hackrfHandler::setLNAGain	(int newGain) {
int	res;
	if ((newGain <= 40) && (newGain >= 0)) {
	   res	= this -> hackrf_set_lna_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_lna_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   lnagainDisplay	-> display (newGain);
	}
}

void	hackrfHandler::setVGAGain	(int newGain) {
int	res;
	if ((newGain <= 62) && (newGain >= 0)) {
	   res	= this -> hackrf_set_vga_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_vga_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   vgagainDisplay	-> display (newGain);
	}
}

void    hackrfHandler::EnableAntenna (int d) {
int res;
bool    b;

	(void)d;
	b = AntEnableButton     -> checkState () == Qt::Checked;
	res = this -> hackrf_set_antenna_enable (theDevice, b);
//      fprintf(stderr,"Passed %d\n",(int)b);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_antenna_enable :\n");
	   fprintf (stderr, "%s \n",
	                    this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//      AntEnableButton -> setChecked (b);
}

void    hackrfHandler::EnableAmpli (int a) {
int res;
bool    b;

	(void)a;
	b = AmpEnableButton     -> checkState () == Qt::Checked;
	res = this -> hackrf_set_amp_enable (theDevice, b);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_amp_enable :\n");
	   fprintf (stderr, "%s \n",
	                   this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//      AmpEnableButton->setChecked (b);
}

//      correction is in Hz
// This function has to be modified to implement ppm correction
// writing in the si5351 register does not seem to work yet
// To be completed
void    hackrfHandler::set_ppmCorrection        (int32_t ppm) {
int res;
uint16_t value;

	res = this -> hackrf_si5351c_write (theDevice,
	                                    162,
	                                    static_cast<uint16_t>(ppm));
	res = this -> hackrf_si5351c_read (theDevice,
	                                   162, &value);
	(void) res;
	qDebug() << "Read si5351c register 162 : " << value <<"\n";
}

//	we use a static large buffer, rather than trying to allocate
//	a buffer on the stack
static std::complex<float>buffer [32 * 32768];
static
int	callback (hackrf_transfer *transfer) {
hackrfHandler *ctx = static_cast <hackrfHandler *>(transfer -> rx_ctx);
int	i;
uint8_t *p	= transfer -> buffer;
RingBuffer<std::complex<float> > * q = & (ctx -> _I_Buffer);

	for (i = 0; i < transfer -> valid_length / 2; i ++) {
	   float re	= (((int8_t *)p) [2 * i]) / 128.0;
	   float im	= (((int8_t *)p) [2 * i + 1]) / 128.0;
	   buffer [i]	= std::complex<float> (re, im);
	}
	q -> putDataIntoBuffer (buffer, transfer -> valid_length / 2);
	return 0;
}

bool	hackrfHandler::restartReader	(void) {
int	res;

	if (running. load ())
	   return true;

	res	= this -> hackrf_start_rx (theDevice, callback, this);	
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_start_rx :\n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return false;
	}
	running. store (this -> hackrf_is_streaming (theDevice));
	return running. load ();
}

void	hackrfHandler::stopReader	(void) {
int	res;

	if (!running. load ())
	   return;

	res	= this -> hackrf_stop_rx (theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_stop_rx : %d\n", res);
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
	running. store (false);
}

//
//	The brave old getSamples. For the hackrf, we get
//	size still in I/Q pairs
int32_t	hackrfHandler::getSamples (std::complex<float> *V,
	                                 int32_t size, uint8_t Mode) { 
	(void)Mode;
	return _I_Buffer. getDataFromBuffer (V, size);
}

int32_t	hackrfHandler::Samples	() {
	return _I_Buffer. GetRingBufferReadAvailable ();
}

uint8_t	hackrfHandler::myIdentity	() {
	return HACKRF;
}

void	hackrfHandler::resetBuffer	() {
	_I_Buffer. FlushRingBuffer ();
}

int16_t	hackrfHandler::bitDepth	() {
	return 8;
}

int32_t	hackrfHandler::getRate	() {
	return inputRate;
}

bool	hackrfHandler::load_hackrfFunctions () {
//
//	link the required procedures
	this -> hackrf_init	= (pfn_hackrf_init)
	                       library_p -> resolve  ("hackrf_init");
	if (this -> hackrf_init == NULL) {
	   fprintf (stderr, "Could not find hackrf_init\n");
	   return false;
	}

	this -> hackrf_open	= (pfn_hackrf_open)
	                       library_p -> resolve ("hackrf_open");
	if (this -> hackrf_open == NULL) {
	   fprintf (stderr, "Could not find hackrf_open\n");
	   return false;
	}

	this -> hackrf_close	= (pfn_hackrf_close)
	                       library_p -> resolve ("hackrf_close");
	if (this -> hackrf_close == NULL) {
	   fprintf (stderr, "Could not find hackrf_close\n");
	   return false;
	}

	this -> hackrf_exit	= (pfn_hackrf_exit)
	                       library_p -> resolve ("hackrf_exit");
	if (this -> hackrf_exit == NULL) {
	   fprintf (stderr, "Could not find hackrf_exit\n");
	   return false;
	}

	this -> hackrf_start_rx	= (pfn_hackrf_start_rx)
	                       library_p -> resolve ("hackrf_start_rx");
	if (this -> hackrf_start_rx == NULL) {
	   fprintf (stderr, "Could not find hackrf_start_rx\n");
	   return false;
	}

	this -> hackrf_stop_rx	= (pfn_hackrf_stop_rx)
	                       library_p -> resolve ("hackrf_stop_rx");
	if (this -> hackrf_stop_rx == NULL) {
	   fprintf (stderr, "Could not find hackrf_stop_rx\n");
	   return false;
	}

	this -> hackrf_device_list	= (pfn_hackrf_device_list)
	                        library_p -> resolve  ("hackrf_device_list");
	if (this -> hackrf_device_list == NULL) {
	   fprintf (stderr, "Could not find hackrf_device_list\n");
	   return false;
	}

	this -> hackrf_set_baseband_filter_bandwidth	=
	                      (pfn_hackrf_set_baseband_filter_bandwidth)
	                      library_p -> resolve (
	                         "hackrf_set_baseband_filter_bandwidth");
	if (this -> hackrf_set_baseband_filter_bandwidth == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_baseband_filter_bandwidth\n");
	   return false;
	}

	this	-> hackrf_compute_baseband_filter_bw	=
	                       (pfn_hackrf_compute_baseband_filter_bw)
	                       library_p -> resolve (
	                          "hackrf_compute_baseband_filter_bw");
	if (this -> hackrf_compute_baseband_filter_bw	== NULL) {
	   fprintf (stderr, "Could not find hackrf_compute_baseband_filter_bw\n");
	   return false;
	}

	this -> hackrf_set_lna_gain	= (pfn_hackrf_set_lna_gain)
	                       library_p -> resolve ("hackrf_set_lna_gain");
	if (this -> hackrf_set_lna_gain == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_lna_gain\n");
	   return false;
	}

	this -> hackrf_set_vga_gain	= (pfn_hackrf_set_vga_gain)
	                       library_p -> resolve ("hackrf_set_vga_gain");
	if (this -> hackrf_set_vga_gain == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_vga_gain\n");
	   return false;
	}

	this -> hackrf_set_freq	= (pfn_hackrf_set_freq)
	                       library_p -> resolve ("hackrf_set_freq");
	if (this -> hackrf_set_freq == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_freq\n");
	   return false;
	}

	this -> hackrf_set_sample_rate	= (pfn_hackrf_set_sample_rate)
	                       library_p -> resolve ("hackrf_set_sample_rate");
	if (this -> hackrf_set_sample_rate == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_sample_rate\n");
	   return false;
	}

	this -> hackrf_is_streaming	= (pfn_hackrf_is_streaming)
	                       library_p -> resolve ("hackrf_is_streaming");
	if (this -> hackrf_is_streaming == NULL) {
	   fprintf (stderr, "Could not find hackrf_is_streaming\n");
	   return false;
	}

	this -> hackrf_error_name	= (pfn_hackrf_error_name)
	                       library_p -> resolve ("hackrf_error_name");
	if (this -> hackrf_error_name == NULL) {
	   fprintf (stderr, "Could not find hackrf_error_name\n");
	   return false;
	}

	this -> hackrf_usb_board_id_name = (pfn_hackrf_usb_board_id_name)
	                       library_p -> resolve ("hackrf_usb_board_id_name");
	if (this -> hackrf_usb_board_id_name == NULL) {
	   fprintf (stderr, "Could not find hackrf_usb_board_id_name\n");
	   return false;
	}

// Aggiunta Fabio
	this -> hackrf_set_antenna_enable = (pfn_hackrf_set_antenna_enable)
	                  library_p -> resolve ("hackrf_set_antenna_enable");
	if (this -> hackrf_set_antenna_enable == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_antenna_enable\n");
	   return false;
	}

	this -> hackrf_set_amp_enable = (pfn_hackrf_set_amp_enable)
	                  library_p -> resolve ("hackrf_set_amp_enable");
	if (this -> hackrf_set_amp_enable == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_amp_enable\n");
	   return false;
	}
	this -> hackrf_si5351c_read = (pfn_hackrf_si5351c_read)
	                 library_p -> resolve ("hackrf_si5351c_read");
	if (this -> hackrf_si5351c_read == nullptr) {
	   fprintf (stderr, "Could not find hackrf_si5351c_read\n");
	   return false;
	}

	this -> hackrf_si5351c_write = (pfn_hackrf_si5351c_write)
	                 library_p -> resolve ("hackrf_si5351c_write");
	if (this -> hackrf_si5351c_write == nullptr) {
	   fprintf (stderr, "Could not find hackrf_si5351c_write\n");
	   return false;
	}

	fprintf (stderr, "OK, functions seem to be loaded\n");
	return true;
}
