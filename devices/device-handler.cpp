#
/*
 *    Copyright (C) 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fmreceiver
 *
 *    fmreceiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fmreceiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 	Default (void) implementation of
 * 	virtual input class
 */
#include	"device-handler.h"

	deviceHandler::deviceHandler () {
	lastFrequency	= Mhz (100);
}

	deviceHandler::~deviceHandler () {
}

int32_t	deviceHandler::getRate	() {
	return KHz (192);
}

void	deviceHandler::setVFOFrequency (int32_t f) {
	lastFrequency = f;
}

int32_t	deviceHandler::getVFOFrequency	() {
	return lastFrequency;
}


int32_t	deviceHandler::defaultFrequency	() {
	return Khz (94700);
}

bool	deviceHandler::legalFrequency	(int freq) {
	return (MHz (86) <= freq) && (freq <= MHz (110));
}

bool	deviceHandler::restartReader	() {
	return true;
}

void	deviceHandler::stopReader	() {
}

int32_t	deviceHandler::getSamples	(DSPCOMPLEX *v, int32_t amount) {
	(void)v; 
	(void)amount; 
	return 0;
}

int32_t	deviceHandler::getSamples	(DSPCOMPLEX *v, int32_t amount, uint8_t M) {
	(void)M;
	(void)v; 
	(void)amount; 
	return 0;
}

int32_t	deviceHandler::Samples		() {
	return 0;
}

void	deviceHandler::resetBuffer	() {
}

int16_t	deviceHandler::bitDepth		() {
	return 10;
}

