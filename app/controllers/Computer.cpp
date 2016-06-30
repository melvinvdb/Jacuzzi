#include "Computer.h"
#include <stdio.h>

void Computer::Init()
{

}

bool Computer::RegisterForCallback(Keys& callback)
{
	if (interruptListIndex > INTERRUPT_LIST_LENGTH)
	{
		return false;
	}
	interruptList[interruptListIndex] = InterruptCallback(&callback, &Keys::KeyDataReceived);
	++interruptListIndex;
	return true;
}

void Computer::Monitor()
{

}
