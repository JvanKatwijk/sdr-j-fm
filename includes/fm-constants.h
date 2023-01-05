#
/*
 *    Copyright (C)  2011, 2012, 2013
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm software
 *
 *    fm software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fm software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#ifndef __FM_CONSTANTS__
#define	__FM_CONSTANTS__

#include	<math.h>
#include	<complex>
#include	<stdint.h>
#include	<limits>
#include	<stdlib.h>
#include	<unistd.h>

#define	QT_STATIC_CONST

/*
Stereo sideband test (after activating compile switch DO_STEREO_SEPARATION_TEST)
  1) Select "S|S" to hear only the sideband signal
  2) Check different station with the IQ balance slider whether the signal get minimal at center "0" position
*/
//#define	DO_STEREO_SEPARATION_TEST

#ifndef __FREEBSD__
#include	<malloc.h>
#endif

#ifdef __MINGW32__
#include	"windows.h"
#else
#ifndef	__FREEBSD__
#include	"alloca.h"
#endif
#include	"dlfcn.h"
typedef	void	*HINSTANCE;
#endif

 
#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif
using namespace std;

//
typedef	float	DSPFLOAT;
//
//
typedef	std::complex<DSPFLOAT>	DSPCOMPLEX;

using namespace std;

#define	Hz(x)		(x)
#define	Khz(x)		(x * 1000)
#define	KHz(x)		(x * 1000)
#define	Mhz(x)		(Khz (x) * 1000)
#define	MHz(x)		(KHz (x) * 1000)
/*
 */
#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

#define	IandQ		0100
#define	QandI		0101
#define I_Only		0102
#define	Q_Only		0104

#define	CURRENT_VERSION	"3.0"

#define	MSECFORTIMER	10
//
//	a ruleof thumb is that the attenuation of a FIR filter
//	is N_taps * (22 * Bt), where Bt = (Fstop - Fpass) /Fsampling
//	(or N_taps = atten / (22 * Bt)
//	For the pilot filter we need an atten of 30  dB,  with an
//	extremely small transition band 
//	For a transition band of 100 Hz and samplingrate 192000, we
//	need 30 / (22 * 1 / 1920) = 2618
//	For a filtersize of 500 we have (at 30 dB)  a transitionband
//	of 30 / (22 * 500), i.e. 0.027, i.e.  192000 * 0.027 = 5.2 KHz
//	or 1.7 Khz with an attenuation of 10 dB
#define	PILOTFILTER_SIZE	(2 * 384)
#define	FFT_SIZE		(2 * 16384)
#define	PILOT_WIDTH		(1200)

/* tomneda: there is a gap of 4 KHz between the stereo diff. signal
*  38+15 = 53kHz and 57kHz RDS signal
*  As I understand the RDS standard the 2400 Hz filter is seen on the
*  downmix side (base band). We use the filter on the MUX signal
*  (around 57 KHz) so the bandwidth has to be seen double.
*  But the Feilen RDS is working a bit more worse with that but it
*  needs also a correct decoding phase (so I provided the PSS
*  for stereo decoding)
*  My RDS is not working well with only 2600/2 bandwith.
*/
#define	RDS_WIDTH		(2 * 2400)
//#define	RDS_WIDTH		(2600)
//
//	common functions
static inline
bool	isIndeterminate (DSPFLOAT x) {
	return x != x;
}

static inline
bool	isInfinite (double x) {
	return x == numeric_limits<DSPFLOAT>::infinity ();
}
//
static inline
DSPCOMPLEX cmul (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) * y, imag (x) * y);
}

static inline
DSPCOMPLEX cdiv (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) / y, imag (x) / y);
}

static inline
float	get_db (DSPFLOAT x, int32_t y) {
	return 20 * log10 ((x + 1) / (float)(y));
}
//
static	inline
DSPFLOAT	PI_Constrain (DSPFLOAT val) {
	if (0 <= val && val < 2 * M_PI)
	   return val;
	if (val >= 2 * M_PI)
	   return fmod (val, 2 * M_PI);
//	apparently val < 0
	if (val > - 2 * M_PI)
	   return val + 2 * M_PI;
	return 2 * M_PI - fmod (- val, 2 * M_PI);
}
#endif

