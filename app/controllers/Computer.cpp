#include "Computer.h"
#include "usart.h"
#include <stdio.h>
#include "Keypad.h"
#include "Display.h"

char rxBuffer[BUFFER_SIZE+1];
volatile int rxInIndex=0;
volatile int rxOutIndex=0;
char rxLine[BUFFER_SIZE+1];
bool responseAvailable=false;

Computer::Computer() : keypad(Keypad::getInstance()), display(Display::getInstance())
{

}

void Computer::Init()
{

}

bool Computer::RegisterForCallback(Keys& callback)
{
	if (interruptListIndex > INTERRUPT_LIST_LENGTH)
	{
		return false;
	}
	keyDataInterruptList[interruptListIndex] = keyDataReceivedCallback(&callback, &Keys::KeyDataReceived);
	audioStateInterruptList[interruptListIndex] = AudioStateChangedCallback(&callback, &Keys::AudioStateChanged);
	++interruptListIndex;
	return true;
}

void Computer::CheckIncomingData()
{
	if (responseAvailable == true)
	{
		printf("Resp: %X\r\n", rxLine);
		if (rxLine[0] == 0x00 || rxLine[0] == 0x01)
		{
			Display::Screens newstate = (Display::Screens)rxLine[0];
			display.SetActiveScreen(newstate);
			if (newstate == Display::MAINSCREEN)
			{
				if (rxLine[3] == '\n' || rxLine[5] == '\n')
				{
					keypad.OnlyCallFirstCallback(false);
					ParseMessageStateMain();
				}
			}
			else
			{
				keypad.OnlyCallFirstCallback(true); //All keypad callbacks only to this class
				ParseMessageStateSecond();
			}
		}
		responseAvailable = false;
	}
}

void Computer::ParseMessageStateMain()
{
	unsigned short pressedKeys = rxLine[1] << 8 | rxLine[2];
	if (pressedKeys != 0xFFFF)
	{
		printf("fire key press event %X\r\n", pressedKeys);
		//fire key press events
		for (int i = 0; i < interruptListIndex; i++)
		{
			keyDataInterruptList[i].execute(pressedKeys, false); //no hold
		}
	}
	if (rxLine[3] != '\n')
	{
		unsigned short audio = rxLine[3] << 8 | rxLine[4];
		printf("fire audio changed event %X\r\n", audio);
		//fire audio events
		for (int i = 0; i < interruptListIndex; i++)
		{
			audioStateInterruptList[i].execute(audio);
		}
	}
}

void Computer::ParseMessageStateSecond()
{
	display.ClearSecondScreen();
	unsigned int i = 1; //start at index 1. index 0 == state
	while (rxLine[i] != '\n')
	{
		int pairEndIndex = SearchCharacter(i, '\r'); //search end of pair
		if (pairEndIndex == -1)
			break;
		int index = rxLine[i+2]; //i==MODE i+1==:
		if (rxLine[i] == 'T')
		{
			//T:index:ascii max 20\r	(max 5 lines)
			int strStartIndex = i+4; //start string
			if (pairEndIndex - strStartIndex >= DISPLAY_SCREEN_TEXT_LENGTH)
			{
				i = pairEndIndex + 1;
				continue;
			}
			int textLength = pairEndIndex - strStartIndex + 1;
			char text[textLength]; //null terminated
			snprintf(text, textLength, rxLine + strStartIndex);
			display.WriteText(index, text);
		}
		else if (rxLine[i] == 'H' || rxLine[i] == 'V')
		{
			//H:index:x:y:w\r			(max 4 lines)
			int x = rxLine[i+4];
			int y = rxLine[i+6];
			int w = rxLine[i+8];
			if (rxLine[i] == 'H')
				display.WriteHLine(index, x, y, w);
			else
				display.WriteVLine(index, x, y, w);
		}
		i = pairEndIndex + 1;
	}
}


int Computer::SearchCharacter(const unsigned int index, const char c)
{
	int found = -1;
	for (unsigned int i = index; rxLine[i] != '\n'; i++)
	{
		if (rxLine[i] == c)
		{
			found = i;
			break;
		}
	}
	return found;
}

void Computer::SendMessage()
{
	if (display.GetActiveScreen() == Display::MAINSCREEN)
		SendMessageStateMain();
	else
		SendMessageStateSecond(0xFFFF);
}

void Computer::SendMessageStateMain()
{

}

void Computer::SendMessageStateSecond(unsigned short key)
{
	printf("out: ");
	char msg[5]; //[state][protocol version][pressed key stroke 2 bytes][\n]
	msg[0] = 0x01;
	msg[1] = PROTOCOL_VERSION;
	msg[2] = (key >> 8) & 0xFF;
	msg[3] = key & 0xFF;
	msg[4] = '\n';
	for (int i = 0; i < 5; i++)
	{
		USART_SendData(USART1, (u8)msg[i]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
}

void Computer::KeyDataReceived(const unsigned short keys, const bool keyshold)
{
	//Should only be fired from keypad
	printf("key data received\n");
	if (display.GetActiveScreen() == Display::SECONDSCREEN)
		SendMessageStateSecond(keys);
}

void Computer::AudioStateChanged(const unsigned short newState)
{
	//not used, never triggered
}

#ifdef __cplusplus
 extern "C" {
#endif

void ReadLine()
{
    int i = 0;
    // Start Critical Section - don't interrupt while changing global buffer variables
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    // Loop reading rx buffer characters until end of line character. Uses \n for line termination
    while ((i==0) || (rxLine[i-1] != '\n')) {
        rxLine[i] = rxBuffer[rxOutIndex];
        i++;
        rxOutIndex = (rxOutIndex + 1) % BUFFER_SIZE;
    }
    // End Critical Section
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    rxLine[i] = 0; //i-1 == \n
    responseAvailable = true;
}

/**********************************************************
 * USART1 interrupt request handler
 *********************************************************/
void USART1_IRQHandler(void)
{
    /* RXNE handler */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
    	char c = (char)USART_ReceiveData(USART1);
        rxBuffer[rxInIndex] = c;
        rxInIndex = (rxInIndex + 1) % BUFFER_SIZE;
        if (c == '\n')
        {
        	ReadLine();
        }
    }
}

#ifdef __cplusplus
 }
#endif
