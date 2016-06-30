#include "Display.h"

void Display::Init()
{
	u8g_InitComFn(&u8g, &u8g_dev_st7920_128x64_8bit, u8g_com_uart_fn);

	GPIO_InitTypeDef PORT;
	// Set LCDEN as output
	PORT.GPIO_Pin = ENABLE_PIN;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&PORT);
	GPIO_WriteBit(GPIOB,ENABLE_PIN,Bit_RESET); // set low for off
	PORT.GPIO_Pin = RW_PIN;
	GPIO_Init(GPIOB,&PORT);
	GPIO_WriteBit(GPIOB,RW_PIN,Bit_SET); // set high for write
	ClearErrorText();
	waterState = false;
	tempState = false;
	ClearStatusBar();
	ClearSecondScreen();
}

void Display::SetDisplayState(bool enable)
{
	BitAction b = (enable == true ? Bit_SET : Bit_RESET);
	GPIO_WriteBit(GPIOB,ENABLE_PIN,b);
}

void Display::Draw()
{
	u8g_FirstPage(&u8g);
	do
	{
		if (activeScreen == 0)
			DrawMainScreen();
		else
			DrawSecondScreen();
	} while ( u8g_NextPage(&u8g) );
}

void Display::DrawMainScreen()
{
	DrawFrostIcon();
	DrawWaterIcon();
	u8g_SetFont(&u8g, u8g_font_5x8);
	u8g_DrawStr(&u8g, 0, 36, "set");

	u8g_SetFont(&u8g, u8g_font_7x13);
	for (int i = 0; i < 3; i++)
	{
		u8g_DrawStr(&u8g, 20, ((i+1)*12), temp[i]);
	}

	if (waterState == true)
	{
		u8g_DrawStr(&u8g, 0, 48, "NO WATER");
	}
	else if (tempState == true)
	{
		u8g_DrawStr(&u8g, 0, 48, "NO SENSOR");
	}
	else if (strlen(errorText) > 0)
	{
		u8g_DrawStr(&u8g, 0, 48, errorText);
	}
	else if (heatingState)
		u8g_DrawStr(&u8g, 0, 48, "Heating");

	u8g_DrawStr(&u8g, 0, 62, statusBar);

	u8g_DrawVLine(&u8g,64,0 ,52);
	u8g_DrawHLine(&u8g,0 ,51,64);

	if (radioState)
	{
		if (radioStereo)
		{
			u8g_SetFont(&u8g, u8g_font_5x8);
			u8g_DrawStr(&u8g, 108, 8, "st");
			u8g_SetFont(&u8g, u8g_font_7x13);
		}
		u8g_DrawStr(&u8g, 73, 26, radioChannel);
		DrawSignalStrength();
	}
	if (audioState)
	{
		u8g_DrawStr(&u8g, 81, 38, "Source:");
		u8g_DrawStr(&u8g, 66, 50, audioSource);
	}
}

void Display::DrawSecondScreen()
{
	for (int i = 0; i < DISPLAY_SCREEN_LINE_COUNT; i++)
	{
		if (screenHLinesWidth[i] > 0)
		{
			u8g_DrawHLine(&u8g, screenHLinesX[i], screenHLinesY[i], screenHLinesWidth[i]);
		}
		if (screenVLinesWidth[i] > 0)
		{
			u8g_DrawHLine(&u8g, screenVLinesX[i], screenVLinesY[i], screenVLinesWidth[i]);
		}
	}
	u8g_SetFont(&u8g, u8g_font_7x13);
	for (int i = 0; i < DISPLAY_SCREEN_TEXT_COUNT; i++)
	{
		u8g_DrawStr(&u8g, 0, ((i+1)*12), screenText[i]);
	}
}

void Display::SetActiveScreen(const Screens s)
{
	activeScreen = s;
}

void Display::SetTemp(const unsigned char tempNr, const char * text)
{
	if (strlen(text) >= 7)
		return;
	if (tempNr >= 3)
		return;
	snprintf(temp[tempNr], sizeof(temp[tempNr]), text); //safe, null terminated
}

void Display::SetHeatingState(const bool state)
{
	heatingState = state;
}

void Display::SetErrorText(const char * text)
{
	if (strlen(text) >= DISPLAY_ERROR_TEXT_LENGTH)
		return;
	snprintf(errorText, DISPLAY_ERROR_TEXT_LENGTH, text); //safe, null terminated
}

void Display::ClearErrorText()
{
	memset(errorText, 0, DISPLAY_ERROR_TEXT_LENGTH);
}

void Display::SetNoWaterAvailable(const bool state)
{
	waterState = state;
}

void Display::SetNoTempAvailable(const bool state)
{
	tempState = state;
}

void Display::SetStatusBar(const char * text)
{
	if (strlen(text) >= DISPLAY_STATUS_BAR_LENGTH)
		return;
	snprintf(statusBar, DISPLAY_STATUS_BAR_LENGTH, text); //safe, null terminated
}

void Display::ClearStatusBar()
{
	memset(statusBar, 0, DISPLAY_STATUS_BAR_LENGTH);
}

void Display::SetAudioState(const bool enabled)
{
	audioState = enabled;
}

void Display::SetAudioSource(const char * text)
{
	if (strlen(text) >= DISPLAY_AUDIO_SOURCE_LENGTH)
		return;
	snprintf(audioSource, DISPLAY_AUDIO_SOURCE_LENGTH, text); //safe, null terminated
}

void Display::SetRadioState(const bool enabled)
{
	radioState = enabled;
}

void Display::SetRadioStereo(const bool stereo)
{
	radioStereo = stereo;
}

void Display::SetRadioChannel(uint16_t channel)
{
	uint16_t integer = channel / 100;
	uint16_t fractional = (channel % 100) / 10;
	snprintf(radioChannel, 9, "%3d.%1dMHz", integer, fractional);
}

void Display::SetRadioSignalstrength(const char strength)
{
	radioSignal = strength;
}
void Display::WriteText(const unsigned char index, const char * text)
{
	if (strlen(text) >= DISPLAY_SCREEN_TEXT_LENGTH)
		return;
	if (index >= DISPLAY_SCREEN_TEXT_COUNT)
		return;
	snprintf(screenText[index], DISPLAY_SCREEN_TEXT_LENGTH, text); //safe, null terminated
}

void Display::WriteHLine(const unsigned char index, const uint8_t x, const uint8_t y, const uint8_t w)
{
	if (index >= DISPLAY_SCREEN_LINE_COUNT)
		return;
	screenHLinesWidth[index] = w;
	screenHLinesX[index] = x;
	screenHLinesY[index] = y;
}

void Display::WriteVLine(const unsigned char index, const uint8_t x, const uint8_t y, const uint8_t w)
{
	if (index >= DISPLAY_SCREEN_LINE_COUNT)
		return;
	screenVLinesWidth[index] = w;
	screenVLinesX[index] = x;
	screenVLinesY[index] = y;
}

void Display::ClearSecondScreen()
{
	for (int i = 0; i < DISPLAY_SCREEN_TEXT_LENGTH; i++)
	{
		memset(screenText[i], 0, strlen(screenText[i]));
	}
	for (int i = 0; i < DISPLAY_SCREEN_LINE_COUNT; i++)
	{
		screenHLinesWidth[i] = 0;
		screenVLinesWidth[i] = 0;
	}
}

void Display::DrawWaterIcon()
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

void Display::DrawFrostIcon()
{
	u8g_DrawHLine(&u8g,FROSTICON_X + 1 ,FROSTICON_Y + 8 ,6);
	u8g_DrawVLine(&u8g,FROSTICON_X     ,FROSTICON_Y     ,8);
	u8g_DrawVLine(&u8g,FROSTICON_X + 7 ,FROSTICON_Y     ,8);

	u8g_DrawHLine(&u8g,FROSTICON_X + 4 ,FROSTICON_Y + 4 ,10);
	u8g_DrawLine(&u8g, FROSTICON_X + 11,FROSTICON_Y + 2 , FROSTICON_X + 13, FROSTICON_Y + 4);
	u8g_DrawLine(&u8g, FROSTICON_X + 13,FROSTICON_Y + 4 , FROSTICON_X + 11, FROSTICON_Y + 6);
}

void Display::DrawSignalStrength()
{
	//percentage 0-5
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X     ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 3 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 6 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 9 ,SIGNALPERCENTAGE_Y + 15,2);
	u8g_DrawHLine(&u8g,SIGNALPERCENTAGE_X + 12,SIGNALPERCENTAGE_Y + 15,2);

	if (radioSignal >= 14) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X     ,SIGNALPERCENTAGE_Y + 12, 3);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 1 ,SIGNALPERCENTAGE_Y + 12, 3);
	}
	if (radioSignal >= 24) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 3 ,SIGNALPERCENTAGE_Y + 9 , 6);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 4 ,SIGNALPERCENTAGE_Y + 9 , 6);
	}
	if (radioSignal >= 34) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 6 ,SIGNALPERCENTAGE_Y + 6 , 9);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 7 ,SIGNALPERCENTAGE_Y + 6 , 9);
	}
	if (radioSignal >= 44) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 9 ,SIGNALPERCENTAGE_Y + 3 , 12);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 10,SIGNALPERCENTAGE_Y + 3 , 12);
	}
	if (radioSignal >= 54) {
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 12,SIGNALPERCENTAGE_Y     , 15);
		u8g_DrawVLine(&u8g,SIGNALPERCENTAGE_X + 13,SIGNALPERCENTAGE_Y     , 15);
	}
}
