#pragma once

#include "DS18B20.h"

#define TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK 0

class TemperatureSensor
{
private:
	DS18B20 sensor;
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	unsigned long conversionTime;
	const unsigned long requiredConversionTime = 100; //100ms for 9 bit resolution
#endif
	bool available;
	bool prevAvailable;
	DS18B20::simple_float temp;

	bool Reset();
public:
	bool Init(uint32_t _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin);
	bool IsAvailable();
	bool StartConversion();
	bool ReadTemperature();
	bool TempIsValid();
	bool GetTemperature(char (&strOut)[7]);
	float GetTemperature();
};
