
    FM receiver V3.1
-------------------------

    SOFTWARE UNDER CONSTRUCTION

The software is contnuously changing since I am restructuring some parts
Be advised NOT to use these sources now

========================================================================

Some significant modifications were made by Thomas Neder, apart from the
"skin", most mods were "under the hood". 

The version number is now 3.15

![fm receiver](/fm-mainwidget.png?raw=true)

New is a so-called PSS (perfect stereo sound) control on the main widget,
contributed by TomNeda.
As known, the stereo sound is encoded as an L+R signal, ccompatible with 
non stereo transmissions, and an L-R signal, modulated as AM signal on
on offset of 38 KHz. The pilot, transmitted on an offset of 19 KHz
is used to precisely synchronize the L-R signal with the L+R signal.

In computing the 38 Khz signal from the 19 KHz pilot signal, an error
may occur, an offset of a few Hz. The PSS algorithm applies some
more advanced techniques to figute out what the correction on the recomputed
39 KHz carrier that has to be applied for a perfect match.

While in previous versions the list of configure devices was
checked on program start up, to find an attacted device, the
current version is slightly different.
On program start up, the software will try to start the device
(if any) that was used on the last occasion, if no such device
can be found, a small devide selection widget will show.
(Note that different from e.g. Qt-DAB, once a device is selected
and opened, no selection for another device is possible
during this program invocation)

![fm receiver](/fm-deviceselectwidget.png?raw=true)

Since the number of controls in the main widget was growing,
some controls were  transferred to a separate "control" widget.
The main widget has a button, labeled *Config* with which one can select
and deselect the controls widget.


![fm receiver](/fm-configwidget.png?raw=true)

Buttons on the control widget are typically those that are not
often touches when just listening to the radio.
Buttons for setting the deemphasis and the pty labels are therefore now
on the configuration widget, since their setting depends on the country
you are in.
Furthermore, the control widget contains a widget, *Combinear* on the
picture, that can be used to dynamically set another "skin"

The filtering, i.e. separation between transmissions close to each other
in frequency, or even with (slightly) overlapping frequencies,
is improved.
The filter is pretty strong, but that also means that the filter requires
some serious cpu use.

If the filter is switched "Off", it is excluded from the processing chain.

As usual, next to the sources, an AppImage (for Linux x64) and a Windows
installer are available in the releases section.

-------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------

Devices that are supported are
 * SDRplay devices. Note that on Windows, whenever you have installed a library 3.10 or up, the 2.13 library is not accessible. On Linux there is the
cjoice between using the 2.13 library and 3.07 (or up) library. Note that
wwhile the 2.13 library supports the original SDRplay RSP I, the 2.13
library does noot provide support for the RSPdx, the 3.0X library
does not provide support for the RSP I.
 * Airspy devices;
 * Lime devices;
 * Hackrf devices;
 * Dabsticks, i.e. rtlsdr devices.

There is also support for file input, however, right now only for
"wav" files with a samplerate of 2304000.

------------------------------------------------------------------------
Buiding an executable
------------------------------------------------------------------------

While there is an installer for Windows, and an appImage for Linux64,
it is of course possible to generate an executable.

For compiling and linking lots of libraries and utilities
have to be installed
On Ubuntu, in any case, the (older) 16 version that I use to
create appImages. I use qmake for generating a Makefile,
if you have cmake installed, it is possible to use that instead,
here it is assumed that qmake is used

- sudo apt-get update
- sudo apt-get install git
- sudo apt-get install qt5-qmake build-essential g++
- sudo apt-get install pkg-config
- sudo apt-get install libsndfile1-dev qt5-default
- sudo apt-get install libfftw3-dev portaudio19-dev 
- sudo apt-get install zlib1g-dev 
- sudo apt-get install libusb-1.0-0-dev mesa-common-dev
- sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev
- sudo apt-get install libsamplerate0-dev libqwt-qt5-dev
- sudo apt-get install qtbase5-dev

Summarizing:

-  Obviously, we need a C++ compiler and associated libraries

-  The GUI is developed using the Qt5 framework, for qt5 one needs
to install the qt5-qmake utility (which may have a lightly different
name on other distributions), the qt5 environment and dependent
libraries (here qt5-default, mesa-common-dev libgl1-mesa-dev,
libqt5opengh5-dev, qtbase5-dev)

- Furthermore, we need qwt, which is built on Qt5, but a separate library
(here libqwt-qt5-dev)
Note that on different systems, the qwt library and coreresponding include 
files may reside on different locations.

- Fast fourier transforms, both for generating spectra and for some
advanced filtering, is using the fftw3 subsystem. Note that the 
implementation uses the fftw3f (i.e. "float") version, not the "double".

- For handling the audio output, the portaudio library is used. Note that
the Version 19 library is used, not the older version 18.

- For handling file I/O the library libsndfile is used, and for
(some) rate conversions the library libsamplerate.

- To allow distributions to run on systems were not all device
libraries are installed, the approach taken is than when selecting
a device, the required functions needed by the interface software
are dynamically loaded. Since all devices use some form
of I/O over USB, the libusb library is also needed.

Once the libraries are loaded, one should look at the "fmreceiver.pro"
file, which is basically a configuration file. The qmake (whatever
version) is able to generate a real Makefile from it.
Of course, the "fmreceiver.pro" file might have to be adapted.
It contains a section for "unix" and one for "win" (i.e. windows.
The system dependencies are two parts:

 - the compiler flags, set by default on maximum optimization (after
all the program is a serious user of resources). In some cases the
"-flto" flag is known not to be available

 - the devices.

On selecting a device, the program dynamically loads the functions
from the device  library. The advantage is that devices for which
no device library is installed can be part of the configuration.
Of course, for a device you want to use, the device library should
be installed.

The program supports sdrplay RSP devices
(both using the "old" 2.13 library (which does not support the RSPdx)
and the 3.XX (XX = 11 on Windows), When using the 3.10 library or
higher on Windows, the 2.13 library cannot be reached, and the 3.10
library does not support the very first RSP, the RSP 1.

The program support the airspy devices, The program supports "dabsticks",
The program supports lime devices, program supports hackrf devices and
the program supports the Adalm pluto.

Note that this version does NOT support devices using a soundcard
for data input.

 Adapt the configuration (the "CONFIG += XXX" elements in the
".pro" file, most likely comment out devices that you do not want
to include

- run 'qmake' (variants of the name are qt5-qmake and qmake-qt5)
which generates a Makefile

- if all libraries are installed run "Make",

-----------------------------------------------------------------------
Building an appImage
----------------------------------------------------------------------

The AppImage, part of the distribution on the releases section, is
created on an Ubuntu 16 VM. If you want to experiment, the file
fm-build-script is there that - as the name suggests - is the script
being used to create such an AppImage

--------------------------------------------------------------------------
CopyRight
--------------------------------------------------------------------------

Copyright of the fm software is by J Van Katwijk, but parts of the mods
to older versions are done by Thomas Neder, obviously for the
parts he contributed, he the copyrights are his.


-
    Good luck

