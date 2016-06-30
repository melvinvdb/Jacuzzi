#pragma once

#include "VariableCallBack.h"
#include "Keys.h"

class Keys; //forward declare otherwise circuit-include issue

class Computer
{
public:
	typedef CallBack <Keys, void, const unsigned short, const bool> InterruptCallback;

	static Computer& getInstance()
	{
		static Computer instance;
		return instance;
	}
	void Init();
	bool RegisterForCallback(Keys& callback);

	void Monitor();

private:
	static const char INTERRUPT_LIST_LENGTH = 5;
	InterruptCallback interruptList[INTERRUPT_LIST_LENGTH];
	int interruptListIndex;
};

