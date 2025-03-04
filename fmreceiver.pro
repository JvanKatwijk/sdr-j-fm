######################################################################
# Automatically generated by qmake (2.01a) Tue Oct 6 19:48:14 2009
# but modified by me to accomodate for the includes for qwt and
# portaudio
######################################################################

TEMPLATE	= app 
TARGET		= fmreceiver-3.20
QT		+= widgets xml
#CONFIG		+= console
CONFIG		-= console
QMAKE_CXXFLAGS  += -std=c++17
#
# Note that the fastmath option eliminates the isinf and isnan
# functions that are used to validate the data
QMAKE_CFLAGS	+= -flto 
QMAKE_CXXFLAGS	+= -flto 
QMAKE_LFLAGS	+= -flto
#QMAKE_CXXFLAGS	+= -g
#QMAKE_CFLAGS	+= -g
#QMAKE_LFLAGS	+= -g
#QMAKE_CXXFLAGS	+= -g -fsanitize=address
#QMAKE_CFLAGS	+= -g -fsanitize=address
#QMAKE_LFLAGS	+= -g -fsanitize=address
QMAKE_CXXFLAGS	+= -isystem $$[QT_INSTALL_HEADERS]
RC_ICONS        =  fm-icon.ico
RESOURCES       += resources.qrc

TRANSLATIONS = i18n/de_DE.ts i18n/it_IT.ts i18n/hu_HU.ts

DEPENDPATH += . \
	      ..\
	      ../.. \
	      ../../../ \
	      ./src \
	      ./includes \
	      ./includes/output \
	      ./includes/various \
	      ./includes/fm \
	      ./includes/rds \
	      ./includes/scopes-qwt6 \
	      ./src \
	      ./src/output \
	      ./src/various \
	      ./src/fm \
	      ./src/rds \
	      ./src/scopes-qwt6 \
	      ./devices \
	      ./devices/filereader

INCLUDEPATH += . \
	      ..\
	      ../.. \
	      ./includes \
	      ./includes/output \
	      ./includes/various \
	      ./includes/fm \
	      ./includes/rds \
	      ./includes/scopes-qwt6 \
	      ./src \
	      ./src/output \
	      ./src/various \
	      ./src/fm \
	      ./src/rds \
	      ./src/scopes-qwt6 \
	      ./devices \
	      ./devices/filereader

# Input
HEADERS += ./radio.h \
	   ./includes/popup-keypad.h \
	   ./includes/fm-constants.h \
	   ./includes/various/program-list.h \
	   ./includes/various/keyboardfilter.h \
	   ./includes/various/squelchClass.h \
	   ./includes/various/oscillator.h \
           ./includes/various/ringbuffer.h \
	   ./includes/various/pllC.h \
	   ./includes/various/sincos.h \
	   ./includes/various/newconverter.h \
	   ./includes/various/fft-complex.h \
	   ./includes/various/fft-filters.h \
	   ./includes/various/fir-filters.h \
	   ./includes/various/iir-filters.h \
	   ./includes/various/agc.h  \
	   ./includes/various/costas.h  \
	   ./includes/various/shaping_filter.h  \
	   ./includes/various/Xtan2.h \
	   ./includes/various/themechoser.h \
	   ./includes/output/audiosink.h \
	   ./includes/scopes-qwt6/iqdisplay.h \
	   ./includes/scopes-qwt6/scope.h \
           ./includes/scopes-qwt6/spectrogramdata.h \
	   ./includes/scopes-qwt6/hs-scope.h \
	   ./includes/scopes-qwt6/ls-scope.h \
	   ./includes/fm/fm-demodulator.h \
	   ./includes/fm/pilot-recover.h \
	   ./includes/fm/stereo-separation.h \
	   ./includes/fm/fm-processor.h \
	   ./includes/rds/rds-decoder.h \
	   ./includes/rds/rds-decoder-1.h \
	   ./includes/rds/rds-decoder-2.h \
	   ./includes/rds/rds-decoder-3.h \
	   ./includes/rds/rds-blocksynchronizer.h \
	   ./includes/rds/rds-group.h \
	   ./includes/rds/rds-groupdecoder.h  \
	   ./devices/device-handler.h \
	   ./devices/deviceselect.h \
	   ./devices/filereader/filereader.h \
	   ./devices/filereader/filehulp.h

FORMS +=   ./forms/radio.ui \
	   ./forms/configwidget.ui \
	   ./devices/filereader/filereader-widget.ui

SOURCES += ./main.cpp \
	   ./radio.cpp \
	   ./src/popup-keypad.cpp \
	   ./src/various/program-list.cpp \
	   ./src/various/keyboardfilter.cpp \
	   ./src/various/squelchClass.cpp \
	   ./src/various/pllC.cpp \
	   ./src/various/oscillator.cpp \
	   ./src/various/sincos.cpp \
	   ./src/various/newconverter.cpp \
	   ./src/various/fft-complex.cpp \
	   ./src/various/fft-filters.cpp \
	   ./src/various/fir-filters.cpp \
	   ./src/various/iir-filters.cpp \
	   ./src/various/Xtan2.cpp \
	   ./src/various/themechoser.cpp \
	   ./src/various/shaping_filter.cpp \
	   ./src/output/audiosink.cpp \
	   ./src/scopes-qwt6/iqdisplay.cpp \
	   ./src/scopes-qwt6/scope.cpp \
	   ./src/scopes-qwt6/hs-scope.cpp \
	   ./src/scopes-qwt6/ls-scope.cpp \
	   ./src/fm/fm-demodulator.cpp \
	   ./src/fm/pilot-recover.cpp \
	   ./src/fm/stereo-separation.cpp \
	   ./src/fm/fm-processor.cpp \
	   ./src/rds/rds-decoder.cpp \
	   ./src/rds/rds-decoder-1.cpp \
	   ./src/rds/rds-decoder-2.cpp \
	   ./src/rds/rds-decoder-3.cpp \
	   ./src/rds/rds-blocksynchronizer.cpp \
	   ./src/rds/rds-group.cpp \
	   ./src/rds/rds-groupdecoder.cpp \
	   ./devices/device-handler.cpp \
	   ./devices/deviceselect.cpp \
	   ./devices/filereader/filereader.cpp \
	   ./devices/filereader/filehulp.cpp
#
# for windows32 we use:
win32 {
DESTDIR	= /usr/shared/w32-programs/windows-fmreceiver
exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}

#CONFIG	-= console
#CONFIG	+= extio
CONFIG	+= dabstick
CONFIG	+= sdrplay
CONFIG	+= sdrplay-v3
CONFIG	+= airspy
CONFIG	+= hackrf
CONFIG	+= lime
CONFIG	+= pluto
# includes in mingw differ from the includes in fedora linux
LIBS            += -L/usr/i686-w64-mingw32/sys-root/mingw/lib
INCLUDEPATH 	+= /usr/i686-w64-mingw32/sys-root/mingw/include
INCLUDEPATH 	+= /usr/i686-w64-mingw32/sys-root/mingw/include/qt5/qwt
LIBS	+= -lportaudio
LIBS	+= -lqwt-qt5
#LIBS	+= -lsndfile
#LIBS	+= -lsamplerate
LIBS            += /usr/i686-w64-mingw32/sys-root/mingw/bin/libsndfile-1.dll
LIBS            += /usr/i686-w64-mingw32/sys-root/mingw/bin/libsamplerate-0.dll
LIBS	+= -lole32
LIBS	+= -lwinmm
LIBS 	+= -lstdc++
LIBS	+= -lusb-1.0
LIBS	+= -lwinpthread
}
#
#for fedora and ubuntu  we use
unix { 
DESTDIR		= ./linux-bin
exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}
CONFIG		+= sdrplay
CONFIG		+= sdrplay-v3
CONFIG		+= airspy
CONFIG		+= dabstick
CONFIG		+= hackrf
CONFIG		+= lime
CONFIG		+= pluto
CONFIG		+= qwt
#INCLUDEPATH 	+= /usr/include/qt5/qwt
#INCLUDEPATH 	+= /usr/local/qwt-6.2.0/include
#for ubuntu the first line
#LIBS +=  -lqwt-qt5 -lusb-1.0 -lrt -lportaudio -lsndfile -ldl
#for fedora 21
LIBS += -lusb-1.0 -lrt -lportaudio -lsndfile -ldl
LIBS += -lsamplerate
#QMAKE_CXXFLAGS += -Wno-hicpp-signed-bitwise
}

#	the devices
dabstick {
	DEFINES		+= HAVE_DABSTICK
	FORMS		+= ./devices/rtlsdr-handler/dabstick-widget.ui
	INCLUDEPATH	+= ./devices/rtlsdr-handler
	DEPENDPATH	+= ./devices/rtlsdr-handler
	HEADERS		+= ./devices/rtlsdr-handler/rtlsdr-handler.h \
	                   ./devices/rtlsdr-handler/dongleselect.h
	SOURCES		+= ./devices/rtlsdr-handler/rtlsdr-handler.cpp \
	                   ./devices/rtlsdr-handler/dongleselect.cpp
}
#
#	the SDRplay
#
sdrplay {
	DEFINES		+= HAVE_SDRPLAY
	FORMS		+= ./devices/sdrplay-handler/sdrplay-widget.ui
	INCLUDEPATH	+= ./devices/sdrplay-handler
	HEADERS		+= ./devices/sdrplay-handler/sdrplay-handler.h \
	                   ./devices/sdrplay-handler/sdrplayselect.h
	SOURCES		+= ./devices/sdrplay-handler/sdrplay-handler.cpp \
	                   ./devices/sdrplay-handler/sdrplayselect.cpp
}
#	the SDRplay
#
sdrplay-v3-old {
	DEFINES		+= HAVE_SDRPLAY_V3
	DEPENDPATH	+= ./devices/sdrplay-handler-v3
	INCLUDEPATH	+= ./devices/sdrplay-handler-v3 \
	                   ./devices/sdrplay-handler-v3/include
        HEADERS         += ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h \
                           ./devices/sdrplay-handler-v3/sdrplay-commands.h 
        SOURCES         += ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp 
	FORMS		+= ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
}
#
sdrplay-v3 {
	DEFINES		+= HAVE_SDRPLAY_V3
	DEPENDPATH	+= ./devices/sdrplay-handler-v3
	INCLUDEPATH	+= ./devices/sdrplay-handler-v3 \
	                   ./devices/sdrplay-handler-v3/include
        HEADERS         += ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h \
                           ./devices/sdrplay-handler-v3/sdrplay-commands.h \
	                   ./devices/sdrplay-handler-v3/Rsp-device.h \
	                   ./devices/sdrplay-handler-v3/RspI-handler.h \
	                   ./devices/sdrplay-handler-v3/Rsp1A-handler.h \
	                   ./devices/sdrplay-handler-v3/RspII-handler.h \
	                   ./devices/sdrplay-handler-v3/RspDuo-handler.h \
	                   ./devices/sdrplay-handler-v3/RspDx-handler.h
        SOURCES         += ./devices/sdrplay-handler-v3/Rsp-device.cpp \
	                   ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp \
	                   ./devices/sdrplay-handler-v3/RspI-handler.cpp \
	                   ./devices/sdrplay-handler-v3/Rsp1A-handler.cpp \
	                   ./devices/sdrplay-handler-v3/RspII-handler.cpp \
	                   ./devices/sdrplay-handler-v3/RspDuo-handler.cpp \
	                   ./devices/sdrplay-handler-v3/RspDx-handler.cpp 
	FORMS		+= ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
}
#
#	the AIRSPY
#
airspy {
	DEFINES		+= HAVE_AIRSPY
	FORMS		+= ./devices/airspy/airspy-widget.ui
	DEPENDPATH	+= ./devices/airspy
	INCLUDEPATH	+= ./devices/airspy \
	                   ./devices/airspy/libairspy
	HEADERS		+= ./devices/airspy/airspy-handler.h 
	SOURCES		+= ./devices/airspy/airspy-handler.cpp 
}
#
#
#	the AIRSPY
#
hackrf {
	DEFINES		+= HAVE_HACKRF
	FORMS		+= ./devices/hackrf-handler/hackrf-widget.ui
	DEPENDPATH	+= ./devices/hackrf-handler
	INCLUDEPATH	+= ./devices/hackrf-handler
	                   /usr/local/include/libhackrf
	HEADERS		+= ./devices/hackrf-handler/hackrf-handler.h 
	SOURCES		+= ./devices/hackrf-handler/hackrf-handler.cpp 
}

#
lime  {
        DEFINES         += HAVE_LIME
        INCLUDEPATH     += ./devices/lime-handler
        DEPENDPATH      += ./devices/lime-handler
        HEADERS         += ./devices/lime-handler/lime-handler.h \
                           ./devices/lime-handler/lime-widget.h \
                           ./devices/lime-handler/LMS7002M_parameters.h
        SOURCES         += ./devices/lime-handler/lime-handler.cpp
}

pluto	{
	DEFINES		+= HAVE_PLUTO
	QT              += network
        INCLUDEPATH     += ./devices/pluto-handler
        HEADERS         += ./devices/pluto-handler/pluto-handler.h \
	                   ./devices/pluto-handler/fmFilter.h
        SOURCES         += ./devices/pluto-handler/pluto-handler.cpp
        FORMS           += ./devices/pluto-handler/pluto-widget.ui
#	LIBS            += -liio -lad9361
}

#
#	the elad-s1
#
elad_s1 {
	DEFINES		+= HAVE_ELAD_S1
	FORMS		+= ./devices/sw-elad-s1/elad_widget.ui
	DEPENDPATH	+= ./devices/sw-elad-s1
	INCLUDEPATH	+= ./devices/sw-elad-s1 
	HEADERS		+= ./devices/sw-elad-s1/elad-s1.h \
	                   ./devices/sw-elad-s1/elad-worker.h \
	                   ./devices/sw-elad-s1/elad-loader.h
	SOURCES		+= ./devices/sw-elad-s1/elad-s1.cpp \
	                   ./devices/sw-elad-s1/elad-worker.cpp \
	                   ./devices/sw-elad-s1/elad-loader.cpp
}
#
#	extio dependencies, windows only
#
extio {
	DEFINES		+= HAVE_EXTIO
	FORMS		+= ./devices/extio-handler/extio-widget.ui
	INCLUDEPATH	+= ./devices/extio-handler
	DEPENDPATH	+= ./devices/extio-handler
	HEADERS		+= ./devices/extio-handler/extio-handler.h \
			   ./devices/extio-handler/virtual-reader.h \
	           	   ./devices/extio-handler/common-readers.h \
	           	   ./devices/extio-handler/card-reader.h 
	SOURCES		+= ./devices/extio-handler/virtual-reader.cpp \
	           	   ./devices/extio-handler/extio-handler.cpp \
	           	   ./devices/extio-handler/common-readers.cpp \
	           	   ./devices/extio-handler/card-reader.cpp 
}

pmsdr {
	DEFINES		+= HAVE_PMSDR
	FORMS		+= ./devices/pmsdr/pmsdr-widget.ui
	INCLUDEPATH	+= ./devices/pmsdr
	DEPENDPATH	+= ./devices/pmsdr
	HEADERS		+= ./devices/pmsdr/pmsdr.h \
	                   ./devices/pmsdr/pmsdr-usb.h \
	                   ./devices/pmsdr/pmsdr-comm.h \
	                   ./devices/pmsdr/pa-reader.h
	SOURCES		+= ./devices/pmsdr/pmsdr.cpp \
	                   ./devices/pmsdr/pmsdr-usb.cpp \
	                   ./devices/pmsdr/pmsdr-comm.cpp \
	                   ./devices/pmsdr/pa-reader.cpp
}

colibri {
        DEFINES         += HAVE_COLIBRI
        DEPENDPATH      += ./devices/colibri-handler
        INCLUDEPATH     += ./devices/colibri-handler
        HEADERS         += ./devices/colibri-handler/colibri-handler.h 
        SOURCES         += ./devices/colibri-handler/colibri-handler.cpp 
        FORMS           += ./devices/colibri-handler/colibri-widget.ui
}

