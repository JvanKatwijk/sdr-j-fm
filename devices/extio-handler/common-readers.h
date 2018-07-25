#
//
//	For the different formats for input, we have
//	different readers, with one "mother" reader.
//	Note that the cardreader is quite different here
#ifndef	__COMMON_READERS
#define	__COMMON_READERS

#include	"virtual-reader.h"

class	reader_16: public virtualReader {
public:
	reader_16	(RingBuffer<DSPCOMPLEX> *p, int32_t);
	~reader_16	(void);
void	processData	(float IQoffs, void *data, int cnt);
int16_t bitDepth	(void);
};

class	reader_24: public virtualReader {
public:
	reader_24	(RingBuffer<DSPCOMPLEX> *p, int32_t);
	~reader_24	(void);
void	processData	(float IQoffs, void *data, int cnt);
int16_t bitDepth	(void);
};

class	reader_32: public virtualReader {
public:
	reader_32	(RingBuffer<DSPCOMPLEX> *p, int32_t);
	~reader_32	(void);
void	processData	(float IQoffs, void *data, int cnt);
int16_t	bitDepth	(void);
};


class	reader_float: public virtualReader {
public:
	reader_float	(RingBuffer<DSPCOMPLEX> *p);
	~reader_float	(void);
void	processData	(float IQoffs, void *data, int cnt);
int16_t	bitDepth	(void);
};

#endif

