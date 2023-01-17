
A simple FM receiver V3.0
-------------------------

========================================================================

Some significant modifications were made by Thomas Neder, apart from the
"skin", most mods were "under the hood". 
The version number is now 3.0

![fm receiver](/fmreceiver.png?raw=true)

The most recent version has an additional button to set the program type
labels correct for the area, one might select "Europe"  or "USA".

The filtering, i.e. separation between transmissions close to each other
in frequency, or even with (slightly) overlapping frequencies,
is improved.
The filter is pretty strong, but that also means that the filter requires
some serious cpu use.

If the filter is switched "Off", it is excluded from the processing chain.

As usual, next to the sources, an AppImage (for Linux x64) and a Windows
installer are available in the releases section.

Supported devices are sdrplay, hackrf, adalm pluto, line and the good 
old dabstick. Note that on Windows, if you have installed a library 3.10
for SDRplay devices, the "old" 2.13 library will not be accessible.
(Note that in the most  recent versions of the SDRplay drivers the biasT selectors seem to work.)

You can choose  one of two "skin"s using a command line parameter
Choose -A for the one, and -B for the other skin. Of course the setting
is maintained between program invocations.
Since the setting is kept in the "ini" file, you can edit
the "ini" file (file ".jsdr-fm.ini"  in the user's home directory (folder)).


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

   sudo apt-get update
   sudo apt-get install git
   sudo apt-get install qt5-qmake build-essential g++
   sudo apt-get install pkg-config
   sudo apt-get install libsndfile1-dev qt5-default
   sudo apt-get install libfftw3-dev portaudio19-dev 
   sudo apt-get install zlib1g-dev 
   sudo apt-get install libusb-1.0-0-dev mesa-common-dev
   sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev
   sudo apt-get install libsamplerate0-dev libqwt-qt5-dev
   sudo apt-get install qtbase5-dev

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


-
    Good luck

