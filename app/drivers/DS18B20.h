//*****************************************************************************
// 
//! \file DS18B20.h
//! \brief Macros used when accessing the DS18B20 control hardware.
//! \version V2.1.1.0
//! \date 6. Nov. 2011
//! \author  CooCox
//!
//! \copy
//!
//! Copyright (c)  2011, CooCox 
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#ifndef __DS18B20_H__
#define __DS18B20_H__

#include <stdio.h>
#include "stm32f10x_gpio.h"

#define DS18B20_SEARCH_ROM_EN   0

class DS18B20
{
public:
// Structure in which temperature is stored
typedef struct {
	short integer;
	unsigned int fractional;
	bool is_valid;
} simple_float;
private:


//
//! DS18B20 9 Bits MAX CONVERSION TIME is 93.75ms (tCONV/8)
// 
static const char DS18B20_9BIT		= 0x1F;
//
//! DS18B20 10 Bits MAX CONVERSION TIME is 187.5ms (tCONV/4)
// 
static const char DS18B20_10BIT	= 0x3F;
//
//! DS18B20 11 Bits MAX CONVERSION TIME is 375ms (tCONV/2)
// 
static const char DS18B20_11BIT	= 0x5F;
//
//! DS18B20 12 Bits MAX CONVERSION TIME is 750ms (tCONV)
// 
static const char DS18B20_12BIT	= 0x7F;

//
//! DS18B20_Family Family number
//
static const char DS18B20_FAMILY_NUM	= 0x28;

//
//! DS18B20_ROM ROM COMMANDS
//
static const char DS18B20_SEARCH		= 0xF0;
static const char DS18B20_READ			= 0x33;

static const char DS18B20_MATCH		= 0x55;
static const char DS18B20_SKIP			= 0xCC;

static const char DS18B20_ALARM_SEARCH	= 0xEC;

//
//! DS18B20_FUNCTION_COMMANDS DS18B20 FUNCTION COMMANDS
//
static const char DS18B20_CONVERT			= 0x44;
static const char DS18B20_WRITE_SCRATCHPAD	= 0x4E;

static const char DS18B20_READ_SCRATCHPAD	= 0xBE;
static const char DS18B20_COPY_SCRATCHPAD	= 0x48;

static const char DS18B20_RECALL			= 0xB8;
static const char DS18B20_READ_POWER_SUPPLY = 0xB4;

//
//! DS18B20 DIO port
//
uint32_t ulRCCPort;
//
//! DS18B20 DIO port
//
GPIO_TypeDef * ulPort;
//
//! DS18B20 DIO pin
//
uint16_t ulPin;
//
//! Precision set
//
unsigned char ucPrecision;

GPIO_InitTypeDef  GPIO_InitStructure;

// Static variables
#if (DS18B20_SEARCH_ROM_EN > 0)
//
// global search state
//
static unsigned char ucROM[8];
static int LastDiscrepancy;
static int LastFamilyDiscrepancy;
static int LastDeviceFlag;
static unsigned char ucCrc8;
#endif

unsigned char BitRead();
unsigned char ByteRead();
void BitWrite(unsigned char ucBit);
void ByteWrite(unsigned char ucByte);
public:
//
//! DS18B20 ROM
//
unsigned char ucROM[8];

bool Init(uint32_t _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin);
bool Reset();
void ROMRead();
void ROMMatch();
void ROMSkip();
void AlarmSearch();
#if (DS18B20_SEARCH_ROM_EN > 0)
xtBoolean ROMSearch();
xtBoolean Verify();
void TargetSetup(unsigned char ucFamily);
void FamilySkipSetup(unsigned char ucFamily);
#endif
void TempConvert();
void ScratchpadCopy();
void ScratchpadSet(char cHigh, char cLow, unsigned char ucBitConfig);
void SetPrecision(unsigned char precision);
void TempRead(float *pfTemp);
void TempReadSimple(simple_float *pfTemp);
void ScratchpadRead(unsigned char *pucTemp);
void EEROMRecall();
bool PowerSupplyRead();

};

#endif

















