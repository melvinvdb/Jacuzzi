#include <stm32f10x_exti.h>
#include "usart.h"
#include <stdio.h>
#include "si4703.h"
#include "ovc3860.h"
#include "Display.h"
#include "Keypad.h"
#include "RelayBoard.h"
#include "LEDS.h"
#include "EntController.h"
#include "SpaController.h"


int iChannel;
char chChannel[8];
int iStrength;
bool bStereo;

int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
	 */
	USART1_Init(115200);
	ovc3860_init();

	SysTick_Init();


	printf("SI4703 initializing\n");
	si4703_init();
	//si4703_setChannel(972); slam fm
	si4703_setChannel(1010);
	delay_nms(800);
	iChannel = si4703_getChannel();
	printf("Freq: %d\r\n", iChannel);
	si4703_seek(Si4703_SEEK_UP,Si4703_WRAP_ON);
	si4703_setVolume(10);
	printf("SI4703 initialized\n");



	printf("OVC3860 enabling\n");
	GPIO_InitTypeDef PORT;
	PORT.GPIO_Pin = GPIO_Pin_13;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&PORT);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET); // set high
	printf("OVC3860 enabled\n");



	ovc3860_playPause();


	printf("Controllers and stuff initializing\n");
	Display& display = Display::getInstance();
	display.Init();
	display.SetActiveScreen(Display::SECONDSCREEN);
	display.WriteText(0, "Starting...");
	display.Draw();
	display.SetDisplayState(true);

	RelayBoard::getInstance().Init();
	Keypad::getInstance().Init();
	LEDS::getInstance().Init();
	EntController entertainment;
	entertainment.Init();
	SpaController spa;
	spa.Init();

	display.SetActiveScreen(Display::MAINSCREEN);
	printf("Starting main loop\n");
	while(1)
	{


		display.SetAudioState(true);
		display.SetAudioSource("Bluetooth");
		display.SetRadioState(true);
		iChannel = si4703_getChannel();
		sprintf(chChannel, "%d.%dMHz", iChannel / 10, iChannel % 10);
		display.SetRadioChannel(chChannel);
		display.SetRadioStereo(si4703_isStereo());
		display.SetRadioSignalstrength(si4703_getSignalStrength());


		Keypad::getInstance().CheckKeysPressed();
		spa.Monitor();
		Display::getInstance().Draw();


		si4703_checkRDS();

		/*printf("Strength: %ddBuV\r\n", iStrength);
		printf(chChannel);
		printf("\r\n");*/
		if (si4703_PSNvalid)
		{
			display.SetStatusBar(si4703_programServiceName);
			printf("RDS:");
			printf(si4703_programServiceName);
			printf(" \r\n");
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void)
{
	TimeTick_Decrement();
}
#ifdef __cplusplus
}
#endif

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
