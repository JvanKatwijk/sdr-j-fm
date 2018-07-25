#
TEMPLATE    = lib
CONFIG      += plugin
QT          += core gui
INCLUDEPATH += . \
	       ../ ../.. ../../.. \
	       ../../../includes \
	       ../../../includes/various
HEADERS     = ../rig-interface.h \
	      ./pmsdr.h \
	      ./pmsdr_usb.h  \
	      ./pmsdr_comm.h \
	      ../pa-reader.h \
	      ../../../includes/fm-constants.h
SOURCES     = ./pmsdr.cpp \	
              ./pmsdr_usb.cpp \
	      ../pa-reader.cpp \
	      ./pmsdr_comm.cpp 
TARGET      = $$qtLibraryTarget(device_pmsdr)
FORMS	+= ./widget.ui

#for windows 32 we could use
#win32 {
#DESTDIR     = ../../../windows32-bin/input-plugins
## includes in mingw differ from the includes in fedora linux
#INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include
#INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include/qwt
#LIBS	+= -lusb-1.0
#LIBS	+= -lportaudio
#LIBS	+= -lole32
#LIBS	+= -lwinmm
#}

#for windows 64 use:
win32 {
DESTDIR     = ../../../windows64-bin/input-plugins
# includes in mingw differ from the includes in fedora linux
INCLUDEPATH += /usr/x86-w64-mingw32/sys-root/mingw/include
INCLUDEPATH += /usr/x86-w64-mingw32/sys-root/mingw/include/qwt
LIBS	+= -lusb-1.0
LIBS	+= -lportaudio
LIBS	+= -lole32
LIBS	+= -lwinmm
}

unix{
DESTDIR     = ../../../linux-bin/input-plugins
LIBS	+= -lusb-1.0 
}

