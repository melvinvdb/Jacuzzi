#ifndef __KEYPADINTERRUPT_H__
#define __KEYPADINTERRUPT_H__

#include "Keypad.h"

//abstract class
class KeypadInterrupt
{
public:
	virtual void KeypadKeysPressed(const unsigned short keys) = 0;
};


#endif
