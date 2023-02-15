#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fmreceiver
 *
 *    fmreceiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fmreceiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include	"fm-constants.h"
#include	"radio.h"
#include	"fm-processor.h"
#include	"audiosink.h"
#include	"device-handler.h"
#include	"fft-complex.h"

#define AUDIO_FREQ_DEV_PROPORTION    0.85f
#define PILOT_FREQUENCY		19000
#define RDS_FREQUENCY		(3 * PILOT_FREQUENCY)
#define RDS_RATE		24000
#define OMEGA_PILOT	((float(PILOT_FREQUENCY)) / fmRate) * (2 * M_PI)

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
	                          int	  spectrumSize,
	                          int32_t repeatRate,
	                          int	ptyLocale,
	                          RingBuffer<std::complex<float>> *hfBuffer,
	                          RingBuffer<std::complex<float>> *lfBuffer,
	                          RingBuffer<std::complex<float>> *iqBuffer,
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
	this	-> spectrumSize		= spectrumSize;
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

	this	-> loFrequency		= 0;
	this	-> fmBandwidth		= 0.95 * fmRate;
//
//	fmFilter is rather heavy on resource consumption, it is usually off
	fmFilter. setLowPass (0.95 * fmRate / 2, inputRate);
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
	
	pilotDelayPSS = 0;
#ifdef DO_STEREO_SEPARATION_TEST
	pilotDelay2 = 0;
#endif
	fmAudioFilterActive . store (false);
//
	rdsBandPassFilter. setBand  (RDS_FREQUENCY - RDS_WIDTH / 2,
	                             RDS_FREQUENCY + RDS_WIDTH / 2,
	                             fmRate);
	memset (rdsPhaseBuffer, 0, RDS_SAMPLE_DELAY * sizeof (float));
	rdsPhaseIndex			= 0;

// for the deemphasis we use an in-line filter with
	lastAudioSample			= 0;
	deemphAlpha			= 1.0 / (fmRate / (1000000.0 / 50.0 + 1));

	dumping				= false;
	dumpFile			= nullptr;

	displayBuffer_lf		= new double [displaySize];
	
	connect (&mySquelch, SIGNAL (setSquelchIsActive (bool)),
	         myRadioInterface, SLOT (setSquelchIsActive (bool)));
	connect (this, SIGNAL (hfBufferLoaded ()),
	         myRadioInterface, SLOT (hfBufferLoaded ()));
	connect (this, SIGNAL (lfBufferLoaded (bool, bool, int)),
	         myRadioInterface, SLOT (lfBufferLoaded (bool, bool, int)));
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

void	fmProcessor::set_squelchValue (int16_t n) {
	squelchValue	= n;
}

bool	fmProcessor::getSquelchState	() {
	return mySquelch. getSquelchActive  ();
}

float fmProcessor::get_demodDcComponent () {
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
	panorama = (float)iStereoPan / 100.0f;
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
float Tau;
	Q_ASSERT(v >= 1);

	Tau   = 1000000.0 / v;
	deemphAlpha = 1.0 / (float(fmRate) / Tau + 1.0);
}

void	fmProcessor::setVolume (const float iVolGainDb) {
	volumeFactor = std::pow (10.0f, iVolGainDb / 20.0f);
}

DSPCOMPLEX fmProcessor::audioGainCorrection (DSPCOMPLEX z) {
const float left  = volumeFactor * leftChannel * real(z);
const float right = volumeFactor * rightChannel * imag(z);

#if 0
	{
	   static float leftAbsMax = -1e38f;
	   static float rightAbsMax = -1e38f;
	   static float lastVolume = 0.0f;
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

void	fmProcessor::setAttenuation (float l, float r) {
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
DSPCOMPLEX	scanBuffer	[1024];
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
	      fmFilter. setLowPass (fmBandwidth / 2, inputRate);
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
//	   const int32_t aa = (amount >= spectrumSize ? spectrumSize : amount);

	   hfBuffer -> putDataIntoBuffer (dataBuffer, amount);
	   emit hfBufferLoaded();

	   if (DCREnabled) {
	      for (int32_t i = 0; i < amount; i++) {
	         RfDC = (dataBuffer[i] - RfDC) * rfDcAlpha + RfDC;

//	limit the maximum DC correction because an AM
//	carrier at exactly 0 Hz could have been suppressed, too
	         constexpr float DCRlimit = 0.01f;
	         float rfDcReal = real (RfDC);
	         float rfDcImag = imag (RfDC);
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

	   if (dumping) {
	      float dumpBuffer [2 * amount];
	      for (int32_t i = 0; i < amount; i++) {
	         dumpBuffer [2 * i]	= real (dataBuffer [i]);
	         dumpBuffer [2 * i + 1] = imag (dataBuffer [i]);
	      }
	      sf_writef_float (dumpFile, dumpBuffer, amount);
	   }

//	   Here we really start

//	   We assume that if/when the pilot is less than 3 db's above
//	   the noise around it, it is better to decode mono
	   for (int32_t i = 0; i < amount; i++) {
	      std::complex<float> v =
	                  std::complex<float> (real (dataBuffer [i]) * Lgain,
	                                       imag (dataBuffer [i]) * Rgain);

	      v = v * localOscillator. nextValue (loFrequency);

//	   first step: optional filtering and decimating
	      if (fmFilterOn. load ())
	         v = fmFilter. Pass (v);
	      if (decimatingScale > 1) {
	         if (!fmBand_1. Pass (v, &v)) 
	            continue;
	         if (!fmBand_2. Pass (v, &v))
	            continue;
	      }

//	   second step: if we are scanning, do the scan
//	   Samplerate here is fmRate
	      if (scanning) {
	         scanBuffer [scanPointer++] = v;

	         if (scanPointer >= 1024) {
	            scanPointer = 0;
	            Fft_transform (scanBuffer, 1024, false);
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

	      float demod = theDemodulator -> demodulate (v);

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

	      const float sumLR  = real (audio);
	      const float diffLR = imag (audio);
	      const float diffLRWeightend =
	                 diffLR * (fmModus == FM_Mode::StereoPano ?
	                                                 panorama : 1.0f);
	      const float left  =
	                 sumLR + diffLRWeightend;  // 2L = (L+R) + (L-R)
	      const float right =
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
	               std::complex<float> rdsSample = rdsOut [k];
	           
	               static std::complex<float> magCplx;
//	   input SR 19000S/s, output SR 19000/16S/s
	               if (myRdsDecoder. doDecode (rdsSample,
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
	                     spectrumBuffer_lf. push_back (20.0f * rdsSample);
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
//
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
	            processLfSpectrum (spectrumBuffer_lf,
	                               zoomFactor, showFullSpectrum,
	                               lfBuffer_newFlag);
	            lfBuffer_newFlag = false;
	            spectrumBuffer_lf. resize (0);
	         }
	         lfCount = 0;
	      }

	      if (++myCount > (fmRate >> 1)) { // each 500ms ...
#ifdef USE_EXTRACT_LEVELS
	         metaData. GuiPilotStrength = get_pilotStrength	();
#endif
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
	float currentPilotPhase =
	                 pilotRecover. getPilotPhase (5 * demod);
	const bool pilotLocked = pilotRecover. isLocked ();

	if (!pilotLocked) {
	   pilotDelayPSS = 0;
	   pPSS. reset ();
	}

	if (fmModus != FM_Mode::Mono &&
	        (pilotLocked || !autoMono)) {
//	Now we have the right - i.e. synchronized - signal to work with
	   float phaseforLRDiff =
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
	   float LRDiff =
	             2.0 * (soundSelector == S_LEFTminusRIGHT_Test ?
	                      mySinCos. getSin (phaseforLRDiff) :
	                      mySinCos. getCos (phaseforLRDiff)) * demod;
	   float LRPlus = demod;
	   *audioOut = DSPCOMPLEX (LRPlus, LRDiff);
	}
	else {
	   *audioOut = DSPCOMPLEX (demod, 0);
	}

//	process RDS
	if (rdsModus != rdsDecoder::ERdsMode::RDS_OFF ) {
//	   The currentPhase and the processed demod are NOT derived
//	   from the same inputsample. The processed demod is delayed by
//	   both the bandpass and the Hilbert filter, 
//	   while the reference phase is from the "current sample"
//	   We therefore store the last RDS_SAMPLE_DELAY "currentPilotPhase"
//	   values  and extract the one  that is RDS_SAMPLE_DELAY samples back

	   float rdsBaseBp	= rdsBandPassFilter. Pass (demod);
	   std::complex<float> rdsBaseHilb =
	                          rdsHilbertFilter. Pass (rdsBaseBp);
	   float thePhase	= 3 * rdsPhaseBuffer [rdsPhaseIndex];
	   rdsPhaseBuffer [rdsPhaseIndex]	= currentPilotPhase;
	   rdsPhaseIndex	= (rdsPhaseIndex + 1) % RDS_SAMPLE_DELAY;
//
//	The "range" of "thePhase" is now theoretically  between
//	-6 * 2 * M_PI .. 6 * 2 * M_PI or more and gives lot of issues
//	with "out of index" occurrences with sincos, that is why
//	it is less dangerous to use the old sin and cos
	   std::complex<float> oscValue = std::complex<float> (cos (thePhase),
	                                                      -sin (thePhase));
	   *rdsValueCmpl = oscValue * rdsBaseHilb;
//
//	Note that offsets in the "curentPilotPhase" have
//	a significant effect.
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
const float absLeft  = std::abs (real (s));
const float absRight = std::abs (imag (s));

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
	   const float smpl		= sin (testTone. CurPhase);
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

float	fmProcessor::getSignal	(DSPCOMPLEX *v, int32_t size) {
float sum = 0;

	for (int i = 5; i < 25; i++)
	   sum += abs (v [i]);
	for (int i = 5; i < 25; i++)
	   sum += abs (v [size - 1 - i]);
	return sum / 40;
}

float	fmProcessor::getNoise	(DSPCOMPLEX *v, int32_t size) {
float sum = 0;

	for (int i = 5; i < 25; i++)
	   sum += abs (v [size / 2 - 1 - i]);
	for (int i = 5; i < 25; i++)
	   sum += abs(v[size / 2 + 1 + i]);
	return sum / 40;
}

void	fmProcessor::processLfSpectrum (std::vector<std::complex<float>> &v,
	                               int zoomFactor,
	                               bool showFull,
	                               bool lfBuffer_newFlag) {
	lfBuffer -> putDataIntoBuffer (v. data (), spectrumSize);
	emit lfBufferLoaded (showFull, lfBuffer_newFlag, zoomFactor);
}

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

