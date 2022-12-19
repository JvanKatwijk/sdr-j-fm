/*
 *    Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013
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
 *
 */
#include	"iqdisplay.h"
#include	"spectrogramdata.h"
#include	<assert.h>
#include	<stdio.h>
/*
 *	iq circle plotter
 */
static IqScopeData * sIQData = nullptr;

		IQDisplay::IQDisplay(QwtPlot * plot, int16_t x):
	                                        QwtPlotSpectrogram () {
	QwtLinearColorMap * colorMap =
	                 new QwtLinearColorMap(Qt::darkBlue, Qt::yellow);

//	setRenderThreadCount (1);
	pointsPerRadius = 50;

	connect	(this, SIGNAL (replot ()), plot, SLOT (replot ()));

// zero position is a extra row and column
	pointsColorRow		= 2 * pointsPerRadius + 1;
	maxPointsOnField	= pointsColorRow * pointsColorRow;
	thePlot			= plot;
	amount			= x;
	inPointer		= 0;
	this	-> setColorMap (colorMap);
//
//	The original setting turned out to give problems with
//	data being accessed outside of the boundaries
	plotData1 = new double [2 * maxPointsOnField + pointsPerRadius + 10];
	plotData2 = new double [2 * maxPointsOnField + pointsPerRadius + 10];

	memset (plotData1, 0, maxPointsOnField * sizeof (double));
	memset (plotData2, 0, maxPointsOnField * sizeof (double));

	sIQData		= new IqScopeData (plotData2, pointsColorRow, 50.0);
	this		-> setData (sIQData);
	thePlot		-> enableAxis (QwtPlot::xBottom, false);
	thePlot		-> enableAxis (QwtPlot::yLeft, false);
	thePlot		-> enableAxis (QwtPlot::xTop, false);
	thePlot		-> enableAxis (QwtPlot::yRight, false);
	this		-> setDisplayMode (QwtPlotSpectrogram::ImageMode, true);
	this		-> attach (thePlot);
	thePlot		-> setFixedHeight (2 * pointsColorRow);
	thePlot		-> setFixedWidth (2 * pointsColorRow);
	replot ();
}

		IQDisplay::~IQDisplay () {
	this	-> detach ();
	delete	sIQData;
	delete [] plotData2;
	delete [] plotData1;
}

template<class T> inline
void symmetric_limit(T & ioVal, const T iLimit) {
	if (ioVal >  iLimit)
	   ioVal =  iLimit;
	else
	if (ioVal < -iLimit)
	   ioVal = -iLimit;
//	if ((x) > (l)) x = (l); else if (x < -(l)) x = -(l);
}

void	IQDisplay::DisplayIQ (const DSPCOMPLEX z, const float scale) {
int32_t h = (int32_t)(pointsPerRadius * scale * real(z));
int32_t v = (int32_t)(pointsPerRadius * scale * imag(z));

//	the field has width of 201 and height of 201, the middle has index 100

	symmetric_limit (v, pointsPerRadius);
	symmetric_limit (h, pointsPerRadius);

	const int32_t index = (v + pointsPerRadius) *
	                       pointsColorRow + h + pointsPerRadius;
	assert (index >= 0);
	assert (index < maxPointsOnField);
	plotData1 [index] = 50;

	if (++inPointer < amount)
	   return;

//	we need an extra data buffer as
//	the  replot() seems to take a while in the background
	memcpy (plotData2, plotData1,
	                   maxPointsOnField * sizeof (double));

	replot ();
	inPointer = 0;

//	clear next write buffer and draw cross and circle
	memset (plotData1, 0, maxPointsOnField * sizeof (double));

    // draw cross
	for (int32_t i = -pointsPerRadius;
	                      i <= pointsPerRadius; i ++) {
	   int32_t ih = pointsPerRadius * pointsColorRow
	                                          + i + pointsPerRadius;
	   int32_t iv = (i + pointsPerRadius) * pointsColorRow +
	                                               pointsPerRadius;
	   plotData1 [ih] = plotData1 [iv] = 10;
	}

//	draw unit circle
	int32_t MAX_CIRCLE_POINTS = 45;
	float SCALE = 0.5f;

	for (int32_t i = 0; i < MAX_CIRCLE_POINTS; ++i) {
	   float phase = 0.5f * M_PI * i / MAX_CIRCLE_POINTS;
	   int32_t h =
	               (int32_t)(pointsPerRadius * SCALE * cosf (phase));
	   const int32_t v =
	               (int32_t)(pointsPerRadius * SCALE * sinf (phase));

	   const int32_t ior =
	               (v + pointsPerRadius) * pointsColorRow + h +
	                                                   pointsPerRadius;
	   const int32_t iol =
	              (v + pointsPerRadius) * pointsColorRow - h +
	                                                   pointsPerRadius;
	   const int32_t ibr =
	              (-v + pointsPerRadius) * pointsColorRow + h +
	                                                  pointsPerRadius;
	   const int32_t ibl =
	              (-v + pointsPerRadius) * pointsColorRow - h +
	                                                  pointsPerRadius;
	   plotData1 [ior] =
	   plotData1 [iol] =
	   plotData1 [ibr] = plotData1 [ibl] = 10;
	}
}

void	IQDisplay::DisplayIQVec (const DSPCOMPLEX * const z,
	                         const int32_t n, const float scale) {
	for (int32_t i = 0; i < n; ++i)
	   DisplayIQ (z [i], scale);
}
