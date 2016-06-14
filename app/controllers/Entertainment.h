#ifndef __ENTERTAINMENT_H__
#define __ENTERTAINMENT_H__

#include "Keypad.h"
#include "KeypadInterrupt.h"

class Entertainment : public KeypadInterrupt
{
public:
	void Init();//Keypad* inst);
	void KeypadKeysPressed(const unsigned short keys);
};

#endif
