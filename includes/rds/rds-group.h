/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm receiver
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __RDSGROUP_H
#define __RDSGROUP_H

#include	"fm-constants.h"

class RDSGroup {
public:
		RDSGroup		();
		~RDSGroup		();
	enum RdsBlock {
		BLOCK_A = 0,
		BLOCK_B = 1,
		BLOCK_C = 2,
		BLOCK_D = 3
		};

	enum GroupType {
		BASIC_TUNING_AND_SWITCHING   =  0,
		SLOW_LABELING_CODES          =  1,
		RADIO_TEXT                   =  2,
		OPEN_DATA                    =  3,
		CLOCKTIME_AND_DATE           =  4,
		ODA                          =  5,
		ODA_2                        =  6,
		RADIO_PAGING                 =  7,
		TMC_DATA                     =  8,
		EMERGENCY_WARNING            =  9,
		PROGRAM_TYPE_NAME            = 10,
		ODA_3                        = 11,
		ODA_4                        = 12,
		ENHANCED_PAGING              = 13,
		ENHANCED_OTHER_NETWORKS_INFO = 14
		};

void		clear			();
uint16_t	getBlock		(RdsBlock) const;
uint16_t	getBlock_A		() const;
uint16_t	getBlock_B		() const;
uint16_t	getBlock_C		() const;
uint16_t	getBlock_D		() const;
void		setBlock		(RdsBlock, uint16_t);
uint16_t	getPiCode		();
uint16_t	getGroupType		();
bool		isTypeBGroup		();
bool		isTpFlagSet		();
uint16_t	getProgrammeType	();

static const uint8_t NUM_RDS_GROUPS	= 16;
static const uint8_t NUM_BLOCKS_PER_RDSGROUP	= BLOCK_D + 1;
private:
uint16_t	rdsBlocks [NUM_BLOCKS_PER_RDSGROUP];
};
#endif
