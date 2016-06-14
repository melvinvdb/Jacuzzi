#include "LEDS.h"
#include <stdio.h>

void LEDS::Init()
{
	printf("LEDS: Init() ........\r\n");
	//addressing MCP
	mcp.Init(I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x44, false);
	mcp.Direction(mcp23017::PORT_A, 0x00); //all output
	mcp.Direction(mcp23017::PORT_B, 0x00);
	mcp.Write(mcp23017::PORT_A, 0x00); //all low
	mcp.Write(mcp23017::PORT_B, 0x00); //all low
	state = 0x0000;
	printf("LEDS: Init() complete\r\n");
}

void LEDS::SwitchLed(LEDS::Leds led, bool enabled)
{
	printf("RelayBoard() Want to switch led state %X with %X to %s\r\n", state, led, (enabled == true ? "true" : "false"));
	if (((state & led) && enabled == false) || ((state & led) == 0 && enabled == true))
	{

			printf("RelayBoard() Switching state\r\n");
			//switch state
			state = state ^ led;
			mcp.Write(mcp23017::PORT_A, (state >> 8));
			mcp.Write(mcp23017::PORT_B, (state & 0xFF));

	}
}
