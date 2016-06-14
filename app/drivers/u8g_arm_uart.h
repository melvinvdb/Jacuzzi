#ifndef _U8G_ARM_H
#define _U8G_ARM_H

#include "u8g.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stm32f10x_usart.h>
#include "delay.h"

#ifdef __cplusplus
extern "C"
{
#endif

//
//! Config the UART port
//
#define ST7920_UART_PORT        USART3
#define ST7920_GPIO_CLOCK		RCC_APB2Periph_GPIOB
#define ST7920_UART_CLOCK		RCC_APB1Periph_USART3
#define ST7920_TX_PORT			GPIO_Pin_10
#define ST7920_RX_PORT			GPIO_Pin_11
#define ST7920_GPIO_PORT		GPIOB
#define ST7920_UART_BAUDRATE	38400

#define ST7920_CMD				0
#define ST7920_DATA				1




//*************************************************************************
void init_UART3(void);
void UART_write_byte(unsigned char data);

uint8_t u8g_com_uart_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);

#ifdef __cplusplus
}
#endif

#endif


