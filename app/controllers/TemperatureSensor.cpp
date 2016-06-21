#include "TemperatureSensor.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
#include "delay.h"
#endif

bool TemperatureSensor::Init(unsigned long _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin)
{
	available = sensor.Init(_ulRCCPort, _ulPort, _ulPin);
	prevAvailable = available;
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	conversionTime = 0;
#endif
	if (available)
	{
		sensor.Reset();
		sensor.Reset();
		sensor.ROMSkip();
		sensor.SetPrecision(0); //0 == 9 bits, 1 == 10 bits, 2 == 11 bits, 3 == 12 bits
		StartConversion();
	}
	temp = {0, 0, false};
	return available;
}

bool TemperatureSensor::Reset()
{
	available = sensor.Reset();
	if (available)
	{
		sensor.Reset();
		sensor.ROMSkip();
		sensor.SetPrecision(0); //0 == 9 bits, 1 == 10 bits, 2 == 11 bits, 3 == 12 bits
	}
	return available;
}

bool TemperatureSensor::IsAvailable()
{
	available = sensor.Reset();
	return available;
}

bool TemperatureSensor::StartConversion()
{
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	if ((unsigned long)(GetSysTick() - conversionTime) < SysTickFormatMs(requiredConversionTime))
	{
		return true;
	}
#endif
	available = sensor.Reset();
	if (available == false)
	{
		prevAvailable = false;
		return false;
	}
	if (prevAvailable == false) //previous time was it unavailable, reset sensor
		Reset();
	sensor.Reset();
	sensor.ROMSkip();
	sensor.TempConvert();
	prevAvailable = available;
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	conversionTime = GetSysTick();
#endif
	return true;
}

bool TemperatureSensor::ReadTemperature()
{
	temp = {0, 0, false};
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	if ((unsigned long)(GetSysTick() - conversionTime) >= SysTickFormatMs(requiredConversionTime))
	{
#endif
		available = sensor.Reset();
		if (available == false || prevAvailable == false) //conversion needs to be succeeded and now available
		{
			prevAvailable = available;
			return false;
		}
		sensor.Reset();
		sensor.ROMSkip();
		sensor.TempReadSimple(&temp);
#if TEMPSENSOR_ENABLE_CONVERSION_TIME_CHECK == 1
	}
#endif
	return true;
}

bool TemperatureSensor::TempIsValid()
{
	return temp.is_valid;
}

bool TemperatureSensor::GetTemperature(char (&strOut)[7])
{
	if (temp.is_valid)
	{
		sprintf(strOut, "%d.%d°C", temp.integer, temp.fractional);
		return true;
	}
	else
	{
		sprintf(strOut, "NC");
		return false;
	}
}

float TemperatureSensor::GetTemperature()
{
	if (temp.is_valid)
	{
		float tempf = temp.integer + ((float)temp.fractional / 10);
		return tempf;
	}
	else
	{
		return 0;
	}
}
