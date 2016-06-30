#include "RelayBoard.h"
#include <stdio.h> //printf

void RelayBoard::Init()
{
	printf("RelayBoard: Init() ........\r\n");
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef tPORT;
	tPORT.GPIO_Pin = GPIO_Pin_8;
	tPORT.GPIO_Mode = GPIO_Mode_Out_PP;
	tPORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&tPORT);
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_RESET); // set low to reset every device
	SysTickDelayMs(200); //resetting
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET); // set high
	SysTickDelayMs(1);	 //startup time
	relays.Init(I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x40, true); //i'm the first one to initialize the i2c
	relays.Direction(mcp23017::PORT_A, 0x00); //all output
	relays.Direction(mcp23017::PORT_B, 0x00);
	relays.Write(mcp23017::PORT_A, 0x00); //all low
	relays.Write(mcp23017::PORT_B, 0x00); //all low
	state = 0x0000; //initial state
	printf("RelayBoard: Init() complete\r\n");
}

void RelayBoard::SwitchRelay(RelayBoard::Relays relay, bool enabled)
{
	printf("RelayBoard() Want to switch relay state %X with %X to %s\r\n", state, relay, (enabled == true ? "true" : "false"));
	if (((state & relay) && enabled == false) || ((state & relay) == 0 && enabled == true))
	{

			printf("RelayBoard() Switching state\r\n");
			//switch state
			state = state ^ relay;
			relays.Write(mcp23017::PORT_A, (state >> 8));
			relays.Write(mcp23017::PORT_B, (state & 0xFF));

	}
}

void RelayBoard::ResetToSafety()
{
	state = state & ~safetyReset; //reset all relays to low except entertainment
	relays.Write(mcp23017::PORT_A, (state >> 8));
	relays.Write(mcp23017::PORT_B, (state & 0xFF));
}

void RelayBoard::ResetAudio()
{
	state = state & ~audioReset; //reset all audio relays and entertainment power
	relays.Write(mcp23017::PORT_A, (state >> 8));
	relays.Write(mcp23017::PORT_B, (state & 0xFF));
}
