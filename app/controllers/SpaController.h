#ifndef __SPACONTROLLER_H__
#define __SPACONTROLLER_H__

#include "Keypad.h"
#include "KeypadInterrupt.h"
#include "LEDS.h"
#include "RelayBoard.h"

class SpaController : public KeypadInterrupt
{
private:
	Keypad& keypad;
	LEDS& leds;
	RelayBoard& relayBoard;
	bool stateJets;
	bool stateGas;
	bool stateHeat;
	bool stateCirc;
	bool stateLED1;
	bool stateLED2;
public:
	SpaController();
	void Init();
	void KeypadKeysPressed(const unsigned short keys, const bool keyshold);
	void ToggleJets();
	void ToggleGas();
	void ToggleHeat();
	void ToggleCirc();
	void ToggleLED1();
	void ToggleLED2();
};

#endif
