#include "delay.h"

static unsigned long ulHclk = 0;

void SysTick_Init() {
	/****************************************
	 *SystemFrequency/1000      1ms         *
	 *SystemFrequency/10000     100us		*
	 *SystemFrequency/100000    10us        *
	 *SystemFrequency/1000000   1us         *
	 *****************************************/
	ulHclk = SystemCoreClock / 1000000;
	while (SysTick_Config(SystemCoreClock / 1000) != 0) { //every 1ms
	}
}

void SysTick_Handler(void)
{
	if (sysTickCounter != 0x00) {
		sysTickCounter--;
	}
}

void delay_setSysTick(unsigned long tick)
{
	sysTickCounter = tick;
}

unsigned long delay_getSysTick()
{
	return sysTickCounter;
}

void delay_sys_nus(unsigned int n) {
	/* not working when systick configured as 1ms
	sysTickCounter = n;
	while (sysTickCounter != 0) {
	}*/
}

void delay_sys_1ms(void) {
	sysTickCounter = 1;
	while (sysTickCounter != 0) {
	}
}

void delay_sys_nms(unsigned int n) {
	while (n--) {
		delay_sys_1ms();
	}
}

void __attribute__((naked))
SysCtlDelay(unsigned long ulCount)
{
    __asm("    subs    r0, #1\n"
          "    bne     SysCtlDelay\n"
          "    bx      lr");
}

inline void delay_nus(unsigned int n) {
	SysCtlDelay(ulHclk*n/4); //2.5 - 3
}

void delay_1ms(void) {
	delay_nus(1000); //6 instructions
}

void delay_nms(unsigned int n) {
	unsigned int i = 0;
	for (i = 0; i < n; i++)
	{
		delay_nus(1000);
	}
}

