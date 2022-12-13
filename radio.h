/*
 *    Copyright (C)  2014
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

#ifndef __RADIO_H
#define __RADIO_H

#include	<QTableWidget>
#include	<QTableWidgetItem>
#include	<QWidget>
#include	<QDialog>
#include	<QInputDialog>
#include	<QLineEdit>
#include	<QQueue>
#include	<QTimer>
#include	<QWheelEvent>
#include	<qwt.h>
#include	<sndfile.h>
#include	"fft.h"
#include	"fir-filters.h"
#include	"fm-constants.h"
#include	"iqdisplay.h"
#include	"ringbuffer.h"
#include	"scope.h"
#include	"ui_radio.h"
#include	"rds-decoder.h"
#include	<atomic>

class	keyPad;
class	QSettings;
class	fmProcessor;
class	rdsDecoder;
class	fft_scope;
class	audioSink;
class	deviceHandler;
class	programList;

#define	IQ_SCOPE_SIZE	64
/*
 *	The main gui object. It inherits from
 *	QDialog and the generated form
 */
class RadioInterface : public QDialog,
	               private Ui_sdr_j_fm {
Q_OBJECT
public:
		RadioInterface (QSettings *,
	                        QString,
	                        int32_t,
	                        QWidget *parent = nullptr);
		~RadioInterface ();

private:
	enum Keyboard {
	   NORMAL	= 0,
	   CONTROL	= 1,
	   ALT		= 2,
	   SHIFT	= 3
	};

//	Processing modes
	enum class ERunStates {
	   IDLE		= 0100,
	   PAUSED	= 0101,
	   RUNNING	= 0102,
	   STOPPING	= 0103
	};

	bool		ExtioLock;
	int16_t		outputDevice;
	void		localConnects		(void);
	void		dumpControlState	(QSettings *);

	RingBuffer<double>	*hfBuffer;
	RingBuffer<double>	*lfBuffer;
	Scope			*hfScope;
	Scope			*lfScope;
//
	keyPad		*mykeyPad;
	QSettings	*fmSettings;
	int32_t		inputRate;
	int32_t		fmRate;
	int32_t		workingRate;
	int32_t		audioRate;
	int16_t		displaySize;
	int16_t		rasterSize;
	int16_t		spectrumSize;
	int32_t		averageCount;
	int16_t		repeatRate;
	double		*displayBuffer;
	int16_t		filterDepth;
	audioSink	*our_audioSink;
	int8_t		channelSelector;
	deviceHandler	*myRig;
	int16_t		*outTable;
	int16_t		numberofDevices;

	uint8_t		HFviewMode;
//	uint8_t		LFviewMode;
	uint8_t		inputMode;
	int16_t		currAttSliderValue;
	DSPFLOAT	attValueL;
	DSPFLOAT	attValueR;

	int32_t		fmBandwidth;
	int32_t		LOFrequency;
	int32_t		currentFreq;

	void		restoreGUIsettings	(QSettings *);
//	void		setDetectorScreen	(int16_t);

	int32_t		mapIncrement		(int32_t);
	int32_t		IncrementInterval	(int16_t);
	void		Display			(int32_t);
	QTimer		*autoIncrementTimer;
	int16_t		IncrementIndex;
	int32_t		autoIncrement_amount;
	int32_t		fmIncrement;
	int32_t		minLoopFrequency;
	int32_t		maxLoopFrequency;

	void		IncrementFrequency	(int32_t);
	void		set_incrementFlag	(int16_t);
	void		stopIncrementing	();

	void		stop_lcdTimer		();
	int32_t		Panel;
	QTimer		*displayTimer;
  /*
   *	dumping
   */
	bool		sourceDumping;
	SNDFILE		*dumpfilePointer;

	bool		audioDumping;
	SNDFILE		*audiofilePointer;

	fmProcessor	*myFMprocessor;
	rdsDecoder	*myRdsDecoder;

	QString		RadioText;
	QString		StationLabel;
	int16_t		thresHold;
	int32_t		currentPIcode;
	int32_t		frequencyforPICode;
	int16_t		logTime;
	FILE		*logFile;

	void		setup_HFScope	();
	void		setup_LFScope	();
	bool		squelchMode;
	void		resetSelector	();
	int32_t		mapRates	(int32_t);

//
//	added or modified
	RingBuffer<DSPCOMPLEX>	*iqBuffer;
	IQDisplay	*iqScope;
	bool		mAfcActive;
	float		mAfcAlpha;
	int32_t		mAfcCurrOffFreq;

	bool		mSuppressTransient;
	float		mPeakLeftDamped;
	float		mPeakRightDamped;
//
//	end added
	void		reset_afc		();
	int32_t		setTuner		(int32_t);
	rdsDecoder::ERdsMode rdsModus;
	std::atomic<ERunStates>	runMode;
	void		setup_IQPlot		();

	QLineEdit	*myLine;
	programList	*myProgramList;
  /*
   *	The private slots link the GUI elements
   *	to the GUI code
   */
private slots:
	void	setStart		();
	void	updateTimeDisplay	();
	void	clickPause		();

//	void	setInputMode		(const QString &);
	void	setAttenuation		(int);
	void	setIQBalance		(int);

	void	setStreamOutSelector	(int);
	void	abortSystem		(int);
	void	TerminateProcess	();
	void	make_newProcessor	();
	void	stopDumping		();
	void	setDevice		(const QString &);
	void	set_dumping		();
	void	set_audioDump		();

	void	setfmBandwidth		(const QString &);
	void	setfmBandwidth		(int);
	void	setfmMode		(const QString &);
	void	setfmRdsSelector	(const QString &);
	void	setfmDecoder		(const QString &);
	void	setfmChannelSelector	(const QString &);
	void	setfmDeemphasis		(const QString &);
	void	setfmLFcutoff		(const QString &);

	void	autoIncrement_timeout	();
	void	autoIncrementButton	();
	void	autoDecrementButton	();
	void	set_fm_increment	(int);
	void	set_minimum		(int);
	void	set_maximum		(int);
	void	IncrementButton		();
	void	DecrementButton		();

	bool	setupSoundOut		(QComboBox *, audioSink *,
	                                 int32_t, int16_t *);
	void	set_squelchValue	(int);
	void	set_freqSave		();
	void	handle_myLine		();

//	added or changed
//	station list
//	void	tableSelect		(int, int);
//	void	removeRow		(int, int);
//
//	void	setVolume		(int);
	void	setfmStereoPanoramaSlider(int);
	void	setfmStereoBalanceSlider(int);
	void	setAudioGainSlider	(int n);
	void	setlfPlotType		(const QString &s);
	void	setlfPlotZoomFactor	(const QString &s);
	void	set_squelchMode		(const QString &);
	void	set_display_delay	(int);

	void	check_afc		(int);
public slots:
	void	quickStart		();
	void	setHFplotterView	(int);
	void	handle_freqButton	();
	void	newFrequency		(int);
	void	hfBufferLoaded		();
	void	wheelEvent		(QWheelEvent *);
	void	setLogging		(const QString &);
	void	setLogsaving		();
	void	AdjustFrequency		(int);
	void	setCRCErrors		(int);
	void	setSyncErrors		(int);
	void	setbitErrorRate		(double);
	void	setGroup		(int);
	void	setPiCode		(int);
	void	setStationLabel		(const QString &);
	void	setRadioText		(const QString &);
	void	setRDSisSynchronized	(bool);
	void	setMusicSpeechFlag	(int);
	void	clearMusicSpeechFlag	();
	void	scanresult		();
	void	closeEvent		(QCloseEvent *event);
//
//	changed or added
	void	lfBufferLoaded		(bool, int);
	void	iqBufferLoaded		();
	void	setPTYCode		(int, const QString &);
//	void	clearStationLabel	();
//	void	clearRadioText		();
	void	setAFDisplay		(int, int);
	void	setSquelchIsActive	(bool);
//	void	showStrength		(float, float);

	void	showPeakLevel		(const float, const float);
	void	showDcComponents	(float, float);
  //
  //	and for the extio handling
	void	set_ExtFrequency	(int);
	void	set_ExtLO		(int);
	void	set_lockLO		();
	void	set_unlockLO		();
	void	set_stopHW		();
	void	set_startHW		();
	void	set_changeRate		(int);
};
#endif
