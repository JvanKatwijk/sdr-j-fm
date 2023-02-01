#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdr-j-fm
 *
 *    sdr-j-fm is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdr-j-fm is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Main program
 */
#include	<QApplication>
#include	<QSettings>
#include	<QDir>
#include	<unistd.h>
#include	"fm-constants.h"
#include	"radio.h"
#include	"themechoser.h"

#define	DEFAULT_INI	".jsdr-fm.ini"
#define	STATION_LIST	".jsdr-fm-stations.bin"

int	main (int argc, char **argv) {
int32_t		opt;
/*
 *	The default values
 */
QSettings	*ISettings;		/* .ini file	*/
int32_t	outputRate	= 48000;
RadioInterface		*myRadioInterface;
QString iniFile		= QDir::homePath ();
QString stationList     = QDir::homePath ();

ThemeChoser	themeChooser;

        iniFile. append ("/");
        iniFile. append (DEFAULT_INI);
        iniFile = QDir::toNativeSeparators (iniFile);

        stationList. append ("/");
        stationList. append (STATION_LIST);
        stationList = QDir::toNativeSeparators (stationList);

	while ((opt = getopt (argc, argv, "m")) != -1) {
	   switch (opt) {
	      case 'm': outputRate = 192000;
	         break;

	      default:
	                break;
	      }
	}

/*
 *	... and the settings of the "environment"
 */
	ISettings	= new QSettings (iniFile, QSettings::IniFormat);
/*
 *	Before we connect control to the gui, we have to
 *	instantiate
 */
#if QT_VERSION >= 0x050600
        QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);
#endif

	QString styleSheetText = ISettings -> value ("styleSheet", ""). toString();
	int styleSheet		= themeChooser.
	                      get_idx_of_sheet_name (styleSheetText. toStdString(). c_str() );

	themeChooser. set_curr_style_sheet_idx (styleSheet);

	int exitCode = 0;

	do {
	   QApplication a (argc, argv);
	   a. setStyleSheet (themeChooser. get_curr_style_sheet_string ());
	   myRadioInterface = new RadioInterface (ISettings,
		                                  stationList,
	                                          &themeChooser,
	                                          outputRate);
	   myRadioInterface -> show ();

	   a.setWindowIcon (QIcon (":fm-icon.ico"));
	   exitCode = a. exec ();
//#ifdef	__MINGW32__
	   delete myRadioInterface;
	   myRadioInterface	= nullptr;
//#endif
	} while (exitCode == PROGRAM_RESTART_EXIT_CODE );

	fprintf (stderr, "Terug van de exec\n");
/*
 *	done:
 */
	fflush (stdout);
	fflush (stderr);
	if (myRadioInterface != nullptr)
	   delete myRadioInterface;
	qDebug ("It is done\n");
	ISettings	-> sync ();
	ISettings	-> ~QSettings ();
	return exitCode;
}

