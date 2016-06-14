#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "SingularCallBack.h"
#include "mcp23017.h"

//#include "KeypadInterrupt.h"
class KeypadInterrupt; //forward declare otherwise circuit-include issue

class Keypad
{
private:
	mcp23017 pad;
public:
	typedef enum Keys : unsigned short { JET=0xFEFF, GAS=0xFDFF, HEAT=0xFBFF, CIRC=0xF7FF, POWER=0xEFFF, LED1=0xDFFF, LED2=0xBFFF, NC=0x7FFF,
										TEMPP=0xFF7F, TEMPM=0xFFBF, SOURCE=0xFFDF, CHP=0xFFEF, CHM=0xFFF7, TV=0xFFFB, VOLP=0xFFFD, VOLM=0xFFFE} Keys;

	typedef SingularCallBack <KeypadInterrupt, void, const unsigned short> InterruptCallback;

	void Init();
	bool RegisterForCallback(KeypadInterrupt& callback);
	void GetKeysPressed();
private:
	static const char INTERRUPT_LIST_LENGTH = 5;
	InterruptCallback interruptList[INTERRUPT_LIST_LENGTH];
	int interruptListIndex = 0;
};


#endif
