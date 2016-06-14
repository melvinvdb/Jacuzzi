#include "ovc3860.h"
#include <string.h>

const char * OVC3860::AT_RESET		= "AT#CZ\r\n";
const char * OVC3860::AT_PLAYPAUSE	= "AT#MA\r\n";
const char * OVC3860::AT_STOP		= "AT#MC\r\n";
const char * OVC3860::AT_NEXT		= "AT#MD\r\n";
const char * OVC3860::AT_PREVIOUS	= "AT#ME\r\n";
const char * OVC3860::AT_VOLUP		= "AT#VU\r\n"; //range 0..14
const char * OVC3860::AT_VOLDOWN	= "AT#VD\r\n";
const unsigned char OVC3860::STARTUP_BYTES[4]			= { 0x04, 0x0F, 0x04, 0x00 };
const unsigned char OVC3860::CONFIGMODE_BYTES[4]		= { 0x04, 0x0F, 0x04, 0x01 };
const unsigned char OVC3860::ENTERCONFIG_BYTES[9]		= { 0xC5, 0xC7, 0xC7, 0xC9, 0xD0, 0xD7, 0xC9, 0xD1, 0xCD };
const char* OVC3860::BLUETOOTH_NAME						= "JACUZZI";
const unsigned char OVC3860::REQUEST_BLUETOOTH_NAME[4]			= { 0x11, 0xC7, 0x00, 0x10 };
const unsigned char OVC3860::RESPONSE_BLUETOOTH_NAME[4]			= { 0x21, 0xC7, 0x00, 0x10 };
const unsigned char OVC3860::SEND_BLUETOOTH_NAME[4]				= { 0x31, 0xC7, 0x00, 0x10 };
const unsigned char OVC3860::SEND_RESPONSE_BLUETOOTH_NAME[4]	= { 0x41, 0xC7, 0x00, 0x10 };
const unsigned char OVC3860::EXITCONFIG_BYTES[4]				= { 0x50, 0x00, 0x00, 0x00 };
const unsigned char OVC3860::EXITCONFIG_RESPONSE_BYTES[4]		= { 0x60, 0x00, 0x00, 0x00 };

void OVC3860::Init(unsigned long _ulRCCPort, GPIO_TypeDef * _ulPort, uint16_t _ulPin)
{
	ulRCCPort = _ulRCCPort;
	ulPort = _ulPort;
	ulPin = _ulPin;
	configMode = false;
	RCC_APB2PeriphClockCmd(ulRCCPort, ENABLE);
	GPIO_InitTypeDef PORT;
	PORT.GPIO_Pin = ulPin;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(ulPort,&PORT);
	USART2_Init(115200); //ovc3860 uart init
	USART2_SetTX(false); //shutdown TX, required to prevent ovc3860 to get its power through uart.
	GPIO_WriteBit(ulPort, ulPin, Bit_RESET); // set low
}

void OVC3860::SetPower(const bool switchOn)
{
	GPIO_WriteBit(ulPort, ulPin, (switchOn == true ? Bit_SET : Bit_RESET));
	USART2_SetTX(switchOn); //switch TX
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

bool OVC3860::EnterConfigMode()
{
	printf("Entering config mode\r\n");
	//don't know the current state so first shut it down
	USART2_SetTX(false); //shutdown TX, required to prevent ovc3860 to get its power through uart.
	GPIO_WriteBit(ulPort, ulPin, Bit_RESET); // set low
	DwtDelayMs(200); //give it some time to shut down
	GPIO_WriteBit(ulPort, ulPin, Bit_SET); // set high
	USART2_SetTX(true); //enable TX

	//long timeout time, device needs to wake up
	if (ReceiveAndCompare(STARTUP_BYTES, 4, 10000) == false)
		return false;

	//we should now immediately send the magic bytes to enter config mode
	for (int i = 0; i < 9; i++)
	{
		USART_SendData(USART2, ENTERCONFIG_BYTES[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}

	//check if we're in config mode
	if (ReceiveAndCompare(CONFIGMODE_BYTES, 4, 2000) == false)
		return false;
	printf("Device now in config mode!!!\r\n");
	configMode = true;
	return true;
}

bool OVC3860::ExitConfigMode()
{
	printf("Exiting config mode\r\n");
	//clear receive buffer
	while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
		USART_ReceiveData(USART2);
	//send exit request
	for (int i = 0; i < 4; i++)
	{
		USART_SendData(USART2, EXITCONFIG_BYTES[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}
	//check if exit was successful
	if (ReceiveAndCompare(EXITCONFIG_RESPONSE_BYTES, 4, 2000) == false)
			return false;
	return true;
}

bool OVC3860::CheckBluetoothName()
{
	if (configMode == false) {
		EnterConfigMode();
	}
	if (configMode == false)
		return false;
	bool isCorrect = CompareBluetoothName();
	if (isCorrect)
	{
		ExitConfigMode();
		return true;
	}
	else
	{
		bool result = SetBluetoothName();
		ExitConfigMode();
		return result;
	}
}

bool OVC3860::SetBluetoothName()
{
	printf("setting BT name\r\n");
	//clear receive buffer
	while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
		USART_ReceiveData(USART2);
	//send BT name request
	for (int i = 0; i < 4; i++)
	{
		USART_SendData(USART2, SEND_BLUETOOTH_NAME[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}
	unsigned int i = 0;
	//send BT name
	for (; i < strlen(BLUETOOTH_NAME); i++)
	{
		USART_SendData(USART2, BLUETOOTH_NAME[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}
	for (; i < 16; i++)
	{
		USART_SendData(USART2, '\0');
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}
	//check if send was successful
	if (ReceiveAndCompare(SEND_RESPONSE_BLUETOOTH_NAME, 4, 2000) == false)
			return false;
	return true;
}

bool OVC3860::CompareBluetoothName()
{
	printf("checking BT name\r\n");
	//clear receive buffer
	while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
		USART_ReceiveData(USART2);
	//send BT name data request
	for (int i = 0; i < 4; i++)
	{
		USART_SendData(USART2, REQUEST_BLUETOOTH_NAME[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	}

	if (ReceiveAndCompare(RESPONSE_BLUETOOTH_NAME, 4, 2000, false) == false)
			return false;
	bool result;
	char btName[16];
	for (int i = 0; i < 16; i++)
	{
		result = ReceiveByte(&btName[i], 2000);
		if (result == false)
			break;
	}
	if (result == false)
		return false;
	printf("We got a BT name %s\r\n", btName);
	if (strcmp(btName, BLUETOOTH_NAME) == 0)
		return true;
	else
		return false;
}

bool OVC3860::ReceiveAndCompare(const unsigned char * data, const int length, unsigned long timeoutTime, bool flushReceiveBuffer)
{
	char incomingBytes[length];
	bool result;
	for (int i = 0; i < length; i++)
	{
		result = ReceiveByte(&incomingBytes[i], timeoutTime);
		if (result == false)
			break;
	    //hack to skip to the first valid byte
	    if (i == 0 && incomingBytes[i] != data[i])
	    	--i;
	}
	if (flushReceiveBuffer == true)
	{
		//clear receive buffer we don't care about the rest
		while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
			USART_ReceiveData(USART2);
	}
	if (result == false)
		return false;
	if (memcmp(incomingBytes, data, length) != 0)
		return false; //ovc not in startup
	return true;
}

bool OVC3860::ReceiveByte(char *byte, unsigned long timeoutTime)
{
	bool timeout = false;
	unsigned long currTime = GetSysTick();
	//wait for a byte with timeout
	while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)
	{
		if ((unsigned long)(GetSysTick() - currTime) > SysTickFormatMs(timeoutTime))
		{
			timeout = true;
			break;
		}
	}
	if (timeout == true)
		return false;
	*byte = (char)USART_ReceiveData(USART2);
	return true;
}

