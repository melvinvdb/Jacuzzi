#include "Keypad.h"
//#include "KeypadInterrupt.h"
#include <stdio.h>
#include "Entertainment.h"

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
/*
void Keypad::test()//InterruptCallback cb)
{
	//singleInterrupt = cb;
	Entertainment e;
	e.Init();
	singleInterrupt = InterruptCallback(&e, &KeypadInterrupt::KeypadKeysPressed);
}*/

void Keypad::GetKeysPressed()
{
	unsigned char port1 = pad.Read(mcp23017::PORT_B);
	unsigned char port0 = pad.Read(mcp23017::PORT_A);
	unsigned short readed = (port0 << 8) | (port1 & 0xFF);

	printf("%X\r\n", readed);

	for (int i = 0; i < interruptListIndex; i++)
	{
		interruptList[i].execute(readed);
	}
}

/*

CheckKeyPress()


GetKeysPressed()


IsKeyPressed(Key)

*/
