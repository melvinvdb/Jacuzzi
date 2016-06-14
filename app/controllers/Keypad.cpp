#include "Keypad.h"
#include <stdio.h>

void Keypad::Init()
{
	//addressing MCP
	pad.Init( I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x42, false);

	pad.Direction(mcp23017::PORT_A, 0xFF); //all input
	pad.Direction(mcp23017::PORT_B, 0xFF);

	pad.ConfigurePullUps(mcp23017::PORT_A, 0xFF); //all pull up
	pad.ConfigurePullUps(mcp23017::PORT_B, 0xFF);

	pad.MirrorInterrupts(true);
	pad.InterruptEnable(mcp23017::PORT_A, 0xFF); //all interrupt enabled
	pad.InterruptEnable(mcp23017::PORT_B, 0xFF); //all interrupt enabled

	pad.Read(mcp23017::PORT_A); //clear interrupt flag
	pad.Read(mcp23017::PORT_B);

	lastRead = 0xFFFF;
	solidTickCount = 0;
	enableKeyCheck = true;
}

bool Keypad::RegisterForCallback(KeypadInterrupt& callback)
{
	if (interruptListIndex > INTERRUPT_LIST_LENGTH)
	{
		return false;
	}
	interruptList[interruptListIndex] = InterruptCallback(&callback, &KeypadInterrupt::KeypadKeysPressed);
	++interruptListIndex;
	return true;
}

unsigned short Keypad::ReadKeys()
{
	unsigned char port1 = pad.Read(mcp23017::PORT_B);
	unsigned char port0 = pad.Read(mcp23017::PORT_A);
	return (port0 << 8) | (port1 & 0xFF);
}

void Keypad::SetKeyCheckEnabled(bool enabled)
{
	enableKeyCheck = enabled;
}

bool Keypad::CheckKeysPressed()
{
	if (enableKeyCheck == false)
		return false;
	unsigned short readed = ReadKeys();

	printf("Keypad DBG: %X\r\n", readed);

	if (readed != 0xFFFF)
	{
		//increment if readed is key press
		++solidTickCount;
		//printf("Keypad DBG: incremented counter %i\r\n", solidTickCount);
	}
	else
	{
		//reset if readed is NO key press
		solidTickCount = 0;
		//printf("Keypad DBG: reset counter 0\r\n");
	}
	if (solidTickCount == LONG_KEY_PRESS_INTERVAL)
	{
		//LONG_KEY_PRESS_INTERVAL times key press fire event.
		for (int i = 0; i < interruptListIndex; i++)
		{
			//printf("Keypad DBG: Calling interrupt for INDEX  %i\r\n", i);
			interruptList[i].execute(readed, true);
		}
		while (ReadKeys() != 0xFFFF)
		{}
		readed = 0xFFFF; //reset needed for LONG_KEY_PRESS_INTERVAL times key press
		solidTickCount = 0;
	}
	else if (lastRead != 0xFFFF && readed == 0xFFFF)
	{
		//on change from key press to NO key press fire event.
		for (int i = 0; i < interruptListIndex; i++)
		{
			//printf("Keypad DBG: Calling interrupt for CHANGE %i\r\n", i);
			interruptList[i].execute(lastRead, false);
		}
		solidTickCount = 0;
	}
	lastRead = readed;
	return (readed == 0xFFFF);
}
