#ifndef __USART_H__
#define __USART_H__
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_rcc.h>
#include <misc.h>

#ifdef __cplusplus
extern "C"
{
#endif

void USART1_IRQHandler(void);

void USART1_Init(unsigned int speed);
void USART2_Init(unsigned int speed);
void USART2_SetTX(uint8_t high);

void usart1_print(char* c);
void usart2_print(char* c);

void usart_string_append(char c);
void usart_clear_string();

volatile char* usart_get_string();
volatile u8 usart_get_string_length();

#ifdef __cplusplus
}
#endif

#endif
