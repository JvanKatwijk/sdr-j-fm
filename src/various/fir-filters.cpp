/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fmreceiver
 *
 *    fmreceiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation as version 2 of the License.
 *
 *    fmreceiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include  "fir-filters.h"
#ifndef __MINGW32__
#ifdef  __FREEBSD__
#include  <stdlib.h>
#else
#include  "alloca.h"
#endif
#endif

//===================================================================
//=====================================================================
//FIR LowPass

	LowPassFIR::LowPassFIR (int firsize,
	                        int32_t Fc, int32_t fs) : Basic_FIR (firsize) {
	sampleRate	= fs;
	newKernel (Fc);
}

void	LowPassFIR::newKernel (int32_t Fc) {
DSPFLOAT	tmp [filterSize];
DSPFLOAT	f    = (DSPFLOAT)Fc / sampleRate;
DSPFLOAT	sum  = 0.0;

	for (int i = 0; i < filterSize; i++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize / 2)) / (i - filterSize / 2);
//
//	Blackman window
	   tmp [i] *= (0.42 -
                0.50 * cos(2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize) +
                0.08 * cos(4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i++)
	   filterKernel [i] = DSPCOMPLEX (tmp [i] / sum, 0);
}

	LowPassFIR::~LowPassFIR () {
}

DSPCOMPLEX	*LowPassFIR::getKernel () {
	return filterKernel;
}

//=====================================================================
//FIR HighPass

	HighPassFIR::HighPassFIR (int firsize,
	                          int32_t Fc, int32_t fs):
	                                         Basic_FIR(firsize) {
	sampleRate = fs;
	newKernel (Fc);
}

void	HighPassFIR::newKernel (int32_t Fc) {
DSPFLOAT *tmp	= (DSPFLOAT *)alloca (filterSize * sizeof(DSPFLOAT));
DSPFLOAT f	= (DSPFLOAT)Fc / sampleRate;
DSPFLOAT sum	= 0.0;

	for (int i = 0; i < filterSize; i++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize / 2)) / (i - filterSize / 2);

	   tmp [i] *= (0.42 -
	           0.50 * cos (2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize) +
	           0.08 * cos (4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i++)
	   if (i == filterSize / 2)
	      filterKernel [i] = DSPCOMPLEX (1.0 - tmp [i] / sum, 0);
	   else
	      filterKernel [i] = DSPCOMPLEX(-tmp [i] / sum, 0);
}

	HighPassFIR::~HighPassFIR () {
}

//===============================================================

/*
 *	for the major bandpass filter, we use a simple basis,
 *	the FIR based on a blackman filter.
 *	The filter itself is somewhat more DSPCOMPLEX
 *	since we take negative frequencies into account
 *	based on the assumption that since we have I and Q
 *	signals, the spectrum is essentially a 0 .. 2N one
 */
static
DSPFLOAT	Blackman (DSPFLOAT *v, int fsize, DSPFLOAT f) {
DSPFLOAT	sum = 0;

	for (int i = 0; i < fsize; i++) {
	   if (i == fsize / 2)
	      v [i] = 2 * M_PI * f;
	   else
	      v [i] = sin (2 * M_PI * f * (i - fsize / 2)) / (i - fsize / 2);

	   v [i] *= (0.42 -
	                0.5 * cos (2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)fsize) +
	                0.08 * cos (4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)fsize));

	   sum += v [i];
	}

	return sum;
}

static
DSPFLOAT	*BandPassKernel (DSPFLOAT *v, int fsize, DSPFLOAT Fcl, DSPFLOAT Fch) {
DSPFLOAT	sumA	= 0.0;
DSPFLOAT	sumB	= 0.0;
DSPFLOAT	*tmp1	= (DSPFLOAT *)alloca (fsize * sizeof (DSPFLOAT));
DSPFLOAT	*tmp2	= (DSPFLOAT *)alloca (fsize * sizeof (DSPFLOAT));

	if ((Fcl > 0.5) || (Fch <= Fcl) || (Fch > 0.5)) {
	   fprintf(stderr, "bandpasskernel ??? (%f, %f) %d\n",
	                 (float)Fcl, (float)Fch, fsize);
	   Fcl	= 0.2;
	   Fch	= 0.4;
	}

	sumA	= Blackman (tmp1, fsize, Fcl);
	sumB	= Blackman(tmp2, fsize, Fch);
	/* normalize	*/
	for (int i = 0; i < fsize; i++) {
	   tmp1 [i] =	tmp1 [i] / sumA;
	   tmp2 [i] = - tmp2 [i] / sumB;
	   v [i]    = - (tmp1 [i] + tmp2 [i]);
	}

	return v;
}

	BasicBandPass::BasicBandPass (int firsize,
		                      int32_t low, int32_t high,
	                              int32_t rate):Basic_FIR(firsize) {
DSPFLOAT	*t1 = (DSPFLOAT *)alloca (firsize * sizeof (DSPFLOAT));

	t1 = BandPassKernel (t1, filterSize, (DSPFLOAT) low / rate,
	                                     (DSPFLOAT)high / rate);
	for (int i = 0; i < filterSize; i++)
	   filterKernel [i] = DSPCOMPLEX (t1 [i], t1 [i]);
}

	BasicBandPass::~BasicBandPass () {
}

DSPCOMPLEX	*BasicBandPass::getKernel () {
	return filterKernel;
}

/*
 *	The bandfilter is for the complex domain.
 *	We create a lowpass filter, which stretches over the
 *	positive and negative half, then shift this filter
 *	to the right position to form a nice bandfilter.
 *	For the real domain, we use the Simple BandPass version.
 */
	BandPassFIR::BandPassFIR (int firSize,
	                          int32_t low, int32_t high,
	                          int32_t fs):Basic_FIR (firSize) {
	sampleRate	= fs;
	newKernel (low, high);
}

void	BandPassFIR::newKernel (int32_t low, int32_t high) {
DSPFLOAT	*tmp	= (DSPFLOAT *)alloca (filterSize * sizeof (DSPFLOAT));
DSPFLOAT	lo	= (DSPFLOAT)((high - low) / 2) / sampleRate;
DSPFLOAT	shift	= (DSPFLOAT) ((high + low) / 2) / sampleRate;
DSPFLOAT	sum	= 0.0;

	for (int i = 0; i < filterSize; i++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize / 2)) / (i - filterSize / 2);
//
//	windowing
	   tmp [i] *= (0.42 -
	           0.5 * cos (2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize) +
	           0.08 * cos (4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i++) {    // shifting
	   DSPFLOAT v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = DSPCOMPLEX (tmp [i] * cos (v) / sum,
	                                  tmp [i] * sin (v) / sum);
	}
}

	BandPassFIR::~BandPassFIR () {
}

DSPCOMPLEX	*BandPassFIR::getKernel () {
	return filterKernel;
}

//=====================================================================
/*
 *	adaptive (noise reduction) filter a la
 *	doug smith (Signal, Samples and stuff 3, qex
 *	july 1998.
 *	basic: h(k+ 1) = lambda h(k) + 2 * u * err * x (k)
 *	it really works
 */
	adaptiveFilter::adaptiveFilter (int fsize) {
	firsize = fsize;
	Kernel  = new DSPFLOAT [fsize];
	Buffer  = new DSPCOMPLEX [fsize + fsize];
/*
 *	we choose a small mu
 */
	adaptFilter	(0.10);
}

	adaptiveFilter::~adaptiveFilter () {
	delete[]	Kernel;
	delete[]	Buffer;
}

void	adaptiveFilter::adaptFilter (DSPFLOAT f) {

	mu = 0.20;
	if (f > 0 && f < 1)
	   mu = f;

	ip		= 0;

	Kernel [0] = 1.0;

	for (int i = 1; i < firsize; i++) {
	   Kernel[i]	= 0;
	   Buffer [i]	= 0;
	   Buffer [firsize + i] = 0;
	}

	err = 0.5;
}
/*
 *  Passing here is more DSPCOMPLEX since we
 *  adapt the filtercoeeficients at the same time
 */
DSPCOMPLEX adaptiveFilter::Pass (DSPCOMPLEX z) {
DSPCOMPLEX	tmp = 0;
DSPFLOAT	sum	= 0.0;
/*
 *	first filter with delayed elements
 *
 *	Buffer is used in a circular way, with insertion and reading
 *	differing by 1 * firsize
 */
	Buffer [ip] = z;

	for (int i = 0; i < firsize; i++) {
	   int index = (ip + firsize - i);
	   if (index < 0)
	      index += 2 * firsize;
	   tmp += cmul (Buffer [index], Kernel [i]);
	}

	ip = (ip + 1) % (2 * firsize);
/*
 *	determine the error
 */
	err = (real (z) - real (tmp)) + (imag (z) - imag (tmp));
/*
 *	... and adapt the kernel elements accordingly
 */
	for (int i = 0; i < firsize; i++) {
	   Kernel [i] = 0.95 * Kernel [i] + 2.0 * mu * err * real(z);
	   sum += Kernel [i];
	}

	 for (int i = 0; i < firsize; i++)
	   Kernel [i] /= sum;
	return tmp;
}
//
//====================================================================
//
//	decimating filter

DecimatingFIR::DecimatingFIR (int firSize,
                              int32_t low,
                              int32_t fs,
                              int Dm):Basic_FIR (firSize) {

	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low);
}

void	DecimatingFIR::newKernel (int32_t low) {
DSPFLOAT	*tmp	= (DSPFLOAT *)alloca (filterSize * sizeof (DSPFLOAT));
DSPFLOAT	f	= (DSPFLOAT)low / sampleRate;
DSPFLOAT	sum	= 0.0;

	for (int i = 0; i < filterSize; i++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize / 2)) / (i - filterSize / 2);

	   tmp [i] *= (0.42 -
	           0.5 * cos (2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize) +
	           0.08 * cos (4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i++)
	   filterKernel [i] = DSPCOMPLEX (tmp [i] / sum, tmp [i]);
}

DSPCOMPLEX *DecimatingFIR::getKernel () {
	return filterKernel;
}

	DecimatingFIR::DecimatingFIR (int firSize,
	                              int32_t low,
	                              int32_t high,
	                              int32_t fs,
	                              int Dm):Basic_FIR (firSize) {
	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low, high);
}

void	DecimatingFIR::newKernel (int32_t low, int32_t high) {
DSPFLOAT	*tmp	= (DSPFLOAT *)alloca (filterSize * sizeof (DSPFLOAT));
DSPFLOAT	lo	= (DSPFLOAT)((high - low) / 2) / sampleRate;
DSPFLOAT	shift	= (DSPFLOAT)((high + low) / 2) / sampleRate;
DSPFLOAT	sum	= 0.0;

	for (int i = 0; i < filterSize; i++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize / 2)) / (i - filterSize / 2);
//
//	windowing
	   tmp [i] *= (0.42 -
	           0.50 * cos (2 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize) +
	           0.08 * cos (4 * M_PI * (DSPFLOAT)i / (DSPFLOAT)filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i++) {    // shifting
	   DSPFLOAT v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = DSPCOMPLEX (tmp [i] * cos (v) / sum,
	                                  tmp [i] * sin (v) / sum);
	}
}

	DecimatingFIR::~DecimatingFIR () {
}
//
//	The real cpu killer: this function is called once for every
//	sample that comes from the dongle. So, it really should be
//	optimized.
bool	DecimatingFIR::Pass (DSPCOMPLEX z, DSPCOMPLEX *z_out) {
DSPCOMPLEX	tmp	= 0;
int		index;

	Buffer [ip] = z;
	if (++decimationCounter < decimationFactor) {
	   ip = (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
//
//	we are working with a circular buffer, we take two steps
//	we move from ip .. 0 with i going from 0 .. ip -1
	for (int i = 0; i <= ip; i++) {
	   index = ip - i;
	   tmp  += Buffer [index] * filterKernel [i];
	}
//	and then we take the rest
	for (int i = ip + 1; i < filterSize; i++) {
	   index = filterSize + ip - i;
	   tmp  += Buffer [index] * filterKernel [i];
	}

	ip     = (ip + 1) % filterSize;
	*z_out = tmp;
	return true;
}

bool	DecimatingFIR::Pass (DSPFLOAT z, DSPFLOAT *z_out) {
	if (++decimationCounter < decimationFactor) {
	   Buffer [ip] = DSPCOMPLEX (z, 0);
	   ip = (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
	*z_out            = Basic_FIR::Pass (z);
	return true;
}


//====================================================================
/*
 *	The Hilbertfilter is derived from QEX Mar/April 1998
 *	to perform a 90 degree phase shift needed for (a.o)
 *	USB and LSB detection.
 */
	HilbertFilter::HilbertFilter (int fsize, DSPFLOAT f, int32_t rate) {
	firsize    	= fsize;
	this	-> rate = rate;
	cosKernel	= new DSPFLOAT [fsize];
	sinKernel	= new DSPFLOAT [fsize];
	Buffer		= new DSPCOMPLEX [fsize];
	adjustFilter(f);
}

	HilbertFilter::~HilbertFilter () {
	delete[]	cosKernel;
	delete[]	sinKernel;
	delete[]	Buffer;
}
/*
 *	the validity of the hilbertshift was validated
 *	by computing (and displaying)
 *	arg (res - DSPCOMPLEX (real (res), real (res)))
 *	where res = ... -> Pass (s1, s1)
 */
void	HilbertFilter::adjustFilter (DSPFLOAT centre) {
DSPFLOAT	*v1	= (DSPFLOAT *)alloca (firsize * sizeof (DSPFLOAT));
DSPFLOAT	sum = Blackman (v1, firsize, centre);

	for (int i = 0; i < firsize; i++)
	   v1 [i] = v1 [i] / sum;

	for (int i = 0; i < firsize; i++) {
	   DSPFLOAT omega = 2.0 * M_PI * centre;
	   cosKernel [i] = v1 [i] *
	             cos (omega * (i - ((DSPFLOAT)firsize - 1) / (2.0 * rate)));
	   sinKernel [i] = v1 [i] *
	             sin (omega * (i - ((DSPFLOAT)firsize - 1) / (2.0 * rate)));
	   Buffer [i] = 0;
	}

	ip = 0;
}

DSPCOMPLEX	HilbertFilter::Pass (DSPFLOAT a, DSPFLOAT b) {
	return Pass (DSPCOMPLEX (a, b));
}

DSPCOMPLEX	HilbertFilter::Pass (DSPCOMPLEX z) {
DSPCOMPLEX	tmp = 0;
DSPFLOAT	re, im;

	Buffer [ip] = z;
	ip = (ip + 1) % firsize;

	for (int i = 0; i < firsize; i++) {
	   int index = ip - i;
	   if (index < 0)
	      index += firsize;
	   re = real (Buffer [index]);
	   im = imag (Buffer [index]);
	   tmp += DSPCOMPLEX (re * cosKernel [i], im * sinKernel [i]);
	}

	return tmp;
}

// taken from GnuRadio to calculate the needed tap size
/*static*/
int32_t	Basic_FIR::compute_ntaps (const double	iSamplFreq,
	                          const double	iTransitionWidth,
	                          const EWinType iWinType,
	                          const double	iBetaKaiser/*=6.76*/) {
auto	get_max_win_att = [&]() -> int32_t {
	switch (iWinType) {
	   case (WIN_HAMMING):		return 53; break;
	   case (WIN_HANN):		return 44; break;
	   case (WIN_BLACKMAN):		return 74; break;
	   case (WIN_RECTANGULAR):	return 21; break;
	   case (WIN_KAISER):		return (iBetaKaiser/0.1102 + 8.7); break;
	   case (WIN_BLACKMAN_HARRIS):	return 92; break;
	   case (WIN_BARTLETT):		return 27; break;
	   case (WIN_FLATTOP):		return 93; break;
	   default:
	      throw std::out_of_range ("window::max_attenuation: unknown window type provided.");
	}
  };

const double a		= get_max_win_att ();
int ntaps		= (int)(a * iSamplFreq / (22.0 * iTransitionWidth));
	if ((ntaps & 1) == 0)
	   ntaps++; // ...make odd
	return ntaps;
}
