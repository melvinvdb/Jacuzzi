#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "VariableCallBack.h"
#include "Keys.h"
#include "mcp23017.h"

class Keys; //forward declare otherwise circuit-include issue

class Keypad
{
private:
	mcp23017 pad;
	bool keypadWorking;
	void McpInit();
public:
	typedef CallBack <Keys, void, const unsigned short, const bool> InterruptCallback;

	static Keypad& getInstance()
	{
		static Keypad instance;
		return instance;
	}
	void Init();
	bool RegisterForCallback(Keys& callback);
	void OnlyCallFirstCallback(bool enabled);
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
	bool onlyCallFirstCallback;
};


#endif
