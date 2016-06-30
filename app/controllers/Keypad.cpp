#include "Keypad.h"
#include <stdio.h>

void Keypad::Init()
{
	//addressing MCP
	McpInit();

	lastRead = 0xFFFF;
	solidTickCount = 0;
	enableKeyCheck = true;
}

void Keypad::McpInit()
{
	bool mcpWorking[10];
	int mcpIndex = 0;
	mcpWorking[mcpIndex++] = pad.Init( I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x42, false);

	mcpWorking[mcpIndex++] = pad.Direction(mcp23017::PORT_A, 0xFF); //all input
	mcpWorking[mcpIndex++] = pad.Direction(mcp23017::PORT_B, 0xFF);

	mcpWorking[mcpIndex++] = pad.ConfigurePullUps(mcp23017::PORT_A, 0xFF); //all pull up
	mcpWorking[mcpIndex++] = pad.ConfigurePullUps(mcp23017::PORT_B, 0xFF);

	mcpWorking[mcpIndex++] = pad.MirrorInterrupts(true);
	mcpWorking[mcpIndex++] = pad.InterruptEnable(mcp23017::PORT_A, 0xFF); //all interrupt enabled
	mcpWorking[mcpIndex++] = pad.InterruptEnable(mcp23017::PORT_B, 0xFF); //all interrupt enabled

	char data;
	mcpWorking[mcpIndex++] = pad.Read(mcp23017::PORT_A, data); //clear interrupt flag
	mcpWorking[mcpIndex++] = pad.Read(mcp23017::PORT_B, data);

	keypadWorking = true;
	for (int i = 0; i < mcpIndex; i++)
	{
		if (mcpWorking[i] == false)
			keypadWorking = false;
	}
}

bool Keypad::IsKeypadWorking()
{
	return keypadWorking;
}

bool Keypad::RegisterForCallback(Keys& callback)
{
	if (interruptListIndex > INTERRUPT_LIST_LENGTH)
	{
		return false;
	}
	interruptList[interruptListIndex] = InterruptCallback(&callback, &Keys::KeyDataReceived);
	++interruptListIndex;
	return true;
}

unsigned short Keypad::ReadKeys()
{
	if (keypadWorking == false)
	{
		McpInit();
		if (keypadWorking == false)
			return 0xFFFF;
	}
	char port1;
	char port0;
	if (!pad.Read(mcp23017::PORT_B, port1))
	{
		keypadWorking = false;
		return 0xFFFF;
	}
	if (!pad.Read(mcp23017::PORT_A, port0))
	{
		keypadWorking = false;
		return 0xFFFF;
	}
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

	if (readed != 0xFFFF)
	{
		printf("Keypad DBG: %X\r\n", readed);
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
