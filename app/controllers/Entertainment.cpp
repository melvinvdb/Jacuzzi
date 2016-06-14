#include "Entertainment.h"
#include <stdio.h> //printf

void Entertainment::Init()//Keypad* inst)
{
	//Keypad::InterruptCallback callback(this, &KeypadInterrupt::KeypadKeysPressed);

}

void Entertainment::KeypadKeysPressed(const unsigned short keys)
{
	printf("Hello from Entertainment! Callback finally working :)\r\n");
	if ((keys | Keypad::GAS) == Keypad::GAS)
	{
		printf("GAS\r\n");
	}
}
