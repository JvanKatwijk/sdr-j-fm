#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the  sdr-j-fm .
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__KEYPAD_H
#define	__KEYPAD_H
#include	<QWidget>
#include	<QGridLayout>
#include	<QButtonGroup>
#include	<QPushButton>
#include	<QLCDNumber>

class	RadioInterface;

class	keyPad:QObject {
Q_OBJECT
public:
		keyPad		(RadioInterface *);
		~keyPad		();
	void	showPad		();
	void	hidePad		();
	bool	isVisible	();
private slots:
	void	collectData	(int);
private:
	RadioInterface	*myRadio;
	QWidget		*theFrame;
	QGridLayout	*theLayout;
	QButtonGroup	*thePad;
	QPushButton		*zeroButton;
	QPushButton		*oneButton;
	QPushButton		*twoButton;
	QPushButton		*threeButton;
	QPushButton		*fourButton;
	QPushButton		*fiveButton;
	QPushButton		*sixButton;
	QPushButton		*sevenButton;
	QPushButton		*eightButton;
	QPushButton		*nineButton;
	QPushButton		*KHzButton;
	QPushButton		*MHzButton;
	QPushButton		*clearButton;
	QPushButton		*correctButton;
	QLCDNumber		*theDisplay;
	int		panel;
	bool		shown;
signals:
	void		newFrequency	(int);
};

#endif

