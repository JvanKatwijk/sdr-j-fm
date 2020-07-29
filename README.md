
A simple FM receiver
-------------------

![fm receiver](/fmreceiver.png?raw=true)

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

The FM receiver will operate with the SDRplay, DABsticks, the AIRspy,
and HACKrf devices. Note that there are two entries for use with the
SDRplay, one using the 2.13 library, the other one using the 3.06 (or 07)
library.
In this version, there is no automatic device selection, one has to select the device, then press the start button.

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


