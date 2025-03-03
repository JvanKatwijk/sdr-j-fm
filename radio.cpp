#
/*
 *    Copyright (C) 2014
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<QDateTime>
#include	<QDebug>
#include	<QFile>
#include	<QFileDialog>
#include	<QMessageBox>
#include	<QTabWidget>
#include	<QHeaderView>
#include	<QSettings>
#include	<Qt>
#include	<fstream>
#include	<iostream>
#include	<array>
#include	"radio.h"
#include	"hs-scope.h"
#include	"ls-scope.h"
#include	"audiosink.h"
#include	"fm-constants.h"
#include	"fm-demodulator.h"
#include	"rds-decoder.h"
#include	"themechoser.h"
#include	"program-list.h"
#include	"device-handler.h"
#include	"filereader.h"
#ifdef HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef HAVE_SDRPLAY_V3
#include	"sdrplay-handler-v3.h"
#endif
#ifdef HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef HAVE_DABSTICK
#include	"rtlsdr-handler.h"
#endif
#ifdef HAVE_EXTIO
#include	"extio-handler.h"
#endif
#ifdef HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef HAVE_LIME
#include	"lime-handler.h"
#endif
#ifdef HAVE_PLUTO
#include	"pluto-handler.h"
#endif
#ifdef HAVE_ELAD_S1
#include	"elad-s1.h"
#endif
#ifdef __MINGW32__
#include	<iostream>
#include	<windows.h>
#endif

#define	FM_RATE			192000
#ifdef	__MINGW32__
__int64 FileTimeToInt64 (FILETIME & ft) {
	ULARGE_INTEGER foo;

	foo.LowPart	= ft.dwLowDateTime;
	foo.HighPart	= ft.dwHighDateTime;
	return (foo.QuadPart);
}

bool get_cpu_times (size_t &idle_time, size_t &total_time) {
FILETIME IdleTime, KernelTime, UserTime;
size_t	thisIdle, thisKernel, thisUser;

	GetSystemTimes (&IdleTime, &KernelTime, &UserTime);
	
	thisIdle	= FileTimeToInt64 (IdleTime);
	thisKernel	= FileTimeToInt64 (KernelTime);
	thisUser	= FileTimeToInt64 (UserTime);
	idle_time	= (size_t) thisIdle;
	total_time	= (size_t)(thisKernel + thisUser);
	return true;
}
#else
std::vector<size_t> get_cpu_times() {
	std::ifstream proc_stat ("/proc/stat");
	proc_stat. ignore (5, ' ');    // Skip the 'cpu' prefix.
	std::vector<size_t> times;
	for (size_t time; proc_stat >> time; times. push_back (time));
	return times;
}
 
bool get_cpu_times (size_t &idle_time, size_t &total_time) {
	const std::vector <size_t> cpu_times = get_cpu_times();
	if (cpu_times. size() < 4)
	   return false;
	idle_time  = cpu_times [3];
	total_time = std::accumulate (cpu_times. begin(), cpu_times. end(), 0);
	return true;
}
#endif

#include	"deviceselect.h"

#define	D_SDRPLAY	"sdrplay"
#define	D_SDRPLAY_V3	"sdrplay-v3"
#define	D_RTL_TCP	"rtl_tcp"
#define	D_HACKRF	"hackrf"
#define	D_LIME		"lime"
#define	D_COLIBRI	"colibri"
#define	D_AIRSPY	"airspy"
#define	D_RTLSDR	"dabstick"
#define	D_PLUTO		"pluto"
#define	D_ELAD_S1	"elad-s1"
#define	D_EXTIO		"extio"
#define	D_PMSDR		"pmsdr"
#define	D_FILEREADER	"filereader"

static 
const char *deviceTable [] = {
#ifdef	HAVE_SDRPLAY
	D_SDRPLAY,
#endif
#ifdef	HAVE_SDRPLAY_V3
	D_SDRPLAY_V3,
#endif
#ifdef	HAVE_PLUTO
	D_PLUTO,
#endif
#ifdef	HAVE_AIRSPY
	D_AIRSPY,
#endif
#ifdef	HAVE_HACKRF
	D_HACKRF,
#endif
#ifdef	HAVE_LIME
	D_LIME,
#endif
#ifdef	HAVE_DABSTICK
	D_RTLSDR,
#endif
#ifdef	HAVE_RTL_TCP
	D_RTL_TCP,
#endif
	D_FILEREADER,
	nullptr
};

#define D_SDRPLAY       "sdrplay"
#define D_SDRPLAY_V3    "sdrplay-v3"
#define D_RTL_TCP       "rtl_tcp"
#define D_HACKRF        "hackrf"
#define D_LIME          "lime"
#define D_COLIBRI       "colibri"
#define D_AIRSPY        "airspy"
#define D_RTLSDR        "dabstick"
#define D_PLUTO         "pluto"
#define D_ELAD_S1       "elad-s1"
#define D_EXTIO         "extio"
#define D_PMSDR         "pmsdr"
#define D_FILEREADER    "filereader"
static int startKnop;
static	QTimer	*starter;
constexpr int16_t delayTable [] = { 1, 3, 5, 7, 9, 10, 15 };
constexpr int16_t delayTableSize = ((int)(sizeof(delayTable) / sizeof(int16_t)));
/**
 * @file gui.cpp
 * @brief gui.cpp : Defines the functions for the GUI of the FM software
 * @author Jan van Katwijk
 * @version 0.98
 * @date 2015-01-07
 */
		RadioInterface::RadioInterface (QSettings	*Si,
						QString		saveName,
						ThemeChoser	*themeChooser,
						int32_t		outputRate,
						QWidget		*parent):
							QDialog (parent),
							iqBuffer (
	                                                         IQ_SCOPE_SIZE),
	                                                hfBuffer (8 * 32768),
	                                                lfBuffer (32768),
	                                                theDemodulator (
	                                                         FM_RATE),
							configDisplay (nullptr),
							mykeyPad () {

	int16_t i;
	QString h;
	int     k;

	setupUi (this);
	fmSettings		= Si;
	this	-> themeChooser	= themeChooser;

	configWidget. setupUi (&configDisplay);
	runMode. store (ERunStates::IDLE);
	squelchMode		= false;
//
//	Added: cannot compile on Ubuntu 16, the system where
//	I build the appImage
//
//	with QT 5.15.2 and Ubuntu 22.04.1 LTS it works
//	so, feel free to uncomment, but leave it commented out for me
//	setWindowFlag (Qt::WindowContextHelpButtonHint, false);
//	setWindowFlag (Qt::WindowMinMaxButtonsHint, true);

	thermoPeakLevelLeft	-> setFillBrush (QColor ("white"));
        thermoPeakLevelRight	-> setFillBrush (QColor ("white"));
	thermoPeakLevelLeft	-> setValue (-30);
	thermoPeakLevelRight	-> setValue (-30);
	
	reset_afc ();
//
//	added inits for various class variables
	afcActive		= false;
	afcAlpha		= 1;
	afcCurrOffFreq		= 0;

	peakLeftDamped		= -100;
	peakRightDamped		= -100;
//	end added
//
	this		-> inputRate = INPUT_RATE;
	this		-> fmRate    = FM_RATE;
	this		-> workingRate = 48000;
/**
  *	We allow the user to set the displaysize
  *	(as long as it is reasonable)
  */
	this	-> displaySize =
	             fmSettings -> value ("displaySize", 512).toInt();
	if ((displaySize & (displaySize - 1)) != 0) 
	   displaySize = 1024;
	this	-> spectrumSize	= 4 * displaySize;

	this		-> rasterSize	=
	                     fmSettings -> value ("rasterSize", 50). toInt ();
	this		-> repeatRate	=
	                     fmSettings -> value ("repeatRate", 10). toInt ();
	this		-> averageCount =
	                     fmSettings -> value ("averageCount", 5). toInt ();
	this		-> audioRate    =
	                     fmSettings -> value ("audioRate",
						     outputRate). toInt ();
//
//	fill the decoder selector
	QStringList names = theDemodulator.  listNameofDecoder ();
	for (QString decoderName: names)
	   configWidget. fmDecoderSelector -> addItem (decoderName);

	h	= fmSettings -> value ("fmDecoder", "PLL Decoder"). toString ();
	k	= configWidget. fmDecoderSelector -> findText (h);

	if (k != -1) {
	   configWidget. fmDecoderSelector -> setCurrentIndex (k);
	   handle_fmDecoderSelector (configWidget. fmDecoderSelector -> currentText ());
	}

	h	= fmSettings -> value ("rdsSelector", "RDS 1"). toString ();
	k	= fmRdsSelector -> findText (h);
	if (k != -1)
	   fmRdsSelector -> setCurrentIndex (k);
//
	myFMprocessor	= nullptr;
	our_audioSink	= new audioSink (this -> audioRate, 16384);
	outTable. resize (our_audioSink -> numberofDevices () + 1);
	for (i = 0; i < our_audioSink -> numberofDevices (); i++) 
	   outTable [i] = -1;

	if (!setupSoundOut (configWidget. streamOutSelector,
			    our_audioSink,
			    this -> audioRate,
			    outTable)) {
	   fprintf(stderr, "Cannot open any output device\n");
	   abortSystem (33);
	}
/**
  *	Use, if possible, the outputstream the user had previous time
  */
	h = fmSettings -> value ("streamOutSelector",
				      "default"). toString ();
	k = configWidget. streamOutSelector -> findText (h);
	if (k != -1) {
	   configWidget. streamOutSelector -> setCurrentIndex (k);
	   handle_StreamOutSelector (k);
	}

	setup_HFScope	();
	setup_LFScope	();
//added
	setup_IQPlot	();
//end added
//
//	Set relevant sliders etc to the value they had last time
	restoreGUIsettings (fmSettings);
//
	int weCloseDirect = fmSettings -> value ("closeDirect", 0). toInt ();
	if (weCloseDirect != 0)
	   configWidget. closeDirect -> setChecked (true);

	configWidget. incrementFlag ->
			 setStyleSheet ("QLabel {background-color:blue}");
	configWidget. incrementFlag -> setText(" ");
	incrementIndex		= 0;
//	settings for the auto tuner
	incrementIndex		=
		     fmSettings -> value ("incrementIndex", 0). toInt ();
	fmIncrement		=
		     fmSettings -> value ("fm_increment", 100). toInt ();

	minLoopFrequency	=
	   fmSettings -> value ("min_loop_frequency", 86500). toInt ();
	if (minLoopFrequency == 0)
	   minLoopFrequency = 86500;

	maxLoopFrequency	=
	   fmSettings -> value ("max_loop_frequency", 110000). toInt ();
	if (maxLoopFrequency == 0)
	   maxLoopFrequency = 110000;

	configWidget. fm_increment	-> setValue (fmIncrement);	//
	configWidget. minimumSelect	-> setValue (KHz (minLoopFrequency) / MHz(1));
	configWidget. maximumSelect	-> setValue (KHz (maxLoopFrequency) / MHz(1));

//	he does the connections from the gui buttons, sliders etc
	localConnects		();

//	Create a timer for autoincrement/decrement of the tuning
	autoIncrementTimer. setSingleShot (true);
	autoIncrementTimer. setInterval (5000);
	connect (&autoIncrementTimer, SIGNAL (timeout()),
		 this, SLOT (autoIncrement_timeout ()));

//	create a timer for displaying the "real" time
	displayTimer. setInterval (1000);
	connect (&displayTimer,
		 SIGNAL (timeout ()),
		 this,
		 SLOT (updateTimeDisplay ()));
//
//	Display the version
	QString v = "sdrJ-FM -V" + QString (CURRENT_VERSION);
	systemindicator -> setText (v);
	copyrightLabel	-> setToolTip (footText ());

	ExtioLock		= false;
	logFile			= nullptr;
	pauseButton		-> setText (QString ("Pause"));

	sourceDumping		= false;
	audioDumping		= false;
	dumpfilePointer		= nullptr;
	audiofilePointer	= nullptr;
	configWidget. dumpButton	-> setText ("inputDump");
	configWidget. audioDumpButton	-> setText ("audioDump");

	currentPIcode		= 0;
	frequencyforPICode	= 0;
	theSelector		-> hide ();
	theDevice		= new deviceHandler ();
	currentFreq		= setTuner (Khz (94700));
	inputRate		= theDevice -> getRate ();

	hfScope		-> setBitDepth (theDevice -> bitDepth ());
	lfScope		-> setBitDepth (theDevice -> bitDepth ());
//
	connect (configWidget. fm_increment, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_fm_increment (int)));
	connect (configWidget. minimumSelect, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_minimumSelect (int)));
	connect (configWidget. maximumSelect, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_maximumSelect (int)));
	displayTimer. start (1000);

	scrollStationList	-> setWidgetResizable (true);
	myProgramList		= new programList (this, saveName, scrollStationList);
//
	connect (freqSave, SIGNAL (clicked ()),
	         this, SLOT (handle_freqSaveButton ()));
	connect	(cbAfc, SIGNAL (stateChanged (int)),
	         this,  SLOT (handle_afcSelector (int)));

	QString country	= 
	         fmSettings -> value ("ptyLocale", "Europe"). toString ();
	if ((country == "Europe") || (country == "USA")) {
	   k = configWidget. countrySelector -> findText (country);
	   if (k != -1)
	      configWidget. countrySelector	-> setCurrentIndex (k);
	}
	connect (configWidget. countrySelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this,  &RadioInterface::handle_countrySelector);

	QString device =
	          fmSettings -> value ("device", "no device").toString ();

	k = -1;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   if (deviceTable [i] == device) {
	      k = i;
	      break;
	}
	if (k != -1) {
	   starter	= new QTimer;
	   startKnop	= k;
	   starter -> setSingleShot (true);
	   starter -> setInterval (500);
	   connect (starter, SIGNAL (timeout()), 
	            this, SLOT (quickStart ()));
	   starter -> start (500);
	}
	else {
//	   deviceSelector	-> setCurrentIndex (0);
	   startKnop	= 0;
	   if (setDevice (fmSettings) == nullptr)
	      TerminateProcess ();
	}

	for (auto name : themeChooser -> get_style_sheet_names ())
	   configWidget. cbThemes -> addItem (name);
	configWidget. cbThemes -> setCurrentIndex (themeChooser -> get_curr_style_sheet_idx());
}


QString RadioInterface::footText () {
QString versionText = "sdr-j-FM version: " + QString(CURRENT_VERSION) + "\n";
	versionText += "Built on " + QString(__TIMESTAMP__) + QString (", Commit ") + QString (GITHASH) + "\n";
	versionText += "Copyright Jan van Katwijk, mailto:J.vanKatwijk@gmail.com\n";
	versionText += "with significant input from Tomneda\n";
	versionText += "Rights of Qt, fftw, portaudio, libfaad, libsamplerate and libsndfile gratefully acknowledged\n";
	versionText += "Rights of developers of RTLSDR library, SDRplay libraries, AIRspy library and others gratefully acknowledged\n";
	versionText += "Rights of other contributors gratefully acknowledged";
	return versionText;
}
void	RadioInterface::quickStart () {
	disconnect (starter, SIGNAL (timeout ()),
	            this, SLOT (quickStart ()));
	fprintf (stderr, "going for quickStart\n");
	delete starter;
	if (getDevice (deviceTable [startKnop]) ==  nullptr)
	   if (setDevice (fmSettings) == nullptr)
	      TerminateProcess ();
}

//
//	The end of all
	RadioInterface::~RadioInterface () {
}

//
//	Function used to "dump" settings into the ini file
//	pointed to by s

void	RadioInterface::dumpControlState	(QSettings *s) {

	if (s == nullptr)
	   return;

	//	s	-> setValue ("device", deviceSelector -> currentText ());
	s	-> setValue ("fm_increment",
				     configWidget. fm_increment -> value ());
	s	-> setValue ("spectrumAmplitudeSlider_hf",
				     spectrumAmplitudeSlider_hf -> value ());
	s	-> setValue ("spectrumAmplitudeSlider_lf",
				     spectrumAmplitudeSlider_lf -> value ());
	s	-> setValue ("IQbalanceSlider",
				     IQbalanceSlider	-> value());
	s	-> setValue ("afc",
				     cbAfc -> checkState ());
	s	-> setValue ("dcRemove",
				     cbDCRemove -> checkState ());
	s	-> setValue ("autoMono",
				     cbAutoMono -> checkState ());
	s	-> setValue ("pss",
				     cbPSS -> checkState ());

	//	now setting the parameters for the fm decoder
	s	-> setValue ("fmFilterSelect",
				     configWidget. fmFilterSelect -> currentText ());
	s	-> setValue ("fmMode",
				     fmModeSelector	-> currentText ());
	s	-> setValue ("fmDecoder",
				     configWidget. fmDecoderSelector	-> currentText ());
	s	-> setValue ("volumeHalfDb",
				     volumeSlider	-> value ());
	s	-> setValue ("fmRdsSelector",
				     fmRdsSelector	-> currentText ());
	s	-> setValue ("fmChannelSelect",
				     fmChannelSelect	-> currentText ());
	s	-> setValue ("fmDeemphasisSelector",
				     configWidget. fmDeemphasisSelector -> currentText ());
	s	-> setValue ("fmStereoPanoramaSlider",
				     fmStereoPanoramaSlider -> value ());
	s	-> setValue ("fmStereoBalanceSlider",
				     fmStereoBalanceSlider	-> value ());
	s	-> setValue ("fmLFcutoff",
				     fmLFcutoff		-> currentText ());
	s	-> setValue ("logging",
				     configWidget. loggingButton	-> currentText ());
	s	-> setValue ("streamOutSelector",
				     configWidget. streamOutSelector	-> currentText ());

	s	-> setValue ("currentFreq",
				     currentFreq);
	s	-> setValue ("min_loop_frequency",
				     minLoopFrequency);
	s	-> setValue ("max_loop_frequency",
				     maxLoopFrequency);

	s	-> setValue ("peakLevelDelaySteps",
				     configWidget. sbDispDelay	-> value ());

	s	-> setValue ("styleSheet",
	                        configWidget. cbThemes -> currentText ());

	s	-> sync ();
}

	//	On start, we ensure that the streams are stopped so
	//	that they can be restarted again.
void	RadioInterface::setStart	() {
bool r = false;

//	someone presses while running, ignore
	if (runMode. load () == ERunStates::RUNNING) {
	   return;
	}

	r = theDevice	-> restartReader ();
//	qDebug ("Starting %d\n", r);
	if (!r) {
	   QMessageBox::warning(this, tr("sdr"),
				   tr("Opening  input stream failed\n"));
	   return;
	}

	if (myFMprocessor == nullptr) 
	   make_newProcessor ();
	myFMprocessor	-> start ();
	our_audioSink	->restart ();

//	and finally: recall that starting overrules pausing
	pauseButton	-> setText (QString ("Pause"));
//
//	New stuff:
	handle_squelchSelector ("NSQ"); // toggle sequelch on
//	toggle sequelch off (TODO: make this nicer)
	handle_squelchSelector ("SQ OFF");


	int k	= fmSettings -> value ("dcRemove", Qt::CheckState::Checked).toInt ();
	cbDCRemove	-> setCheckState (k ? Qt::CheckState::Checked :
	                                  Qt::CheckState::Unchecked);

	k	= fmSettings -> value ("autoMono", Qt::CheckState::Checked).toInt ();
	cbAutoMono -> setCheckState (k ? Qt::CheckState::Checked :
	                                 Qt::CheckState::Unchecked);

	k	= fmSettings -> value ("pss", Qt::CheckState::Checked).toInt ();
	cbPSS-> setCheckState (k ? Qt::CheckState::Checked :
	                           Qt::CheckState::Unchecked);

	myFMprocessor -> setDCRemove	(cbDCRemove -> checkState());
	myFMprocessor -> setAutoMonoMode	(cbAutoMono -> checkState());
	myFMprocessor -> setPSSMode	(cbPSS -> checkState());

	int vol = fmSettings -> value ("volumeHalfDb", -12).toInt();
	volumeSlider -> setValue (vol);
	handle_AudioGainSlider (vol);


	connect (cbAutoMono, &QCheckBox::clicked,
	         this, [this](bool isChecked){myFMprocessor -> setAutoMonoMode(isChecked); });
	connect (cbPSS, &QCheckBox::clicked,
	         this, [this](bool isChecked){myFMprocessor -> setPSSMode(isChecked); });
	connect (cbDCRemove, &QCheckBox::clicked,
	         this, [this](bool isChecked){ myFMprocessor->setDCRemove(isChecked); });
	connect (volumeSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_AudioGainSlider (int)));
	connect (configWidget. cbTestTone, SIGNAL (stateChanged (int)),
	         this,  SLOT (handle_cbTestTone (int)));
	connect (configWidget. sbDispDelay, SIGNAL (valueChanged (int)),
	         this,  SLOT (handle_sbDispDelay (int)));
	connect (btnRestartPSS, &QAbstractButton::clicked,
	         this, [this](){myFMprocessor -> restartPssAnalyzer(); });

	runMode. store (ERunStates::RUNNING);
}

//
//	always tricky to kill tasks
void	RadioInterface::TerminateProcess () {
	runMode. store (ERunStates::STOPPING);
	if (myFMprocessor == nullptr)
	   exit (1);
	if (sourceDumping && (myFMprocessor != nullptr)) {
	   myFMprocessor -> stopDumping ();
	   sf_close (dumpfilePointer);
	}

	if (audioDumping) {
	   our_audioSink -> stopDumping();
	   sf_close (audiofilePointer);
	}

	if (myProgramList != nullptr)
	   myProgramList -> saveTable ();

	stopIncrementing ();
	dumpControlState (fmSettings);
	fmSettings		-> sync ();

	configDisplay. hide ();
  //	It is pretty important that no one is attempting to
  //	set things within the FMprocessor when it is
  //	being deleted
	theDevice		-> stopReader ();
  	myFMprocessor	-> stop ();
//
//	fmProcessor and device are stopped
	if (myFMprocessor != nullptr) 
	   delete myFMprocessor;

//	setDevice (QString ("dummy"));	// will select a virtualinput
	accept();

	qDebug () << "Termination started";
	delete		theDevice;
	delete		our_audioSink;
	delete		myProgramList;
	delete		hfScope;
	delete		lfScope;
}

void	RadioInterface::abortSystem (int d) {
	qDebug ("aborting for reason %d\n", d);
	accept ();
}
//
//	The following signals originate from the Winrad Extio interface
//
//	Note: the extio interface provides two signals
//	one ExtLO signals that the external LO is set
//	to a different value,
//	the other one, ExtFreq, requests the client program
//	to adapt its (local) tuning settings to a new frequency
void	RadioInterface::set_ExtFrequency (int f) {
int32_t vfo	= theDevice -> getVFOFrequency ();
	(void)f;
	currentFreq	= vfo + inputRate / 4;
	loFrequency	= inputRate / 4;
	displayFrequency (currentFreq);
	if (myFMprocessor != nullptr) {
	   myFMprocessor	-> set_localOscillator (loFrequency);
	   myFMprocessor	-> triggerFrequencyChange ();
	}
}
//
//	From our perspective, the external device only provides us
//	with a vfo
void	RadioInterface::set_ExtLO	(int f) {
	set_ExtFrequency (f);
}

void	RadioInterface::set_lockLO	() {
//	fprintf (stderr, "ExtioLock is true\n");
	ExtioLock	= true;
}

void	RadioInterface::set_unlockLO	() {
//	fprintf (stderr, "ExtioLock is false\n");
	ExtioLock	= false;
}

void	RadioInterface::set_stopHW	() {
	theDevice	-> stopReader ();
}

void	RadioInterface::set_startHW	() {
	if (runMode. load () == ERunStates::RUNNING)
	   theDevice -> restartReader();
}
//
////	This is a difficult one, everything should go down first
////	and then restart with the new samplerate
//void	RadioInterface::set_changeRate	(int r) {
//	if (r == inputRate)
//	   return;
//	fprintf (stderr, "request for changerate\n");
//	theDevice	-> stopReader ();
//	if (myFMprocessor != nullptr) {
//	   myFMprocessor	-> stop();
//	   delete myFMprocessor;
//	   myFMprocessor	= nullptr;
//	}
//
//	runMode. store (ERunStates::IDLE);
////
////	Now we need to rebuild the prerequisites for the "new" processor
//	inputRate = r;
//	if (inputRate < Khz (176)) { // rather arbitrarily
//	   QMessageBox::warning (this, tr("sdr"),
//	                         tr("Sorry, rate low\n"));
//	   delete theDevice;
//	   theDevice	= new deviceHandler ();
//	   inputRate	= theDevice -> getRate ();
//	}
////
////	compute the new fmRate
////	fmRate			= mapRates (inputRate);
////	ask the new for the frequency
//	currentFreq		= theDevice -> getVFOFrequency () + fmRate / 4;
////	and show everything
//	Display (currentFreq);
//	lcd_fmRate		-> display ((int)this -> fmRate);
//	lcd_inputRate		-> display ((int)this -> inputRate);
//	lcd_OutputRate		-> display ((int)this -> audioRate);
////
////	The device is still the same, so now we wait for a start
//}
//
//	@brief setDevice is called upon pressing the device button
//	@params: the name (string) on the button

deviceHandler	*RadioInterface::getDevice (const QString &s) {
QString file;
bool    success;

//	The fm processor is a client of the rig, so the
//	fm processor has to go first
	if (theDevice != nullptr) {
	   theDevice	-> stopReader ();
	   delete theDevice;
	   theDevice	= nullptr;
	}
	if (myFMprocessor != nullptr) {
	   myFMprocessor	->stop ();
	   delete myFMprocessor;
	   myFMprocessor	= nullptr;
	}

	runMode. store (ERunStates::IDLE);
	ExtioLock	= false;
	success		= true;		// default for now

#ifdef HAVE_SDRPLAY
	if (s == D_SDRPLAY) {
	   try {
	      theDevice = new sdrplayHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_SDRPLAY_V3
	if (s == D_SDRPLAY_V3) {
	   try {
	      theDevice = new sdrplayHandler_v3 (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_AIRSPY
	if (s == D_AIRSPY) {
	   try {
	      theDevice = new airspyHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_HACKRF
	if (s == D_HACKRF) {
	   try {
	      theDevice = new hackrfHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_LIME
	if (s == D_LIME) {
	   try {
	      theDevice = new limeHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_PLUTO
	if (s == D_PLUTO) {
	   try {
	      theDevice = new plutoHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_ELAD_S1
	if (s == D_ELAD_S1) {
	   try {
	      theDevice = new eladHandler (fmSettings, true, &success);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_DABSTICK
	if (s == D_RTLSDR) {
	   try {
	      theDevice = new rtlsdrHandler (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_EXTIO
	if (s == D_EXTIO) {
	   try {
	      theDevice = new ExtioHandler (fmSettings, theSelector, &success);
	   } catch (int e) {
	      success = false;
	}
	else
#endif
	if (s == "filereader") {
	   try {
	      theDevice	= new fileReader (fmSettings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else 
	   theDevice	= new deviceHandler ();

	if (!success) {
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("loading device failed"));
	   if (theDevice == nullptr)
	      theDevice = new deviceHandler ();	// the empty one
	   return nullptr;
	}

	inputRate = theDevice -> getRate ();
	if (inputRate < Khz (176)) { // rather arbitrarily
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("Sorry, rate low\n"));
	   delete theDevice;
	   theDevice	= new deviceHandler ();
	   inputRate	= theDevice -> getRate ();
	}
//
//	ask the new rig for the frequency
//	fmRate		= mapRates (inputRate);
	currentFreq	= theDevice -> defaultFrequency () + fmRate / 4;
	currentFreq	= fmSettings -> value ("currentFreq",
	                                        currentFreq). toInt ();
	displayFrequency (currentFreq);
	lcd_fmRate		-> display ((int)this -> fmRate);
	lcd_inputRate		-> display ((int)this -> inputRate);
	lcd_OutputRate		-> display ((int)this -> audioRate);
//	connect (theDevice, SIGNAL (set_changeRate (int)),
//	         this, SLOT (set_changeRate (int)));

#ifdef __MINGW32__
//	communication from the dll to the main program is through signals
	if (s == D_EXTIO) {
//	and for the extio:
//	The following signals originate from the Winrad Extio interface
	   connect (theDevice, SIGNAL (set_ExtFrequency (int)),
	            this, SLOT (set_ExtFrequency (int)));
	   connect (theDevice, SIGNAL (set_ExtLO (int)),
	            this, SLOT (set_ExtLO (int)));
	   connect (theDevice, SIGNAL (set_lockLO ()),
	            this, SLOT (set_lockLO ()));
	   connect (theDevice, SIGNAL (set_unlockLO ()),
	            this, SLOT (set_unlockLO ()));
	   connect (theDevice, SIGNAL (set_stopHW ()),
	            this, SLOT (set_stopHW ()));
	   connect (theDevice, SIGNAL (set_startHW ()),
	            this, SLOT (set_startHW ()));
	}
#endif
	theDevice -> setVFOFrequency (currentFreq);
	setStart ();
	fmSettings	-> setValue ("device", s);
	return theDevice;
}
//
//
deviceHandler	*RadioInterface::setDevice (QSettings *fmSettings) {
(void)fmSettings;
deviceSelect	deviceSelect;
deviceHandler	*theDevice	= nullptr;
QStringList devices;

	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   devices += deviceTable [i];
	devices	+= "quit";

	deviceSelect. addList (devices);
	int theIndex = -1;
	while (theDevice == nullptr) {
	   theIndex = deviceSelect. QDialog::exec ();
	   if (theIndex < 0)
	      continue;
	   QString s = devices. at (theIndex);
	   if (s == "quit")
	      return nullptr;
	   theDevice	= getDevice (s);
	}
	return theDevice;
}
//
//	Just for convenience packed as a function
void	RadioInterface::make_newProcessor () {
	QString area
	         = fmSettings -> value ("ptyLocale", "Europe"). toString ();
	int ptyLocale	= area == "Europe" ? 0 : 1;
	int thresHold	
	         =  fmSettings -> value ("threshold", 20). toInt ();
	 
	myFMprocessor = new fmProcessor (theDevice,
	                                 this,
	                                 our_audioSink,
	                                 &theDemodulator,
	                                 inputRate,
	                                 fmRate,
	                                 workingRate,
	                                 this -> audioRate,
	                                 displaySize,
	                                 spectrumSize,
	                                 repeatRate,
	                                 ptyLocale,
	                                 &hfBuffer,
	                                 &lfBuffer,
	                                 &iqBuffer,
	                                 thresHold);
	lcd_fmRate		-> display ((int)this -> fmRate);
	lcd_inputRate		-> display ((int)this -> inputRate);
	lcd_OutputRate		-> display ((int)this -> audioRate);
	hfScope			-> setBitDepth (theDevice -> bitDepth ());

	handle_fmFilterSelect	(configWidget. fmFilterSelect	-> currentText ());
	handle_fmModeSelector	(fmModeSelector		-> currentText ());
	handle_fmRdsSelector	(fmRdsSelector 		-> currentText ());
	handle_fmChannelSelector (fmChannelSelect	-> currentText ());
	handle_fmDeemphasis	(configWidget. fmDeemphasisSelector	-> currentText ());
	handle_squelchSlider	(squelchSlider		-> value ());
	handle_fmLFcutoff	(fmLFcutoff		-> currentText ());
	handle_loggingButton	(configWidget. loggingButton	-> currentText ());
	hfScope			->setBitDepth		(theDevice	-> bitDepth ());

	handle_sbDispDelay	(configWidget. sbDispDelay	-> value ());
	handle_fmStereoBalanceSlider	(fmStereoBalanceSlider	-> value ());
	handle_fmStereoPanoramaSlider	(fmStereoPanoramaSlider -> value ());
}

void	RadioInterface::handle_fmChannelSelector (const QString &s) {

	if (s == "L | R")
	   channelSelector = fmProcessor::S_STEREO;
	else
	if (s == "R | L")
	   channelSelector = fmProcessor::S_STEREO_SWAPPED;
	else
	if (s == "L | L")
	   channelSelector = fmProcessor::S_LEFT;
	else
	if (s == "R | R")
	   channelSelector = fmProcessor::S_RIGHT;
	else
	if (s == "M | M")
	   channelSelector = fmProcessor::S_LEFTplusRIGHT;
	else
	if (s == "S | S")
	   channelSelector = fmProcessor::S_LEFTminusRIGHT;
	else
	if (s == "T | T")
		channelSelector = fmProcessor::S_LEFTminusRIGHT_Test;
	else		// the default
	   channelSelector = fmProcessor::S_STEREO;
	if (myFMprocessor != nullptr)
	   myFMprocessor	-> setSoundMode (channelSelector);
}

/*
 *  the balance slider runs between -100 .. 100
 */
void	RadioInterface::setIQBalance (int n) {
int16_t bl, br;

	IQBalanceDisplay->display(n);
#ifdef DO_STEREO_SEPARATION_TEST
	if (myFMprocessor != nullptr)
	   myFMprocessor	-> setAttenuation ((float)n, 0);
#else
	bl		= 100 - n;
	br		= 100 + n;
	attValueL	= (float)bl / 100;
	attValueR	= (float)br / 100;
	if (myFMprocessor != nullptr)
	   myFMprocessor	-> setAttenuation (attValueL, attValueR);
#endif
}
//
//	Increment frequency: with amount N, depending
//	on the mode of operation
//
int32_t		RadioInterface::mapIncrement (int32_t n) {
	return Khz (n);
}
//
//	incrementFrequency is called from the handlers
//	for the autoincrement, the increment and the
//	++, + etc knobs.
//	Deviations within 15K of the VFO are handled with an
//	offset "onscreen"
void	RadioInterface::incrementFrequency (int32_t n) {
int32_t vfoFreq;

	stopIncrementing	();
	vfoFreq		= theDevice -> getVFOFrequency	();
	currentFreq	= setTuner (vfoFreq + loFrequency + n);
}
//	adjustFrequency is called whenever someone clicks
//	with the button on the screen. The amount
//	has to be multiplied with 1000
void	RadioInterface::adjustFrequency (int n) {
	incrementFrequency (Khz (n));
}
//
//	Whenever the mousewheel is changed, the frequency
//	is adapted
void	RadioInterface::wheelEvent (QWheelEvent *e) {
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 0)
	if (e -> angleDelta (). y () > 0)
#else
	if (e -> delta () > 0)
#endif
	   incrementFrequency (KHz (1));
	else
	   incrementFrequency (-KHz (1));
}
//
//	The generic setTuner.
//
int32_t	RadioInterface::setTuner (int32_t n) {
int32_t	vfo;

	if (!theDevice -> legalFrequency (n))
	   return Khz (94700);
	vfo = theDevice -> getVFOFrequency ();

//	check whether new frequency fits in current window
	if (abs (n - vfo) > inputRate / 2 - fmRate) {
	   theDevice -> setVFOFrequency (n);
	   vfo = theDevice -> getVFOFrequency ();
//
//	we create a new spectrum on a different frequency
	   if (runMode. load () == ERunStates::RUNNING) 
	      hfScope -> clearAverage ();
	}
	loFrequency = n - vfo;

//	constrain loFrequency, since that is used as an index in a table
	if (loFrequency > inputRate / 2)
	   loFrequency = inputRate / 2;
	else
	if (loFrequency < -inputRate / 2)
	   loFrequency = -inputRate / 2;

	if (myFMprocessor != nullptr) {
	   myFMprocessor -> set_localOscillator (loFrequency);
//	redraw LF frequency and reset RDS only with bigger frequency steos
//	AFC will trigger this too
	   if (std::abs (vfo + loFrequency - currentFreq) >= KHz (100))
	      myFMprocessor -> triggerFrequencyChange ();
	}
	
	displayFrequency (vfo + loFrequency);
	return vfo + loFrequency;
}
//
//===== code for auto increment/decrement
//	lots of code for something simple,

static inline
bool	frequencyInBounds (int32_t f, int32_t l, int32_t u) {
	return l <= f && f <= u;
}

int32_t RadioInterface::incrementInterval (int16_t index) {
	if (index < 0)
	   index = -index;

	if (index == 0)
	   index = 1;
	if (index >= delayTableSize)
	   index = delayTableSize;

	return 1000 * delayTable [index - 1];
}

void	RadioInterface::set_incrementFlag (int16_t incr) {
char temp [128];

	if (incr == 0) {
	   configWidget. incrementFlag ->
	                 setStyleSheet ("QLabel {background-color:blue}");
	   configWidget. incrementFlag -> setText (" ");
	   return;
	}
	if (incr < 0) 
	   sprintf (temp, " << %d", incrementInterval(incr) / 1000);
	else
	   sprintf (temp, "%d >> ", incrementInterval(incr) / 1000);
	configWidget. incrementFlag ->
	                 setStyleSheet ("QLabel {background-color:green}");
	configWidget. incrementFlag -> setText (temp);
}
//
void	RadioInterface::autoIncrement_timeout () {
int32_t	amount;
int32_t	frequency;
int32_t	low, high;

	low	= KHz (minLoopFrequency);
	high	= KHz (maxLoopFrequency);
	amount	=  fmIncrement;
	if (incrementIndex < 0)
	   amount = - amount;
//
	frequency	= currentFreq + KHz (amount);

	if ((incrementIndex < 0) &&
	   !frequencyInBounds (frequency, low, high))
	   frequency = high;

	if ((incrementIndex > 0) &&
	   !frequencyInBounds (frequency, low, high))
	   frequency = low;

	currentFreq = setTuner (frequency);

	autoIncrementTimer. start (incrementInterval (incrementIndex));
	if (myFMprocessor != nullptr)
	   myFMprocessor -> startScanning ();
}

void	RadioInterface::scanresult	() {
	stopIncrementing ();
}
//
//	stopIncrementing is called from various places to
//	just interrupt the autoincrementing
void	RadioInterface::stopIncrementing	() {
	set_incrementFlag (0);

	if (autoIncrementTimer. isActive ())
	   autoIncrementTimer. stop ();

	incrementIndex = 0;
	if (myFMprocessor != nullptr)
	   myFMprocessor	-> stopScanning ();
}

void	RadioInterface::handle_fc_plus	() {

	if (autoIncrementTimer. isActive ())
	   autoIncrementTimer. stop ();

	if (++incrementIndex > delayTableSize)
	   incrementIndex = delayTableSize;

	if (incrementIndex == 0) {
	   set_incrementFlag (0);
	   return;
	}
  //
	autoIncrementTimer. start (incrementInterval (incrementIndex));
	set_incrementFlag (incrementIndex);
}

void	RadioInterface::handle_fc_min () {

	if (autoIncrementTimer. isActive ())
	   autoIncrementTimer. stop ();

	if (--incrementIndex < - delayTableSize)
	   incrementIndex = - delayTableSize;

	if (incrementIndex == 0) {
	   set_incrementFlag(0);
	   return;
	}
//
	autoIncrementTimer. start (incrementInterval (incrementIndex));
	set_incrementFlag (incrementIndex);
}

void	RadioInterface::handle_fm_increment (int v) {
	fmIncrement = v; // in Khz
}

//
//	min and max frequencies are specified in Mhz
void	RadioInterface::handle_minimumSelect	(int f) {
	minLoopFrequency	= Khz (f);
}

void	RadioInterface::handle_maximumSelect	(int f) {
	maxLoopFrequency	= Khz (f);
}

void	RadioInterface::handle_f_plus	() {
	stopIncrementing ();
	currentFreq	= setTuner (currentFreq + Khz (fmIncrement));
}

void	RadioInterface::handle_f_min	() {
	stopIncrementing ();
	currentFreq = setTuner (currentFreq - Khz(fmIncrement));
}

//
static size_t previous_idle_time        = 0;
static size_t previous_total_time       = 0;

void	RadioInterface::updateTimeDisplay () {
QDateTime	currentTime = QDateTime::currentDateTime ();
static int numberofSeconds	= 0;

	numberofSeconds ++;
	if ((numberofSeconds % 2) == 0) {
	   size_t idle_time, total_time;
	   get_cpu_times (idle_time, total_time);
	   const float idle_time_delta = idle_time - previous_idle_time;
	   const float total_time_delta = total_time - previous_total_time;
	   const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);
	   configWidget. cpuMonitor -> display (utilization);
	   previous_idle_time	= idle_time;
	   previous_total_time	= total_time;
	}
	timeDisplay	-> setText (currentTime.
	                            toString (QString ("dd.MM.yy hh:mm:ss")));
}

void	RadioInterface::handle_dumpButton	() {
SF_INFO *sf_info	= (SF_INFO *)alloca (sizeof (SF_INFO));

	if (myFMprocessor == nullptr)
	   return;

	if (sourceDumping) {
	   myFMprocessor	-> stopDumping ();
	   sf_close (dumpfilePointer);
	   sourceDumping = false;
	   configWidget.dumpButton	-> setText ("input dump");
	   return;
	}

	QString file = QFileDialog::getSaveFileName (this,
	                                             tr ("open file ..."),
	                                             QDir::homePath (),
	                                             tr ("Sound (*.wav)"));

	file = QDir::toNativeSeparators (file);
	if (!file. endsWith (".wav", Qt::CaseInsensitive))
	   file. append (".wav");
	sf_info		-> frames	= 0;
	sf_info		-> samplerate	= inputRate;
	sf_info		-> channels	= 2;
	sf_info		-> format	= SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	dumpfilePointer = sf_open (file. toUtf8 (). data (),
	                                   SFM_WRITE, sf_info);
	if (dumpfilePointer == nullptr) {
	   fprintf (stderr, "Openen mislukt %s\n", sf_strerror (dumpfilePointer));
	   qDebug() << "Cannot open " << file. toUtf8 (). data ();
	   return;
	}

	configWidget. dumpButton	-> setText ("WRITING");
	sourceDumping		= true;
	myFMprocessor		-> startDumping (dumpfilePointer);
}

#ifndef	__MINGW32__
void	RadioInterface::handle_audioDumpButton	() {
SF_INFO sf_info;
static QString audioTempFile;

	if (audioDumping) {
	   our_audioSink	-> stopDumping ();
	   sf_close (audiofilePointer);
	   audioDumping		= false;
	   configWidget. audioDumpButton	-> setText ("audioDump");
	   QString file		= QFileDialog::getSaveFileName (this,
	                                        tr ("open file .."),
	                                        QDir::homePath (),
	                                        tr ("Sound (*.wav)"));

	   file            = QDir::toNativeSeparators (file);
	   if (!file. endsWith (".wav", Qt::CaseInsensitive))
	      file.append (".wav");
	   QString cmdline = QString ("mv ") + audioTempFile + " "  + file;
	   system (cmdline. toUtf8 (). data ());
	   return;
	}

	audioTempFile = QDir::homePath () + "/tmp" +
	                               QString::number (getpid ());
	sf_info. samplerate = this -> audioRate;
	sf_info. channels   = 2;
	sf_info. format     = SF_FORMAT_WAV | SF_FORMAT_PCM_24;

	audiofilePointer	= sf_open (audioTempFile. toUtf8 (). data (),
	                                   SFM_WRITE, &sf_info);
	if (audiofilePointer == nullptr) {
	   qDebug() << "Cannot open " << audioTempFile. toUtf8 (). data ();
	   return;
	}

	configWidget. audioDumpButton -> setText ("WRITING");
	audioDumping		= true;
	our_audioSink		-> startDumping (audiofilePointer);
}

#else
//
//	windows version differs since it is (for me) to
//	complicated to run a "mv"  command in a shell
void	RadioInterface::handle_audioDumpButton	() {
SF_INFO sf_info;

	if (audioDumping) {
	   our_audioSink	-> stopDumping ();
	   sf_close (audiofilePointer);
	   audioDumping		= false;
	   configWidget. audioDumpButton	-> setText ("audioDump");
	   audiofilePointer	= nullptr;
	   return;
	}

	QString audioFile	= QFileDialog::getSaveFileName (this,
	                                        tr ("open file .."),
	                                        QDir::homePath (),
	                                        tr ("Sound (*.wav)"));

	audioFile		= QDir::toNativeSeparators (audioFile);
	if (!audioFile. endsWith (".wav", Qt::CaseInsensitive))
	   audioFile. append (".wav");
	sf_info. samplerate = this -> audioRate;
	sf_info. channels   = 2;
	sf_info. format     = SF_FORMAT_WAV | SF_FORMAT_PCM_24;
	audiofilePointer	= sf_open (audioFile. toUtf8 (). data (),
	                                   SFM_WRITE, &sf_info);
	if (audiofilePointer == nullptr) {
	   qDebug() << "Cannot open " << audioFile. toUtf8 (). data ();
	   return;
	}

	configWidget. audioDumpButton	-> setText ("WRITING");
	audioDumping		= true;
	our_audioSink		-> startDumping (audiofilePointer);
}
#endif
/*
 *      there is a tremendous amount of signal/slot connections
 *      The local connects, buttons, sliders and displays,
 *      are connected here.
 */
void    RadioInterface::localConnects () {
	connect (freqButton, SIGNAL (clicked ()),
		 this, SLOT (handle_freqButton ()));
	connect (&mykeyPad, SIGNAL (newFrequency (int)),
		 this, SLOT (newFrequency (int)));
	connect (configButton, SIGNAL (clicked ()),
		 this, SLOT (handle_configButton ()));
	connect (pauseButton, SIGNAL (clicked ()),
	         this, SLOT (handle_pauseButton ()));

	connect (squelchSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_squelchSelector);
	connect (squelchSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_squelchSlider (int)));
	connect (IQbalanceSlider, SIGNAL (valueChanged (int) ),
	              this, SLOT (setIQBalance (int) ));

	connect (fmModeSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmModeSelector);

	connect (fmChannelSelect,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmChannelSelector);

	connect (fmRdsSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmRdsSelector);
	connect (fmStereoPanoramaSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_fmStereoPanoramaSlider (int)));
	connect (fmStereoBalanceSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_fmStereoBalanceSlider (int)));

	connect (fmLFcutoff,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmLFcutoff);

	connect (plotSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_plotTypeSelector);

	connect (plotFactor,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_PlotZoomFactor);
//
//	and for the configuration widget:
	connect (configWidget. streamOutSelector, SIGNAL (activated (int)),
	         this, SLOT (handle_StreamOutSelector (int)));
	connect (configWidget. dumpButton, SIGNAL (clicked ()),
	         this, SLOT (handle_dumpButton ()));
	connect (configWidget. audioDumpButton, SIGNAL (clicked ()),
	         this, SLOT (handle_audioDumpButton ()));
	connect (configWidget. fc_plus, SIGNAL (clicked ()),
	         this, SLOT (handle_fc_plus ()));
	connect (configWidget. fc_minus, SIGNAL (clicked ()),
	         this, SLOT (handle_fc_min ()));
	connect (configWidget. f_plus, SIGNAL (clicked ()),
	         this, SLOT (handle_f_plus ()));
	connect (configWidget. f_minus, SIGNAL (clicked ()),
	         this, SLOT (handle_f_min ()));
	connect (configWidget. loggingButton,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_loggingButton);
	connect (configWidget. logSaving, SIGNAL (clicked ()),
	         this, SLOT (handle_logSavingButton ()));
	connect (configWidget. fmFilterSelect,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmFilterSelect);
	connect (configWidget. fmDecoderSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmDecoderSelector);
	connect (configWidget. fmDeemphasisSelector,
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 2)
                    qOverload<const QString &> (&QComboBox::textActivated),
#else 
                    qOverload<const QString &> (&QComboBox::activated),
#endif
	         this, &RadioInterface::handle_fmDeemphasis);

	connect (configWidget. cbThemes, &QComboBox::activated,
	         this, &RadioInterface::handle_cbThemes);
}

void	RadioInterface::handle_fmStereoPanoramaSlider (int n) {
	if (myFMprocessor != nullptr)
	   myFMprocessor -> setStereoPanorama (n);
}

void	RadioInterface::handle_fmStereoBalanceSlider (int n) {
	if (myFMprocessor != nullptr) {
	   myFMprocessor -> setSoundBalance (n);
	   balanceDisplay -> display (n);
	}
}

void	RadioInterface::handle_AudioGainSlider (int n) {
	if (myFMprocessor != nullptr) {
	   float gainDB = (n < -59 ? -99.9f : n / 2.0f);
	   myFMprocessor -> setVolume (gainDB);
// allow one fix digit after decimal point
	   audioGainDisplay -> display (QString ("%1").arg (gainDB, 0, 'f', 1));
	}
}

//	Deemphasis	= 50 usec (3183 Hz, Europe)
//	Deemphasis	= 75 usec (2122 Hz US)
void	RadioInterface::handle_fmDeemphasis (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;
	if (s == "Off (AM)") {
	   myFMprocessor -> setDeemphasis (1);
	}
	else {
	   myFMprocessor -> setDeemphasis (std::stol(s.toStdString()));
// toInt will not work with text after the number
	}
	fmSettings	-> setValue ("deemphasis", s);
}

void	RadioInterface::setCRCErrors (int n) {
	crcErrors	-> display (n);
}

void	RadioInterface::setSyncErrors (int n) {
	syncErrors	-> display (n);
}

void	RadioInterface::setbitErrorRate (double v) {
	bitErrorRate	-> display (v);
}

void	RadioInterface::setGroup (int n) {
	(void)n;
//	rdsGroupDisplay	-> display (n);
}

void	RadioInterface::setPTYCode (int n, const QString &ptyText) {
	(void)n;
	pty_text -> setText (ptyText);
}

void	RadioInterface::setAFDisplay (int n1, int n2) {
//	reset constellation (n1 and n2 are set together to zero)
	if ((n1 == 0) && (n2 == 0)) 
	   rdsAF2Display -> display (n2);
	else  //if (n2 > 0)
	   rdsAF2Display -> display (n2 > 0 ? n2 : n1);
}

void	RadioInterface::setPiCode (int n) {
int32_t t = currentFreq;

	if ((frequencyforPICode != t) || (n != 0)) {
	   currentPIcode      = n;
	   frequencyforPICode = t;
	}

	rdsPiDisplay	-> display (n);
}

void	RadioInterface::setStationLabel (const QString &s) {
	stationLabelTextBox	-> setText (s);
}

void	RadioInterface::setMusicSpeechFlag (int n) {
	speechLabel ->
	       setText (n != 0 ? QString ("Music") : QString ("Speech"));
}

void	RadioInterface::clearMusicSpeechFlag () {
	speechLabel	-> setText (QString (""));
}

void	RadioInterface::clearRadioText () {
	radioTextBox    -> setText (QString (""));
}

void	RadioInterface::setRadioText (const QString &s) {
	radioTextBox	-> setText (s);
}

void	RadioInterface::setRDSisSynchronized (bool syn) {
	if (!syn)
	   rdsSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	else
	   rdsSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
}

void	RadioInterface::handle_fmModeSelector (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;
	if (s == "Stereo") {
	   myFMprocessor -> setfmMode (fmProcessor::FM_Mode::Stereo);
	   fmStereoPanoramaSlider -> setEnabled (false);
	}
	else
	if (s == "Stereo (Pano)") {
	   myFMprocessor -> setfmMode (fmProcessor::FM_Mode::StereoPano);
	   fmStereoPanoramaSlider -> setEnabled (true);
	}
	else
	if (s == "Mono") {
	    myFMprocessor -> setfmMode (fmProcessor::FM_Mode::Mono);
	   fmStereoPanoramaSlider -> setEnabled (false);
	}
	else
	   Q_ASSERT(0);
}

void	RadioInterface::handle_plotTypeSelector (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;

	if (s == "OFF")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::OFF);
	else
	if (s == "IF Filtered")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::IF_FILTERED);
	else
	if (s == "Demodulator")
	    myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::DEMODULATOR);
	else
	if (s == "AF SUM")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::AF_SUM);
	else
	if (s == "AF DIFF")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::AF_DIFF);
	else
	if (s == "AF MONO Filtered")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::AF_MONO_FILTERED);
	else
	if (s == "AF LEFT Filtered")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::AF_LEFT_FILTERED);
	else
	if (s == "AF RIGHT Filtered")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::AF_RIGHT_FILTERED);
	else
	if (s == "RDS Input")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::RDS_INPUT);
	else
	if (s == "RDS Demod")
	   myFMprocessor -> setlfPlotType (fmProcessor::ELfPlot::RDS_DEMOD);
	else
	   Q_ASSERT(0);
}

void	RadioInterface::handle_cbThemes (int idx) {
const int idxCur = themeChooser -> get_curr_style_sheet_idx ();
	themeChooser -> set_curr_style_sheet_idx (idx);

// restart program if theme changed
	if (idx != idxCur) {
	   TerminateProcess ();
	   qApp -> exit (PROGRAM_RESTART_EXIT_CODE);
	}
}

void	RadioInterface::handle_PlotZoomFactor (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;
	myFMprocessor -> setlfPlotZoomFactor (std::stol (s.toStdString()));
}

void	RadioInterface::handle_fmRdsSelector (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;

	rdsModus = (s == "RDS 1" ?
	            rdsDecoder::ERdsMode::RDS_1:
	         s == "RDS 2" ?
	            rdsDecoder::ERdsMode::RDS_2:
	         s == "RDS 3" ?
	            rdsDecoder::ERdsMode::RDS_3:
	            rdsDecoder::ERdsMode::RDS_OFF);

	myFMprocessor	-> setfmRdsSelector (rdsModus);
	myFMprocessor	-> resetRds ();
}

void	RadioInterface::handle_fmDecoderSelector (const QString &decoder) {
	theDemodulator. setDecoder (decoder);
}

void	RadioInterface::handle_fmLFcutoff (const QString &s) {

	if (myFMprocessor == nullptr)
	   return;

	if (s == "Off")
	    myFMprocessor -> setlfcutoff (-1);
	else
	   myFMprocessor -> setlfcutoff (std::stol (s.toStdString ()));
}

void	RadioInterface::displayFrequency	(int32_t freq) {
	// do rounding as AFC can cause a tiny drift in frequency
	lcd_Frequency	-> display ((int)(freq + Hz (500)) / KHz (1));
}

void	RadioInterface::handle_fmFilterSelect (const QString &s) {
	if (myFMprocessor == nullptr)
	   return;

	myFMprocessor -> setBandwidth (s);
	fmSettings	-> setValue ("fmFilterSelect", s);
}

//
void	RadioInterface::showPeakLevel (const float iPeakLeft,
	                               const float iPeakRight) {
	auto peak_avr = [](float iPeak, float & ioPeakAvr) -> void {
	   ioPeakAvr = (iPeak > ioPeakAvr ? iPeak : ioPeakAvr - 0.5f /*decay*/);
	};

	peak_avr (iPeakLeft,  peakLeftDamped);
	peak_avr (iPeakRight, peakRightDamped);

	thermoPeakLevelLeft	-> setValue (peakLeftDamped);
	thermoPeakLevelRight	-> setValue (peakRightDamped);

//	simple overflow avoidance ->
//	reduce volume slider about -0.5dB (one step)
	if (iPeakLeft > 0.0f || iPeakRight > 0.0f)
	   volumeSlider -> setValue (volumeSlider -> value () - 1);
}
//
//	This signal will arrive once every half "inputRate" samples (500ms)
void	RadioInterface::showMetaData (const fmProcessor::SMetaData *ipMD) {
static std::array<char, 1024> s{};
static int teller = 0;
bool triggerLog = false;

	if (runMode. load () != ERunStates::RUNNING)
	   return;

	//static const float w = 1.0f / std::log10(2.0f);

	if ((logTime > 0) && (++teller == logTime)) {
	   triggerLog = true;
	   teller = 0;
	}

	if (ipMD -> PilotPllLocked) {
	   pll_isLocked -> setStyleSheet ("QLabel {background-color:green} QLabel {color:white}");
	   pll_isLocked -> setText("Pilot PLL Locked");
	}
	else {
	   pll_isLocked -> setStyleSheet ("QLabel {background-color:red} QLabel {color:white}");
	   pll_isLocked -> setText("Pilot PLL Unlocked");
	}

	rf_dc_component -> display (QString ("%1").arg(ipMD -> DcValRf, 0, 'f', 2));
	demod_dc_component -> display (QString("%1").arg(ipMD -> DcValIf, 0, 'f', 2));

	//thermoDcComponent -> setValue ((ipMD -> DcValIf < 0.0f ? 1 : -1) * w * std::log10(std::abs(ipMD -> DcValIf) + 1.0f));
	thermoDcComponent -> setValue (-ipMD -> DcValIf);

	switch (ipMD -> PssState) {
	case fmProcessor::SMetaData::EPssState::OFF:
	   pss_state -> setStyleSheet ("QLabel {background-color:grey} QLabel {color:black}");
	   pss_state -> setText("PSS off");
	   break;
	case fmProcessor::SMetaData::EPssState::ANALYZING:
	   pss_state -> setStyleSheet ("QLabel {background-color:yellow} QLabel {color:black}");
	   pss_state -> setText ("PSS analyzing ... ");
	   break;
	case fmProcessor::SMetaData::EPssState::ESTABLISHED:
	   pss_state -> setStyleSheet ("QLabel {background-color:green} QLabel {color:white}");
	   pss_state -> setText("PSS established");
	   break;
	}

	pss_phase_corr -> display (QString("%1").arg(ipMD -> PssPhaseShiftDegree, 0, 'f', 2));
	pss_phase_error -> display (QString("%1").arg(ipMD -> PssPhaseChange, 0, 'f', 2));

//	thermoPSSCorr -> setValue ((ipMD -> PssPhaseChange < 0.0f ? -1 : 1) * w * std::log10(std::abs(ipMD -> PssPhaseChange) + 1.0f));
	thermoPSSCorr -> setValue (ipMD -> PssPhaseChange);

//	some kind of AFC
	if (afcActive) {
	   int32_t afcOffFreq = ipMD -> DcValIf * 10000;
// the_dcComponent is positive with too little frequency
	   afcCurrOffFreq = (1 - afcAlpha) * afcCurrOffFreq +
	                                           afcAlpha * afcOffFreq;

	   float absAfcCurrOffFreq = abs (afcCurrOffFreq);

	   afcAlpha	= absAfcCurrOffFreq < 10 ? 0.005f :
	                  absAfcCurrOffFreq < 100 ? 0.050f : 0.800f;

	   uint32_t newFreq = currentFreq + afcCurrOffFreq;

	   if (triggerLog) {
	      fprintf (stderr, "AFC:  DC %f, NewFreq %d = CurrFreq %d + AfcOffFreq %d (unfiltered %d), AFC_Alpha %f\n",
			       ipMD-> DcValIf, newFreq,
	                       currentFreq, afcCurrOffFreq,
	                       afcOffFreq, afcAlpha);
	   }

//	avoid re-tunings of HW when only a residual frequency offset remains
	   if (absAfcCurrOffFreq > 3)
	      currentFreq = setTuner (newFreq);
	}

	if (triggerLog) {
	   QDateTime currentTime = QDateTime::currentDateTime ();
	   sprintf (s.begin(),
	            "%s : Freq = %d,\n PI code = %4X, pilot = %f\n",
	            currentTime.toString(QString("dd.MM.yy hh:mm:ss"))
	                                      .toStdString() .c_str(),
		        currentFreq, currentPIcode, ipMD -> DcValRf);

	   fputs (s. cbegin(), stderr);
//	and into the logfile
	   if (logFile != nullptr)
	      fputs (s.cbegin (), logFile);
	}
}

void	RadioInterface::handle_loggingButton (const QString &s) {

	if (s == "log off")
	   logTime	= 0;
	else
	if (s == "log 1 sec")
	   logTime = 1;
	else
	if (s == "log 2 sec")
	   logTime = 2;
	else
	if (s == "log 3 sec")
	   logTime = 3;
	else
	if (s == "log 4 sec")
	   logTime = 4;
	else
	   logTime = 5;
}

void	RadioInterface::handle_logSavingButton () {

	if (logFile != nullptr) { // just stop it
	   fclose (logFile);
	   logFile		= nullptr;
	   configWidget. logSaving	-> setText ("save");
	   return;
	}
	else {
	   QString file = QFileDialog::getSaveFileName (this,
	                                                tr ("open file .."),
	                                                QDir::homePath (),
	                                                tr ("text (*.txt)"));
	   logFile = fopen (file. toUtf8 (). data (), "w");
	   if (logFile == nullptr) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                 tr ("/dev/null is used"));
	   }
	   else {   // logFile != nullptr
	      configWidget. logSaving		-> setText ("halt");
	      fprintf (logFile, "\nlogging starting\n\n\n");
	   }
	}
}
//
void	RadioInterface::handle_pauseButton () {
	if (runMode. load () == ERunStates::IDLE)
	   return;

	if (runMode. load () == ERunStates::RUNNING) {
	   if (autoIncrementTimer. isActive ())
	      autoIncrementTimer. stop ();

	   theDevice		-> stopReader	();
	   our_audioSink	-> stop		();
	   pauseButton -> setText (QString ("Continue"));
	   runMode = ERunStates::PAUSED;
	}
	else
	if (runMode. load () == ERunStates::PAUSED) {
	   if (incrementIndex != 0)	// restart the incrementtimer if needed
	      autoIncrementTimer. start (incrementInterval (incrementIndex));
	   theDevice		-> restartReader ();
	   our_audioSink	-> restart ();
	   pauseButton		-> setText (QString ("Pause"));
	   runMode. store (ERunStates::RUNNING);
	}
}
//
//	do not forget that ocnt starts with 1, due
//	to Qt list conventions
bool	RadioInterface::setupSoundOut (QComboBox *streamOutSelector,
	                               audioSink *our_audioSink,
	                               int32_t cardRate,
	                               std::vector<int16_t> &table) {
uint16_t ocnt = 1;

	for (int i = 0; i < our_audioSink -> numberofDevices (); i++) {
	  QString channelName =
	             our_audioSink -> outputChannelwithRate (i, cardRate);
	   qDebug ("Investigating Device %d\n", i);

	   if (channelName == "") 
	      continue;

	   streamOutSelector -> insertItem (ocnt, channelName, QVariant(i));
	   table [ocnt] = i;
	   qDebug (" (output):item %d wordt stream %d\n", ocnt, i);
	   ocnt++;
	}

	qDebug() << "added items to combobox";
	return ocnt > 1;
}

void	RadioInterface::handle_StreamOutSelector (int idx) {
	if (idx == 0)
	   return;

	outputDevice = outTable [idx];
	if (!our_audioSink -> isValidDevice (outputDevice))
	   return;

	our_audioSink	-> stop ();
	if (!our_audioSink -> selectDevice (outputDevice)) {
	   QMessageBox::warning (this, tr("sdr"),
	                               tr ("Selecting  output stream failed\n"));
	   return;
	}

	qWarning () << "selected output device " << idx << outputDevice;
	our_audioSink	-> restart ();
}

void	RadioInterface::handle_squelchSlider (int n) {
	if (myFMprocessor != nullptr)
	   myFMprocessor -> set_squelchValue (n);
}
//
void	RadioInterface::hfBufferLoaded () {
int32_t vfoFrequency;
std::complex<float> tempBuffer [displaySize];
	if (runMode. load () != ERunStates::RUNNING) 
	   return;

	vfoFrequency	= theDevice -> getVFOFrequency ();
	hfScope		-> setZero (vfoFrequency);
	hfScope		-> setNeedle (loFrequency);
	hfScope		-> setAmplification (
	                   spectrumAmplitudeSlider_hf -> value ());
	while (hfBuffer. GetRingBufferReadAvailable () > displaySize) {
	   hfBuffer. getDataFromBuffer (tempBuffer, displaySize);
	   hfScope -> addElements (tempBuffer, displaySize);
	}
}

void	RadioInterface::lfBufferLoaded (bool showFull,
	                                bool refresh,
	                                int	zoomFactor) {
std::vector<std::complex<float>> v (spectrumSize);
	if (runMode. load () != ERunStates::RUNNING) 
	   return;
	while (lfBuffer. GetRingBufferReadAvailable () > spectrumSize) {
	   lfBuffer.  getDataFromBuffer (v. data (), spectrumSize);
	   lfScope -> processLFSpectrum (v, 
	                                 zoomFactor,
	                                 showFull,
	                                 refresh,
	                                 spectrumAmplitudeSlider_lf -> value ());
	}
}

void	RadioInterface::iqBufferLoaded () {
std::complex<float> iq_values [IQ_SCOPE_SIZE];
int32_t sizeRead = iqBuffer. getDataFromBuffer (iq_values, IQ_SCOPE_SIZE);
	iqScope -> DisplayIQVec (iq_values, sizeRead, 1.0f);
}

void	RadioInterface::setHFplotterView (int offset) {
	(void)offset;
	if (hfScope ->  currentMode () == WATERFALL_MODE)
	   hfScope -> SelectView (SPECTRUM_MODE);
	else
	   hfScope -> SelectView (WATERFALL_MODE);
}

void	RadioInterface::setup_HFScope () {
	hfScope		= new hs_scope (hfscope,
	                                this -> displaySize,
	                                1,
	                                this -> rasterSize,
	                                2304000,
	                                8);
	HFviewMode	= SPECTRUM_MODE;
	hfScope		-> SelectView (SPECTRUM_MODE);
	connect (hfScope, SIGNAL (clickedwithLeft (int)),
	         this, SLOT (adjustFrequency (int)));
	connect (hfScope, SIGNAL (clickedwithRight (int)),
	         this, SLOT (setHFplotterView (int)));
}

void	RadioInterface::setup_LFScope () {
	lfScope		= new ls_scope (lfscope,
	                                8,
//	                                this	-> averageCount,
	                                this	-> displaySize,
	                                this	-> spectrumSize,
	                                this	-> fmRate);
	lfScope		-> SelectView (SPECTRUM_MODE);
}

void	RadioInterface::setup_IQPlot () {
	iqScope		= new IQDisplay (iqscope, IQ_SCOPE_SIZE);
}

//
void	RadioInterface::handle_squelchSelector (const QString& s) {
	if (myFMprocessor == nullptr)
	   return;

	fmProcessor::ESqMode sqMode = fmProcessor::ESqMode::OFF;

	if (s == "SQ OFF")
	   sqMode = fmProcessor::ESqMode::OFF;
	else
	if (s == "NSQ")
	   sqMode = fmProcessor::ESqMode::NSQ;
	else
	if (s == "LSQ")
	   sqMode = fmProcessor::ESqMode::LSQ;
	else
	   Q_ASSERT(0);

	squelchMode = (sqMode != fmProcessor::ESqMode::OFF);
	squelchSlider	-> setEnabled (squelchMode);
	myFMprocessor	-> set_squelchMode (sqMode);
	
	setSquelchIsActive (myFMprocessor -> getSquelchState ());
}

void	RadioInterface::setSquelchIsActive (bool active) {
	if (squelchMode) {
	   if (active)
	      sqlStatusLabel -> setStyleSheet ("QLabel {background-color:red}");
	   else
	      sqlStatusLabel -> setStyleSheet ("QLabel {background-color:green}");
	}
	else
	   sqlStatusLabel -> setStyleSheet ("QLabel {background-color:gray}");
}

void	RadioInterface::handle_sbDispDelay (int n) {
	if (myFMprocessor != nullptr)
	   myFMprocessor -> setDispDelay (n);
}

//	Just a simple routine to set the sliders and boxes
//	to their previous values
void	RadioInterface::restoreGUIsettings (QSettings *s) {
QString h;
int     k;

	h	= s -> value ("deemphasis", "Off"). toString ();
	k	= configWidget. fmDeemphasisSelector -> findText (h);
	if (k != -1)
	   configWidget. fmDeemphasisSelector -> setCurrentIndex (k);


	k	= s -> value ("afc", Qt::CheckState::Unchecked).toInt ();
	cbAfc	-> setCheckState (k ? Qt::CheckState::Checked :
	                                 Qt::CheckState::Unchecked);
	afcActive = (k != 0);

	k	= s -> value ("spectrumAmplitudeSlider_hf",
	                      spectrumAmplitudeSlider_hf -> value ()).toInt ();
	spectrumAmplitudeSlider_hf -> setValue (k);

	k	= s -> value ("spectrumAmplitudeSlider_lf",
	                      spectrumAmplitudeSlider_lf -> value ()). toInt ();
	spectrumAmplitudeSlider_lf -> setValue (k);

	k	= s -> value ("fmStereoPanoramaSlider", 100). toInt ();
	fmStereoPanoramaSlider	-> setValue (k);

	k	= s -> value ("fmStereoBalanceSlider",
	                       fmStereoBalanceSlider -> value ()). toInt ();
	fmStereoBalanceSlider	-> setValue (k);

	k	= s -> value ("squelchSlider",
	                      squelchSlider -> value ()). toInt ();
	squelchSlider		-> setValue (k);

	h	= s -> value ("fmFilterSelect", "165kHz"). toString ();
	k	= configWidget. fmFilterSelect -> findText (h);
	if (k != -1)
	   configWidget. fmFilterSelect -> setCurrentIndex (k);

	h	= s -> value ("fmMode",
	                      fmModeSelector -> currentText ()). toString ();
	k	= fmModeSelector -> findText (h);
	if (k != -1)
	   fmModeSelector -> setCurrentIndex(k);

	h	= s -> value ("fmRdsSelector",
	                      fmRdsSelector -> currentText ()). toString ();
	k = fmRdsSelector->findText(h);
	if (k != -1)
	   fmRdsSelector -> setCurrentIndex (k);

//	the FM decoder combobox is not filled yet
//	h	= s -> value ("fmDecoder",
//		              fmDecoder -> currentText ()). toString ();
//	k	= fmDecoder -> findText (h);
//	if (k != -1)
//	   fmDecoder -> setCurrentIndex (k);

	h	= s -> value ("fmChannelSelect",
	                      fmChannelSelect -> currentText ()). toString ();
	k	= fmChannelSelect -> findText (h);
	if (k != -1)
	   fmChannelSelect -> setCurrentIndex (k);

	h	= s -> value ("fmDeemphasisSelector",
	                             "50us  (Europe, non-USA)"). toString ();
	k	= configWidget. fmDeemphasisSelector -> findText(h);
	if (k != -1)
	   configWidget. fmDeemphasisSelector -> setCurrentIndex (k);

	h	= s -> value ("fmLFcutoff", "15000Hz"). toString ();
	k	= fmLFcutoff -> findText (h);
	if (k != -1)
	   fmLFcutoff -> setCurrentIndex (k);

	k	= s -> value ("peakLevelDelaySteps", 20). toInt ();
	configWidget. sbDispDelay -> setValue (k);
}

//	For different input rates we select different rates for the
//	fm decoding (the fmrate). Decimating from inputRate to fmRate
//	is always integer. Decimating from fmRate to audioRate maybe
//	fractional which costs a lot of power.
//	NOT USED IN THIS VERSION
//int32_t	RadioInterface::mapRates (int32_t inputRate) {
//int32_t res;
//
//	res	= inputRate % 256000 == 0 ? 256000 :
//	          inputRate % 192000 == 0 ? 192000 :
//	          inputRate < Khz(400) ? inputRate :
//	          inputRate < Khz(850) ? inputRate / 4 :
//	          inputRate < Khz(1300) ? inputRate / 6 :
//	          inputRate < Khz(1900) ? inputRate / 8 :
//	          inputRate < Khz(3000) ? inputRate / 10 :
//	          inputRate < Khz(4000) ? inputRate / 15 :
//	          inputRate < Khz(5000) ? inputRate / 20 :
//	          inputRate < Khz(6000) ? inputRate / 25 :
//	          inputRate / 30;
//	return res;
//}

void	RadioInterface::handle_freqButton () {
	if (mykeyPad. isVisible ())
	   mykeyPad. hidePad ();
	else
	   mykeyPad. showPad ();
}

void	RadioInterface::newFrequency	(int f) {
	stopIncrementing ();
	currentFreq = setTuner (f);
}

void	RadioInterface::handle_freqSaveButton	() {
	myLine. show ();
	connect (&myLine, SIGNAL (returnPressed ()),
	         this, SLOT (handle_myLine ()));
}

void	RadioInterface::handle_myLine () {
int32_t freq        = theDevice -> getVFOFrequency () + loFrequency;
QString programName	= myLine . text ();

	myLine. setText (""); // next "show" should be empty text
	disconnect (&myLine, SIGNAL (returnPressed ()),
	            this, SLOT (handle_myLine ()));
	myLine. hide ();

	fprintf (stderr, "adding %s %s\n",
	                 programName. toUtf8 (). data (),
	                 QString::number ((freq + 500) / Khz(1)). toUtf8 ().data());
	myProgramList	-> addRow (programName, QString::number ((freq + Hz (500)) / Khz(1)));
}

void	RadioInterface::reset_afc () {
	afcAlpha = 1.0f;
	afcCurrOffFreq = 0;
}

#include <QCloseEvent>
void	RadioInterface::closeEvent (QCloseEvent *event) {
	bool weCloseDirect = configWidget. closeDirect -> isChecked () ? 1 : 0;
	fmSettings -> setValue ("closeDirect", weCloseDirect ? 1 : 0);
	if (weCloseDirect != 0) {
	   TerminateProcess ();
	   event	-> accept ();
	   return;
	}
	   
	QMessageBox::StandardButton resultButton =
	        QMessageBox::question (this, "Quitting fmreceiver?",
	                               tr ("Are you sure?\n"),
	                               QMessageBox::No | QMessageBox::Yes,
	                                               QMessageBox::Yes);

	if (resultButton != QMessageBox::Yes) {
	   event -> ignore ();
	}
	else {
	   TerminateProcess ();
	   event -> accept();
	}
}

void	RadioInterface::handle_afcSelector	(int b) {
	(void)b;
	afcActive	= cbAfc -> isChecked ();
	fprintf (stderr, "afcActive = %s\n", afcActive ? "true" : "false");
	reset_afc ();
}

void	RadioInterface::handle_countrySelector	(const QString &country) {
int ptyLocale	= country == "Europe" ? 0 : 1;
	fmSettings -> setValue ("ptyLocale", country);
	if (myFMprocessor != nullptr)
	   myFMprocessor -> set_ptyLocale (ptyLocale);
}

void	RadioInterface::handle_configButton	() {
	if (configDisplay. isHidden ())
	   configDisplay. show ();
	else
	   configDisplay. hide ();
}

void	RadioInterface::handle_cbTestTone (int v) {
bool b	= configWidget. cbTestTone -> isChecked ();
	myFMprocessor -> setTestTone (b);
	(void)v;
}

