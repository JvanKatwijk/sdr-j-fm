#
//
//	For the different formats for input, we have
//	different readers, with one "mother" reader.
//	Note that the cardreader is quite different here
//
//	This is the - almost empty - default implementation
#include	"virtual-reader.h"

	virtualReader::virtualReader	(RingBuffer<DSPCOMPLEX> *p) {
	theBuffer	= p;
	blockSize	= -1;
}

	virtualReader::~virtualReader		(void) {
}

void	virtualReader::restartReader	(int32_t s) {
	fprintf (stderr, "Restart met block %d\n", s);
	blockSize	= s;
}

void	virtualReader::stopReader	(void) {
}

void	virtualReader::processData	(float IQoffs, void *data, int cnt) {
	(void)IQoffs;
	(void)data;
	(void)cnt;
}

int16_t	virtualReader::bitDepth	(void) {
	return 12;
}

