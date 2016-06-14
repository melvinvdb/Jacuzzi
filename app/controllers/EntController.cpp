#include "EntController.h"
#include <stdio.h> //printf

EntController::EntController() : keypad(Keypad::getInstance()), relayBoard(RelayBoard::getInstance())
{}

void EntController::Init()
{
	keypad.RegisterForCallback(*this);
}

void EntController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	//printf("Hello from EntController, long press: %s\r\n", keyshold ? "true" : "false");
	if ((keys | Keypad::SOURCE) == Keypad::SOURCE)
		printf("SOURCE\r\n");
	else if ((keys | Keypad::CHP) == Keypad::CHP)
		printf("CH+\r\n");
	else if ((keys | Keypad::CHM) == Keypad::CHM)
		printf("CH-\r\n");
	else if ((keys | Keypad::TV) == Keypad::TV)
		printf("TV\r\n");
	else if ((keys | Keypad::VOLP) == Keypad::VOLP)
		printf("VOL+\r\n");
	else if ((keys | Keypad::VOLM) == Keypad::VOLM)
		printf("VOL-\r\n");
	else if ((keys | Keypad::POWER) == Keypad::POWER)
		printf("POWER\r\n");
}

