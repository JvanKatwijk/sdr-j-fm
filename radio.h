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
#include	"fm-constants.h"
#include	"fm-processor.h"
#include	"iqdisplay.h"
#include	"ringbuffer.h"
#include	"iqdisplay.h"
#include	"scope.h"
#include	"ui_radio.h"
#include	"rds-decoder.h"
#include	<atomic>
#include	"popup-keypad.h"

#include	"ui_configwidget.h"
//class	keyPad;
class	QSettings;
class	fmProcessor;
class	rdsDecoder;
class	fft_scope;
class	audioSink;
class	deviceHandler;
class	programList;

#define THEME1NAME  "Adaptic"
#define THEME1FILE  "./stylesheets/Adaptic.qss"
#define THEME2NAME  "Combinear"
#define THEME2FILE  "./stylesheets/Combinear.qss"

#define	IQ_SCOPE_SIZE	64

constexpr int PROGRAM_RESTART_EXIT_CODE = -1234;  // value is arbitrary

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
	RingBuffer<std::complex<float>>	iqBuffer;
	IQDisplay	*iqScope;
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

	Ui_configWidget		configWidget;
	QFrame			configDisplay;
	RingBuffer<double>	*hfBuffer;
	RingBuffer<double>	*lfBuffer;
	Scope			*hfScope;
	Scope			*lfScope;
//
	keyPad		mykeyPad;
	QTimer		autoIncrementTimer;
	QTimer		displayTimer;
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
	audioSink	*our_audioSink;
	int8_t		channelSelector;
	deviceHandler	*myRig;
	std::vector<int16_t>	outTable;
	int16_t		numberofDevices;

	uint8_t		HFviewMode;
//	uint8_t		LFviewMode;
	uint8_t		inputMode;
	int16_t		currAttSliderValue;
	float		attValueL;
	float		attValueR;

	int32_t		fmBandwidth;
	int32_t		LOFrequency;
	int32_t		currentFreq;

	void		restoreGUIsettings	(QSettings *);
//	void		setDetectorScreen	(int16_t);

	int32_t		mapIncrement		(int32_t);
	int32_t		IncrementInterval	(int16_t);
	void		Display			(int32_t);
	int16_t		IncrementIndex;
	int32_t		autoIncrement_amount;
	int32_t		fmIncrement;
	int32_t		minLoopFrequency;
	int32_t		maxLoopFrequency;

	void		IncrementFrequency	(int32_t);
	void		set_incrementFlag	(int16_t);
	void		stopIncrementing	();

	void		stop_lcdTimer		();
	void		stopDumping		();
	int32_t		Panel;
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
	int32_t		currentPIcode;
	int32_t		frequencyforPICode;
	int16_t		logTime;
	FILE		*logFile;

	bool		setupSoundOut		(QComboBox *, audioSink *,
	                                         int32_t,
	                                         std::vector<int16_t> &);
	void		setup_HFScope	();
	void		setup_LFScope	();
	bool		squelchMode;
	void		resetSelector	();
	int32_t		mapRates	(int32_t);
	void		abortSystem		(int);
	void		TerminateProcess	();
	void		make_newProcessor	();
	deviceHandler	*getDevice	(const QString &);
	deviceHandler	*setDevice	(QSettings *);

//
//	added or modified
	bool		afcActive;
	float		afcAlpha;
	int32_t		afcCurrOffFreq;

	float		peakLeftDamped;
	float		peakRightDamped;
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
	void		setStart		();
	void		updateTimeDisplay	();
	void		handle_pauseButton	();

	void		setIQBalance		(int);

	void		handle_StreamOutSelector	(int);
	void		handle_dumpButton	();
	void		handle_audioDumpButton	();
	void		handle_configButton	();

	void		handle_fmFilterSelect	(const QString &);
	void		handle_fmModeSelector	(const QString &);
	void		handle_fmRdsSelector	(const QString &);
	void		handle_fmDecoder	(const QString &);
	void		handle_fmChannelSelector (const QString &);
	void		handle_fmDeemphasis	(const QString &);
	void		handle_fmLFcutoff	(const QString &);

	void		autoIncrement_timeout	();
	void		handle_fc_plus		();
	void		handle_fc_min		();
	void		handle_fm_increment	(int);
	void		handle_minimumSelect	(int);
	void		handle_maximumSelect	(int);
	void		handle_f_plus		();
	void		handle_f_min		();

	void		handle_squelchSlider	(int);
	void		handle_squelchSelector	(const QString &);
	void		handle_freqSaveButton	();
	void		handle_myLine		();
	void		handle_loggingButton	(const QString &);
	void		handle_logSavingButton	();
	void		handle_afcSelector	(int);

	void		handle_AudioGainSlider	(int n);
	void		handle_fmStereoBalanceSlider	(int);
	void		handle_fmStereoPanoramaSlider	(int);
	void		handle_plotTypeSelector	(const QString &s);
	void		handle_cbThemes		(int);
	void		handle_PlotZoomFactor	(const QString &s);
	void		handle_sbDispDelay	(int);

	void		handle_countrySelector	(const QString &);
	void		handle_freqButton	();
	void		newFrequency		(int);
	void		closeEvent		(QCloseEvent *event);

public slots:
	void		quickStart		();
	void		setHFplotterView	(int);
	void		hfBufferLoaded		();
	void		wheelEvent		(QWheelEvent *);
	void		AdjustFrequency		(int);
	void		setCRCErrors		(int);
	void		setSyncErrors		(int);
	void		setbitErrorRate		(double);
	void		setGroup		(int);
	void		setPiCode		(int);
	void		setStationLabel		(const QString &);
	void		setRadioText		(const QString &);
	void		setRDSisSynchronized	(bool);
	void		setMusicSpeechFlag	(int);
	void		clearMusicSpeechFlag	();
	void		scanresult		();
//
//	changed or added
	void		lfBufferLoaded		(bool, int);
	void		iqBufferLoaded		();
	void		setPTYCode		(int, const QString &);
//	void		clearStationLabel	();
//	void		clearRadioText		();
	void		setAFDisplay		(int, int);
	void		setSquelchIsActive	(bool);
//	void		showStrength		(float, float);

	void		showPeakLevel		(const float, const float);
	void		showMetaData		(const fmProcessor::SMetaData *);
  //
  //	and for the extio handling NOT AVAILABLE IN THIS VERSION
	void	set_ExtFrequency	(int);
	void	set_ExtLO		(int);
	void	set_lockLO		();
	void	set_unlockLO		();
	void	set_stopHW		();
	void	set_startHW		();
//	void	set_changeRate		(int);
};
#endif
