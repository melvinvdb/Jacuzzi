#pragma once

#include <stdio.h>
#include "usart.h"
#include <printf.h>
#include "delay.h"

class OVC3860
{
private:

	const char * AT_RESET		= "AT#CZ\r\n";
	const char * AT_PLAYPAUSE	= "AT#MA\r\n";
	const char * AT_STOP		= "AT#MC\r\n";
	const char * AT_NEXT		= "AT#MD\r\n";
	const char * AT_PREVIOUS	= "AT#ME\r\n";
	const char * AT_VOLUP		= "AT#VU\r\n"; //range 0..14
	const char * AT_VOLDOWN		= "AT#VD\r\n";
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
public:
	void Init(unsigned long _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin);
	void SetPower(const bool switchOn);
	void Reset();
	void PlayPause();
	void Stop();
	void PlayPrevious();
	void PlayNext();
	void VolumeUp();
	void VolumeDown();
	void SetVolume(const uint8_t vol);

};
