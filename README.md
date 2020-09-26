
A simple FM receiver
-------------------

------------------------------------------------------------------------
New: support for the Adalm Pluto
------------------------------------------------------------------------

The Adalm pluto is now supported for both the Windows and the Linux version.
Since the software has to run on systems where support for the Pluto is
not installed, the driver software had to be adapted as with
other devices: on selecting the device, functions will be read-in from the
device library. To reduce dependency on external libraries, the filtering
software does not depend on the AD9361 library, but is "hard coded",
based on output from this library

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

The FM receiver does not support automatic device selection,
one has to select the device, then press the start button.
The FM receiver will operate with the SDRplay, DABsticks, the AIRspy,
Lime devices, Pluto and HACKrf devices. Note that there are two entries for use with the
SDRplay, one using the 2.13 library, the other one using the 3.06 (or 07)
library.

--------------------------------------------------------------------------------
Linux
--------------------------------------------------------------------------------
For creating an executable under: install the required libraries use qmake.

Note that the software is made for Qt5 and qwt6. Unfortunately, some
linux distributions do not support qwt in cooperation with Qt5.

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


