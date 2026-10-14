#
/*
 *    Copyright (C)  2016 .. 2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm receiver
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include        "fm-constants.h"
#include	"basic-scope.h"
#include	"fft-handler.h"

class	RadioInterface;
class	QSettings;

class	halfScope: public basicScope {
Q_OBJECT
public:
		halfScope	(clickableChart *,
	                         int,
	                         QSettings *, const QString &);
		~halfScope	();
	void	display		(std::vector<Complex> &, int, int, int);
	void	set_bitDepth	(int);
	void	clean		();

private:
	fftHandler	theFFT;
	QSettings	*fmSettings;
	int		bitDepth;
	std::vector<float> Window;
	std::vector<double> displayBuffer;
	int		normalizer;
	int16_t		displaySize;
	bool		hasMarker;
	float		get_db		(float);
};

