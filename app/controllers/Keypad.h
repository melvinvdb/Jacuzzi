#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "VariableCallBack.h"
#include "KeypadInterrupt.h"
#include "mcp23017.h"

//#include "KeypadInterrupt.h"
class KeypadInterrupt; //forward declare otherwise circuit-include issue

class Keypad
{
private:
	mcp23017 pad;
	bool keypadWorking;
	void McpInit();
public:
	typedef enum Keys : unsigned short { JET=0xFEFF, GAS=0xFDFF, HEAT=0xFBFF, CIRC=0xF7FF, POWER=0xEFFF, LED1=0xDFFF, LED2=0xBFFF, NC=0x7FFF,
										TEMPP=0xFF7F, TEMPM=0xFFBF, SOURCE=0xFFDF, CHP=0xFFEF, CHM=0xFFF7, TV=0xFFFB, VOLP=0xFFFD, VOLM=0xFFFE} Keys;

	typedef CallBack <KeypadInterrupt, void, const unsigned short, const bool> InterruptCallback;

	static Keypad& getInstance()
	{
		static Keypad instance;
		return instance;
	}
	void Init();
	bool RegisterForCallback(KeypadInterrupt& callback);
	unsigned short ReadKeys();
	void SetKeyCheckEnabled(bool enabled);
	bool CheckKeysPressed();
	bool IsKeypadWorking();
private:
	static const char LONG_KEY_PRESS_INTERVAL = 3; //trigger long key press on x CheckKeysPressed()
	static const char INTERRUPT_LIST_LENGTH = 5;
	InterruptCallback interruptList[INTERRUPT_LIST_LENGTH];
	int interruptListIndex;
	unsigned short lastRead;
	unsigned char solidTickCount; //counter how many CheckKeysPressed() received a key press state
	bool enableKeyCheck;
};


#endif
