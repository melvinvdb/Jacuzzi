#include "ovc3860.h"

void OVC3860::Init(unsigned long _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin)
{
	ulRCCPort = _ulRCCPort;
	ulPort = _ulPort;
	ulPin = _ulPin;
	RCC_APB2PeriphClockCmd(ulRCCPort, ENABLE);
	GPIO_InitTypeDef PORT;
	PORT.GPIO_Pin = ulPin;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(ulPort,&PORT);
	GPIO_WriteBit(ulPort, ulPin, Bit_RESET); // set low
	USART2_Init(115200); //ovc3860 uart init
}

void OVC3860::SetPower(const bool switchOn)
{
	GPIO_WriteBit(ulPort, ulPin, (switchOn == true ? Bit_SET : Bit_RESET));
	if (switchOn)
	{
		Reset();
		DwtDelayMs(100); //just give him some time to start up.
	}
}

void OVC3860::Reset()
{
	SetOutputUart(2);
	printf(AT_RESET);
	SetOutputUart(1);
}


void OVC3860::PlayPause()
{
	SetOutputUart(2);
	printf(AT_PLAYPAUSE);
	SetOutputUart(1);
}

void OVC3860::Stop()
{
	SetOutputUart(2);
	printf(AT_STOP);
	SetOutputUart(1);
}

void OVC3860::PlayPrevious()
{
	SetOutputUart(2);
	printf(AT_PREVIOUS);
	SetOutputUart(1);
}

void OVC3860::PlayNext()
{
	SetOutputUart(2);
	printf(AT_NEXT);
	SetOutputUart(1);
}

void OVC3860::VolumeUp()
{
	SetOutputUart(2);
	printf(AT_VOLUP);
	SetOutputUart(1);
}

void OVC3860::VolumeDown()
{
	SetOutputUart(2);
	printf(AT_VOLDOWN);
	SetOutputUart(1);
}

void OVC3860::SetVolume(const uint8_t vol)
{
	SetOutputUart(2);
	//hack to set volume to desired setting
	for (char i = 0; i < 14; i++)
	{
		printf(AT_VOLDOWN);
		DwtDelayMs(50); //40ms is too fast
	}
	for (char i = 0; i < vol; i++)
	{
		printf(AT_VOLUP);
		DwtDelayMs(50);
	}
	SetOutputUart(1);
}
