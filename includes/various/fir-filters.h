/*
 *
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdr-j-fm
 *
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
 *    along with sdr-j-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef FIR_FILTERS_H
#define FIR_FILTERS_H

#include  "fm-constants.h"
#include  <stdlib.h>
#include  <math.h>

class HilbertFilter;

class	Basic_FIR {
public:
// taken from GnuRadio to calculate the needed tap size
	enum EWinType {
	   WIN_HAMMING = 0,         //!< Hamming window; max attenuation 53 dB
	   WIN_HANN = 1,            //!< Hann window; max attenuation 44 dB
	   WIN_BLACKMAN = 2,        //!< Blackman window; max attenuation 74 dB
	   WIN_RECTANGULAR = 3,     //!< Basic rectangular window; max attenuation 21 dB
	   WIN_KAISER = 4,          //!< Kaiser window; max attenuation see window::max_attenuation
	   WIN_BLACKMAN_HARRIS = 5, //!< alias to WIN_BLACKMAN_hARRIS for capitalization consistency
	   WIN_BARTLETT = 6,        //!< Barlett (triangular) window; max attenuation 26 dB
	   WIN_FLATTOP = 7,         //!< flat top window; useful in FFTs; max attenuation 93 dB
	};

static
int32_t		compute_ntaps (const double	iSamplFreq,
	                       const double	iTransitionWidth,
	                       const EWinType	iWinType,
	                       const double	iBetaKaiser = 6.76);

public:
	int		filterSize;
	DSPCOMPLEX	*filterKernel;
	DSPCOMPLEX	*Buffer;
	int		ip;
	int32_t		sampleRate;

	Basic_FIR (int size) {
	   filterSize	= size;
	   filterKernel = new DSPCOMPLEX [filterSize];
	   Buffer	= new DSPCOMPLEX [filterSize];
	   ip		= 0;

	   for (int i = 0; i < filterSize; i++) {
	      filterKernel [i] = 0;
	      Buffer [i]       = 0;
	   }
	}

	~Basic_FIR () {
	   delete[]	filterKernel;
	   delete[]	Buffer;
	}

//	we process the samples backwards rather than reversing
//	the kernel
	DSPCOMPLEX	Pass (DSPCOMPLEX z) {
	   DSPCOMPLEX tmp = DSPCOMPLEX (0, 0);
	   Buffer [ip] = z;

	   for (int i = 0; i < filterSize; i++) {
	      int index = ip - i;
	      if (index < 0)
	         index += filterSize;
	      tmp += Buffer [index] * filterKernel [i];
	   }

	   ip = (ip + 1) % filterSize;
	   return tmp;
	}

	DSPFLOAT	Pass (DSPFLOAT v) {
	   DSPFLOAT tmp = 0;
	   Buffer [ip] = DSPCOMPLEX (v, 0);

	   for (int i = 0; i < filterSize; i++) {
	      int index = ip - i;
	      if (index < 0)
	         index += filterSize;
	      tmp += real(Buffer [index]) * real(filterKernel [i]);
	   }

	   ip = (ip + 1) % filterSize;
	   return tmp;
	}
};

class	LowPassFIR : public Basic_FIR {
public:
		LowPassFIR (int,  // order
	                    int32_t,  // cutoff frequency
	                    int32_t   // samplerate
	                   );
		~LowPassFIR ();
	DSPCOMPLEX  *getKernel ();
	void	newKernel (int32_t);        // cutoff
};

//	Both for lowpass band bandpass, we provide:
class	DecimatingFIR: public Basic_FIR {
public:
		DecimatingFIR	(int, int32_t, int32_t, int);
		DecimatingFIR	(int, int32_t, int32_t, int32_t, int);
		~DecimatingFIR  ();

	void	newKernel	(int32_t);
	void	newKernel	(int32_t, int32_t);
	bool	Pass		(DSPCOMPLEX, DSPCOMPLEX *);
	bool	Pass		(DSPFLOAT, DSPFLOAT *);
	DSPCOMPLEX *getKernel	();

private:
	int	decimationFactor;
	int decimationCounter;
};

class	HighPassFIR: public Basic_FIR {
public:
		HighPassFIR	(int, int32_t, int32_t);
		~HighPassFIR	();
	void	newKernel	(int32_t);
};

class	BandPassFIR: public Basic_FIR {
public:
		BandPassFIR	(int, int32_t, int32_t, int32_t);
		~BandPassFIR	();

	DSPCOMPLEX *getKernel	();
	void	newKernel	(int32_t, int32_t);
private:
};

class	BasicBandPass: public Basic_FIR {
public:
		BasicBandPass	(int, int32_t, int32_t, int32_t);
		~BasicBandPass	();

	DSPCOMPLEX *getKernel	();
private:
};

class	adaptiveFilter {
public:
		 adaptiveFilter	(int);
		~adaptiveFilter ();
	void	adaptFilter	(DSPFLOAT);
	DSPCOMPLEX Pass		(DSPCOMPLEX);

private:
	int		ip;
	DSPFLOAT	err;
	DSPFLOAT	mu;
	int		firsize;
	DSPFLOAT	*Kernel;
	DSPCOMPLEX	*Buffer;
};

class	HilbertFilter {
public:
		HilbertFilter	(int, DSPFLOAT, int32_t);
		~HilbertFilter	();
	DSPCOMPLEX Pass		(DSPCOMPLEX);
	DSPCOMPLEX Pass		(DSPFLOAT, DSPFLOAT);

private:
	int		ip;
	int		firsize;
	int32_t		rate;
	DSPFLOAT	*cosKernel;
	DSPFLOAT	*sinKernel;
	DSPCOMPLEX	*Buffer;
	void		adjustFilter	(DSPFLOAT);
};

#endif
