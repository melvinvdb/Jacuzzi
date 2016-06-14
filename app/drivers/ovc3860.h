#pragma once

#include <stdio.h>
#include "usart.h"
#include <printf.h>
#include "delay.h"

class OVC3860
{
private:

	static const char * AT_RESET;
	static const char * AT_PLAYPAUSE;
	static const char * AT_STOP;
	static const char * AT_NEXT;
	static const char * AT_PREVIOUS;
	static const char * AT_VOLUP;
	static const char * AT_VOLDOWN;
	static const unsigned char STARTUP_BYTES[4];
	static const unsigned char CONFIGMODE_BYTES[4];
	static const unsigned char ENTERCONFIG_BYTES[9];
	static const char* BLUETOOTH_NAME;
	static const unsigned char REQUEST_BLUETOOTH_NAME[4];
	static const unsigned char RESPONSE_BLUETOOTH_NAME[4];
	static const unsigned char SEND_BLUETOOTH_NAME[4];
	static const unsigned char SEND_RESPONSE_BLUETOOTH_NAME[4];
	static const unsigned char EXITCONFIG_BYTES[4];
	static const unsigned char EXITCONFIG_RESPONSE_BYTES[4];
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
	bool configMode;
	bool EnterConfigMode();
	bool ExitConfigMode();
	bool SetBluetoothName();
	bool CompareBluetoothName();
	bool ReceiveAndCompare(const unsigned char * data, const int length, unsigned long timeoutTime, bool flushReceiveBuffer = true);
	bool ReceiveByte(char *byte, unsigned long timeoutTime);
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
	bool CheckBluetoothName();
};
