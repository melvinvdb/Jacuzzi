//*****************************************************************************
//
//! \file DS18B20.c
//! \brief Driver for digital thermometer DS18B20.
//! \version V2.1.1.0
//! \date 10/18/2011
//! \author CooCox
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

#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "delay.h"
#include "DS18B20.h"

//*****************************************************************************
//
//! \brief Initializes the DS18B20 device.
//!
//! \param _ulRCCPort RCC Port to use example RCC_APB2Periph_GPIOA
//! \param _ulPort Port to use example GPIOA
//! \param _ulPin Pin to use example GPIO_Pin_0
//!
//! \return boolean indicating if a DS18B20 device has been found.
//
//*****************************************************************************
bool DS18B20::Init(unsigned long _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin)
{
	ulRCCPort = _ulRCCPort;
	ulPort = _ulPort;
	ulPin = _ulPin;
	bool found = false;

    //
    // Enable the GPIOx port which is connected with DS18B20 
    //
	RCC_APB2PeriphClockCmd(ulRCCPort, ENABLE);
    GPIO_InitStructure.GPIO_Pin = ulPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ulPort, &GPIO_InitStructure);

    GPIO_WriteBit(ulPort, ulPin, Bit_SET);
    
    //default 12 bit resolution
    ucPrecision = 3;

    //divide by 4
    //200us = 150us
    //100us = 80us
    //20us  = 18-19us
    //10us  = 11us
    //5us   = 9us
    //1us   = 6us
    //divide by 3
    //200us = 210us
    //100us = 104us
    //20us  = 23us
    //10us  = 14us
    //5us   = 9us
    //1us   = ~5us

    //
    // check
    //
    for (int i = 0; i < 3; i++)
    {
    	found = Reset();
    }
    
#if (DS18B20_SEARCH_ROM_EN > 0)
    //
    // reset the search state
    //
    LastDiscrepancy = 0;
    LastDeviceFlag = 0;
    LastFamilyDiscrepancy = 0;
#endif
    return found;
}

//*****************************************************************************
//
//! \brief Reset the DS18B20 device.
//!
//! \return None.
//
//*****************************************************************************
bool DS18B20::Reset()
{
    unsigned long i = 1;
    unsigned int retries = 125;
    //
    // DS18B20 dq pin be set as output
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ulPort, &GPIO_InitStructure);

    while (GPIO_ReadOutputDataBit(ulPort, ulPin) == 0)
    {
    	if (--retries == 0) return false;
    	DwtDelayUs(1);
    }

    //
    // DS18B20 dq_pin be set to low
    //
    GPIO_WriteBit(ulPort, ulPin, Bit_RESET);

    DwtDelayUs(480); //pull low min 480us @ page 3

    //
    // DS18B20 dq_pin be set to high
    //
    GPIO_WriteBit(ulPort, ulPin, Bit_SET);

    DwtDelayUs(70); //wait before reading, wait between 15-60us then presence pulse 60-240us @ page 3

    //
    // DS18B20 dq pin be set as input
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ulPort, &GPIO_InitStructure);

    i = GPIO_ReadInputDataBit(ulPort, ulPin);

    //
    // DS18B20 dq pin be set as output
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ulPort, &GPIO_InitStructure);
    
    DwtDelayUs(410); //480us - 70us previous = 410us @ page 3

    if(i)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//*****************************************************************************
//
//! \brief Read a bit from the DS18B20.
//!
//! \return 0 or 1.
//
//*****************************************************************************
unsigned char DS18B20::BitRead()
{
    unsigned char ucData = 0;

    //
    // DS18B20 dq pin be set as output
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ulPort, &GPIO_InitStructure);

    //
    // DS18B20 dq_pin be set to low
    //
    GPIO_WriteBit(ulPort, ulPin, Bit_RESET);

    DwtDelayUs(3); //initialize time slot (min 1us max 15us) default 6

    //
    // DS18B20 dq_pin be set to high
    //
    //GPIO_WriteBit(ulPort, ulPin, Bit_SET);

    //DwtDelayUs(3); //DS18B20 starts pulling bus with time slot max 15us default 3

    //
    // DS18B20 dq pin be set as input
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ulPort, &GPIO_InitStructure);
    DwtDelayUs(15); //initialize time slot is 1us-15us. we waited 3 + 15 = 18us so time to read. Read time slot is 45us

    if(GPIO_ReadInputDataBit(ulPort, ulPin))
    {
        ucData = 1;
    }
    else
    {
        ucData = 0;
    }

    DwtDelayUs(45); //save to pull up after 60us

    return ucData;
}

//*****************************************************************************
//
//! \brief Read a byte from the DS18B20.
//!
//! \return a byte of data.
//
//*****************************************************************************
unsigned char DS18B20::ByteRead()
{
    unsigned char i,ucData;
    ucData = 0;

    for (i=0;i<8;i++)
    {
        ucData = ucData >> 1;
        if(BitRead())
            ucData |= 0x80;
    }

    return ucData;
}

//*****************************************************************************
//
//! \brief Write a bit from the DS18B20.
//!
//! \return None.
//
//*****************************************************************************
void DS18B20::BitWrite(unsigned char ucBit)
{
	//
	// DS18B20 dq pin be set as output
	//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ulPort, &GPIO_InitStructure);

    if (ucBit)
    {
        //
        // DS18B20 dq_pin be set to low
        //
    	GPIO_WriteBit(ulPort, ulPin, Bit_RESET);
    	DwtDelayUs(5); //initialize time slot (min 1us max 15us) default 5
        //
        // DS18B20 dq_pin be set to high
        //
        GPIO_WriteBit(ulPort, ulPin, Bit_SET);
        DwtDelayUs(60); //sample window between 15us - 60us
    }
    else
    {
        //
        // DS18B20 dq_pin be set to low
        //
    	GPIO_WriteBit(ulPort, ulPin, Bit_RESET);
    	DwtDelayUs(65); //sample window between 15us - 60us default 65
        //
		// DS18B20 dq_pin be set to high
		//
        GPIO_WriteBit(ulPort, ulPin, Bit_SET);
        DwtDelayUs(5);
    }
}

//*****************************************************************************
//
//! \brief Write a byte from the DS18B20.
//!
//! \return None.
//
//*****************************************************************************
void DS18B20::ByteWrite(unsigned char ucByte)
{
    unsigned char i;

    for(i = 0;i<8 ;i++)
    {
        BitWrite(((ucByte>>i) & 0x1));
    }
}

#if (DS18B20_SEARCH_ROM_EN > 0)
//*****************************************************************************
//
//! \brief Perform the 1-Wire Search Algorithm on the 1-Wire bus using the 
//! existing search state.
//!
//! \return xTRUE  : device found, ROM number in ucROM buffer
//!         xFALSE : device not found, end of search
//
//*****************************************************************************
xtBoolean DS18B20ROMSearch()
{
    int IdBitNumber,i;
    int LastZero, ROMByteNumber, SearchResult;
    int IDBit, ComplementIDBitR;
    unsigned char ROMByteMask, SearchDirection;
    //
    // initialize for search
    //
    IdBitNumber = 1;
    LastZero = 0;
    ROMByteNumber = 0;
    ROMByteMask = 1;
    SearchResult = 0;
    ucCrc8 = 0;
    //
    // if the last call was not the last one
    //
    if (!LastDeviceFlag)
    {
        //
        // 1-Wire reset
        //
        if (Reset())
        {
            //
            // reset the search
            //
            LastDiscrepancy = 0;
            LastDeviceFlag = 0;
            LastFamilyDiscrepancy = 0;
            return xfalse;
        }
        //
        // issue the search command 
        //
        ByteWrite(DS18B20_SEARCH);
        //
        // loop to do the search
        //
        do
        {
            //
            // read a bit and its complement
            //
            IDBit = BitRead();
            ComplementIDBitR = BitRead();
            //
            // check for no devices on 1-wire
            //
            if ((IDBit == 1) && (ComplementIDBitR == 1))
                {
                    break;
                }
            else
            {
                //
                // all devices coupled have 0 or 1
                //
                if (IDBit != ComplementIDBitR)
                {
                    //
                    // bit write value for search
                    //
                    SearchDirection = IDBit;  
                }
                else
                {
                    //
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    //
                    if (IdBitNumber < LastDiscrepancy)
                    {
                        SearchDirection = ((ucROM[ROMByteNumber] & ROMByteMask) 
                                           > 0);
                    }
                    else
                    {
                        //
                        // if equal to last pick 1, if not then pick 0
                        //
                        SearchDirection = (IdBitNumber == LastDiscrepancy);
                    }
                    //
                    // if 0 was picked then record its position in LastZero
                    //   
                    if (SearchDirection == 0)
                    {
                        LastZero = IdBitNumber;
                        //
                        // check for Last discrepancy in family
                        //
                        if (LastZero < 9)
                            LastFamilyDiscrepancy = LastZero;
                    }
                }
                //
                // set or clear the bit in the ROM byte ROMByteNumber
                // with mask ROMByteMask
                //
                if (SearchDirection == 1)
                    ucROM[ROMByteNumber] |= ROMByteMask;
                else
                    ucROM[ROMByteNumber] &= ~ROMByteMask;
                //
                // serial number search direction write bit
                //
                BitWrite(SearchDirection);
                //
                // increment the byte counter IdBitNumber
                // and shift the mask ROMByteMask
                //
                IdBitNumber++;
                ROMByteMask <<= 1;
                //
                // if the mask is 0 then go to new SerialNum byte ROMByteNumber
                // and reset mask
                //
                if (ROMByteMask == 0)
                {
                    //
                    // accumulate the CRC
                    //
                    //doucCrc8(ucROM[ROMByteNumber]);  
                    ROMByteNumber++;
                    ROMByteMask = 1;
                }
            }
        }
        //
        // loop until through all ROM bytes 0-7
        //
        while(ROMByteNumber < 8); 
        //
        // if the search was successful then
        //
        if (!((IdBitNumber < 65) || (ucCrc8 != 0)))
        {
            //
            // search successful so set LastDiscrepancy,LastDeviceFlag,SearchResult
            //
            LastDiscrepancy = LastZero;
            //
            // check for last device
            //
            if (LastDiscrepancy == 0)
                LastDeviceFlag = 1;
            SearchResult = xtrue; 
            for(i=0; i<8; i++)
            {
                ucROM[i] = ucROM[i];
            }
        }        
    }
    //
    // if no device found then reset counters so next 'search' will be like a first
    //
    if (!SearchResult || !ucROM[0])
    {
        LastDiscrepancy = 0;
        LastDeviceFlag = 0;
        LastFamilyDiscrepancy = 0;
        SearchResult = xfalse;
    }
    return SearchResult;
}

//*****************************************************************************
//
//! \brief Verify the device with the ROM number in ROM buffer is present. 
//!
//! \return xTRUE  : device verified present
//!         xFALSE : device not present
//
//*****************************************************************************
xtBoolean DS18B20::Verify()
{
    unsigned char ucROMBackup[8];
    int i,result,LDBackup,LDFBackup,LFDBackup;
    //
    // keep a backup copy of the current state
    //
    for (i = 0; i < 8; i++)
        ucROMBackup[i] = ucROM[i];
    LDBackup = LastDiscrepancy;
    LDFBackup = LastDeviceFlag;
    LFDBackup = LastFamilyDiscrepancy;
    //
    // set search to find the same device
    //
    LastDiscrepancy = 64;
    LastDeviceFlag = 0;
    if (ROMSearch())
    {
        //
        // check if same device found
        //
        result = 1;
        for (i = 0; i < 8; i++)
        {
            if (ucROMBackup[i] != ucROM[i])
            {
                result = 0;
                break;
            }
        }
    }
    else
        result = 0;
    //
    // restore the search state 
    //
    for (i = 0; i < 8; i++)
        ucROM[i] = ucROMBackup[i];
    LastDiscrepancy = LDBackup;
    LastDeviceFlag = LDFBackup;
    LastFamilyDiscrepancy = LFDBackup;
    //
    // return the result of the verify
    //
    return result;
}

//*****************************************************************************
//
//! \brief Setup the search to find the device type 'family_code' on the 
//! next call to DS18B20ROMSearch() if it is present. 
//!
//! \return None
//
//*****************************************************************************
void DS18B20::TargetSetup(unsigned char ucFamily)
{
    int i;
    //
    // set the search state to find SearchFamily type devices
    //
    ucROM[0] = ucFamily;
    for (i = 1; i < 8; i++)
        ucROM[i] = 0;
    LastDiscrepancy = 64;
    LastFamilyDiscrepancy = 0;
    LastDeviceFlag = 0;
}

//*****************************************************************************
//
//! \brief Setup the search to skip the current device type on the next call
//! to DS18B20ROMSearch().
//!
//! \param ucFamily.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::FamilySkipSetup(unsigned char ucFamily)
{
    //
    // set the Last discrepancy to last family discrepancy
    //
    LastDiscrepancy = LastFamilyDiscrepancy;
    //
    // check for end of list
    //
    if (LastDiscrepancy == 0)
        LastDeviceFlag = 1;
}
#endif

//*****************************************************************************
//
//! \brief Get the DS18B20's alarm flag is set or not.
//!
//! \return None
//! \note This command can only be used when there is one slave on the bus.
//
//*****************************************************************************
void DS18B20::AlarmSearch()
{
    //ByteWrite(DS18B20_ALARM_SEARCH);

    //if (BitRead())
    //{
    //    return xtrue;
    //}
    //else
    //{
    //    return xfalse;
    //}
}
//*****************************************************************************
//
//! \brief Read the slave¡¯s 64-bit ROM code without using the Search ROM 
//! procedure.
//!
//! \return None
//! \note This command can only be used when there is one slave on the bus.
//
//*****************************************************************************
void DS18B20::ROMRead()
{
    int i;
    ByteWrite(DS18B20_READ);
    for (i=0; i<8; i++)
    {
        ucROM[i] = ByteRead();
    }
}

//*****************************************************************************
//
//! \brief The match ROM command followed by a 64-bit ROM code sequence allows
//! the bus master to address a specific slave device on a multidrop or 
//! single-drop bus.
//!
//! Only the slave that exactly matches the 64-bit ROM code sequence will 
//! respond to the function command issued by the master;
//!
//! \return None
//
//*****************************************************************************
void DS18B20::ROMMatch()
{
    int i;
    ByteWrite(DS18B20_MATCH);
    for (i=0; i<8; i++)
    {
        ByteWrite(ucROM[i]);
    }
}


//*****************************************************************************
//
//! \brief The master can use this command to address all devices on the bus
//! simultaneously without sending out any ROM code information.
//!
//! \return None
//! \note that the Read Scratchpad [BEh] command can follow the Skip ROM command
//! only if there is a single slave device on the bus. In this case, time is 
//! saved by allowing the master to read from the slave without sending the 
//! device¡¯s 64-bit ROM code. A Skip ROM command followed by a Read Scratchpad
//! command will cause a data collision on the bus if there is more than one
//! slave since multiple devices will attempt to transmit data simultaneously.
//
//*****************************************************************************
void DS18B20::ROMSkip()
{
    ByteWrite(DS18B20_SKIP);
}

//*****************************************************************************
//
//! \brief Initiates temperature conversion.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::TempConvert()
{
    ByteWrite(DS18B20_CONVERT);
    //DelayNus(750000);
}

//*****************************************************************************
//
//! \brief copies the contents of the scratchpad TH, TL and configuration 
//! registers (bytes 2, 3 and 4) to EEPROM.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::ScratchpadCopy()
{
    ByteWrite(DS18B20_COPY_SCRATCHPAD);
    DwtDelayUs(10);
}

//*****************************************************************************
//
//! \brief Set the Th and Tl register.
//!
//! \param cHigh The data is written into the TH register (byte 2 of the 
//! scratchpad).
//! \param cLow The data is written into the TL register (byte 3).
//! \param ucBitConfig The data is written into the configuration register 
//! (byte 4).
//!
//! \return None
//
//*****************************************************************************
void DS18B20::ScratchpadSet(char cHigh, char cLow,
                          unsigned char ucBitConfig)
{
    ByteWrite(DS18B20_WRITE_SCRATCHPAD);
    ByteWrite(cHigh);
    ByteWrite(cLow);
    ByteWrite(ucBitConfig);
}

void DS18B20::SetPrecision(unsigned char precision)
{
	ScratchpadSet(0x4B, 0x46, (0x1F | (precision << 5)));
	ucPrecision = precision;
}

//*****************************************************************************
//
//! \brief Get the Temperature of DS18B20.
//!
//! \param pfTemp The flaot Temperature vaule.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::TempRead(float *pfTemp)
{
    unsigned short ulTemp = 0;
    ByteWrite(DS18B20_READ_SCRATCHPAD);
    ulTemp = ByteRead();
    ulTemp |= (ByteRead() << 8);
    if (ulTemp > 2097)
    {
        ulTemp = 65536 - ulTemp;
        *pfTemp = -(((ulTemp & 0x7F0) >> 4)*1.0 + (ulTemp & 0xf)*0.0625);
    }
    else
    {
        *pfTemp = ((ulTemp & 0x7F0) >> 4)*1.0 + (ulTemp & 0xf)*0.0625;
    }
}

//*****************************************************************************
//
//! \brief Get the Temperature of DS18B20.
//!
//! \param pfTemp The flaot Temperature vaule.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::TempReadSimple(simple_float *pfTemp)
{
    unsigned short ulTemp = 0;
    float fractional = 0;
    ByteWrite(DS18B20_READ_SCRATCHPAD);
    ulTemp = ByteRead();
    ulTemp |= (ByteRead() << 8);
    if (ucPrecision == 0)
    	fractional = 0.625;
    else if (ucPrecision == 1)
    	fractional = 6.25;
    else if (ucPrecision == 2)
    	fractional = 62.5;
    else
		fractional = 625;
    if (ulTemp > 2097)
    {
        ulTemp = 65536 - ulTemp;
        pfTemp->integer = -(((ulTemp & 0x7F0) >> 4)*1.0);
        pfTemp->fractional = (ulTemp & 0xf)*fractional;
        pfTemp->is_valid = true;
    }
    else
    {
        pfTemp->integer = ((ulTemp & 0x7F0) >> 4)*1.0;
        pfTemp->fractional = (ulTemp & 0xf)*fractional;
        pfTemp->is_valid = true;
    }
}

//*****************************************************************************
//
//! \brief Recalls TH, TL, and configuration register data from EEPROM to the
//! scratchpad..
//!
//! \return None
//
//*****************************************************************************
void DS18B20::EEROMRecall()
{
    ByteWrite(DS18B20_RECALL);
    //
    // DS18B20 dq_pin be set to high
    //
    GPIO_WriteBit(ulPort, ulPin, Bit_SET);
    DwtDelayUs(5);
    //
    // Wait utill the recall is done.
    //
    while (BitRead());
}

//*****************************************************************************
//
//! \brief Get DS18B20s's power supply.
//!
//! \return xTRUE  : externally powered
//!         xFALSE : parasite powered
//
//*****************************************************************************
bool DS18B20::PowerSupplyRead()
{
    ByteWrite(DS18B20_READ_POWER_SUPPLY);
    if (BitRead())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//*****************************************************************************
//
//! \brief Get the Scratchpad of DS18B20.
//!
//! \param pucTemp The point of Scratchpad.
//!
//! \return None
//
//*****************************************************************************
void DS18B20::ScratchpadRead(unsigned char *pucTemp)
{
    int i;
    ByteWrite(DS18B20_READ_SCRATCHPAD);
    for(i=0; i<8; i++)
    {  
        *pucTemp++ = ByteRead();
    }
    
}
