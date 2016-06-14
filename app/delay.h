#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"
{
#endif

static volatile unsigned long sysTickCounter = 0;

void SysTick_Init();
void delay_setSysTick(unsigned long tick);
unsigned long delay_getSysTick();
void delay_sys_nus(unsigned int n);
void delay_sys_1ms(void);
void delay_sys_nms(unsigned int n);
void SysCtlDelay(unsigned long ulCount);
inline void delay_nus(unsigned int n);
void delay_1ms(void);
void delay_nms(unsigned int n);

#ifdef __cplusplus
}
#endif

#endif
