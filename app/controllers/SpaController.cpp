#include "SpaController.h"
#include <stdio.h> //printf

SpaController::SpaController() : keypad(Keypad::getInstance()), leds(LEDS::getInstance()), relayBoard(RelayBoard::getInstance())
{}

void SpaController::Init()
{
	printf("SpaController: Init() ........\r\n");
	keypad.RegisterForCallback(*this);
	printf("SpaController: Init() complete\r\n");
}

void SpaController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	printf("SpaController: KeypadKeysPressed() long press: %s\r\n", keyshold ? "true" : "false");
	//printf("SpaControler DBG: %X\r\n", keys);
	if ((keys | Keypad::JET) == Keypad::JET)
		ToggleJets();
	else if ((keys | Keypad::GAS) == Keypad::GAS)
		ToggleGas();
	else if ((keys | Keypad::HEAT) == Keypad::HEAT)
		ToggleHeat();
	else if ((keys | Keypad::CIRC) == Keypad::CIRC)
		ToggleCirc();
	else if ((keys | Keypad::LED1) == Keypad::LED1)
		ToggleLED1();
	else if ((keys | Keypad::LED2) == Keypad::LED2)
		ToggleLED2();
}

void SpaController::ToggleJets()
{
	stateJets = !stateJets;
	relayBoard.SwitchRelay(RelayBoard::JET, stateJets);
	leds.SwitchLed(LEDS::JET, stateJets);
}

void SpaController::ToggleGas()
{
	stateGas = !stateGas;
	relayBoard.SwitchRelay(RelayBoard::GAS, stateGas);
	leds.SwitchLed(LEDS::GAS, stateJets);
}

void SpaController::ToggleHeat()
{
	stateHeat = !stateHeat;
	relayBoard.SwitchRelay(RelayBoard::HEAT, stateHeat);
	leds.SwitchLed(LEDS::HEAT, stateJets);
}

void SpaController::ToggleCirc()
{
	stateCirc = !stateCirc;
	relayBoard.SwitchRelay(RelayBoard::CIRC, stateCirc);
	leds.SwitchLed(LEDS::CIRC, stateJets);
}

void SpaController::ToggleLED1()
{
	stateLED1 = !stateLED1;
	relayBoard.SwitchRelay(RelayBoard::LED1, stateLED1);
	leds.SwitchLed(LEDS::LED1, stateJets);
}

void SpaController::ToggleLED2()
{
	stateLED2 = !stateLED2;
	relayBoard.SwitchRelay(RelayBoard::LED2, stateLED2);
	leds.SwitchLed(LEDS::LED2, stateJets);
}
