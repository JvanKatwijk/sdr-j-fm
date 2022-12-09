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
//#define OMEGA_DEMOD		(2 * M_PI / fmRate)
#define OMEGA_PILOT	((DSPFLOAT(PILOT_FREQUENCY)) / fmRate) * (2 * M_PI)
//#define OMEGA_RDS	((DSPFLOAT)RDS_FREQUENCY / fmRate) * (2 * M_PI)
//#define RDS_DECIMATOR                8

//
//	Note that no decimation done as yet: the samplestream is still
//	full speed
	fmProcessor::fmProcessor (deviceHandler *vi,
	                          RadioInterface *RI,
                                  audioSink *mySink,
	                          int32_t inputRate,
	                          int32_t fmRate,
                                  int32_t workingRate,
	                          int32_t audioRate,
                                  int32_t displaySize,
	                          int32_t spectrumSize,
                                  int32_t averageCount,
	                          int32_t repeatRate,
                                  RingBuffer<double> *hfBuffer,
                                  RingBuffer<double> *lfBuffer,
                                  RingBuffer<DSPCOMPLEX> *iqBuffer,
                                  int16_t filterDepth,
                                  int16_t thresHold):
	                             myRdsDecoder (RI, RDS_RATE),
	                             localOscillator (inputRate),
	                             rdsOscillator (fmRate),
	                             mySinCos (fmRate),
	                             audioDecimator (fmRate,
	                                             workingRate,
	                                             fmRate / 1000),
	                             fmBandfilter (5 * inputRate / fmRate,
	                                          fmRate / 2,
	                                          inputRate,
	                                          inputRate / fmRate),
	                             fmAudioFilter (1024, 431) {
	this	-> running. store (false);
	this	-> myRig	= vi;
	this	-> myRadioInterface = RI;
	this	-> theSink	= mySink;
	this	-> inputRate	= inputRate;
	this	-> fmRate	= fmRate;
	this	-> decimatingScale  = inputRate / fmRate;
	this	-> workingRate	= workingRate;
	this	-> audioRate	= audioRate;
	this	-> displaySize	= displaySize;
	this	-> averageCount	= averageCount;
	this	-> repeatRate	= repeatRate;
	this	-> hfBuffer	= hfBuffer;
	this	-> lfBuffer	= lfBuffer;
	this	-> filterDepth	= filterDepth;
	this	-> thresHold	= thresHold;
	this	-> scanning	= false;
	this	-> Lgain	= 20;
	this	-> Rgain	= 20;

	this	-> squelchMode	= ESqMode::OFF;
	this	-> spectrumSampleRate = fmRate;
	this	-> iqBuffer	= iqBuffer;
//
//	inits that cannot be done by older GCC versions
	this	-> fillAveragehfBuffer	= true;
	this	-> fillAveragelfBuffer	= true;
	this	-> displayBuffer_lf	= nullptr;
	this	-> autoMono		= true;
	this	-> peakLevelCurSampleCnt	= 0;
	this	-> peakLevelSampleMax	= 0x7FFFFFF;
	this	-> absPeakLeft		= 0.0f;
	this	-> absPeakRight		= 0.0f;
	this	-> rdsSampleCntSrc	= 0;
	this	-> rdsSampleCntDst	= 0;
	this	-> volumeFactor		= 0.5f;
	this	-> panorama		= 1.0f;

	this	-> rdsModus		= rdsDecoder::ERdsMode::RDS_OFF;
	this	-> DCREnabled		= true;
	this	-> RfDC			= DSPCOMPLEX (0, 0);

	this	-> lfPlotType		= ELfPlot::DEMODULATOR;
	this	-> showFullSpectrum	= false;
	this	-> spectrumSampleRate	= fmRate;
	this	-> zoomFactor		= 1;

	peakLevelSampleMax = workingRate / 50;  // workingRate is typ. 48000S/s

//	this	-> localBuffer		= new double [displaySize];
//	we already verified that displaySize has a decent value
//	(power of 2), so we keep it simple
	this	-> spectrumSize		= 4 * displaySize;

	this	-> spectrum_fft_hf	= new common_fft (this -> spectrumSize);
	this	-> spectrumBuffer_hf	= spectrum_fft_hf -> getVector ();

	this	-> spectrum_fft_lf	= new common_fft (spectrumSize);
	this	-> spectrumBuffer_lf. set_data_ptr (this -> spectrum_fft_lf->getVector (), spectrumSize);

	this	-> loFrequency      = 0;
	this	-> omegaDemod       = 2 * M_PI / fmRate;
	this	-> fmBandwidth      = 0.95 * fmRate;
	this	-> fmFilterDegree   = 21;
	this	-> fmFilter		= new LowPassFIR (21,
	                                          0.95 * fmRate / 2,
	                                          fmRate);
	this	-> newFilter. store (false);
  /*
   *	default values, will be set through the user interface
   *	to their appropriate values
   */
	this	-> fmModus		= FM_Mode::Stereo;
	this	-> Selector		= S_STEREO;
	this	-> balance		= 0;
	this	-> leftChannel		= 1.0;
	this	-> rightChannel		= 1.0;
//	this	-> inputMode		= IandQ;
	
//	this	-> audioDecimator = new newConverter (fmRate,
//	                                              workingRate,
//	                                              fmRate / 1000);
	this	-> audioOut	=
	              new DSPCOMPLEX [audioDecimator. getOutputsize ()];

	this	-> maxFreqDeviation  = 0.95 * (0.5 * fmRate);
	this	-> normFreqDeviation = 0.6 * maxFreqDeviation;

#ifdef USE_EXTRACT_LEVELS
	this	-> noiseLevel = 0;
	this	-> pilotLevel = 0;
	this	-> rdsLevel   = 0;
#endif

//	Since data is coming with a pretty high rate, we need to filter
//	and decimate in an efficient way. We have an optimized
//	decimating filter (optimized or not, it takes quite some
//	cycles when entering with high rates)
//	fmBandfilter = new DecimatingFIR (15 * decimatingScale,
//	                                  fmRate / 2,
//	                                  inputRate,
//	                                  decimatingScale);

//	to isolate the pilot signal, we need a reasonable
//	filter. The filtered signal is beautified by a pll
	pilotBandFilter = new fftFilter (FFT_SIZE, PILOTFILTER_SIZE);
	pilotBandFilter -> setBand (PILOT_FREQUENCY - PILOT_WIDTH / 2,
	                            PILOT_FREQUENCY + PILOT_WIDTH / 2,
	                            fmRate);
	pilotRecover	= new pilotRecovery (fmRate,
	                                     OMEGA_PILOT,
	                                     25 * omegaDemod,
	                                     &mySinCos);
	pilotDelay	= (FFT_SIZE - PILOTFILTER_SIZE) * OMEGA_PILOT;
//	fmAudioFilter	= new fftFilter (1024, 431);
	fmAudioFilterActive . store (false);

	rdsDecimator = new newConverter (fmRate, RDS_RATE, fmRate / 1000);
	rdsOut = new DSPCOMPLEX [rdsDecimator -> getOutputsize ()];

//
//	the constant K_FM is still subject to many questions
	DSPFLOAT F_G     = 0.65 * fmRate / 2;// highest freq in message
	DSPFLOAT Delta_F = 0.95 * fmRate / 2;    //

	DSPFLOAT B_FM    = 2 * (Delta_F + F_G);

	K_FM           = B_FM * M_PI / F_G;
	theDemodulator = new fm_Demodulator(fmRate, &mySinCos, K_FM);
//
//	In the case of mono we do not assume a pilot
//	to be available. We borrow the approach from CuteSDR
	rdsHilbertFilter = new fftFilterHilbert (FFT_SIZE,
	                                           RDSBANDFILTER_SIZE);

	rdsBandFilter = new fftFilter (FFT_SIZE, RDSBANDFILTER_SIZE);
	rdsBandFilter -> setSimple (RDS_FREQUENCY - RDS_WIDTH,
	                            RDS_FREQUENCY + RDS_WIDTH, fmRate);

// for the deemphasis we use an in-line filter with
	lastAudioSample = 0;
	deemphAlpha = 1.0 / (fmRate / (1000000.0 / 50.0 + 1));

	dumping  = false;
	dumpFile = nullptr;

	mySquelch = new squelch (1, 70000, fmRate / 20, fmRate);

	displayBuffer_lf = new double [displaySize];

	connect (mySquelch, &squelch::setSquelchIsActive,
	         myRadioInterface, &RadioInterface::setSquelchIsActive);
	connect (this, &fmProcessor::hfBufferLoaded,
	         myRadioInterface, &RadioInterface::hfBufferLoaded);
	connect (this, &fmProcessor::lfBufferLoaded,
	         myRadioInterface, &RadioInterface::lfBufferLoaded);
	connect (this, &fmProcessor::iqBufferLoaded,
	         myRadioInterface, &RadioInterface::iqBufferLoaded);
	connect (this, &fmProcessor::showPeakLevel,
	         myRadioInterface, &RadioInterface::showPeakLevel);
	connect (this, SIGNAL (showDcComponents (float,float)),
	         myRadioInterface, SLOT (showDcComponents (float,float)));
	connect (this, SIGNAL (scanresult ()),
	         myRadioInterface,SLOT(scanresult()));

	squelchValue     = 0;
	oldSquelchValue = 0;

	theConverter = nullptr;
	if (audioRate != workingRate) {
	   theConverter = new newConverter (workingRate,
	                                    audioRate, workingRate / 20);
	}
	myCount = 0;
}

	fmProcessor::~fmProcessor() {
	   stop();

	delete[] displayBuffer_lf;
	delete mySquelch;
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
	if (running. load ()) {
//	get_db (0, 128) == 42.14dB
	   return get_db (PilotLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}

DSPFLOAT fmProcessor::get_rdsStrength () {
	if (running. load ()) {
	   return get_db (RdsLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}

DSPFLOAT fmProcessor::get_noiseStrength () {
	if (running. load ()) {
	   return get_db (NoiseLevel, 128) - get_db (0, 128);
	}
	return 0.0;
}
#endif

void	fmProcessor::set_squelchValue (int16_t n) {
	squelchValue	= n;
}

DSPFLOAT fmProcessor::get_demodDcComponent () {
	if (running. load ()) {
	   return theDemodulator -> get_DcComponent ();
	}
	return 0.0;
}

fm_Demodulator::TDecoderListNames & fmProcessor::listNameofDecoder() {
	return theDemodulator -> listNameofDecoder();
}
//
//	changing a filter is in two steps: here we set a marker,
//	but the actual filter is created in the mainloop of
//	the processor
void	fmProcessor::setBandwidth(int32_t b) {
	fmBandwidth = b;
	newFilter. store (false);
}

void	fmProcessor::setBandfilterDegree (int32_t d) {
	fmFilterDegree = d;
	newFilter      = true;
}

void	fmProcessor::setfmMode (FM_Mode m) {
	fmModus = m;
}

void	fmProcessor::setlfPlotType (ELfPlot m) {
	lfPlotType = m;

	switch (m) {
	   case ELfPlot::IF_FILTERED:
	      spectrumSampleRate = fmRate;
	      showFullSpectrum = true;
	      break;
	   case ELfPlot::RDS_INPUT:
	      spectrumSampleRate = RDS_RATE;
	      showFullSpectrum = true;
	      break;
	   case ELfPlot::RDS_DEMOD:
	      spectrumSampleRate = RDS_RATE / 16;  
	      showFullSpectrum = true;
	      break; // TODO: 16 should not be fixed here
	   default:
	      spectrumSampleRate = fmRate; 
	      showFullSpectrum = false;
	      break;
	}

	triggerDrawNewLfSpectrum ();
}

void	fmProcessor::setlfPlotZoomFactor (int32_t ZoomFactor) {
	zoomFactor = ZoomFactor;
	triggerDrawNewLfSpectrum ();
}

void	fmProcessor::setFMdecoder (const QString &decoder) {
	theDemodulator -> setDecoder (decoder);
}

void	fmProcessor::setSoundMode (uint8_t selector) {
	Selector = selector;
}

void	fmProcessor::setStereoPanorama(int16_t iStereoPan) {
// iStereoPan range: 0 (Mono) ... +100 (Stereo) ... +200 (Stereo with widen panorama)
	panorama = (DSPFLOAT)iStereoPan / 100.0f;
}

void	fmProcessor::setSoundBalance (int16_t new_balance) {
// range: -100 <= balance <= +100
	balance = new_balance;
//  leftChannel   = -(balance - 50.0) / 100.0;
//  rightChannel  = (balance + 50.0) / 100.0;
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
	Lgain = l;
	Rgain = r;
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

void	fmProcessor::run() {
const int32_t bufferSize	= 2 * 8192;
DSPCOMPLEX	dataBuffer [bufferSize];
double		displayBuffer_hf [displaySize];
int32_t		hfCount		= 0;
int32_t		lfCount		= 0;
int32_t		scanPointer	= 0;
common_fft	*scan_fft	= new common_fft (1024);
DSPCOMPLEX	*scanBuffer	= scan_fft -> getVector ();
const float rfDcAlpha = 1.0f / inputRate;

// check whether not calling next news twice
//	myRdsDecoder	= new rdsDecoder (myRadioInterface, RDS_RATE);
	running. store (true); // will be set from the outside

	while (running. load ()) {
	   while (running. load () && (myRig -> Samples () < bufferSize)) {
	      msleep(1); // should be enough
	   }

	   if (!running. load ()) {
	      break;
	   }

//	First: update according to potentially changed settings
	   if (newFilter && (fmBandwidth < 0.95 * fmRate)) {
	      delete fmFilter;
	      fmFilter = new LowPassFIR (fmFilterDegree,
	                                 fmBandwidth / 2, fmRate);
	   }
	   newFilter = false;

	   if (squelchValue != oldSquelchValue) {
	      mySquelch -> setSquelchLevel (squelchValue);
	      oldSquelchValue = squelchValue;
	   }

	   const int32_t amount =
	              myRig -> getSamples (dataBuffer, bufferSize, IandQ);
	   const int32_t aa = (amount >= spectrumSize ? spectrumSize : amount);

	   if (DCREnabled) {
	      for (int32_t i = 0; i < amount; i++) {
	         RfDC = (dataBuffer[i] - RfDC) * rfDcAlpha + RfDC;

//	limit the maximum DC correction because an AM
//	carrier at exactly 0Hz could has been suppressed, too
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

	         dataBuffer [i] -= DSPCOMPLEX (rfDcReal, rfDcImag);
	      }
	   }

//	for the HFscope
	   if (++hfCount > (inputRate / bufferSize) / repeatRate) {
	      double Y_Values [displaySize];

	      for (int32_t i = 0; i < aa; i++) {
	         spectrumBuffer_hf [i] = dataBuffer[i];
	      }

	      for (int32_t i = aa; i < spectrumSize; i++) {
	         spectrumBuffer_hf [i] = 0;
	      }

	      spectrum_fft_hf -> do_FFT ();

	      int32_t zoomFactor = 1;
	      mapSpectrum (spectrumBuffer_hf, Y_Values, zoomFactor);

	      if (fillAveragehfBuffer) {
	         fill_average_buffer (Y_Values, displayBuffer_hf);
	         fillAveragehfBuffer = false;
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
	         dumpBuffer [2 * i]	= real(dataBuffer[i]);
	         dumpBuffer [2 * i + 1] = imag(dataBuffer[i]);
	      }
	      sf_writef_float (dumpFile, dumpBuffer, amount);
	   }

//	   Here we really start

//	   We assume that if/when the pilot is no more than 3 db's above
//	   the noise around it, it is better to decode mono
	   for (int32_t i = 0; i < amount; i++) {
	      DSPCOMPLEX v = DSPCOMPLEX (real (dataBuffer [i]) * Lgain,
	                                 imag (dataBuffer [i]) * Rgain);

	      v = v * localOscillator. nextValue (loFrequency);

//	   first step: decimating (and filtering)
	      if ((decimatingScale > 1) && !fmBandfilter. Pass (v, &v)) {
	         continue;
	      }

//	   second step: if we are scanning, do the scan
	      if (scanning) {
	         scanBuffer [scanPointer++] = v;

                 if (scanPointer >= 1024) {
	            scanPointer = 0;
	            scan_fft -> do_FFT ();
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

//	   third step: if requested, apply filtering
	      if (fmBandwidth < 0.95 * fmRate) {
	         v = fmFilter -> Pass (v);
	      }

	      DSPFLOAT demod = theDemodulator -> demodulate (v);

	      switch (squelchMode) {
	         case ESqMode::NSQ:
	            demod = mySquelch -> do_noise_squelch (demod);
	            break;
	         case ESqMode::LSQ:
	            demod = mySquelch -> do_level_squelch (demod,
	                                    theDemodulator -> get_carrier_ampl());
	            break;
	         default:;
	      }

	      DSPCOMPLEX audio;
	      DSPFLOAT rdsData;
	      DSPCOMPLEX rdsDataCplx;

	      process_stereo_or_mono_with_rds (demod, &audio,
	                                       &rdsData, &rdsDataCplx);

	      const DSPFLOAT sumLR  = real (audio);
	      const DSPFLOAT diffLR = imag (audio);
	      const DSPFLOAT diffLRWeightend =
	         diffLR * (fmModus == FM_Mode::StereoPano ? panorama : 1.0f);

	      const DSPFLOAT left  =
	                 sumLR + diffLRWeightend;  // 2L = (L+R) + (L-R)
	      const DSPFLOAT right =
	                 sumLR - diffLRWeightend;  // 2R = (L+R) - (L-R)

	      switch (Selector) {
	         default:
	         case S_STEREO:
	            audio = DSPCOMPLEX (left,  right);
	            break;
	         case S_STEREO_SWAPPED:
	            audio = DSPCOMPLEX (right, left);
	            break;
	         case S_LEFT:
	            audio = DSPCOMPLEX (left,  left);
	            break;
	         case S_RIGHT:
	            audio = DSPCOMPLEX (right, right);
	            break;
	         case S_LEFTplusRIGHT:
	            audio = DSPCOMPLEX (sumLR, sumLR);
	            break;
	         case S_LEFTminusRIGHT:
	            audio = DSPCOMPLEX (diffLRWeightend, diffLRWeightend);
	            break;
	      }

	      if (rdsModus != rdsDecoder::ERdsMode::RDS_OFF) {
	         int32_t rdsAmount;

	         if (rdsDecimator -> convert (rdsDataCplx,
	                                      rdsOut, &rdsAmount)) {
//	   here the sample rate is rdsRate (typ. 19000S/s)
	            for (int32_t k = 0; k < rdsAmount; k++) {
	               const DSPCOMPLEX pcmSample = rdsOut [k];

	               static DSPCOMPLEX magCplx;
//	   input SR 19000S/s, output SR 19000/16S/s
	               if (myRdsDecoder. doDecode (pcmSample,
	                                           &magCplx, rdsModus)) {
	                  iqBuffer -> putDataIntoBuffer (&magCplx, 1);
	                  emit iqBufferLoaded ();
	               }

	               switch (lfPlotType) {
	                  case ELfPlot::RDS_INPUT:
	                     spectrumBuffer_lf = pcmSample;
	                     break;
	                  case ELfPlot::RDS_DEMOD:
	                     spectrumBuffer_lf = magCplx; break;
	                  default:;
	               }
	            }

	         }
	      }
	      else {
	         switch (lfPlotType) {
	            case ELfPlot::RDS_INPUT:
	            case ELfPlot::RDS_DEMOD:
	               spectrumBuffer_lf = 0; break;
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
	            spectrumBuffer_lf = 0;
	            break;
	         case ELfPlot::IF_FILTERED:
	            spectrumBuffer_lf = v;
	            break;
	         case ELfPlot::DEMODULATOR:
	            spectrumBuffer_lf = demod;
	            break;
	         case ELfPlot::AF_SUM:
	            spectrumBuffer_lf = sumLR;
	            break;
	         case ELfPlot::AF_DIFF:
	            spectrumBuffer_lf = diffLR;
	            break;
	         case ELfPlot::AF_MONO_FILTERED:
	            spectrumBuffer_lf = audio.real () + audio.imag ();
	            break;
	         case ELfPlot::AF_LEFT_FILTERED:
	            spectrumBuffer_lf = audio.real ();
	            break;
	         case ELfPlot::AF_RIGHT_FILTERED:
	            spectrumBuffer_lf = audio.imag ();
	            break;
//	         case ELfPlot::RDS:
//	         	 mpspectrumBuffer_lf = rdsDataCplx;
//	         	 break;
	         default:;
	      }

//	   "result" now contains the audio sample, either stereo or mono
	      audio = audioGainCorrection (audio);

	      int32_t audioAmount;
	      if (audioDecimator. convert (audio, audioOut, &audioAmount)) {
//	   here the sample rate is "workingRate" (typ. 48000Ss)
	         for (int32_t k = 0; k < audioAmount; k++) {
	            DSPCOMPLEX pcmSample = audioOut [k];
	            insertTestTone (pcmSample);
	            evaluatePeakLevel (pcmSample);
	            sendSampletoOutput (pcmSample);
	         }
	      }

	      if (++lfCount > (fmRate / repeatRate) &&
	                           spectrumBuffer_lf. is_full ()) {
	         processLfSpectrum ();
	         spectrumBuffer_lf. reset_write_pointer ();
	         lfCount = 0;
	      }

	      if (++myCount > (fmRate >> 1)) { // each 500ms ...
#ifdef USE_EXTRACT_LEVELS
	         emit showDcComponents ((DCREnabled ? 20 * log10 (abs(RfDC) + 1.0f/32768) : get_pilotStrength()), get_demodDcComponent());
#else
	         emit showDcComponents ((DCREnabled ? 20 * log10 (abs(RfDC) + 1.0f/32768) : -99.9), get_demodDcComponent());
#endif
	         myCount = 0;
	      }
	   }
	}
}

void	fmProcessor::process_stereo_or_mono_with_rds (const float demod,
	                                              DSPCOMPLEX *audioOut,
	                                              DSPFLOAT *rdsValue,
	                                              DSPCOMPLEX *rdsValueCmpl){
//	Get the phase for the "carrier to be inserted" right.
//	Do this alwas to be able to check of a locked pilot PLL.
DSPFLOAT pilot = pilotBandFilter -> Pass(5 * demod);
DSPFLOAT currentPilotPhase = pilotRecover -> getPilotPhase (5 * pilot);

	if (fmModus != FM_Mode::Mono &&
	         (pilotRecover -> isLocked() || autoMono == false)) {
//	Now we have the right - i.e. synchronized - signal to work with
	   DSPFLOAT PhaseforLRDiff = 2 * (currentPilotPhase + pilotDelay);
//	Due to filtering the real amplitude of the LRDiff might have
//	to be adjusted, we guess
	   DSPFLOAT LRDiff = 2.0 * mySinCos. getCos (PhaseforLRDiff) * demod;
	   DSPFLOAT LRPlus = demod;
	   *audioOut = DSPCOMPLEX (LRPlus, LRDiff);
	}
	else {
	   *audioOut = DSPCOMPLEX (demod, 0);
	}

//	process RDS
	if (rdsModus != rdsDecoder::ERdsMode::RDS_OFF) {
	   DSPFLOAT rdsBaseBp	= rdsBandFilter -> Pass(5 * demod);
	   DSPCOMPLEX rdsBaseHilb = rdsHilbertFilter -> Pass (rdsBaseBp);
// the oscillator shifts the signal down (== -57000 Hz shift)
	   DSPCOMPLEX rdsDelayCplx = rdsBaseHilb *
	              rdsOscillator. nextValue (RDS_FREQUENCY);
	   DSPFLOAT rdsDelay	= imag (rdsDelayCplx);
	   *rdsValue		= rdsDelay;
	   *rdsValueCmpl	= rdsDelayCplx;
	}
}
//
//	Since setLFcutoff is only called from within the "run"  function
//	from where the filter is also called, it is safe to remove
//	it here
//
void	fmProcessor::setlfcutoff (int32_t Hz) {

	if (Hz > 0) {
	   fmAudioFilterActive. store (false);
	   fmAudioFilter. setLowPass (Hz, fmRate);
	   fmAudioFilterActive. store (true);
	}
	else {
	   fmAudioFilterActive. store (false);
	}
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
	   DSPCOMPLEX delayed = mDelayLine.
	                   get_set_value (DSPCOMPLEX (leftDb, rightDb));

	   emit showPeakLevel (real (delayed), imag (delayed));
	   absPeakLeft	= 0.0f;
	   absPeakRight = 0.0f;
	}
}

void	fmProcessor::insertTestTone (DSPCOMPLEX & ioS) {
float toneFreqHz = 1000.0f;
float level = 0.9f;

	if (!mTestTone. Enabled)
	   return;

	ioS *= (1.0f - level);
	if (mTestTone. NoSamplRemain > 0) {
	   mTestTone. NoSamplRemain --;
	   mTestTone. CurPhase += mTestTone. PhaseIncr;
	   mTestTone. CurPhase = PI_Constrain (mTestTone. CurPhase);
	   const DSPFLOAT smpl = sin (mTestTone.CurPhase);
	   ioS += level * DSPCOMPLEX (smpl, smpl);
	}
	else
	if (++mTestTone.TimePeriodCounter >
	                workingRate * mTestTone.TimePeriod) {
	   mTestTone. TimePeriodCounter = 0;
	   mTestTone. NoSamplRemain = workingRate * mTestTone. SignalDuration;
	   mTestTone. CurPhase = 0.0f;
	   mTestTone. PhaseIncr = 2 * M_PI / workingRate * toneFreqHz;
	}
}

void	fmProcessor::sendSampletoOutput (DSPCOMPLEX s) {

	if (audioRate == workingRate) {
	   theSink -> putSample (s);
	}
	else {
	   DSPCOMPLEX out [theConverter->getOutputsize()];
	   int32_t    amount;
	   if (theConverter -> convert (s, out, &amount)) {
	      for (int32_t i = 0; i < amount; i++) {
	         theSink -> putSample (out [i]);
	      }
	   }
	}
}

void	fmProcessor::setfmRdsSelector (rdsDecoder::ERdsMode m) {
	rdsModus = m;

	if (lfPlotType == ELfPlot::RDS_INPUT ||
	                 lfPlotType == ELfPlot::RDS_DEMOD) {
	   triggerDrawNewLfSpectrum ();
	}
}

void	fmProcessor::resetRds	() {
	myRdsDecoder. reset ();
}

void	fmProcessor::set_localOscillator (int32_t lo) {
	loFrequency = lo;
}

//bool	fmProcessor::ok	() {
//	return running. load ();
//}

bool	fmProcessor::isPilotLocked (float &oLockStrength) const {

	if (fmModus != FM_Mode::Mono && pilotRecover) {
	   oLockStrength = pilotRecover -> getLockedStrength ();
	   return pilotRecover -> isLocked ();
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

//void	fmProcessor::setInputMode (uint8_t m) {
//	inputMode = m;
//}

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
	                          double * const out, int32_t &ioZoomFactor) {
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

void	fmProcessor::processLfSpectrum () {
double Y_Values [displaySize];
int32_t l_zoomFactor = zoomFactor; // copy value because it may be changed

	spectrum_fft_lf -> do_FFT ();

	if (showFullSpectrum) 
	   mapSpectrum (spectrumBuffer_lf. get_ptr (),
	                                       Y_Values,l_zoomFactor);
	else 
	   mapHalfSpectrum (spectrumBuffer_lf. get_ptr (),
	                                       Y_Values, l_zoomFactor);

	if (fillAveragelfBuffer) {
	   fill_average_buffer (Y_Values, displayBuffer_lf);
	   fillAveragelfBuffer = false;
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

void	fmProcessor::fill_average_buffer (const double *const in,
	                                  double * const buffer) {
	for (int32_t i = 0; i < displaySize; i++) {
	   buffer [i] = in [i];
	}
}

void	fmProcessor::add_to_average (const double * const in,
	                             double * const buffer) {
const double alpha = 1.0 / averageCount;
const double beta = (averageCount - 1.0) / averageCount;

	for (int32_t i = 0; i < displaySize; i++) {
	   buffer [i] = alpha * in [i] + beta * buffer [i];
	}
}

#ifdef USE_EXTRACT_LEVELS
void	fmProcessor::extractLevels (const double * const in,
	                            const int32_t range) {
const float binWidth = (float)range / ZoomFactor / mdisplaySize;
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

	NoiseLevel = 0.95 * NoiseLevel + 0.05 * noiseAvg / 7;
	PilotLevel = 0.95 * PilotLevel + 0.05 * pilotAvg / 3;
	RdsLevel   = 0.95 * RdsLevel   + 0.05 * rdsAvg / 7;
}

void	fmProcessor::extractLevelsHalfSpectrum (const double * const in,
	                                        const int32_t range) {
const float binWidth	= (float)range / ZoomFactor / displaySize / 2;
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
	NoiseLevel	= (1.0f - ALPHA) * NoiseLevel +
	                          ALPHA * noiseAvg / avgNoiseRdsSize;
	PilotLevel	= (1.0f - ALPHA) * PilotLevel +
	                          ALPHA * pilotAvg / avgPilotSize;
	mRdsLevel	= (1.0f - ALPHA) * mRdsLevel +
	                          ALPHA * rdsAvg / avgNoiseRdsSize;
}
#endif
