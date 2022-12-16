/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J-FM
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
 */

#ifndef __FM_PROCESSOR_H
#define __FM_PROCESSOR_H

#include	<QThread>
#include	<QObject>
#include	<sndfile.h>
#include	<atomic>
#include	"fm-constants.h"
#include	"fir-filters.h"
#include	"fft-filters.h"
#include	"sincos.h"
#include	"pllC.h"
#include	"ringbuffer.h"
#include	"oscillator.h"

#include	"pilot-recover.h"
#include	"fm-demodulator.h"
#include	"rds-decoder.h"
#include	"newconverter.h"
#include	"squelchClass.h"

class deviceHandler;
class RadioInterface;
class audioSink;
class newConverter;

#define USE_EXTRACT_LEVELS
template<typename T> class DelayLine {
public:
		DelayLine (const T & iDefault) : mDefault (iDefault) {
	   set_delay_steps(0); // reserve memory for at least one sample
	}

	void	set_delay_steps (const uint32_t iSteps) {
	   DataPtrIdx = 0;
	   DelayBuffer. resize (iSteps + 1, mDefault);
	}

	const T & get_set_value (const T & iVal) {
	   DelayBuffer [DataPtrIdx] = iVal;
	   DataPtrIdx = (DataPtrIdx + 1) % DelayBuffer. size();
	   return DelayBuffer [DataPtrIdx];
	}

private:
	uint32_t DataPtrIdx = 0;
	std::vector<T> DelayBuffer;
	T mDefault;
};


template<class T> class DataBufferCtrl {
public:
	DataBufferCtrl () = default;

	void set_data_ptr (T * const ipData, const int32_t iDataSize) {
	   mpData = ipData;
	   DataSize = iDataSize;
	   clear_content();
	}

	~DataBufferCtrl() = default;

	void operator = (const T & rhs) {
	   if (CurIdx < DataSize) {
	      mpData [CurIdx] = rhs;
	      CurIdx ++;
	   }
	}

	T * get_ptr	() const { return mpData; }
	bool is_full	() const { return (CurIdx >= DataSize); }
	void reset_write_pointer () { CurIdx = 0; }

	void clear_content () {
	   for (int32_t idx = 0; idx < DataSize; ++idx) {
	      mpData [idx] = T();
	   }
	   CurIdx = 0;
	}

private:
	T * mpData	= nullptr;
	int32_t DataSize;
	int32_t CurIdx = 0;
};


class fmProcessor : public QThread {
Q_OBJECT

public:
	enum class FM_Mode { Stereo, StereoPano, Mono };
	enum class ELfPlot { OFF, IF_FILTERED, DEMODULATOR, AF_SUM,
	                     AF_DIFF, AF_MONO_FILTERED, AF_LEFT_FILTERED,
	                     AF_RIGHT_FILTERED, RDS_INPUT, RDS_DEMOD };
	enum class ESqMode { OFF, NSQ, LSQ };
	enum Channels { S_STEREO, S_STEREO_SWAPPED, S_LEFT,
	                S_RIGHT, S_LEFTplusRIGHT, S_LEFTminusRIGHT };

public:
		fmProcessor (deviceHandler *,
	              RadioInterface *,
	              audioSink *,
	              int32_t,                  // inputRate
	              int32_t,                  // decimation
	              int32_t,                  // workingRate
	              int32_t,                  // audioRate,
	              int32_t,                  // displaySize
	              int32_t,                  // spectrumSize
	              int32_t,                  // averageCount
	              int32_t,                  // repeatRate
	              RingBuffer<double> *,     // HFScope
	              RingBuffer<double> *,     // LFScope
	              RingBuffer<DSPCOMPLEX> *, // IQScope
	              int16_t,                  // filterDepth
	              int16_t);                 // threshold scanning
		~fmProcessor () override;

	void		stop			();   // stop the processor
	void		setfmMode		(FM_Mode);
	void		setFMdecoder		(const QString &);
	void		setSoundMode		(uint8_t);
	void		setStereoPanorama	(int16_t iStereoPan);
	void		setSoundBalance		(int16_t);
	void		setDeemphasis		(int16_t);
	void		setVolume		(const float iVolGainDb);
	void		setlfcutoff		(int32_t);
	void		startDumping		(SNDFILE *);
	void		stopDumping		();
	void		setBandwidth		(int32_t);
	void		setBandfilterDegree	(int32_t);
	void		setAttenuation		(DSPFLOAT, DSPFLOAT);
	void		setfmRdsSelector	(rdsDecoder::ERdsMode);
	void		resetRds		();
	void		set_localOscillator	(int32_t);
	void		set_squelchMode		(ESqMode iSqMode);
	bool		getSquelchState		();
//	void		setInputMode		(uint8_t);
	void		setlfPlotType		(ELfPlot);
	void		setlfPlotZoomFactor	(int32_t);

	bool		isPilotLocked		(float &oLockStrength) const;
	void		setAutoMonoMode		(const bool iAutoMonoMode);
	void		setDCRemove		(const bool iDCREnabled);
	void		triggerDrawNewHfSpectrum ();
	void		triggerDrawNewLfSpectrum ();
	void		setTestTone		(const bool iTTEnabled);
	void		setDispDelay		(const int iDelay);

#ifdef USE_EXTRACT_LEVELS
	DSPFLOAT	get_pilotStrength	();
	DSPFLOAT	get_rdsStrength		();
	DSPFLOAT	get_noiseStrength	();
#endif

	DSPFLOAT	get_demodDcComponent	();
	void		startScanning		();
	void		stopScanning		();
	fm_Demodulator::TDecoderListNames & listNameofDecoder();
	void		set_squelchValue	(int16_t);
//
//	some private functions:
private:
	void		run			();
	void		mapSpectrum		(const DSPCOMPLEX * const,
	                                         double * const, int32_t &);
	void		mapHalfSpectrum		(const DSPCOMPLEX * const,
	                                         double * const, int32_t &);
	void		processLfSpectrum	();
	void		fill_average_buffer	(const double * const,
	                                             double * const);
	void		add_to_average		(const double * const,
	                                             double * const);
	void		extractLevels		(const double * const,
	                                             const int32_t);
	void		extractLevelsHalfSpectrum(const double * const,
	                                             const int32_t);
	void		sendSampletoOutput	(DSPCOMPLEX);
	void		insertTestTone		(DSPCOMPLEX & ioS);
	void		evaluatePeakLevel	(const DSPCOMPLEX s);

	DSPFLOAT 	getSignal		(DSPCOMPLEX *, int32_t);
	DSPFLOAT 	getNoise		(DSPCOMPLEX *, int32_t);

//	the privates
private:
	rdsDecoder	myRdsDecoder;
	Oscillator	localOscillator;
	Oscillator	rdsOscillator;
	SinCos		mySinCos;
	newConverter	audioDecimator;
	DecimatingFIR	fmBandfilter;
	fftFilter	fmAudioFilter;
	std::atomic<bool>	newAudioFilter;
	int		audioFrequency;

	LowPassFIR	*fmFilter;
	deviceHandler	*myRig;
	RadioInterface	*myRadioInterface;
	audioSink	*theSink;
	int32_t		inputRate;    // typ. 2112 kSpS
	int32_t		fmRate;       // typ.  192 kSpS = InputRate / 12
	int32_t		workingRate;  // typ.   48 kSpS
	int32_t		audioRate;    // typ.   48 kSpS
	int32_t		displaySize;
	int32_t		averageCount;
	int32_t		repeatRate;
	bool		fillAveragehfBuffer;
	bool		fillAveragelfBuffer;
	RingBuffer<double> *hfBuffer;
	RingBuffer<double> *lfBuffer;
	RingBuffer<DSPCOMPLEX> *iqBuffer;
	int16_t		filterDepth;
	uint8_t		inputMode;
	bool		scanning;
	int16_t		thresHold;

	squelch		*mySquelch;
	ESqMode		squelchMode;
	int32_t		spectrumSize;
	common_fft	*spectrum_fft_hf;
	common_fft	*spectrum_fft_lf;
	DSPCOMPLEX	*spectrumBuffer_hf;
	DataBufferCtrl<DSPCOMPLEX> spectrumBuffer_lf;

	double		*displayBuffer_lf;
	newConverter	*theConverter;
	int32_t		loFrequency;
	std::atomic<bool> running;
	int32_t		fmBandwidth;
	int32_t		fmFilterDegree;
	std::atomic<bool>	newFilter;
	bool		autoMono;

	int16_t		oldSquelchValue;
	int16_t		squelchValue;

	bool		dumping;
	SNDFILE		*dumpFile;
	int32_t		decimatingScale;

	int32_t		myCount;
	DSPFLOAT	Lgain;
	DSPFLOAT	Rgain;

	int32_t		peakLevelCurSampleCnt;
	int32_t		peakLevelSampleMax;
	DSPFLOAT	absPeakLeft;
	DSPFLOAT	absPeakRight;

	DSPCOMPLEX	*audioOut;

	struct TestTone {
	   bool		Enabled		= false;
	   float	TimePeriod	= 2.0f;
	   float	SignalDuration	= 0.025f;
	   uint32_t	TimePeriodCounter = 0;
	   uint32_t	NoSamplRemain	= 0;
	   float	CurPhase	= 0.0f;
	   float	PhaseIncr	= 0.0f;
	} testTone {};

	DelayLine<DSPCOMPLEX> delayLine {DSPCOMPLEX (-40.0f, -40.0f)};

	void	process_stereo_or_mono_with_rds (const float,
	                                         DSPCOMPLEX *,
	                                         DSPFLOAT *,
	                                         DSPCOMPLEX * rdsValueCmpl);
//	RDS

	fftFilter	*pilotBandFilter;
	fftFilter	*rdsBandFilter;
	pilotRecovery	*pilotRecover;
	fftFilterHilbert *rdsHilbertFilter;
	uint32_t	rdsSampleCntSrc;
	uint32_t	rdsSampleCntDst;
	newConverter	*rdsDecimator;
	DSPCOMPLEX	*rdsOut;
	DSPFLOAT	pilotDelay;
	DSPCOMPLEX	audioGainCorrection (DSPCOMPLEX);
//
//	Volume
	DSPCOMPLEX	lastAudioSample;
	DSPFLOAT	deemphAlpha;
	DSPFLOAT	volumeFactor;
	int32_t		maxFreqDeviation;
	int32_t		normFreqDeviation;
	DSPFLOAT	omegaDemod;
	std::atomic<bool>	fmAudioFilterActive;


	DSPFLOAT	panorama;
	int16_t		balance;
	DSPFLOAT	leftChannel;    // -(balance - 50.0) / 100.0;;
	DSPFLOAT	rightChannel;   // (balance + 50.0) / 100.0;;
	FM_Mode		fmModus;
	uint8_t		Selector;
	fm_Demodulator	*theDemodulator;
	rdsDecoder::ERdsMode rdsModus;

#ifdef USE_EXTRACT_LEVELS
	float		noiseLevel;
	float		pilotLevel;
	float		rdsLevel;
#endif

	DSPFLOAT	K_FM;
	bool		DCREnabled;
	DSPCOMPLEX	RfDC;

	ELfPlot		lfPlotType;
	bool		showFullSpectrum;
	int32_t		spectrumSampleRate;
	int32_t		zoomFactor;

signals:
	void		setPLLisLocked		(bool);
	void		hfBufferLoaded		();
	void		lfBufferLoaded		(bool, int);
	void		iqBufferLoaded		();
	void		showDcComponents(float, float);
	void		scanresult		();
	void		showPeakLevel		(const float, const float);
};

#endif
