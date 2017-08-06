
A simple FM receiver
-------------------

The FM receiver will operate with AIRSPY, SDRplay and DABsticks.

For creating an executable: install the required libraries use qmake.

Note that the software is made for Qt5 and qwt6. Unfortunately, some
linux distributions do not support qwt in cooperation with Qt5.

Note that in the current version, the keypad on the GUI disappeared,
it is replaced by a button that can be pressed to activate a separate
keypad.

Note further that this version now supports a maintained programlist,
a list maintained between invocations of the program.

Finally, I was experimenting with a scanning function, one that stops
whenever the S/N ration is above a certain level. Still lots to be done.

This version is not supported, but works pretty well.


