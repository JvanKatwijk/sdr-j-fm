#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J-FM program.
 *
 *    SDR-J-FM is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J-FM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J-FM; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "fm-processor.h"
#include "audiosink.h"
#include "device-handler.h"
#include "fm-constants.h"
#include "radio.h"


#define AUDIO_FREQ_DEV_PROPORTION    0.85f
#define PILOT_FREQUENCY		19000
#define RDS_FREQUENCY		(3 * PILOT_FREQUENCY)
#define RDS_RATE		19000   // 16 samples for one RDS sympols
#define OMEGA_PILOT	((DSPFLOAT(PILOT_FREQUENCY)) / fmRate) * (2 * M_PI)

#define	IRate	(inputRate / 6)

// predefine to make some line more compact later
#ifdef DO_STEREO_SEPARATION_TEST
   #define PILOTTESTDELAY (pilotDelay2)
#else
   #define PILOTTESTDELAY (0)
#endif


//
//	Note that no decimation done as yet: the samplestream is still
//	full speed, up to 2304000 samples/second
	fmProcessor::fmProcessor (deviceHandler *theDevice,
	                          RadioInterface *RI,
	                          audioSink *mySink,
	                          fm_Demodulator *theDemodulator,
	                          int32_t inputRate,
	                          int32_t fmRate,
	                          int32_t workingRate,
	                          int32_t audioRate,
	                          int32_t displaySize,
	                          int32_t spectrumSize,
	                          int32_t averageCount,
	                          int32_t repeatRate,
	                          int	ptyLocale,
	                          RingBuffer<double> *hfBuffer,
	                          RingBuffer<double> *lfBuffer,
	                          RingBuffer<DSPCOMPLEX> *iqBuffer,
	                          int16_t thresHold):
	                             myRdsDecoder (RI, RDS_RATE),
	                             localOscillator (inputRate),
	                             mySinCos (fmRate),
	                             pssAGC (100.0f/fmRate, 0.3f, 2.0f),
	                             fmBand_1     (4 * inputRate / IRate + 1,
	                                           fmRate / 2,
	                                           inputRate,
	                                           inputRate / IRate),
	                             fmBand_2     (IRate / fmRate + 1,
	                                           fmRate / 2,
	                                           IRate,
	                                           IRate / fmRate),
	                             fmAudioFilter (2 * 4096, 756),
	                             fmFilter	   (2 * 32768, 251),
	                             pilotRecover (fmRate, OMEGA_PILOT,
	                                           10 * (2 * M_PI) / fmRate,
	                                           &mySinCos), 
	                             pPSS (fmRate, 10.0f / fmRate,
	                                            &mySinCos) ,
	                             rdsBandPassFilter (FFT_SIZE,
	                                           PILOTFILTER_SIZE),
	                             rdsHilbertFilter (FFT_SIZE,
	                                                PILOTFILTER_SIZE),
	                             mySquelch (1, 70000, fmRate / 20,
	                                                   fmRate),
		                     theConverter  (workingRate,
	                                            audioRate,
	                                            workingRate / 20) {
//
	this	-> running. store (false);
	this	-> theDemodulator	= theDemodulator;
	this	-> myRig		= theDevice;
	this	-> myRadioInterface	= RI;
	this	-> theSink		= mySink;
	this	-> inputRate		= inputRate;
	this	-> fmRate		= fmRate;
	this	-> decimatingScale	= inputRate / fmRate;
	this	-> workingRate		= workingRate;
	this	-> audioRate		= audioRate;
	this	-> displaySize		= displaySize;
	this	-> averageCount		= averageCount;
	this	-> repeatRate		= repeatRate;
	this	-> ptyLocale		= ptyLocale;
	this	-> hfBuffer		= hfBuffer;
	this	-> lfBuffer		= lfBuffer;
	this	-> thresHold		= thresHold;
	this	-> scanning		= false;
	this	-> Lgain		= 20;
	this	-> Rgain		= 20;

	this	-> audioFrequency	= 15000;
	this	-> newAudioFilter. store (false);
	this	-> squelchMode		= ESqMode::OFF;
	this	-> spectrumSampleRate = fmRate;
	this	-> iqBuffer		= iqBuffer;
//
//	inits that cannot be done by older GCC versions
	this	-> hfBuffer_newFlag	= true;
	this	-> lfBuffer_newFlag	= true;
	this	-> displayBuffer_lf	= nullptr;
	this	-> autoMono		= true;
	this	-> pssActive		= true;
	this	-> peakLevelCurSampleCnt	= 0;
	this	-> peakLevelSampleMax	= 0x7FFFFFF;
	this	-> absPeakLeft		= 0.0f;
	this	-> absPeakRight		= 0.0f;
	this	-> volumeFactor		= 0.5f;
	this	-> panorama		= 1.0f;

	this	-> suppressAudioSampleCntMax	= workingRate / 2; // suppress audio 0.5 second
	this	-> suppressAudioSampleCnt	= suppressAudioSampleCntMax; // suppress audio while startup

	this	-> rdsModus		= rdsDecoder::ERdsMode::RDS_OFF;
	this	-> DCREnabled		= true;
	this	-> RfDC			= DSPCOMPLEX (0, 0);

	this	-> lfPlotType		= ELfPlot::DEMODULATOR;
	this	-> showFullSpectrum	= false;
	this	-> spectrumSampleRate	= fmRate;
	this	-> zoomFactor		= 1;

	int	Df			= 1000;
	int	f			= 192000;
//	fprintf (stderr, "order = %f\n", (float)f / Df * 40 / 22);
// workingRate is typ. 48000S/s
	peakLevelSampleMax		= workingRate / 50;

	this	-> spectrumSize		= 4 * displaySize;

	this	-> spectrum_fft_hf	= new common_fft (this -> spectrumSize);
	this	-> spectrumBuffer_hf	= spectrum_fft_hf -> getVector ();

	this	-> spectrum_fft_lf	= new common_fft (spectrumSize);

	this	-> loFrequency		= 0;
	this	-> fmBandwidth		= 0.95 * fmRate;
//
//	fmFilter is rather heavy on resource consumption, it is usually off
	fmFilter. setLowPass (0.95 * fmRate / 2, 2 * fmRate);
	this	-> fmFilterOn. store (false);
	this	-> newFilter. store (false);
  /*
   *	default values, will be set through the user interface
   *	to their appropriate values
   */
	this	-> fmModus		= FM_Mode::Stereo;
	this	-> soundSelector	= S_STEREO;
	this	-> balance		= 0;
	this	-> leftChannel		= 1.0;
	this	-> rightChannel		= 1.0;
	
#ifdef USE_EXTRACT_LEVELS
	this	-> noiseLevel 		= 0;
	this	-> pilotLevel 		= 0;
	this	-> rdsLevel		= 0;
#endif

	pilotDelayPSS = 0;
#ifdef DO_STEREO_SEPARATION_TEST
	pilotDelay2 = 0;
#endif
	fmAudioFilterActive . store (false);
//
//	the constant K_FM is still subject to many questions
// highest freq in message
	DSPFLOAT F_G			= 0.65 * fmRate / 2;
	DSPFLOAT Delta_F		= 0.95 * fmRate / 2;    //
	DSPFLOAT B_FM			= 2 * (Delta_F + F_G);

//
//	K_FM depends on fmRate which is known, it turns out that
//	K_FM	is about 15
	K_FM				= B_FM * M_PI / F_G;

//
	rdsBandPassFilter. setSimple(RDS_FREQUENCY - RDS_WIDTH / 2,
//	rdsBandPassFilter. setBand  (RDS_FREQUENCY - RDS_WIDTH / 2,
	                             RDS_FREQUENCY + RDS_WIDTH / 2,
	                             fmRate);

// for the deemphasis we use an in-line filter with
	lastAudioSample			= 0;
	deemphAlpha			= 1.0 / (fmRate / (1000000.0 / 50.0 + 1));

	dumping				= false;
	dumpFile			= nullptr;


	displayBuffer_lf = new double [displaySize];

	connect (&mySquelch, SIGNAL (setSquelchIsActive (bool)),
	         myRadioInterface, SLOT (setSquelchIsActive (bool)));
	connect (this, SIGNAL (hfBufferLoaded ()),
	         myRadioInterface, SLOT (hfBufferLoaded ()));
	connect (this, SIGNAL (lfBufferLoaded (bool, int)),
	         myRadioInterface, SLOT (lfBufferLoaded (bool, int)));
	connect (this, SIGNAL (iqBufferLoaded ()),
	         myRadioInterface, SLOT (iqBufferLoaded ()));
	connect (this, SIGNAL (showPeakLevel (float, float)),
	         myRadioInterface, SLOT (showPeakLevel (float, float)));
	connect (this, &fmProcessor::showMetaData,
	         myRadioInterface, &RadioInterface::showMetaData);  // the macro version is not working here!?
	connect (this, SIGNAL (scanresult ()),
	         myRadioInterface,SLOT(scanresult()));

	squelchValue     = 0;
	oldSquelchValue = 0;

	myCount = 0;
}

		fmProcessor::~fmProcessor () {
	stop();
	delete	spectrum_fft_hf;
	delete	spectrum_fft_lf;
	delete[] displayBuffer_lf;
}

void	fmProcessor::stop () {
	if (running. load ()) {
	   running . store (false);
	   while (!isFinished()) {
	      usleep(100);
	   }
	}
}

#ifdef USE_EXTRACT_LEVELS

DSPFLOAT fmProcessor::get_pilotStrength () {
	if (running. load ()) 
	   return get_db (pilotLevel, 128) - get_db (0, 128);
	return 0.0;
}

DSPFLOAT fmProcessor::get_rdsStrength () {
	if (running. load ()) {
	   return get_db (rdsLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}

DSPFLOAT fmProcessor::get_noiseStrength () {
	if (running. load ()) {
	   return get_db (noiseLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}
#endif

void	fmProcessor::set_squelchValue (int16_t n) {
	squelchValue	= n;
}

bool	fmProcessor::getSquelchState	() {
	return mySquelch. getSquelchActive  ();
}

DSPFLOAT fmProcessor::get_demodDcComponent () {
	if (running. load ()) {
	   return theDemodulator -> get_DcComponent ();
	}
	return 0.0;
}

//	changing a filter is in two steps: here we set a marker,
//	but the actual filter is created in the mainloop of
//	the processor
//
void	fmProcessor::setBandwidth (const QString &f) {
	if (f == "Off")
	   fmFilterOn . store (false);
	else {
	   fmBandwidth = Khz (std::stol (f.toStdString ()));
	   newFilter. store (true);
	}
}

//void	fmProcessor::setBandfilterDegree (int32_t d) {
//	fmFilterDegree = d;
//	newFilter. store (false);
//}

void	fmProcessor::setfmMode (FM_Mode m) {
	fmModus = m;
}

void	fmProcessor::setlfPlotType (ELfPlot m) {
	lfPlotType = m;

	showFullSpectrum	= true;
	switch (m) {
	   case ELfPlot::IF_FILTERED:
	      spectrumSampleRate = fmRate;
	      break;
	   case ELfPlot::RDS_INPUT:
	      spectrumSampleRate = RDS_RATE;
	      break;
	   case ELfPlot::RDS_DEMOD:
	      spectrumSampleRate = RDS_RATE / 16;  
	      break; // TODO: 16 should not be fixed here
	   default:
	      spectrumSampleRate = fmRate; 
	      showFullSpectrum = false;
	      break;
	}

	lfBuffer_newFlag = true;
}

void	fmProcessor::setlfPlotZoomFactor (int32_t ZoomFactor) {
	zoomFactor = ZoomFactor;
	lfBuffer_newFlag = true;
}

void	fmProcessor::setSoundMode (uint8_t selector) {
	this -> soundSelector = selector;
}

void	fmProcessor::setStereoPanorama(int16_t iStereoPan) {
// iStereoPan range: 0 (Mono) ... +100 (Stereo) ... +200 (Stereo with widen panorama)
	panorama = (DSPFLOAT)iStereoPan / 100.0f;
}

void	fmProcessor::setSoundBalance (int16_t new_balance) {
//	range: -100 <= balance <= +100
	balance = new_balance;
//	leftChannel   = -(balance - 50.0) / 100.0;
//	rightChannel  = (balance + 50.0) / 100.0;
	leftChannel  = (balance > 0 ? (100 - balance) / 100.0 : 1.0f);
	rightChannel = (balance < 0 ? (100 + balance) / 100.0 : 1.0f);
}

//	Deemphasis	= 50 usec (3183 Hz, Europe)
//	Deemphasis	= 75 usec (2122 Hz US)
//	tau		= 2 * M_PI * Freq = 1000000 / time
void	fmProcessor::setDeemphasis (int16_t v) {
DSPFLOAT Tau;
	Q_ASSERT(v >= 1);

	Tau   = 1000000.0 / v;
	deemphAlpha = 1.0 / (DSPFLOAT(fmRate) / Tau + 1.0);
}

void	fmProcessor::setVolume (const float iVolGainDb) {
	volumeFactor = std::pow (10.0f, iVolGainDb / 20.0f);
}

DSPCOMPLEX fmProcessor::audioGainCorrection (DSPCOMPLEX z) {
const DSPFLOAT left  = volumeFactor * leftChannel * real(z);
const DSPFLOAT right = volumeFactor * rightChannel * imag(z);

#if 0
	{
	   static DSPFLOAT leftAbsMax = -1e38f;
	   static DSPFLOAT rightAbsMax = -1e38f;
	   static DSPFLOAT lastVolume = 0.0f;
	   bool printMaxValues = false;

	   if (lastVolume != volumeFactor) {
	      lastVolume = volumeFactor;
	      leftAbsMax = rightAbsMax = -1e38f;
	   }

	   if (abs (left) > leftAbsMax) {
	      leftAbsMax = abs (left);
	      printMaxValues = true;
	   }

	   if (abs (right) > rightAbsMax) {
	      rightAbsMax = abs (right);
	      printMaxValues = true;
	   }

	   if (printMaxValues) {
	      qInfo ("leftAbsMax: %f, rightAbsMax: %f",
	                                    leftAbsMax, rightAbsMax);
	   }
	}
#endif

	return { left, right };
}

void	fmProcessor::startDumping (SNDFILE *f) {
	if (dumping) {
	   return;
	}
//	do not change the order here, another thread might get confused
	dumpFile = f;
	dumping  = true;
}

void	fmProcessor::stopDumping () {
	dumping = false;
}

void	fmProcessor::setAttenuation (DSPFLOAT l, DSPFLOAT r) {
#ifdef DO_STEREO_SEPARATION_TEST
	(void)r;
	PILOTTESTDELAY = l / 180.0f * M_PI;
#else
	Lgain = l;
	Rgain = r;
#endif
}

void	fmProcessor::startScanning () {
	scanning = true;
}

void	fmProcessor::stopScanning () {
	scanning = false;
}

//
//	In this variant, we have a separate thread for the
//	fm processing

void	fmProcessor::run () {
const int32_t bufferSize	= 2 * 8192;
DSPCOMPLEX	dataBuffer [bufferSize];
double		displayBuffer_hf [displaySize];
int32_t		hfCount		= 0;
int32_t		lfCount		= 0;
int32_t		scanPointer	= 0;
common_fft	scan_fft (1024);
DSPCOMPLEX	*scanBuffer	= scan_fft. getVector ();
const float rfDcAlpha = 1.0f / inputRate;
newConverter	audioDecimator (fmRate,  workingRate,  fmRate / 1000);
DSPCOMPLEX	audioOut [audioDecimator. getOutputsize ()];
newConverter	rdsDecimator (fmRate, RDS_RATE, fmRate / 1000);
int		iqCounter	= 0;

	running. store (true); // will be set to false from the outside

	while (running. load ()) {
	   while (running. load () && (myRig -> Samples () < bufferSize)) {
	      msleep(1); // should be enough
	   }

	   if (!running. load ()) {
	      break;
	   }

//	First: update according to potentially changed settings
	   if (newFilter. load ()) {
	      fmFilter. setLowPass (fmBandwidth / 2, 2 * fmRate);
	      fmFilterOn. store (true);
	   }
	   newFilter. store (false);

	   if (newAudioFilter. load ()) {
	      fmAudioFilter. setLowPass (audioFrequency, fmRate);
	      fmAudioFilterActive. store (true);
	      fprintf (stderr, "audiofilter set to %d\n", audioFrequency);
	      newAudioFilter. store (false);
	   }

	   if (squelchValue != oldSquelchValue) {
	      mySquelch. setSquelchLevel (squelchValue);
	      oldSquelchValue = squelchValue;
	   }
//
//	next phase
	   const int32_t amount =
	              myRig -> getSamples (dataBuffer, bufferSize, IandQ);
	   const int32_t aa = (amount >= spectrumSize ? spectrumSize : amount);

	   if (DCREnabled) {
	      for (int32_t i = 0; i < amount; i++) {
	         RfDC = (dataBuffer[i] - RfDC) * rfDcAlpha + RfDC;

//	limit the maximum DC correction because an AM
//	carrier at exactly 0 Hz could have been suppressed, too
	         constexpr DSPFLOAT DCRlimit = 0.01f;
	         DSPFLOAT rfDcReal = real (RfDC);
	         DSPFLOAT rfDcImag = imag (RfDC);
	         if (rfDcReal > +DCRlimit)
	            rfDcReal = +DCRlimit;
	         else
	         if (rfDcReal < -DCRlimit)
	            rfDcReal = -DCRlimit;

	         if (rfDcImag > +DCRlimit)
	            rfDcImag = +DCRlimit;
	         else
	         if (rfDcImag < -DCRlimit)
	            rfDcImag = -DCRlimit;

	         dataBuffer [i] -= std::complex<float> (rfDcReal, rfDcImag);
	      }
	   }

//	for the HFscope
	   if (++hfCount > (inputRate / bufferSize) / repeatRate) {
	      double Y_Values [displaySize];

	      for (int32_t i = 0; i < aa; i++) 
	         spectrumBuffer_hf [i] = dataBuffer[i];

	      for (int32_t i = aa; i < spectrumSize; i++) 
	         spectrumBuffer_hf [i] = std::complex<float> (0, 0);

	      spectrum_fft_hf -> do_FFT ();

	      int32_t zoomFactor = 1;
	      mapSpectrum (spectrumBuffer_hf, Y_Values, zoomFactor);

	      if (hfBuffer_newFlag) {
	         set_average_buffer (Y_Values, displayBuffer_hf);
	         hfBuffer_newFlag = false;
	      }
	      else {
	         add_to_average (Y_Values, displayBuffer_hf);
	      }

	      hfBuffer -> putDataIntoBuffer (displayBuffer_hf, displaySize);
	      hfCount = 0;

// and signal the GUI thread that we have data
	      emit hfBufferLoaded();
	   }

	   if (dumping) {
	      float dumpBuffer [2 * amount];

	      for (int32_t i = 0; i < amount; i++) {
	         dumpBuffer [2 * i]	= real (dataBuffer [i]);
	         dumpBuffer [2 * i + 1] = imag (dataBuffer [i]);
	      }
	      sf_writef_float (dumpFile, dumpBuffer, amount);
	   }

//	   Here we really start

//	   We assume that if/when the pilot is no more than 3 db's above
//	   the noise around it, it is better to decode mono
	   for (int32_t i = 0; i < amount; i++) {
	      std::complex<float> v =
	                  std::complex<float> (real (dataBuffer [i]) * Lgain,
	                                       imag (dataBuffer [i]) * Rgain);

	      v = v * localOscillator. nextValue (loFrequency);

//	   first step: decimating and  - optional - filtering
	      if (decimatingScale > 1) {
	         if (!fmBand_1. Pass (v, &v)) 
	            continue;
	         if (fmFilterOn. load ())
	            v = fmFilter. Pass (v);
	         if (!fmBand_2. Pass (v, &v))
	            continue;
	      }

//	   second step: if we are scanning, do the scan
//	   Samplerate here is fmRate
//
	      if (scanning) {
	         scanBuffer [scanPointer++] = v;

	         if (scanPointer >= 1024) {
	            scanPointer = 0;
	            scan_fft. do_FFT ();
	            float signal	= getSignal (scanBuffer, 1024);
	            float Noise		= getNoise (scanBuffer, 1024);
	            if (get_db (signal, 256) - get_db (Noise, 256) > thresHold) {
	               fprintf (stderr, "signal found %f %f\n",
	                              get_db (signal, 256), get_db (Noise, 256));
	               emit scanresult ();
	            }
	         }
	         continue; // no signal processing!!!!
	      }

	      DSPFLOAT demod = theDemodulator -> demodulate (v);

	      switch (squelchMode) {
	         case ESqMode::NSQ:
	            demod = mySquelch. do_noise_squelch (demod);
	            break;

	         case ESqMode::LSQ:
	            demod = mySquelch. do_level_squelch (demod,
	                                theDemodulator -> get_carrier_ampl ());
	            break;

	         default:;
	      }

	      std::complex<float> audio;
	      std::complex<float> rdsDataCplx;

	      process_signal_with_rds (demod, &audio, &rdsDataCplx);

	      const DSPFLOAT sumLR  = real (audio);
	      const DSPFLOAT diffLR = imag (audio);
	      const DSPFLOAT diffLRWeightend =
	                 diffLR * (fmModus == FM_Mode::StereoPano ?
	                                                 panorama : 1.0f);
	      const DSPFLOAT left  =
	                 sumLR + diffLRWeightend;  // 2L = (L+R) + (L-R)
	      const DSPFLOAT right =
	                 sumLR - diffLRWeightend;  // 2R = (L+R) - (L-R)

	      switch (soundSelector) {
	         default:
	         case S_STEREO:
	            audio = std::complex<float> (left,  right);
	            break;
	         case S_STEREO_SWAPPED:
	            audio = std::complex<float> (right, left);
	            break;
	         case S_LEFT:
	            audio = std::complex<float> (left,  left);
	            break;
	         case S_RIGHT:
	            audio = std::complex<float> (right, right);
	            break;
	         case S_LEFTplusRIGHT:
	            audio = std::complex<float> (sumLR, sumLR);
	            break;
	         case S_LEFTminusRIGHT:
	         case S_LEFTminusRIGHT_Test:
	            audio = std::complex<float> (diffLRWeightend,
	                                         diffLRWeightend);
	            break;
	      }

	      if (rdsModus != rdsDecoder::ERdsMode::RDS_OFF) {
	         int32_t rdsAmount;
	         
	         std::complex<float> rdsOut [rdsDecimator. getOutputsize ()];
	         if (rdsDecimator. convert (rdsDataCplx, rdsOut, &rdsAmount)) {
//	   here the sample rate is rdsRate (typ. 19000S/s)
	            for (int32_t k = 0; k < rdsAmount; k++) {
	               std::complex<float> pcmSample = rdsOut [k];
	           
	               static std::complex<float> magCplx;
//	   input SR 19000S/s, output SR 19000/16S/s
	               if (myRdsDecoder. doDecode (pcmSample,
	                                           &magCplx,
	                                           rdsModus, ptyLocale)) {
	                  iqBuffer -> putDataIntoBuffer (&magCplx, 1);
	                  iqCounter ++;
	                  if (iqCounter > 100) {
	                     emit iqBufferLoaded ();
	                     iqCounter = 0;
	                  }
	               }

	               switch (lfPlotType) {
	                  case ELfPlot::RDS_INPUT:
	                     spectrumBuffer_lf. push_back (20.0f * pcmSample);
	                     break;

	                  case ELfPlot::RDS_DEMOD:
	                     spectrumBuffer_lf. push_back (magCplx);

	                     break;

	                  default:;
	               }
	            }
	         }
	      }
	      else {
	         switch (lfPlotType) {
	            case ELfPlot::RDS_INPUT:
	            case ELfPlot::RDS_DEMOD:
	               spectrumBuffer_lf. push_back (std::complex<float> (0, 0)); 
	               break;
	            default:;
	         }
	      }

	      if (fmAudioFilterActive. load ()) {
	         audio = fmAudioFilter. Pass (audio);
	      }

//	   apply deemphasis
	      audio = lastAudioSample =
	         (audio - lastAudioSample) * deemphAlpha + lastAudioSample;

	      switch (lfPlotType) {
	         case ELfPlot::OFF:
	            spectrumBuffer_lf. push_back (std::complex<float> (0, 0));
	            break;
	         case ELfPlot::IF_FILTERED:
	            // TODO: somehow the IF level got much higher, reduce level here for the scope
	            spectrumBuffer_lf. push_back (v * 0.05f);
	            break;
	         case ELfPlot::DEMODULATOR:
	            spectrumBuffer_lf. push_back (demod);
	            break;
	         case ELfPlot::AF_SUM:
	            spectrumBuffer_lf. push_back (sumLR);
	            break;
	         case ELfPlot::AF_DIFF:
	            spectrumBuffer_lf. push_back (diffLR);
	            break;
	         case ELfPlot::AF_MONO_FILTERED:
	            spectrumBuffer_lf. push_back (std::complex<float> (audio.real () + audio.imag (), 0));
	            break;
	         case ELfPlot::AF_LEFT_FILTERED:
	            spectrumBuffer_lf. push_back (std::complex<float> (audio.real (), 0));
	            break;
	         case ELfPlot::AF_RIGHT_FILTERED:
	            spectrumBuffer_lf. push_back (std::complex<float> (audio.imag (), 0));
	            break;
//	         case ELfPlot::RDS:
//	            spectrumBuffer_lf. push_back (rdsDataCplx);
//	            break;
	         default:;
	      }

//	   "result" now contains the audio sample, either stereo or mono
	      audio = audioGainCorrection (audio);

	      int32_t audioAmount;
	      if (audioDecimator.
	              convert (audio, audioOut, &audioAmount)) {
//	   here the sample rate is "workingRate" (typ. 48000Ss)
	         for (int32_t k = 0; k < audioAmount; k++) {
	            std::complex<float> pcmSample = audioOut [k];
	            if (suppressAudioSampleCnt > 0) {
	               pcmSample *= ((float)suppressAudioSampleCntMax - (float)suppressAudioSampleCnt)
						             / (float)suppressAudioSampleCntMax;
	              --suppressAudioSampleCnt;
	            }
	            insertTestTone (pcmSample);
	            evaluatePeakLevel (pcmSample);
	            sendSampletoOutput (pcmSample);
	         }
	      }

	      if (++lfCount > (fmRate / repeatRate)) {
	         if (spectrumBuffer_lf. size () >= (unsigned)spectrumSize) {
	            processLfSpectrum (spectrumBuffer_lf);
	            spectrumBuffer_lf. resize (0);
	         }
	         lfCount = 0;
	      }

	      if (++myCount > (fmRate >> 1)) { // each 500ms ...
	         metaData. GuiPilotStrength = get_pilotStrength	();
	         metaData. PilotPllLocked =
	                    isPilotLocked (metaData. PilotPllLockStrength);
	         metaData. DcValRf =
	                    (DCREnabled ?
	                         20 * log10 (abs(RfDC) + 1.0f/32768) : -99.99);
	         metaData. DcValIf =
	                    get_demodDcComponent ();
	         metaData. PssPhaseShiftDegree =
	                   pilotDelayPSS / M_PI * 180.0f;
	         metaData. PssPhaseChange = pPSS. get_mean_error () * 1000;
	         metaData. PssState =
	                   (pssActive && metaData. PilotPllLocked ?
	                      (pPSS. is_error_minimized () ?
	                          SMetaData::EPssState::ESTABLISHED :
	                                SMetaData::EPssState::ANALYZING) :
	                                SMetaData::EPssState::OFF);
	         emit showMetaData (&metaData);
	         myCount = 0;
	      }
	   }
	}
}

void	fmProcessor::process_signal_with_rds (const float demod,
	                                      std::complex<float> *audioOut,
                                         std::complex<float> *rdsValueCmpl) {

//	Get the phase for the "carrier to be inserted" right.
//	Do this always to be able to check of a locked pilot PLL.
	DSPFLOAT currentPilotPhase =
	                 pilotRecover. getPilotPhase (5 * demod);
	const bool pilotLocked = pilotRecover. isLocked ();

	if (!pilotLocked) {
	   pilotDelayPSS = 0;
	   pPSS. reset ();
	}

	if (fmModus != FM_Mode::Mono &&
	        (pilotLocked || !autoMono)) {
//	Now we have the right - i.e. synchronized - signal to work with
	   DSPFLOAT phaseforLRDiff =
	               2 * (currentPilotPhase + M_PI_4 + PILOTTESTDELAY) -
	                                                   pilotDelayPSS;
//
//	Note that there is no constraint on the phase yet
	   if (phaseforLRDiff < - 2 * M_PI)
	      phaseforLRDiff += 4 * M_PI;
	   phaseforLRDiff	= fmod (phaseforLRDiff, 2 * M_PI);
//	perform perfect stereo separation (PSS)
	   pilotDelayPSS = pssActive ?
	              this -> pPSS. process_sample (demod, phaseforLRDiff)
	              : 0;

//	we look for minimum correlation so mix with PI/2 phase shift
	   DSPFLOAT LRDiff =
	             2.0 * (soundSelector == S_LEFTminusRIGHT_Test ?
	                      mySinCos. getSin (phaseforLRDiff) :
	                      mySinCos. getCos (phaseforLRDiff)) * demod;
	   DSPFLOAT LRPlus = demod;
	   *audioOut = DSPCOMPLEX (LRPlus, LRDiff);
	}
	else {
	   *audioOut = DSPCOMPLEX (demod, 0);
	}

//	process RDS
	if (rdsModus != rdsDecoder::ERdsMode::RDS_OFF ) {
//	currentPilotPhase shifts also about 19kHz without
//	existing pilot signal
	   float thePhase	= 3 * (currentPilotPhase + PILOTTESTDELAY);
	   float rdsBaseBp	= rdsBandPassFilter. Pass (demod);
	   std::complex<float> rdsBaseHilb =
	                          rdsHilbertFilter. Pass (rdsBaseBp);
//
//	The "range" of the Phase is now at least  -6 * 2 * M_PI .. 6 * 2 * M_PI
	   std::complex<float> xxx = std::complex<float> (cos (thePhase),
	                                                  -sin (thePhase));
	   *rdsValueCmpl = xxx * rdsBaseBp;
//	   *rdsValueCmpl = rdsBaseHilb * mySinCos. getComplex (-thePhase);
	}
}
//
//	
void	fmProcessor::setlfcutoff (int32_t Hz) {
	if (Hz > 0) {
	   audioFrequency	= Hz;
	   newAudioFilter. store (true);
	}
	else {
	   fmAudioFilterActive . store (false);
//	   fprintf (stderr, "audiofilter switched off\n");
	};
}

void	fmProcessor::evaluatePeakLevel (const DSPCOMPLEX s) {
const DSPFLOAT absLeft  = std::abs (real (s));
const DSPFLOAT absRight = std::abs (imag (s));

	if (absLeft  > absPeakLeft)  
	   absPeakLeft  = absLeft;
	if (absRight > absPeakRight)
	   absPeakRight = absRight;

	peakLevelCurSampleCnt ++;
	if (peakLevelCurSampleCnt > peakLevelSampleMax) {
	   peakLevelCurSampleCnt = 0;

	   float leftDb  = (absPeakLeft  > 0.0f ?
	                   20.0f * std::log10 (absPeakLeft)  : -40.0f);
	   float rightDb = (absPeakRight > 0.0f ?
	                   20.0f * std::log10 (absPeakRight) : -40.0f);

//	correct audio sample buffer delay for peak display
	   DSPCOMPLEX delayed = delayLine.
	                   get_set_value (DSPCOMPLEX (leftDb, rightDb));

	   emit showPeakLevel (real (delayed), imag (delayed));
	   absPeakLeft	= 0.0f;
	   absPeakRight = 0.0f;
	}
}

void	fmProcessor::insertTestTone (DSPCOMPLEX & ioS) {
float toneFreqHz = 1000.0f;
float level = 0.9f;

	if (!testTone. Enabled)
	   return;

	ioS *= (1.0f - level);
	if (testTone. NoSamplRemain > 0) {
	   testTone. NoSamplRemain --;
	   testTone. CurPhase		+= testTone. PhaseIncr;
	   testTone. CurPhase		= PI_Constrain (testTone. CurPhase);
	   const DSPFLOAT smpl		= sin (testTone. CurPhase);
	   ioS += level * DSPCOMPLEX (smpl, smpl);
	}
	else
	if (++testTone.TimePeriodCounter >
	                workingRate * testTone.TimePeriod) {
	   testTone. TimePeriodCounter = 0;
	   testTone. NoSamplRemain = workingRate * testTone. SignalDuration;
	   testTone. CurPhase = 0.0f;
	   testTone. PhaseIncr = 2 * M_PI / workingRate * toneFreqHz;
	}
}

void	fmProcessor::sendSampletoOutput (DSPCOMPLEX s) {

	if (audioRate == workingRate) {
	   theSink -> putSample (s);
	   return;
	}

	DSPCOMPLEX out [theConverter. getOutputsize ()];
	int32_t    amount;
	if (theConverter. convert (s, out, &amount)) {
	   for (int32_t i = 0; i < amount; i++) {
	      theSink -> putSample (out [i]);
	   }
	}
}

void	fmProcessor::setfmRdsSelector (rdsDecoder::ERdsMode m) {
	this -> rdsModus = m;

	if (lfPlotType == ELfPlot::RDS_INPUT ||
	                 lfPlotType == ELfPlot::RDS_DEMOD) {
	   new_lfSpectrum ();
	}
}

void	fmProcessor::triggerFrequencyChange() {
// this function is called at a frequency change

// suppress audio to avoid transient noise
	suppressAudioSampleCnt = suppressAudioSampleCntMax;

	resetRds ();
	restartPssAnalyzer ();
	new_lfSpectrum ();
}

void	fmProcessor::restartPssAnalyzer	() {
	pilotDelayPSS = 0;
	pPSS. reset (); // TODO shift this as it is called while RDS switch, too
}

void	fmProcessor::resetRds	() {
	myRdsDecoder. reset ();
}

void	fmProcessor::set_localOscillator (int32_t lo) {
	loFrequency = lo;
}

bool	fmProcessor::isPilotLocked (float &oLockStrength) const {
	if (fmModus != FM_Mode::Mono){
	   oLockStrength = pilotRecover. getLockedStrength ();
	   return pilotRecover. isLocked ();
	}
	else {
	   oLockStrength = 0;
	   return false;
	}
	return false;		// cannot happen
}

void	fmProcessor::set_squelchMode	(ESqMode iSqMode) {
	squelchMode = iSqMode;
}

DSPFLOAT	fmProcessor::getSignal	(DSPCOMPLEX *v, int32_t size) {
DSPFLOAT sum = 0;

	for (int i = 5; i < 25; i++)
	   sum += abs (v [i]);
	for (int i = 5; i < 25; i++)
	   sum += abs (v [size - 1 - i]);
	return sum / 40;
}

DSPFLOAT	fmProcessor::getNoise	(DSPCOMPLEX *v, int32_t size) {
DSPFLOAT sum = 0;

	for (int i = 5; i < 25; i++)
	   sum += abs (v [size / 2 - 1 - i]);
	for (int i = 5; i < 25; i++)
	   sum += abs(v[size / 2 + 1 + i]);
	return sum / 40;
}

void	fmProcessor::mapSpectrum (const DSPCOMPLEX * const in,
	                          double * const out,
	                          int32_t &ioZoomFactor) {
int16_t factor = spectrumSize / displaySize;  // typ factor = 4 (whole divider)

	if (factor / ioZoomFactor >= 1) {
	   factor /= ioZoomFactor;
	}
	else {
	   ioZoomFactor = factor;
	   factor = 1;
	}

//	work from inside (0Hz) to outside for filling display data
	for (int32_t i = 0; i < displaySize / 2; i++) {
	   double f = 0;
//	read 0Hz to rate/2 -> map to mid to end of display
	   for (int32_t j = 0; j < factor; j++) {
	      f += abs (in [i * factor + j]);
	   }
	   out [displaySize / 2 + i] = f / factor;

	   f = 0;
//	read rate/2 down to 0Hz -> map to begin to mid of display
	   for (int32_t j = 0; j < factor; j++) {
	      f += abs (in [spectrumSize - 1 - (i * factor + j)]);
	   }
	   out [displaySize / 2 - 1 - i] = f / factor;
	}
}

void	fmProcessor::mapHalfSpectrum (const DSPCOMPLEX * const in,
	                              double * const out,
	                              int32_t &ioZoomFactor) {
int16_t factor = spectrumSize / displaySize / 2;  // typ factor = 2 (whole divider)

	if (factor / ioZoomFactor >= 1) {
	   factor /= ioZoomFactor;
	}
	else {
	   ioZoomFactor = factor;
	   factor = 1;
	}

	for (int32_t i = 0; i < displaySize; i++) {
	   double f = 0;
//	read 0Hz to rate/2 -> map to mid to end of display
	   for (int32_t j = 0; j < factor; j++) {
	      f += abs (in [i * factor + j]);
	   }

	   out [i] = f / factor;
	}
}

void	fmProcessor::processLfSpectrum (std::vector<std::complex<float>> &v) {
double Y_Values [displaySize];
int32_t l_zoomFactor = zoomFactor; // copy value because it may be changed
std::complex<float> *spectrumBuffer	= spectrum_fft_lf -> getVector ();

	for (int i = 0; i < spectrumSize; i ++)
	   spectrumBuffer [i] = v [i];

	spectrum_fft_lf -> do_FFT ();

	if (showFullSpectrum) 
	   mapSpectrum (spectrumBuffer, Y_Values, l_zoomFactor);
	else 
	   mapHalfSpectrum (spectrumBuffer, Y_Values, l_zoomFactor);

	if (lfBuffer_newFlag) {
	   set_average_buffer (Y_Values, displayBuffer_lf);
	   lfBuffer_newFlag = false;
	}
	else 
	  add_to_average (Y_Values, displayBuffer_lf);

#ifdef USE_EXTRACT_LEVELS
	if (showFullSpectrum) {
	   extractLevels (displayBuffer_lf, fmRate);
	}
	else {
	   extractLevelsHalfSpectrum (displayBuffer_lf, fmRate);
	}
#endif

	lfBuffer -> putDataIntoBuffer (displayBuffer_lf, displaySize);
//	and signal the GUI thread that we have data
	emit lfBufferLoaded (showFullSpectrum,
	                             spectrumSampleRate / l_zoomFactor);
}

void	fmProcessor::set_average_buffer (const double *const in,
	                                  double * const buffer) {
	for (int32_t i = 0; i < displaySize; i++) {
	   buffer [i] = in [i];
	}
}

void	fmProcessor::add_to_average (const double * const in,
	                             double *const buffer) {
const double alpha = 1.0 / averageCount;
const double beta = (averageCount - 1.0) / averageCount;

	for (int32_t i = 0; i < displaySize; i++) {
	   buffer [i] = alpha * in [i] + beta * buffer [i];
	}
}

#ifdef USE_EXTRACT_LEVELS
void	fmProcessor::extractLevels (const double * const in,
	                            const int32_t range) {
const float binWidth = (float)range / zoomFactor / displaySize;
const int32_t pilotOffset = displaySize / 2 - 19000 / binWidth;
const int32_t rdsOffset   = displaySize / 2 - 57000 / binWidth;
const int32_t noiseOffset = displaySize / 2 - 70000 / binWidth;

//  int   a = myRig->bitDepth() - 1;
//  int   b = 1;

//  while (--a > 0)
//  {
//    b <<= 1;
//  }

float noiseAvg = 0, pilotAvg = 0, rdsAvg = 0;

	for (int32_t i = 0; i < 7; i++) {
	   noiseAvg += in [noiseOffset - 3 + i];
	   rdsAvg += in [rdsOffset - 3 + i];
	}

	for (int32_t i = 0; i < 3; i++) {
	   pilotAvg += in [pilotOffset - 1 + i];
	}

	noiseLevel = 0.95 * noiseLevel + 0.05 * noiseAvg / 7;
	pilotLevel = 0.95 * pilotLevel + 0.05 * pilotAvg / 3;
	rdsLevel   = 0.95 * rdsLevel   + 0.05 * rdsAvg / 7;
}

void	fmProcessor::extractLevelsHalfSpectrum (const double * const in,
	                                        const int32_t range) {
const float binWidth	= (float)range / zoomFactor / displaySize / 2;
const int32_t pilotOffset = 19000 / binWidth;
const int32_t rdsOffset   = 57000 / binWidth;
const int32_t noiseOffset = 70000 / binWidth;

constexpr int32_t avgNoiseRdsSize = 1 + 2 * 6; // mid plus two times sidebands
constexpr int32_t avgPilotSize    = 1 + 2 * 2;

float	noiseAvg = 0;
float	pilotAvg = 0;
float	rdsAvg	= 0;

	for (int32_t i = 0; i < avgNoiseRdsSize; i++) {
	   noiseAvg += in [noiseOffset - 3 + i];
	   rdsAvg += in [rdsOffset - 3 + i];
	}

	for (int32_t i = 0; i < avgPilotSize; i++) {
	   pilotAvg += in [pilotOffset - 1 + i];
	}

	constexpr float ALPHA = 0.2f;
	noiseLevel	= (1.0f - ALPHA) * noiseLevel +
	                          ALPHA * noiseAvg / avgNoiseRdsSize;
	pilotLevel	= (1.0f - ALPHA) * pilotLevel +
	                          ALPHA * pilotAvg / avgPilotSize;
	rdsLevel	= (1.0f - ALPHA) * rdsLevel +
	                          ALPHA * rdsAvg / avgNoiseRdsSize;
}
#endif

void	fmProcessor::setAutoMonoMode	(const bool iAutoMonoMode) {
	autoMono = iAutoMonoMode;
}

void	fmProcessor::setPSSMode		(const bool iPSSMode) {
	pssActive = iPSSMode;
}

void	fmProcessor::setDCRemove	(const bool iDCREnabled) {
	DCREnabled = iDCREnabled;
	RfDC = 0.0f;
}

void	fmProcessor::new_hfSpectrum	() {
	hfBuffer_newFlag = true;
}

void	fmProcessor::new_lfSpectrum	() {
	lfBuffer_newFlag = true;
}

void	fmProcessor::setTestTone		(const bool iTTEnabled) {
	testTone. Enabled = iTTEnabled;
}

void	fmProcessor::setDispDelay		(const int iDelay) {
	delayLine. set_delay_steps (iDelay);
}

void	fmProcessor::set_ptyLocale		(int ptyLocale) {
	this	-> ptyLocale	= ptyLocale;
}

