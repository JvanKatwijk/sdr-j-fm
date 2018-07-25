#
/*
 *    Copyright (C) 2010, 2011, 2012
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
 *
 * 	Elements of this code are adapted from:
 *	1. mbed SI570 Library, for driving the SI570 programmable VCXO
 *      Copyright (c) 2010, Gerrit Polder, PA3BYA
 *	2. control a PMSDR hardware on Linux
 *	Modified to comply to firmware version 2.1.x
 *	Copyright (C) 2008,2009,2010  Andrea Montefusco IW0HDV,
 *	Martin Pernter IW3AUT
 */

#include	<QHBoxLayout>
#include	<QLabel>
#include	<QSettings>
#include	<QtPlugin>
#include	<QMessageBox>
#include	"pmsdr.h"
#include	"pa-reader.h"

//these must be floating point number especially 2^28 so that 
//there is enough memory to use them in the calculation 
#define POW_2_16         65536.0 
#define POW_2_24         16777216.0 
#define POW_2_28         268435456.0 
#define FOUT_START_UP	 56.320       //MHz 
#define FXTAL_DEVICE	 114.27902015
#define	PPM		 3500

#ifndef I2C_SLA_SI570
#define I2C_SLA_SI570     0x55
#endif
 
#ifndef STARTUP_FREF
#define STARTUP_FREF   56320000UL
#endif

static const float FDCO_MAX 	= 5670; //MHz 
static const float FDCO_MIN 	= 4850; //MHz 
static const unsigned char HS_DIV[6] = {11, 9, 7, 6, 5, 4};
static const float fout0 	= FOUT_START_UP;
static const float SI570_FOUT_MIN = 10; //MHz

	pmsdrHandler::pmsdrHandler (QSettings *s, bool *success) {
	this	-> pmsdrSettings	= s;
	*success			= false;
//
//	inputrate is fixed here for FM
	this	-> inputRate	= Khz (192);
	myFrame			= new QFrame;
	setupUi	(myFrame);
	myFrame	 -> show ();
//
	radioOK		= false;
	vfoOffset	= pmsdrSettings -> value ("pmsdr-vfoOffset",
	                                           0). toInt ();
	setup_device ();
	if (pmsdrDevice == NULL) {	// something failed
	   statusLabel	-> setText ("No valid Device");
	   return;
	}
//
//	Initial settings
	qsdBfig. QSDmute	= false;
	pmsdrDevice -> set_qsd_bias (&qsdBfig, 0);
	setGain		(gainSlider	-> value ());
	setBias		(biasSlider	-> value ());
//
//	and the connects
	offset_KHz	-> setValue (vfoOffset);
	connect (muteButton, SIGNAL (clicked (void)),
	         this, SLOT (setMute (void)));
	connect (gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setGain (int)));
	connect (biasSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setBias (int)));
	connect (offset_KHz, SIGNAL (valueChanged (int)),
	         this, SLOT (set_offset_KHz (int)));
	statusLabel	-> setText ("pmsdr ready");
	myReader	= new paReader (inputRate, cardSelector);
	connect (cardSelector, SIGNAL (activated (int)),
	         this, SLOT (set_StreamSelector (int)));
	radioOK		= true;
	*success	= true;
}

	pmsdrHandler::~pmsdrHandler (void) {
	if (pmsdrDevice != NULL) {
	   delete pmsdrDevice;
	   delete myReader;
	}
	pmsdrSettings	-> setValue ("pmsdr-vfoOffset", vfoOffset);
	delete	myFrame;
}

void	pmsdrHandler::setVFOFrequency (int32_t VFO_freq) {
int32_t	realFrequency	= VFO_freq + vfoOffset;

	if (!radioOK)
	   return;
	
	if (realFrequency < 0)
	   return;

	if (realFrequency > Mhz (55)) {
	   thirdHarmonic	= true;
	   realFrequency /= 3;
	}
	else
	   thirdHarmonic	= false;

	ComputeandSetSi570 ((double) realFrequency/1000000.0);

//	read back the si570 and set the si570 soft registers
	ReadandConvertRegisters (RD_SI570);
	LOfreq	= (int32_t) (1000000 *
	                     (si570pmsdr. rfreq	* si570pmsdr. fxtal) /
	                     (si570pmsdr. hsdiv * si570pmsdr. n1));
	LOfreq	= postdivider == 0 ? LOfreq : LOfreq / (4 * postdivider);
	fprintf (stderr, "asked for %d, got %d\n",
	          (int32_t)VFO_freq, (int32_t)LOfreq);
//	if (automaticFiltering && !thirdHarmonic)
//	   setFilterAutomatic (realFrequency);
}

int32_t	pmsdrHandler::getVFOFrequency (void) {
	if (!radioOK)
	   return Khz (94700);

	if (thirdHarmonic)		// likely when processing fm
	   return 3 * LOfreq - vfoOffset ;
	else
	   return LOfreq - vfoOffset;
}

bool	pmsdrHandler::legalFrequency (int32_t Freq) {
	return ((Khz (100) <= Freq) && (Freq <= 155000000));
}

int32_t	pmsdrHandler::defaultFrequency (void) {
	return Khz (95700);
}
//
int32_t	pmsdrHandler::getRate		(void) {
	return Khz (192);
}

int16_t	pmsdrHandler::bitDepth		(void) {
	return 16;		// has to be reconsidered !!!
}

bool	pmsdrHandler::restartReader	(void) {
	if (!radioOK)
	   return false;
	return myReader	-> restartReader ();
}

void	pmsdrHandler::stopReader	(void) {
	if (!radioOK)
	   return;

	return myReader	-> stopReader ();
}

int32_t	pmsdrHandler::Samples		(void) {
	if (!radioOK)
	   return 0;
	return myReader	-> Samples ();
}

int32_t	pmsdrHandler::getSamples	(DSPCOMPLEX *b, int32_t a, uint8_t m) {
	if (!radioOK)
	   return 0;
	return myReader -> getSamples (b, a, m);
}

int32_t	pmsdrHandler::getSamples	(DSPCOMPLEX *b, int32_t a) {
	if (!radioOK)
	   return 0;
	return myReader -> getSamples (b, a, IandQ);
}

void	pmsdrHandler::set_StreamSelector (int idx) {
	fprintf (stderr, "Setting stream %d\n", idx);
	if (!myReader	-> set_StreamSelector (idx))
	   QMessageBox::warning (myFrame, tr ("sdr"),
	                               tr ("Selecting  input stream failed\n"));
}

uint8_t	pmsdrHandler::myIdentity	(void) {
	return PMSDR;
}

///////////////////////////////////////////////////////////////////////
void	pmsdrHandler::setup_device	(void) {
bool	fFound;

	qsdBfig			= qsdBfigDefault;
	fLcd			= false;
	LOfreq			= KHz (14070);
	thirdHarmonic		= false;

/*
 */
	pmsdrDevice	= new pmsdr_comm ();
	if (!pmsdrDevice -> pmsdr_isWorking ()) {
	   fprintf (stderr, "No valid device found (%s)\n",
	                     pmsdrDevice -> getError ());
//	   delete pmsdrDevice;
	   pmsdrDevice	= NULL;
	   return;
	}

	pmsdrDevice	-> get_version (&usbFirmwareMajor,
	                                &usbFirmwareMinor,
	                                &usbFirmwareSub);
//
// Check for Si570 and CY22393 on I2C internal bus.
//
        if ( ((usbFirmwareMajor == 2) && (usbFirmwareMinor >= 2)) ||
	     ((usbFirmwareMajor >= 2) && (usbFirmwareMinor >= 1) &&
	      (usbFirmwareSub >= 7 ))) {
	   fFound = false;
			
	   if (pmsdrDevice -> ScanI2C (0x55, &fFound, ScanI2C_PMSDR))
	      fprintf (stderr, "Si570 %s\n", fFound? "detected": "not found");
           else {
	      fprintf (stderr, "Error searching for Si570 found !\n");
	   }
	   if (!fFound) {
	      fprintf (stderr, "Sorry, without Si570 we cannot do much\n");
	      return;
	   }
//
//	Now for the CY22393
//
	   fFound = false;
	   if (pmsdrDevice -> ScanI2C (0x69, &fFound, ScanI2C_PMSDR)) 
	      fprintf (stderr, "CY22393 %s\n", fFound? "detected": "not found");
	   else 
	      fprintf (stderr, "Error searching for CY22393 !\n");
	}
//
//	reset the Si570, get the registers .....
        pmsdrDevice -> WriteI2C (I2C_SLA_SI570, 135, 0x01, WriteI2C_PMSDR);
	pmsdrDevice -> ReadSi570 (si570_reg, RD_SI570);
	postdivider = si570_reg [6];
	calc_si570registers (si570_reg, &si570pmsdr);

//	and calculate the fxtal
	si570pmsdr. fxtal =
	    (fout0 * si570pmsdr. n1 * si570pmsdr. hsdiv) /
	             si570pmsdr. rfreq; //MHz
	si570pmsdr. currentFreq	= fout0;

	fprintf (stderr, "Crystal frequency found in this si570 %f\n", 
	                                si570pmsdr. fxtal);

	automaticFiltering	= false;
	currentFilter		= 0;
	setFilter	(currentFilter);
//	initialize with a low frequency
	ComputeandSetSi570 ((double) 14070000/1000000.0);

//	read back the si570 and set the si570 soft registers
	ReadandConvertRegisters (RD_SI570);
	LOfreq	= (int32_t) (1000000 *
	                     (si570pmsdr. rfreq	* si570pmsdr. fxtal) /
	                     (si570pmsdr. hsdiv * si570pmsdr. n1));
	LOfreq	= postdivider == 0 ? LOfreq : LOfreq / (4 * postdivider);
//	fprintf (stderr, "asked for %d, got %d\n",
//	          (int32_t)VFO_freq, (int32_t)LOfreq);
}
//
//
bool	pmsdrHandler::ReadandConvertRegisters (int8_t choice) {
//	for now
	(void)choice;

	if (!pmsdrDevice -> ReadSi570 (si570_reg, RD_SI570)) 
	   return false;

	postdivider = si570_reg [6];
	calc_si570registers (si570_reg, &si570pmsdr);
	return true;
}
//
void	pmsdrHandler::calc_si570registers (uint8_t* reg,
	                            si570data* si570struct) {
	// HS_DIV conversion 
	si570struct -> hsdiv =
	               ((reg [0] & 0xE0) >> 5) + 4; // get reg 7 bits 5, 6, 7
	// hsdiv's value could be verified here to ensure that it is one
	// of the valid HS_DIV values from the datasheet. 
	// n1 conversion 
	si570struct -> n1 =
	               ((reg [0] & 0x1F ) << 2) + // get reg 7 bits 0 to 4 
	               ((reg [1] & 0xC0 ) >> 6);  // add with reg 8 bits 7 and 8 

	if (si570struct -> n1 == 0)
	   si570struct -> n1 = 1;
        else
	if ((si570struct -> n1 & 1) != 0) // add one to an odd number
	   si570struct -> n1 += 1;

	si570struct -> frac_bits = (reg[2] & 0xF) * POW_2_24; 
	si570struct -> frac_bits += reg [3] * POW_2_16;
	si570struct -> frac_bits += reg [4] * 256; 
	si570struct -> frac_bits += reg[5]; 
	
	si570struct -> rfreq = si570struct -> frac_bits; 
	si570struct -> rfreq /= POW_2_28;
	si570struct -> rfreq += ((reg [1] & 0x3F) << 4) + ((reg [2] & 0xF0) >> 4);
}

uint8_t pmsdrHandler:: SetBits (uint8_t value,
	                        uint8_t reset_mask, uint8_t new_val) { 
	return ((value & reset_mask ) | new_val); 
} 
//  
//	compute and set the frequency for the regular Si570
//
void	pmsdrHandler::ComputeandSetSi570 (double Frequency) { 
bool	smooth_tune;
uint16_t reg [8];

	smooth_tune	= false;	// until shown otherwise

	if ((usbFirmwareMajor >= 2 &&
             usbFirmwareMinor >= 1 && usbFirmwareSub >= 4) ||
	    (usbFirmwareMajor == 2 && usbFirmwareMinor >= 2) ) {
	   float diff = abs (Frequency -  si570pmsdr. currentFreq) / si570pmsdr. currentFreq;
	   smooth_tune = diff * 1000000 < PPM;
	}
	   
	reg [6] = 1;   // default set the CY2239X Postdivider = 1

	Frequency *= 4.0; // QSD Clock = 4 * frx

	// find first Frequency above SI570_FOUT_MIN
	while (Frequency < SI570_FOUT_MIN) {
	   reg [6] *= 2;       // set the CY2239X Postdivider
	   Frequency *= 2.0;
	}

	postdivider = reg [6];
	if (smooth_tune) { 
	   setFrequencySmallChange (&si570pmsdr, Frequency, reg);
//	and update registers in hardware
	   pmsdrDevice -> SetSmoothSi570 (reg, 8, SET_SMOOTHSI570);
	   si570pmsdr. currentFreq = Frequency;
	}
	else {
	   setFrequencyLargeChange (&si570pmsdr, Frequency, reg);
//	and update the hardware registers
	   pmsdrDevice -> SetSi570 (reg, 8, SET_SI570);
	   si570pmsdr. currentFreq = Frequency;
	}
}

//	Find dividers (get the max and min divider range for
//	the HS_DIV and N1 combo)
bool	pmsdrHandler::computeDividers (double Frequency,
	                               uint8_t *hsdiv, uint8_t *n1) {
int32_t	divider_max;
int32_t	curr_div;
int16_t	counter;
int16_t	curr_hsdiv;
double	curr_n1;
double	n1_tmp;
uint8_t	local_n1;

	divider_max	= floor (FDCO_MAX / Frequency);	// floorf for SDCC 
	curr_div	= ceil (FDCO_MIN / Frequency);	//ceilf for SDCC 

	while (curr_div <= divider_max) { 
	   // check all HS_DIV values with the next curr_div 
	   for (counter = 0; counter < 6; counter++) { 
// get the next possible n1 value 
	      curr_hsdiv = HS_DIV [counter]; 
	      curr_n1 = double (curr_div) / double (curr_hsdiv); 
	      // determine if curr_n1 is an integer and an even number or one
	      // then curr_div % (curr_n1 * hsdiv) == 0
	      n1_tmp = curr_n1 - floor (curr_n1); 
	      if (n1_tmp == 0.0) { 
	         // then curr_n1 has zero as fraction and equals an integer 
	         local_n1 = (uint8_t) curr_n1; 
	         if ((local_n1 == 1) ||
	             ((local_n1 & 1) == 0))  {
//	then the calculated N1 is either 1 or an even number and we are through
	             *n1 = local_n1;
	             *hsdiv = curr_hsdiv;
	             return true;
	         }
	      }
	   }	// we end the forloop 

	   // increment curr_div to find the next divider 
	   // since the current one was not valid 
	   curr_div = curr_div + 1; 
	} 
	return false;
}

void	pmsdrHandler::convertFloattoHex (si570data *si570, uint16_t *reg) {
int16_t		counter;
uint32_t	whole;
uint32_t	fraction;

//	first separate fraction and integer part
	whole = floor (si570 -> rfreq);

// get the binary representation of the fractional part
	fraction = floor ((si570 -> rfreq - whole) * POW_2_28);
//
//	12 - 8 =  5 - 1
//	set reg 12 to 10 (i.e. the top 24 bits),
	for (counter = 5; counter >= 3; counter--){
	   reg [counter] = fraction & 0xFF;
	   fraction = fraction >> 8;
	}

// set the last 4 bits of the fractional portion  are set in reg 9
	reg [2] = SetBits (reg [2], 0xF0, (fraction & 0xF));

// set the integer portion of RFREQ across reg 8 and 9
	reg [2] = SetBits (reg [2], 0x0F, (whole & 0xF) << 4);
	reg [1] = SetBits (reg [1], 0xC0, (whole >> 4) & 0x3F);
}

//	easy: computing the parameters for the Si570
//	in case of a small change
//
bool	pmsdrHandler::setFrequencySmallChange (si570data *si570,
	                                       double Frequency,
	                                       uint16_t *reg) {
//	New Rfreq is easy to compute:
	si570 -> rfreq = si570 -> rfreq * Frequency /si570 -> currentFreq;
//
//	convert new RFREQ to the binary representation
//	in the software registerset
//	The algorithm to do so can be found on
//	data sheet.
	memset (reg, 0, 6 * sizeof (uint16_t));
	convertFloattoHex (si570, reg);
//
	return true;
}

bool	pmsdrHandler::setFrequencyLargeChange (si570data *si570,
	                                       double Frequency,
	                                       uint16_t *reg) {
uint8_t	local_hsdiv, local_n1;

	if (!computeDividers (Frequency, &local_hsdiv, &local_n1)) {
	   fprintf (stderr, "FreqProgSi570: error in calculation !!! \n");
	   return false;
	}
//
//	we have a new n1 and hsdiv, so we can compute a new rfreq
	si570 -> rfreq =
	   (Frequency * local_hsdiv * local_n1) / si570 -> fxtal; 
//
//	clear registers
	memset (reg, 0, 6 * sizeof (uint16_t));

//	new HS_DIV conversion 
	local_hsdiv =  local_hsdiv - 4; 
//	reset this memory 
	reg [0] = 0; 

//	set the top 3 bits of reg 13 
	reg [0] = local_hsdiv << 5; 
//	convert new N1 to the binary representation 
	if (local_n1 == 1)
	   local_n1 = 0;
	else
	if ((local_n1 & 1) == 0)
	   local_n1 = local_n1 - 1;
//	set reg 7 bits 0 to 4 
	reg [0] = SetBits (reg [0], 0xE0, local_n1 >> 2); 

//	set reg 8 bits 6 and 7 
	reg [1] = (local_n1 & 3) << 6; 
//	convert new RFREQ to the binary representation 

	convertFloattoHex (si570, reg);
	return true;
}
//
//	Two functions for writing and reading (back) data
//	into and from the EEPROM. Currently not used by
//	jsdr software

bool	pmsdrHandler::store_hwlo (int32_t newFreq) {
int16_t	i;
// write the soft SI570 registers into the EEPROM
	for (i = 0; i < 6; i++)
	   if (!pmsdrDevice -> WriteE2P (SI570_REG_ADR+i, si570_reg [i]))
	      return false;
	if (!pmsdrDevice -> WriteE2P (POST_DIV_ADR, si570_reg [6]))
	   return false;
	if (!pmsdrDevice -> WriteE2P (DEF_FILTER_ADR, qsdBfig.filter))
	   return false;

//	write the current generated frequency into the EEPROM
//	it is useful only to display the frequency on hardware startup;
//	in this way the PMSDR micro doesn't need to do heavy computation

	if (!pmsdrDevice -> WriteE2P (DEFAULT_FRQ_ADR + 0, (newFreq & 0x000000FF)))
	   return false;
	if (!pmsdrDevice -> WriteE2P (DEFAULT_FRQ_ADR + 1, (newFreq & 0x0000FF00) >> 8))
	   return false;
	if (!pmsdrDevice -> WriteE2P (DEFAULT_FRQ_ADR + 2, (newFreq & 0x00FF0000) >> 16))
	   return false;
	if (!pmsdrDevice -> WriteE2P (DEFAULT_FRQ_ADR + 3, (newFreq & 0xFF000000) >> 24))
	   return false;

	return true;
}

bool	pmsdrHandler::read_hwlo (int32_t *pNewFreq) {
uint8_t	x;
int16_t	i;
int32_t	tmp	= 0;

	*pNewFreq = 0;
	// read the SI570 registers from EEPROM
	for (i = 0; i < 6; i++) 
	   if (!pmsdrDevice -> ReadE2P (SI570_REG_ADR + i, &si570_reg [i]))
	      return false;
	if (!pmsdrDevice -> ReadE2P (POST_DIV_ADR, &si570_reg [6]))
	   return false;
//	if (!WriteE2P (DEF_FILTER_ADR, qsdBfig. filter))
//	   return false;
	if (!pmsdrDevice -> ReadE2P (DEF_FILTER_ADR, &qsdBfig. filter))
	   return false;

	// read the current generated frequency into the EEPROM
	if (!pmsdrDevice -> ReadE2P (DEFAULT_FRQ_ADR + 3, &x))
	   return false;
	tmp = tmp + x ; 
	if (!pmsdrDevice -> ReadE2P (DEFAULT_FRQ_ADR + 2, &x))
	   return false;
	tmp = (tmp << 8) + x ; 
	if (!pmsdrDevice -> ReadE2P (DEFAULT_FRQ_ADR + 1, &x))
	   return false;
	tmp =  (tmp << 8) + x ; 
	if (!pmsdrDevice -> ReadE2P (DEFAULT_FRQ_ADR + 0, &x))
	   return false;
	tmp =  (tmp << 8) + x ; 
	*pNewFreq = tmp;

	return true;
}
//
void	pmsdrHandler::setFilter	(int16_t filter) {
	if (!radioOK || (filter < 0) || (filter > 4))
	   return;

	qsdBfig. filter	= filter;
	pmsdrDevice -> set_qsd_bias (&qsdBfig, 0);
}

void	pmsdrHandler::setFilterAutomatic (int32_t freq) {
	if (freq < Khz (2500)) {
	   setFilter  (4);
	}
	else
	if (freq < Khz (6000)) {
	   setFilter (1);
	}
	else
	if (freq < Khz (12000)) { 
	   setFilter (2);
	}
	else
	if (freq < Khz (24000)) {
	   setFilter (3);
	}
	else {
	   setFilter (0);
	}
}

void	pmsdrHandler::setMute	(void) {
	if (!radioOK)
	   return;
	qsdBfig. QSDmute	= !qsdBfig. QSDmute;
	pmsdrDevice -> set_qsd_bias (&qsdBfig, 0);
}
/*
 *	gain is a value between 0, 1, 2, 3 4, but only working for older 
 *	versions
 */
void	pmsdrHandler::setGain	(int gain) {
	if (!radioOK || gain < 0)
	   return;

	if (gain > 4)
	   gain = 4;
	qsdBfig. IfGain	= gain;
	pmsdrDevice -> set_qsd_bias (&qsdBfig, 0);
	gainDisplay	-> display (qsdBfig. IfGain);
}
/*
 *	parameter is an integer between -100 .. 100
 */
void	pmsdrHandler::setBias	(int bias) {
	if (!radioOK)
	   return;

	qsdBfig. QSDbias	= 512 + bias * 3/2;
	pmsdrDevice -> set_qsd_bias (&qsdBfig, 0);
	biasDisplay	-> display (qsdBfig. QSDbias);
}

void	pmsdrHandler::set_offset_KHz	(int v) {
	vfoOffset	= Khz (v);
}

