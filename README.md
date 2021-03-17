
A simple FM receiver V2.0
-------------------------

-----------------------------------------------------------------------
New: resizable widgets
-----------------------------------------------------------------------

It was an item on my todo list, redesiging the GUI such that the main
widget - and most of the widgets for device support - are resizable.
Having that done was the main reason to renumber the version to 2.0

-----------------------------------------------------------------------
New: resizable widgets
-----------------------------------------------------------------------

It was an item on my todo list, redesiging the GUI such that the main
widget - and most of the widgets for device support - are resizable.
Having that dome wasd the main reason to renumber the version into 3.0

------------------------------------------------------------------------
New: support for the Adalm Pluto and experimentally for ColibriNano
------------------------------------------------------------------------

The Adalm pluto is now supported for both the Windows and the Linux version.
Since the software has to run on systems where support for the Pluto is
not installed, the driver software had to be adapted as with
other devices: on selecting the device, functions will be read-in from the
device library. To reduce dependency on external libraries, the filtering
software does not depend on the AD9361 library, but is "hard coded",
based on output from this library

As an experiment support for the ColibriNano is added. While I do not 
possess such a device, am SDR collegue has and helped me with the
installation and incorporation of support for the Colibri.
Note that since the colibri supports to up to 55 MHz, it is the second Nyquist
zone that is used for FM

![fm receiver](/fmreceiver.png?raw=true)

-------------------------------------------------------------------------
-------------------------------------------------------------------------

In spite of the foreseen move from analog to digital radio, there
are still lots of FM stations in the FM broadcast band.

The FM receiver software is an experimental receiver, with quite some settings.
A simpler version, just to listen to radio, is in the WFM-RPI repository.

The program shows 2 spectra, one is the spectrum of the incoming signal,
the other is the spectrum of the decoded signal.

-------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------

The FM software supports (obviously depending on the configuration)

* rtlsdr based sticks

* SDRplay devices, both  for the 2.13 and 3.07 library

* airspy

* hackrf

* lime

* pluto

--------------------------------------------------------------------------------
Linux
--------------------------------------------------------------------------------
For creating an executable under: install the required libraries use qmake.
For e,g. Ubuntu (i.e. Debian like) systems, the required libraries
can be installed by the following script

	sudo apt-get update
	sudo apt-get install git cmake
	sudo apt-get install qt5-qmake build-essential g++
	sudo apt-get install pkg-config
	sudo apt-get install libsndfile1-dev qt5-default
	sudo apt-get install libfftw3-dev portaudio19-dev
	sudo apt-get install zlib1g-dev
	sudo apt-get install libusb-1.0-0-dev mesa-common-dev
	sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev
	sudo apt-get install libsamplerate0-dev libqwt-qt5-dev
	sudo apt-get install qtbase5-dev

Of course, library support for devices that are part of the configuration
needs to be installed.

For SDRplay devices it is easy: download them from www.sdrplay.com

for "dabsticks" one might try the library that can be downloaded from
the distribution's repository, however, in some cases one needs
to blacklist the device.

For other devices there are no (known) precompiled libraries

-------------------------------------------------------------------------------
Windows
-------------------------------------------------------------------------------

For Windows, the releases section of this repository contains an installer, setup-fmreceiver-  that will
install the executable as well as required libraries. Note that the installer will call upon
an installer for the dll implementing the api to get access to the SDRplay

------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------

Note that this version now supports a maintained programlist,
a list maintained between invocations of the program.

Finally, I was experimenting with a scanning function, one that stops
whenever the S/N ration is above a certain level. Still lots to be done.


