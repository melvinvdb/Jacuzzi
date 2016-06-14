#include <stm32f10x_exti.h>
#include "usart.h"
#include "DS18B20.h"
#include <stdio.h>
#include "u8g_arm_uart.h"
#include "u8g.h"
#include "si4703.h"
#include "ovc3860.h"
#include "mcp23017.h"
#include "Keypad.h"
#include "Entertainment.h"

static u8g_t u8g;
char chTemp1[6];
char chTemp2[6];
char chTempSet[6];
int iChannel;
char chChannel[8];
int iStrength;
bool bStereo;

mcp23017 relais;
DS18B20 temp1;
mcp23017 leds;
Keypad keypad;
Entertainment entertainment;





#define WATERICON_X 0
#define WATERICON_Y 15
void drawWaterIcon()
{
	/*
	 * v 0.1
	 * /
	unsigned char i, j, x;
	for (j = 0; j <= 6; j+=3)
	{
		x = 0;
		for (i = 0; i <= 10; i+=2)
		{
			u8g_DrawLine(&u8g, WATERICON_X + i, WATERICON_Y + j + (x == 1), WATERICON_X + i + 1 , WATERICON_Y + j + (x == 0));
			x = !x;
		}
	}*/

	/*
	 * v 0.2
	 */
	unsigned char i, x = 0;
	u8g_DrawHLine(&u8g,WATERICON_X + 1 ,WATERICON_Y + 8 ,12);
	u8g_DrawVLine(&u8g,WATERICON_X     ,WATERICON_Y     ,8);
	u8g_DrawVLine(&u8g,WATERICON_X + 13,WATERICON_Y     ,8);
	for (i = 1; i <= 11; i+=2)
	{
		u8g_DrawLine(&u8g, WATERICON_X + i, WATERICON_Y + 1 + (x == 1), WATERICON_X + i + 1 , WATERICON_Y + 1 + (x == 0));
		x = !x;
	}
}

#define FROSTICON_X 0
#define FROSTICON_Y 3
void drawFrostIcon()
{
	u8g_DrawHLine(&u8g,FROSTICON_X + 1 ,FROSTICON_Y + 8 ,6);
	u8g_DrawVLine(&u8g,FROSTICON_X     ,FROSTICON_Y     ,8);
	u8g_DrawVLine(&u8g,FROSTICON_X + 7 ,FROSTICON_Y     ,8);

	u8g_DrawHLine(&u8g,FROSTICON_X + 4 ,FROSTICON_Y + 4 ,10);
	u8g_DrawLine(&u8g, FROSTICON_X + 11,FROSTICON_Y + 2 , FROSTICON_X + 13, FROSTICON_Y + 4);
	u8g_DrawLine(&u8g, FROSTICON_X + 13,FROSTICON_Y + 4 , FROSTICON_X + 11, FROSTICON_Y + 6);
}

#define SIGNALPERCENTAGE_X 114 //114
#define SIGNALPERCENTAGE_Y 0   //0
void drawSignalStrength(int strength)
{
	//percentage 0-5
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X     ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 3 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 6 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 9 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 12,SIGNALPERCENTAGE_Y + 15,2);

	if (strength >= 14) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X     ,SIGNALPERCENTAGE_Y + 12, 3);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 1 ,SIGNALPERCENTAGE_Y + 12, 3);
	}
	if (strength >= 24) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 3 ,SIGNALPERCENTAGE_Y + 9 , 6);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 4 ,SIGNALPERCENTAGE_Y + 9 , 6);
	}
	if (strength >= 34) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 6 ,SIGNALPERCENTAGE_Y + 6 , 9);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 7 ,SIGNALPERCENTAGE_Y + 6 , 9);
	}
	if (strength >= 44) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 9 ,SIGNALPERCENTAGE_Y + 3 , 12);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 10,SIGNALPERCENTAGE_Y + 3 , 12);
	}
	if (strength >= 54) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 12,SIGNALPERCENTAGE_Y     , 15);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 13,SIGNALPERCENTAGE_Y     , 15);
	}
}

void draw()
{
  //u8g_SetFont(&u8g, u8g_font_5x8);

  //u8g_SetFont(&u8g, u8g_font_6x10);

  //u8g_SetFont(&u8g, u8g_font_6x12);

  u8g_SetFont(&u8g, u8g_font_7x13);

  drawFrostIcon();

  u8g_DrawStr(&u8g, 20, 12, chTemp1);

  drawWaterIcon();

  u8g_DrawStr(&u8g, 20, 24, chTemp2);

  u8g_SetFont(&u8g, u8g_font_5x8);
  u8g_DrawStr(&u8g, 0, 36, "set");
  u8g_SetFont(&u8g, u8g_font_7x13);

  u8g_DrawStr(&u8g, 20, 36, chTempSet);

  u8g_DrawStr(&u8g, 0, 48, "Heating");

  if (si4703_PSNvalid)
  {
	  u8g_DrawStr(&u8g, 0, 64, si4703_programServiceName);
  }
  else
  {
	  u8g_DrawStr(&u8g, 0, 64, "lcd - st7920");
  }

  u8g_DrawVLine(&u8g,64,0 ,52);
  u8g_DrawHLine(&u8g,0 ,51,64);

  if (bStereo)
  {
	  u8g_SetFont(&u8g, u8g_font_5x8);
	  u8g_DrawStr(&u8g, 108, 8, "st");
	  u8g_SetFont(&u8g, u8g_font_7x13);
  }

  u8g_DrawStr(&u8g, 73, 26, chChannel);

  u8g_DrawStr(&u8g, 81, 38, "Source:");
  u8g_DrawStr(&u8g, 66, 50, "Bluetooth");

  drawSignalStrength(si4703_getSignalStrength());

  /*u8g_DrawBox(&u8g, 3,41,20,10);

  u8g_DrawFrame(&u8g, 25,41,20,10);

  u8g_DrawCircle(&u8g, 10,58,5,U8G_DRAW_ALL);

  u8g_DrawDisc(&u8g, 30,58,5,U8G_DRAW_ALL);


  u8g_DrawEllipse(&u8g,55,46,8,4,U8G_DRAW_ALL);

  u8g_DrawFilledEllipse(&u8g,75,46,8,4,U8G_DRAW_ALL);

  u8g_DrawHLine(&u8g,55,55,28);

  u8g_DrawVLine(&u8g,88,38,23);

  //u8g_DrawLine(&u8g,84,1,127,16);

  u8g_DrawRFrame(&u8g, 90,38,36,24,8);*/


}

int main(void)
{
	unsigned int i;

	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
	 */
	USART1_Init(115200);
	ovc3860_init();

	SysTick_Init();

	/*while (1)
	{
		printf("test\r\n");
		printf("test\r\n");
		printf("test\r\n");
		printf("test\r\n");
		printf("test\r\n");
		printf("test\r\n");
	}*/
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

	printf("MCP Initializing C++\n");
	relais.Init(I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x40, false);
	relais.Direction(relais.PORT_A, 0x00); //all output
	relais.Direction(relais.PORT_B, 0x00);
	relais.Write(relais.PORT_A, 0xFF); //all high
	relais.Write(relais.PORT_B, 0x00); //all low
	printf("MCP initialized\n");

	printf("DS18B20 Initializing C++\n");
	temp1.Init(RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0);
	temp1.Reset();
	temp1.Reset();
	temp1.ROMSkip();
	temp1.SetPrecision(0); //0 == 9 bits, 1 == 10 bits, 2 == 11 bits, 3 == 12 bits
	temp1.Reset();
	temp1.ROMRead();


	/*tDS18B20Dev temp1 = { RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0 };
	DS18B20Init(&temp1);
	DS18B20Reset(&temp1);

	DS18B20Reset(&temp1);
	DS18B20ROMSkip(&temp1);
	DS18B20SetPrecision(&temp1, 0);  //0 == 9 bits, 1 == 10 bits, 2 == 11 bits, 3 == 12 bits

	DS18B20Reset(&temp1);
	DS18B20ROMRead(&temp1);*/
	printf("DS18B20 family code response \r\n");
	for (i = 0; i < 8; i++)
	{
		printf("0x%X\r\n", temp1.ucROM[i]);
	}
	printf("DS18B20 done \r\n");

	printf("Display initializing\n");

	printf("Display enabling\n");
	GPIO_InitTypeDef PORT;
	// Set LCDEN as output
	PORT.GPIO_Pin = GPIO_Pin_14;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&PORT);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET); // set high
	printf("Display enabled\n");

	printf("OVC3860 enabling\n");
	// Set LCDEN as output
	PORT.GPIO_Pin = GPIO_Pin_13;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&PORT);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET); // set high
	printf("OVC3860 enabled\n");

	//u8g_InitComFn(&u8g, &u8g_dev_st7920_128x64_hw_spi, u8g_com_hw_spi_fn);  hw spi
	u8g_InitComFn(&u8g, &u8g_dev_st7920_128x64_8bit, u8g_com_uart_fn);


	ovc3860_playPause();

	printf("LEDS initializing\n");
	leds.Init(I2C1, RCC_APB1Periph_I2C1, GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_6, GPIO_Pin_7, 0x44, false);
	leds.Direction(leds.PORT_A, 0x00); //all output
	leds.Direction(leds.PORT_B, 0x00);
	leds.Write(leds.PORT_A, 0x01); //all low except one
	leds.Write(leds.PORT_B, 0x00); //all low

	printf("Keypad initializing\n");
	keypad.Init();
	entertainment.Init();
	keypad.RegisterForCallback(entertainment);
	//keypad.test(Keypad::InterruptCallback(&entertainment, &KeypadInterrupt::KeypadKeysPressed));

	printf("Starting main loop\n");
	while(1)
	{
		u8g_FirstPage(&u8g);
		do
		{
			//DS18B20Reset(&temp1);
			//DS18B20ROMSkip(&temp1);
			temp1.Reset();
			temp1.ROMSkip();
			DS18B20::simple_float sFloat;
			temp1.TempReadSimple(&sFloat);
			sprintf(chTemp1, "%d.%d�C", sFloat.integer, sFloat.fractional);
			sprintf(chTemp2, "%d.%d�C", sFloat.integer, sFloat.fractional);
			sprintf(chTempSet, "%d.%d�C", sFloat.integer, sFloat.fractional);
			draw();
		} while ( u8g_NextPage(&u8g) );

		keypad.GetKeysPressed();
		/*DS18B20Reset(&temp1);
		DS18B20ROMSkip(&temp1);
		DS18B20TempConvert(&temp1);*/
		temp1.Reset();
		temp1.ROMSkip();
		temp1.TempConvert();

		iChannel = si4703_getChannel();
		sprintf(chChannel, "%d.%dMHz", iChannel / 10, iChannel % 10);
		bStereo = si4703_isStereo();
		iStrength = si4703_getSignalStrength();
		si4703_checkRDS();

		printf("Strength: %ddBuV\r\n", iStrength);
		printf(chChannel);
		printf("\r\n");
		if (si4703_PSNvalid)
		{
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
