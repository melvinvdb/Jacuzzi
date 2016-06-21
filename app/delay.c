#include "delay.h"

static unsigned long ulHclk = 0;

void TimerInit() {
	/****************************************
	 *SystemFrequency/1000      1ms         *
	 *SystemFrequency/10000     100us		*
	 *SystemFrequency/100000    10us        *
	 *SystemFrequency/1000000   1us         *
	 *****************************************/
	ulHclk = SystemCoreClock / 1000000;
	while (SysTick_Config(SystemCoreClock / SYSTICKDEVIDER) != 0) {
	}
	//Data Watchpoint and Trace Registers (DWT) for accurate delay
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}
}

void SysTick_Handler(void)
{
	++sysTickCounter;
}

void SetSysTick(unsigned long tick)
{
	sysTickCounter = tick;
}

unsigned long GetSysTick()
{
	return sysTickCounter;
}

#ifdef SYSTICK_1MS
void SysTickDelay1ms(void) {
	unsigned long currSysTick = sysTickCounter;
	while (currSysTick == sysTickCounter) { //wait for a change
	}
}

void SysTickDelayMs(unsigned int n) {
	while (n--) {
		SysTickDelay1ms();
	}
}
#endif

void DwtDelayUs(uint32_t us) // microseconds
{
  int32_t tp = DwtGet() + us * (SystemCoreClock/1000000);
  while (DwtCompare(tp));
}

void DwtDelay1ms() // milliseconds
{
	DwtDelayUs(1000);
}

void DwtDelayMs(uint32_t ms) // milliseconds
{
	while (ms--) {
		DwtDelayUs(1000);
	}
}
