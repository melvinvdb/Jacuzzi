#include <stm32f10x_exti.h>
#include "usart.h"
#include <stdio.h>
#include "Display.h"
#include "Keypad.h"
#include "RelayBoard.h"
#include "LEDS.h"
#include "EntController.h"
#include "SpaController.h"


int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
	 */
	USART1_Init(115200);
	TimerInit();


	/*printf("start delay test\r\n");
	GPIO_InitTypeDef tPORT;
	tPORT.GPIO_Pin = GPIO_Pin_5;
	tPORT.GPIO_Mode = GPIO_Mode_Out_PP;
	tPORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&tPORT);
	GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET); // set high
	while (1)
	{
		GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET); // set high
		DwtDelayUs(1000);
		GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_RESET); // set high
		DwtDelayUs(1000);
	}*/




	printf("Controllers and stuff initializing\n");
	RelayBoard::getInstance().Init();
	LEDS::getInstance().Init();
	Display& display = Display::getInstance();
	display.Init();
	display.SetActiveScreen(Display::SECONDSCREEN);
	display.WriteText(0, "Starting...");
	display.Draw();
	display.SetDisplayState(true);

	Keypad& keypad = Keypad::getInstance();
	keypad.Init();
	EntController entertainment;
	entertainment.Init();
	keypad.RegisterForCallback(entertainment);
	SpaController spa;
	spa.Init();
	keypad.RegisterForCallback(spa);

	display.SetActiveScreen(Display::MAINSCREEN);
	printf("Starting main loop\n");
	unsigned long benchtest = 0;
	unsigned long result = 0;
	while(1)
	{

		//benchtest = DwtGet();
		keypad.CheckKeysPressed();
		//result = ((DwtGet()-benchtest)/72000);
		//printf("KP %d ms\r\n", result);

		//benchtest = DwtGet();
		spa.Monitor();
		//result = ((DwtGet()-benchtest)/72000);
		//printf("SPA %d ms\r\n", result);

		//benchtest = DwtGet();
		entertainment.Monitor();
		//result = ((DwtGet()-benchtest)/72000);
		//printf("ENT %d ms\r\n", result);

		benchtest = DwtGet();
		Display::getInstance().Draw();
		result = ((DwtGet()-benchtest)/72000);
		printf("DISPLAY %d ms\r\n", result);

	}
}

/**********************************************************
 * USART1 interrupt request handler: on reception of a
 * character 't', toggle LED and transmit a character 'T'
 *********************************************************/
void USART1_IRQHandler(void)
{
    /* RXNE handler */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* If received 't', toggle LED and transmit 'T' */
        if((char)USART_ReceiveData(USART1) == 't')
        {
        	printf("Test 123\r\n");
            /* Wait until Tx data register is empty, not really
             * required for this example but put in here anyway.
             */
            /*
            while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            {
            }*/
        }
    }

    /* ------------------------------------------------------------ */
    /* Other USART1 interrupts handler can go here ...             */
}

/**********************************************************
 * USART1 interrupt request handler: on reception of a
 * character 't', toggle LED and transmit a character 'T'
 *********************************************************/
/*void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        char byte = (char)USART_ReceiveData(USART2);
        printf("IT %02x\r\n", byte);
    }
}
*/
/*
void EXTI0_IRQHandler(void)
{
	static u8 i = 0;

    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        unsigned int j = 0;
        while (j != 200000) ++j;
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != Bit_RESET) {
			return;
		}

    	i = (i + 1) % 4;
		ds18b20_set_precission(i);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

*/
