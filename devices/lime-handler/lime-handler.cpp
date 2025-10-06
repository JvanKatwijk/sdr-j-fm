#
/*
 *    Copyright (C) 2014 .. 2019
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
 */

#include	<QTime>
#include	"lime-handler.h"
#include	"device-exceptions.h"

#define	FIFO_SIZE	32768
static
int16_t localBuffer [4 * FIFO_SIZE];
lms_info_str_t limedevices [10];

	limeHandler::limeHandler (QSettings *s):
	                             myFrame (nullptr),
	                             _I_Buffer (4* 1024 * 1024) {
	this	-> limeSettings		= s;
	setupUi (&myFrame);
	myFrame. show	();

#ifdef  __MINGW32__
        const char *libraryString = "LimeSuite.dll";
#elif  __clang__
        const char *libraryString = "/opt/local/lib/libLimeSuite.dylib";
#else
        const char *libraryString = "libLimeSuite.so";
#endif

	library_p	= new QLibrary (libraryString);
	library_p	-> load ();

	if (!library_p -> isLoaded ()) {
           throw (device_exception (std::string ("failed to open ") +
                                        std::string (libraryString)));
        }

        libraryLoaded   = true;
	if (!load_limeFunctions()) {
	delete library_p;
           throw (device_exception (std::string ("functions missing in ") +
                                        std::string (libraryString)));
	
        }
//
//      From here we have a library available

	int ndevs	= LMS_GetDeviceList (limedevices);
	if (ndevs == 0) {	// no devices found
	   delete library_p;
           throw (device_exception (std::string ("no devices found ")));
	}

	for (int i = 0; i < ndevs; i ++)
	   fprintf (stderr, "device %s\n", limedevices [i]);

	int res		= LMS_Open (&theDevice, nullptr, nullptr);
	if (res < 0) {	// some error
	   delete library_p;
	   throw (device_exception (std::string ("device open problem ")));
	}

	res		= LMS_Init (theDevice);
	if (res < 0) {	// some error
	   LMS_Close (&theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("device init problem")));
	}

	res		= LMS_GetNumChannels (theDevice, LMS_CH_RX);
	if (res < 0) {	// some error
	   LMS_Close (&theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("device channel problem")));
	}

	fprintf (stderr, "device %s supports %d channels\n",
	                            limedevices [0], res);
	res		= LMS_EnableChannel (theDevice, LMS_CH_RX, 0, true);
	if (res < 0) {	// some error
	   LMS_Close (theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("device channel problem")));
	}

	res	= LMS_SetSampleRate (theDevice, 2304000.0, 0);
	if (res < 0) {
	   LMS_Close (theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("samplerate problem")));
	}

	float_type host_Hz, rf_Hz;
	res	= LMS_GetSampleRate (theDevice, LMS_CH_RX, 0,
	                                            &host_Hz, &rf_Hz);

	fprintf (stderr, "samplerate = %f %f\n", (float)host_Hz, (float)rf_Hz);
	
	res		= LMS_GetAntennaList (theDevice, LMS_CH_RX, 0, antennas);
	for (int i = 0; i < res; i ++) 	
	   antennaList	-> addItem (QString (antennas [i]));

	limeSettings -> beginGroup ("limeSettings");
	QString antenne	= limeSettings -> value ("antenna", "default"). toString();
	limeSettings	-> endGroup();

	int k       = antennaList -> findText (antenne);
        if (k != -1) 
           antennaList -> setCurrentIndex (k);
	
	connect (antennaList, SIGNAL (activated (int)),
	         this, SLOT (setAntenna (int)));

//	default antenna setting
	res		= LMS_SetAntenna (theDevice, LMS_CH_RX, 0, 
	                           antennaList -> currentIndex());

//	default frequency
	res		= LMS_SetLOFrequency (theDevice, LMS_CH_RX,
	                                                 0, 220000000.0);
	if (res < 0) {
	   LMS_Close (theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("frequency problem")));
	}

	res		= LMS_SetLPFBW (theDevice, LMS_CH_RX,
	                                               0, 1536000.0);
	if (res < 0) {
	   LMS_Close (theDevice);
	   delete library_p;
	   throw (device_exception (std::string ("bandwidth problem")));
	}

	LMS_SetGaindB (theDevice, LMS_CH_RX, 0, 50);

	LMS_Calibrate (theDevice, LMS_CH_RX, 0, 2500000.0, 0);
	
	
	limeSettings	-> beginGroup ("limeSettings");
	k	= limeSettings	-> value ("gain", 50). toInt();
	limeSettings	-> endGroup();
	gainSelector -> setValue (k);
	setGain (k);
	connect (gainSelector, SIGNAL (valueChanged (int)),
	         this, SLOT (setGain (int)));
	connect (dumpButton, SIGNAL (clicked ()),
	         this, SLOT (set_xmlDump ()));
	running. store (false);
}

	limeHandler::~limeHandler() {
	stopReader ();
	running. store (false);
	while (isRunning())
	   usleep (100);
	limeSettings	-> beginGroup ("limeSettings");
	limeSettings	-> setValue ("antenna", antennaList -> currentText());
	limeSettings	-> setValue ("gain", gainSelector -> value());
	limeSettings	-> endGroup();
	LMS_Close (theDevice);
	delete library_p;
}

int32_t	limeHandler::getRate		() {
	return KHz (2304);
}

void	limeHandler::setVFOFrequency	(int32_t f) {
	if (legalFrequency (f))
	   lastFrequency = f;
	LMS_SetLOFrequency (theDevice, LMS_CH_RX, 0, f);
}

int32_t	limeHandler::getVFOFrequency	() {
float_type freq;
	int res = LMS_GetLOFrequency (theDevice, LMS_CH_RX, 0, &freq);
	(void)res;
	return (int)freq;
}

uint8_t	limeHandler::myIdentity		() {
	return LIME;
}

bool	limeHandler::legalFrequency	(int32_t f) {
	(void)f;
	return true;
}

int32_t	limeHandler::defaultFrequency	() {
	return KHz (94700);
}

bool	limeHandler::restartReader	() {
int	res;

	if (isRunning())
	   return true;
	LMS_SetLOFrequency (theDevice, LMS_CH_RX, 0, lastFrequency);
	stream. isTx            = false;
        stream. channel         = 0;
        stream. fifoSize        = FIFO_SIZE;
        stream. throughputVsLatency     = 0.1;  // ???
        stream. dataFmt         = lms_stream_t::LMS_FMT_I12;    // 12 bit ints
	res     = LMS_SetupStream (theDevice, &stream);
        if (res < 0)
           return false;
        res     = LMS_StartStream (&stream);
        if (res < 0)
           return false;

	start	();
	return true;
}
	
void	limeHandler::stopReader		() {
	if (!isRunning	())
	   return;
	running. store (false);
	while (isRunning())
	   usleep (200);
	(void)LMS_StopStream	(&stream);	
	(void)LMS_DestroyStream	(theDevice, &stream);
}

int	limeHandler::getSamples	(std::complex<float> *V, int32_t size) {
std::complex<int16_t> temp [size];
int i;
        int amount      = _I_Buffer. getDataFromBuffer (temp, size);
        for (i = 0; i < amount; i ++)
           V [i] = std::complex<float> (real (temp [i]) / 2048.0,
                                        imag (temp [i]) / 2048.0);
        return amount;
}

int	limeHandler::getSamples	(std::complex<float> *V,
	                              int32_t size, uint8_t Mode) {
std::complex<int16_t> temp [size];
int i;
	(void)Mode;
        int amount      = _I_Buffer. getDataFromBuffer (temp, size);
        for (i = 0; i < amount; i ++)
           V [i] = std::complex<float> (real (temp [i]) / 2048.0,
                                        imag (temp [i]) / 2048.0);
        return amount;
}

int	limeHandler::Samples() {
	return _I_Buffer. GetRingBufferReadAvailable();
}

void	limeHandler::resetBuffer() {
	_I_Buffer. FlushRingBuffer();
}

int16_t	limeHandler::bitDepth() {
	return 12;
}
//
//	GUI handlers
void	limeHandler::setGain		(int g) {
float_type gg;
	LMS_SetGaindB (theDevice, LMS_CH_RX, 0, g);
	LMS_GetNormalizedGain (theDevice, LMS_CH_RX, 0, &gg);
	actualGain	-> display (gg);
}

void	limeHandler::setAntenna		(int ind) {
	(void)LMS_SetAntenna (theDevice, LMS_CH_RX, 0, ind);
}

void	limeHandler::showErrors		(int underrun, int overrun) {
	underrunDisplay	-> display (underrun);
	overrunDisplay	-> display (overrun);
}


void	limeHandler::run () {
int	res;
lms_stream_status_t streamStatus;
int	underruns	= 0;
int	overruns	= 0;
int	amountRead	= 0;

	running. store (true);
	while (running. load()) {
	   res = LMS_RecvStream (&stream, localBuffer,
	                                     FIFO_SIZE,  &meta, 1000);
	   if (res > 0) {
	      _I_Buffer. putDataIntoBuffer (localBuffer, res);
	      amountRead	+= res;
	      res	= LMS_GetStreamStatus (&stream, &streamStatus);
	      underruns	+= streamStatus. underrun;
	      overruns	+= streamStatus. overrun;
	   }
	   if (amountRead > 4 * 2048000) {
	      amountRead = 0;
	      showErrors (underruns, overruns);
	      underruns	= 0;
	      overruns	= 0;
	   }
	}
}

bool	limeHandler::load_limeFunctions() {

	this	-> LMS_GetDeviceList = (pfn_LMS_GetDeviceList)
	                    library_p -> resolve ("LMS_GetDeviceList");
	if (this -> LMS_GetDeviceList == nullptr) {
	   fprintf (stderr, "could not find LMS_GetdeviceList\n");
	   return false;
	}
	this	-> LMS_Open = (pfn_LMS_Open)
	                    library_p -> resolve ("LMS_Open");
	if (this -> LMS_Open == nullptr) {
	   fprintf (stderr, "could not find LMS_Open\n");
	   return false;
	}
	this	-> LMS_Close = (pfn_LMS_Close)
	                    library_p -> resolve ("LMS_Close");
	if (this -> LMS_Close == nullptr) {
	   fprintf (stderr, "could not find LMS_Close\n");
	   return false;
	}
	this	-> LMS_Init = (pfn_LMS_Init)
	                    library_p -> resolve ("LMS_Init");
	if (this -> LMS_Init == nullptr) {
	   fprintf (stderr, "could not find LMS_Init\n");
	   return false;
	}
	this	-> LMS_GetNumChannels = (pfn_LMS_GetNumChannels)
	                    library_p -> resolve ("LMS_GetNumChannels");
	if (this -> LMS_GetNumChannels == nullptr) {
	   fprintf (stderr, "could not find LMS_GetNumChannels\n");
	   return false;
	}
	this	-> LMS_EnableChannel = (pfn_LMS_EnableChannel)
	                    library_p -> resolve ("LMS_EnableChannel");
	if (this -> LMS_EnableChannel == nullptr) {
	   fprintf (stderr, "could not find LMS_EnableChannel\n");
	   return false;
	}
	this	-> LMS_SetSampleRate = (pfn_LMS_SetSampleRate)
	                    library_p -> resolve ("LMS_SetSampleRate");
	if (this -> LMS_SetSampleRate == nullptr) {
	   fprintf (stderr, "could not find LMS_SetSampleRate\n");
	   return false;
	}
	this	-> LMS_GetSampleRate = (pfn_LMS_GetSampleRate)
	                    library_p -> resolve ("LMS_GetSampleRate");
	if (this -> LMS_GetSampleRate == nullptr) {
	   fprintf (stderr, "could not find LMS_GetSampleRate\n");
	   return false;
	}
	this	-> LMS_SetLOFrequency = (pfn_LMS_SetLOFrequency)
	                    library_p -> resolve ("LMS_SetLOFrequency");
	if (this -> LMS_SetLOFrequency == nullptr) {
	   fprintf (stderr, "could not find LMS_SetLOFrequency\n");
	   return false;
	}
	this	-> LMS_GetLOFrequency = (pfn_LMS_GetLOFrequency)
	                    library_p -> resolve ("LMS_GetLOFrequency");
	if (this -> LMS_GetLOFrequency == nullptr) {
	   fprintf (stderr, "could not find LMS_GetLOFrequency\n");
	   return false;
	}
	this	-> LMS_GetAntennaList = (pfn_LMS_GetAntennaList)
	                    library_p -> resolve ("LMS_GetAntennaList");
	if (this -> LMS_GetAntennaList == nullptr) {
	   fprintf (stderr, "could not find LMS_GetAntennaList\n");
	   return false;
	}
	this	-> LMS_SetAntenna = (pfn_LMS_SetAntenna)
	                    library_p -> resolve ("LMS_SetAntenna");
	if (this -> LMS_SetAntenna == nullptr) {
	   fprintf (stderr, "could not find LMS_SetAntenna\n");
	   return false;
	}
	this	-> LMS_GetAntenna = (pfn_LMS_GetAntenna)
	                    library_p -> resolve ("LMS_GetAntenna");
	if (this -> LMS_GetAntenna == nullptr) {
	   fprintf (stderr, "could not find LMS_GetAntenna\n");
	   return false;
	}
	this	-> LMS_GetAntennaBW = (pfn_LMS_GetAntennaBW)
	                    library_p -> resolve ("LMS_GetAntennaBW");
	if (this -> LMS_GetAntennaBW == nullptr) {
	   fprintf (stderr, "could not find LMS_GetAntennaBW\n");
	   return false;
	}
	this	-> LMS_SetNormalizedGain = (pfn_LMS_SetNormalizedGain)
	                    library_p -> resolve ("LMS_SetNormalizedGain");
	if (this -> LMS_SetNormalizedGain == nullptr) {
	   fprintf (stderr, "could not find LMS_SetNormalizedGain\n");
	   return false;
	}
	this	-> LMS_GetNormalizedGain = (pfn_LMS_GetNormalizedGain)
	                    library_p -> resolve ("LMS_GetNormalizedGain");
	if (this -> LMS_GetNormalizedGain == nullptr) {
	   fprintf (stderr, "could not find LMS_GetNormalizedGain\n");
	   return false;
	}
	this	-> LMS_SetGaindB = (pfn_LMS_SetGaindB)
	                    library_p -> resolve ("LMS_SetGaindB");
	if (this -> LMS_SetGaindB == nullptr) {
	   fprintf (stderr, "could not find LMS_SetGaindB\n");
	   return false;
	}
	this	-> LMS_GetGaindB = (pfn_LMS_GetGaindB)
	                    library_p -> resolve ("LMS_GetGaindB");
	if (this -> LMS_GetGaindB == nullptr) {
	   fprintf (stderr, "could not find LMS_GetGaindB\n");
	   return false;
	}
	this	-> LMS_SetLPFBW = (pfn_LMS_SetLPFBW)
	                    library_p -> resolve ("LMS_SetLPFBW");
	if (this -> LMS_SetLPFBW == nullptr) {
	   fprintf (stderr, "could not find LMS_SetLPFBW\n");
	   return false;
	}
	this	-> LMS_GetLPFBW = (pfn_LMS_GetLPFBW)
	                    library_p -> resolve ("LMS_GetLPFBW");
	if (this -> LMS_GetLPFBW == nullptr) {
	   fprintf (stderr, "could not find LMS_GetLPFBW\n");
	   return false;
	}
	this	-> LMS_Calibrate = (pfn_LMS_Calibrate)
	                    library_p -> resolve ("LMS_Calibrate");
	if (this -> LMS_Calibrate == nullptr) {
	   fprintf (stderr, "could not find LMS_Calibrate\n");
	   return false;
	}
	this	-> LMS_SetupStream = (pfn_LMS_SetupStream)
	                    library_p -> resolve ("LMS_SetupStream");
	if (this -> LMS_SetupStream == nullptr) {
	   fprintf (stderr, "could not find LMS_SetupStream\n");
	   return false;
	}
	this	-> LMS_DestroyStream = (pfn_LMS_DestroyStream)
	                    library_p -> resolve ("LMS_DestroyStream");
	if (this -> LMS_DestroyStream == nullptr) {
	   fprintf (stderr, "could not find LMS_DestroyStream\n");
	   return false;
	}
	this	-> LMS_StartStream = (pfn_LMS_StartStream)
	                    library_p -> resolve ("LMS_StartStream");
	if (this -> LMS_StartStream == nullptr) {
	   fprintf (stderr, "could not find LMS_StartStream\n");
	   return false;
	}
	this	-> LMS_StopStream = (pfn_LMS_StopStream)
	                    library_p -> resolve ("LMS_StopStream");
	if (this -> LMS_StopStream == nullptr) {
	   fprintf (stderr, "could not find LMS_StopStream\n");
	   return false;
	}
	this	-> LMS_RecvStream = (pfn_LMS_RecvStream)
	                    library_p -> resolve ("LMS_RecvStream");
	if (this -> LMS_RecvStream == nullptr) {
	   fprintf (stderr, "could not find LMS_RecvStream\n");
	   return false;
	}
	this	-> LMS_GetStreamStatus = (pfn_LMS_GetStreamStatus)
	                    library_p -> resolve ("LMS_GetStreamStatus");
	if (this -> LMS_GetStreamStatus == nullptr) {
	   fprintf (stderr, "could not find LMS_GetStreamStatus\n");
	   return false;
	}

	return true;
}

