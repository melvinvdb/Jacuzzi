
#include "u8g_arm_uart.h"
#include "delay.h"

uint8_t control = ST7920_CMD;

/*========================================================================*/
/*
  The following delay procedures must be implemented for u8glib

  void u8g_Delay(uint16_t val)		Delay by "val" milliseconds
  void u8g_MicroDelay(void)		Delay be one microsecond
  void u8g_10MicroDelay(void)	Delay by 10 microseconds

*/

void u8g_Delay(uint16_t val)
{
	DwtDelayMs(val);
}
void u8g_MicroDelay(void)
{
	DwtDelayUs(1);
}
void u8g_10MicroDelay(void)
{
	DwtDelayUs(10);
}


//*************************************************************************************
//*************************************************************************************
uint8_t u8g_com_uart_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{ 
  switch(msg)
  {
    case U8G_COM_MSG_STOP:
    	//STOP THE DEVICE
    	break;
 
    case U8G_COM_MSG_INIT:
    	//INIT HARDWARE INTERFACES, TIMERS, GPIOS...
    	u8g_Delay(500);
    	init_UART3();
    	//init_LCD(); //set 8 bit etc
    	break;
 
    case U8G_COM_MSG_ADDRESS:  
    	//SWITCH FROM DATA TO COMMAND MODE (arg_val == 0 for command mode)
    	// define cmd (arg_val = 0) or data mode (arg_val = 1)
    	// cmd -
    	if (arg_val == 0)
    	{
    		control = ST7920_CMD;
    	}
    	else
    	{
    		control = ST7920_DATA;
    	}
    	break;

    case U8G_COM_MSG_RESET:
    	//TOGGLE THE RESET PIN ON THE DISPLAY BY THE VALUE IN arg_val
    	break;
 
    case U8G_COM_MSG_WRITE_BYTE:
    	//WRITE BYTE TO DEVICE
    	UART_write_byte(control << 1); //shift RS left, RW == 0 == write
    	UART_write_byte(arg_val);
    	DwtDelayUs(110); //required min 100us
    	//u8g_MicroDelay();
    	break;
 
    case U8G_COM_MSG_WRITE_SEQ:
    case U8G_COM_MSG_WRITE_SEQ_P:
    	//WRITE A SEQUENCE OF BYTES TO THE DEVICE
    	{
    		register uint8_t *ptr = arg_ptr;
    		//UART_write_byte((1 << 2) | (control << 1)); //0x04 for initialization of byte sequence, shift RS left, RW == 0 == write
    		while( arg_val > 0 )
    		{
    			UART_write_byte(control << 1); //shift RS left, RW == 0 == write
    			UART_write_byte( *ptr);
    			DwtDelayUs(110); //required min 100us
    			ptr++;
    			arg_val--;
    		}
    	}
    	break;
  }
  return 1;
}

//****************************************************************************
//****************************************************************************
void init_UART3(void)
{
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(ST7920_GPIO_CLOCK, ENABLE);

	/* Enable UART clock */
	RCC_APB1PeriphClockCmd(ST7920_UART_CLOCK, ENABLE);

	// Use PB10 and PB11
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = ST7920_TX_PORT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ST7920_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = ST7920_RX_PORT;
	GPIO_Init(ST7920_GPIO_PORT, &GPIO_InitStructure);

	// USART3
	USART_InitStructure.USART_BaudRate = ST7920_UART_BAUDRATE;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;

	USART_Init(ST7920_UART_PORT, &USART_InitStructure);

	// Enable usart
	USART_Cmd(ST7920_UART_PORT, ENABLE);
}


void UART_write_byte(unsigned char data)
{
    USART_SendData(USART3, (u8)data);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}
