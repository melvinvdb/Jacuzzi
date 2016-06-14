#include "EntController.h"
#include <stdio.h> //printf

EntController::EntController(Keypad& kp, RelayBoard& rb) : keypad(kp), relayBoard(rb)
{}

void EntController::Init()
{
	keypad.RegisterForCallback(*this);
}

void EntController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	printf("Hello from EntController, long press: %s\r\n", keyshold ? "true" : "false");
	/*if ((keys | Keypad::GAS) == Keypad::GAS)
	{
		relayBoard.SwitchRelay(RelayBoard::GAS, true);
		printf("GAS\r\n");
	}*/
}
