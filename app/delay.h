#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"
{
#endif

static volatile unsigned long sysTickCounter = 0;
#define SYSTICK_1MS 1

#ifdef SYSTICK_1MS
#define SYSTICKDEVIDER 1000
#define SYSTICKFORMAT 1
#elif SYSTICK_10MS
#define SYSTICKDEVIDER 100
#define SYSTICKFORMAT 10
#elif SYSTICK_100MS
#define SYSTICKDEVIDER 10
#define SYSTICKFORMAT 100
#endif


void TimerInit();
void SetSysTick(unsigned long tick);
unsigned long GetSysTick();
extern inline unsigned long SysTickFormatMs(unsigned long timeInMs)
{
	return timeInMs / SYSTICKFORMAT;
}
#ifdef SYSTICK_1MS
void SysTickDelay1ms(void);
void SysTickDelayMs(unsigned int n);
#endif
uint32_t DwtGet(void);
extern inline uint8_t DwtCompare(int32_t tp)
{
  return (((int32_t)DwtGet() - tp) < 0);
}
void DwtDelayUs(uint32_t us);
void DwtDelay1ms();
void DwtDelayMs(uint32_t ms);
#ifdef __cplusplus
}
#endif

#endif
