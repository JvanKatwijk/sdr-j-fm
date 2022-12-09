/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J-FM
 *
 *    SDR-J-FM is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J-FM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J-FM; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __IQDISPLAY_H
#define __IQDISPLAY_H

#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>

#include	<stdint.h>
#include	"fm-constants.h"

class IQDisplay : public QObject, public QwtPlotSpectrogram {
Q_OBJECT
public:
		IQDisplay	(QwtPlot *, int16_t);
		~IQDisplay	(void);
	void	DisplayIQ	(const DSPCOMPLEX, const float);
	void	DisplayIQVec	(const DSPCOMPLEX * const,
	                                 const int32_t n, const float);

private:
	int32_t		mAmount;
	double		*mpPlotData1;
	double		*mpPlotData2;
//	DSPCOMPLEX	*mpPoints;
	QwtPlot		*mpQwtPlot;
	int32_t		mNoPointsPerRadius;
	int32_t		mNoPointsColOrRow;
	int32_t		mNoMaxPointsOnField;
	int32_t		mInpInx;

private slots:

};
#endif
