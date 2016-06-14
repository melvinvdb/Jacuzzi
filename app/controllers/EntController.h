#ifndef __ENTCONTROLLER_H__
#define __ENTCONTROLLER_H__

#include "Keypad.h"
#include "KeypadInterrupt.h"
#include "RelayBoard.h"

class EntController : public KeypadInterrupt
{
private:
	Keypad& keypad;
	RelayBoard& relayBoard;
public:
	EntController(Keypad& kp, RelayBoard& rb);
	void Init();
	void KeypadKeysPressed(const unsigned short keys, const bool keyshold);
};

#endif
