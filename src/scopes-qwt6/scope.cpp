#
/*
 *    Copyright (C)  2012, 2013, 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J.
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License. 
 *    All copyrights of the original authors are recognized.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"scope.h"
#include	<qwt_picker_machine.h>
/*
 *	The "scope" combines the Qwt widgets and control for both
 *	the spectrumdisplay and the waterfall display.
 */
	Scope::Scope (QwtPlot		*scope,
	              uint16_t		displaysize,
	              uint16_t		rastersize) {
	Plotter			= scope;
	Displaysize		= displaysize;
	Rastersize		= rastersize;
	bitDepth		= 24;
	Spectrum	= NULL;
	Waterfall	= new WaterfallViewer (Plotter, Displaysize, Rastersize);
	CurrentWidget	= WATERFALL_MODE;
	connect (Waterfall,
	         SIGNAL (leftClicked (int)),
	         this,
	         SLOT (leftClicked (int)));
	connect (Waterfall,
	         SIGNAL (rightClicked (int)),
	         this,
	         SLOT (rightClicked (int)));
}

	Scope::~Scope (void) {
	if (Spectrum != NULL)
	   delete Spectrum;
	if (Waterfall != NULL)
	   delete Waterfall;
}

int	Scope::currentMode (void) {
	return CurrentWidget;
}

void	Scope::leftClicked (int n) {
	clickedwithLeft (n);
}

void	Scope::rightClicked (int n) {
	clickedwithRight (n);
}

void	Scope::SelectView (uint8_t n) {
	if (CurrentWidget == n)
	   return;

	if (n == SPECTRUM_MODE) {
	   if (Waterfall != NULL)
	      delete Waterfall;
	   Plotter	-> detachItems ();
	   Waterfall	= NULL;
	   Spectrum	= new SpectrumViewer  (Plotter,
	                                       Displaysize);
	   connect (Spectrum,
	            SIGNAL (leftClicked (int)),
	            this,
	            SLOT (leftClicked (int)));
	   connect (Spectrum,
	            SIGNAL (rightClicked (int)),
	            this,
	            SLOT (rightClicked (int)));
	   CurrentWidget = SPECTRUM_MODE;
	   Spectrum -> setBitDepth (bitDepth);
	}
	else	// n == WATERFALL_MODE
	if (n == WATERFALL_MODE) {
	   if (Spectrum != NULL)
	      delete Spectrum;
//	   Plotter	-> detachItems ();
	   Spectrum	= NULL;
	   Waterfall	= new WaterfallViewer (Plotter,
	                                       Displaysize,
	                                       Rastersize);
	   connect (Waterfall,
	            SIGNAL (leftClicked (int)),
	            this,
	            SLOT (leftClicked (int)));
	   connect (Waterfall,
	            SIGNAL (rightClicked (int)),
	            this,
	            SLOT (rightClicked (int)));
	   
	   CurrentWidget = WATERFALL_MODE;
	}
}

void	Scope::Display (double	*x_axis,
	                double	*buffer,
	                double	amp,
	                int32_t	marker) {
	if (CurrentWidget == WATERFALL_MODE)
	   Waterfall	-> ViewWaterfall (x_axis,
	                                  buffer,
	                                  amp,
	                                  marker);
	else
	   Spectrum	-> ViewSpectrum (x_axis,
	                                 buffer,
	                                 amp,
	                                 marker);
}

void	Scope::setBitDepth	(int16_t b) {
	bitDepth = b;
	if (CurrentWidget == SPECTRUM_MODE)
	   Spectrum -> setBitDepth (b);
}
/*
 *	The spectrumDisplay
 */
	SpectrumViewer::SpectrumViewer (QwtPlot *plot, uint16_t displaysize) {

	plotgrid		= plot;
	this	-> Displaysize	= displaysize;
	plotgrid-> setCanvasBackground (Qt::black);
	grid	= new QwtPlotGrid;
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
	grid	-> setMajPen (QPen(Qt::white, 0, Qt::DotLine));
#else
	grid	-> setMajorPen (QPen(Qt::white, 0, Qt::DotLine));
#endif
	grid	-> enableXMin (true);
	grid	-> enableYMin (true);
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
	grid	-> setMinPen (QPen(Qt::white, 0, Qt::DotLine));
#else
	grid	-> setMinorPen (QPen(Qt::white, 0, Qt::DotLine));
#endif
	grid	-> attach (plotgrid);

	SpectrumCurve	= new QwtPlotCurve ("");
   	SpectrumCurve	-> setPen (QPen(Qt::white));
//	SpectrumCurve	-> setStyle	(QwtPlotCurve::Sticks);
	SpectrumCurve	-> setOrientation (Qt::Horizontal);
	SpectrumCurve	-> setBaseline	(get_db (0));
	ourBrush	= new QBrush (Qt::white);
	ourBrush	-> setStyle (Qt::Dense3Pattern);
	SpectrumCurve	-> setBrush (*ourBrush);
	SpectrumCurve	-> attach (plotgrid);
	
	Marker		= new QwtPlotMarker ();
	Marker		-> setLineStyle (QwtPlotMarker::VLine);
	Marker		-> setLinePen (QPen (Qt::red));
	Marker		-> attach (plotgrid);
	oldmarkerValue	= -1;
	counter		= 0;

	maxLabel	= new QwtPlotTextLabel ();
	minLabel	= new QwtPlotTextLabel ();

	plotgrid	-> enableAxis (QwtPlot::yLeft);
	lm_picker	= new QwtPlotPicker (plot -> canvas ());
	QwtPickerMachine *lpickerMachine =
	              new QwtPickerClickPointMachine ();
	lm_picker	-> setStateMachine (lpickerMachine);
	lm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::LeftButton);
	connect (lm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (leftMouseClick (const QPointF &)));

	rm_picker	= new QwtPlotPicker (plot -> canvas ());
	QwtPickerMachine *rpickerMachine =
	              new QwtPickerClickPointMachine ();
	rm_picker	-> setStateMachine (rpickerMachine);
	rm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::RightButton);
	connect (rm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (rightMouseClick (const QPointF &)));

	IndexforMarker	= 0;
}

	SpectrumViewer::~SpectrumViewer () {
	disconnect (lm_picker,
	            SIGNAL (selected (const QPointF &)),
	            this,
	            SLOT (leftMouseClick (const QPointF &)));
	disconnect (rm_picker,
	            SIGNAL (selected (const QPointF &)),
	            this,
	            SLOT (rightMouseClick (const QPointF &)));
	plotgrid	-> enableAxis (QwtPlot::yLeft, false);
	Marker		-> detach ();
	SpectrumCurve	-> detach ();
	grid		-> detach ();
	delete		Marker;
	delete		SpectrumCurve;
	delete		grid;
	delete		lm_picker;
	delete		rm_picker;
}

void	SpectrumViewer::leftMouseClick (const QPointF &point) {
	leftClicked ((int)(point. x()) - IndexforMarker);
}

void	SpectrumViewer::rightMouseClick (const QPointF &point) {
	rightClicked ((int)(point. x()) - IndexforMarker);
}

void	SpectrumViewer::ViewSpectrum (double *X_axis,
		                      double *Y1_value,
	                              double amp,
	                              int32_t markerValue) {
uint16_t	i;

	amp		= amp / 100 * (-get_db (0));
	IndexforMarker	= markerValue;
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
				         (double)X_axis [0],
				         X_axis [Displaysize - 1]);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	plotgrid	-> setAxisScale (QwtPlot::yLeft,
				         get_db (0), get_db (0) + amp);

	for (i = 0; i < Displaysize; i ++) 
	   Y1_value [i] = get_db (Y1_value [i]); 

	double s = 0, max = get_db (0);

	for (i = 0; i < Displaysize; i ++) {
	   s += Y1_value [i];
	   if (!isnan (Y1_value [i]) && (Y1_value [i] > max))
	      max = Y1_value [i];
	}
	double avg	= s / Displaysize;

	SpectrumCurve	-> setBaseline (get_db (0));
	Y1_value [0]	= get_db (0);
	Y1_value [Displaysize - 1] = get_db (0);

	SpectrumCurve	-> setSamples (X_axis, Y1_value, Displaysize);
	if (--counter < 0) {
	   counter = 10;
	   QwtText MarkerLabel_1  =  QwtText (QString::number (max));
	   QwtText MarkerLabel_2  =  QwtText (QString::number (avg));
	   QFont font1 ("Courier New");
	   font1.	setPixelSize (30);;
	   MarkerLabel_1.    setFont (font1);
	   MarkerLabel_1.    setColor (Qt::white);
	   MarkerLabel_1. setRenderFlags (Qt::AlignLeft | Qt::AlignTop);
	   MarkerLabel_2.    setFont (font1);
	   MarkerLabel_2.    setColor (Qt::white);
	   MarkerLabel_2. setRenderFlags (Qt::AlignRight | Qt::AlignTop);
	   maxLabel	-> detach ();
	   maxLabel	-> setText (MarkerLabel_1);
	   maxLabel	-> attach (plotgrid);
	   minLabel	-> detach ();
	   minLabel	-> setText (MarkerLabel_2);
	   minLabel	-> attach (plotgrid);
	   Marker       -> detach ();
	   Marker       = new QwtPlotMarker ();
	   Marker       -> setLineStyle (QwtPlotMarker::VLine);
	   Marker       -> setLinePen (QPen (Qt::white, 3.0));
	   Marker       -> attach (plotgrid);
	   Marker       -> setXValue (markerValue);
	   oldmarkerValue   = markerValue;
	}
	plotgrid	-> replot(); 
}

float	SpectrumViewer::get_db (float x) {
	return 20 * log10 ((x + 1) / (float)(normalizer));
}

void	SpectrumViewer::setBitDepth	(int16_t d) {

	if (d < 0 || d > 32)
	   d = 24;

	normalizer	= 1;
	while (-- d >= 0) 
	   normalizer <<= 1;
}

/*
 *	The waterfall display
 */
	WaterfallViewer::WaterfallViewer (QwtPlot	*plot,
	                                  uint16_t	displaysize,
	                                  uint16_t	rastersize):
	                                  QwtPlotSpectrogram () {
int	i, j;
	colorMap  = new QwtLinearColorMap (Qt::darkCyan, Qt::red);
	QwtLinearColorMap *c2 = new QwtLinearColorMap (Qt::darkCyan, Qt::red);
	plotgrid		= plot;
	this	-> Displaysize	= displaysize;
	this	-> Rastersize	= rastersize;
	colorMap 	-> addColorStop	(0.1, Qt::cyan);
	colorMap 	-> addColorStop	(0.4, Qt::green);
	colorMap	-> addColorStop	(0.7, Qt::yellow);
	c2 	-> addColorStop	(0.1, Qt::cyan);
	c2 	-> addColorStop	(0.4, Qt::green);
	c2	-> addColorStop	(0.7, Qt::yellow);
	this -> setColorMap (colorMap);
	OneofTwo		= 0;
	rightAxis = plotgrid -> axisWidget (QwtPlot::yRight);
// A color bar on the right axis
	rightAxis -> setColorBarEnabled (true);

	plotData = new double [2 * Displaysize * Rastersize];
	for (i = 0; i < Rastersize; i ++)
 	   for (j = 0; j < Displaysize; j ++)
	      plotData [i * Displaysize + j] = (double)i / Rastersize;

	WaterfallData	= new SpectrogramData (plotData,
	                                       10000,
	                                       1000,
	                                       Rastersize,
	                                       Displaysize,
	                                       50.0);
	this -> setData (WaterfallData);
	this -> setDisplayMode (QwtPlotSpectrogram::ImageMode, true);
	rightAxis -> setColorMap (this -> data () -> interval (Qt::YAxis),
	                          c2);
	plotgrid	-> setAxisScale (QwtPlot::yRight, 0, 50.0);
	plotgrid	-> enableAxis (QwtPlot::yRight);
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
	                                 10000,
	                                 11000);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	plotgrid	-> enableAxis (QwtPlot::yLeft);
	plotgrid	-> setAxisScale (QwtPlot::yLeft, 0, Rastersize);

	Marker		= new QwtPlotMarker ();
	Marker		-> setLineStyle (QwtPlotMarker::VLine);
	Marker		-> setLinePen (QPen (Qt::black));
	Marker		-> attach (plotgrid);
//	this		-> attach (plotgrid);
	IndexforMarker	= 0;

	lm_picker	= new QwtPlotPicker (plot -> canvas ());
	QwtPickerMachine *lpickerMachine =
	              new QwtPickerClickPointMachine ();
	lm_picker	-> setStateMachine (lpickerMachine);
	lm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::LeftButton);
	connect (lm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (leftMouseClick (const QPointF &)));

	rm_picker	= new QwtPlotPicker (plot -> canvas ());
	QwtPickerMachine *rpickerMachine =
	              new QwtPickerClickPointMachine ();
	rm_picker	-> setStateMachine (rpickerMachine);
	rm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::RightButton);
	connect (rm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (rightMouseClick (const QPointF &)));

	plotgrid	-> replot ();
}

	WaterfallViewer::~WaterfallViewer () {
	plotgrid	-> enableAxis (QwtPlot::yRight, false);
	plotgrid	-> enableAxis (QwtPlot::xBottom, false);
	plotgrid	-> enableAxis (QwtPlot::yLeft, false);
	this		-> detach ();
}

void	WaterfallViewer::leftMouseClick (const QPointF &point) {
	leftClicked ((int)(point. x()) - IndexforMarker);
}

void	WaterfallViewer::rightMouseClick (const QPointF &point) {
	rightClicked ((int)(point. x()) - IndexforMarker);
}

void	WaterfallViewer::ViewWaterfall (double *X_axis,
	                                double *Y1_value,
	                                double amp,
	                                int32_t marker) {
int	orig	= (int)(X_axis [0]);
int	width	= (int)(X_axis [Displaysize - 1] - orig);

	IndexforMarker	= marker;
	if (OneofTwo) {
	   OneofTwo = 0;
	   return;
	}
	OneofTwo	= 1;
/*
 *	shift one row, faster with memmove than writing out
 *	the loops. Note that source and destination overlap
 *	and we therefore use memmove rather than memcpy
 */
	memmove (&plotData [0],
	         &plotData [Displaysize], 
	         (Rastersize - 1) * Displaysize * sizeof (double));
/*
 *	... and insert the new line
 */
	memcpy (&plotData [(Rastersize - 1) * Displaysize],
	        &Y1_value [0],
	        Displaysize * sizeof (double));

	this		-> detach	();
//	if (WaterfallData != NULL)
//	   delete WaterfallData;

	WaterfallData = new SpectrogramData (plotData,
	                                     orig,
	                                     width,
	                                     Rastersize,
	                                     Displaysize,
	                                     amp);
//	this		-> detach	();
	this		-> setData	(WaterfallData);
	this		-> setDisplayMode (QwtPlotSpectrogram::ImageMode,
	                                                               true);
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
	                                 orig,
	                                 orig + width);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	Marker		-> setXValue (marker);
	this		-> attach     (plotgrid);
	plotgrid	-> replot();
}

