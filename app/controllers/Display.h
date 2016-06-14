#ifndef __DISPLAY_H__
#define __DISPLAY_H__

extern "C" {
#include "u8g_arm_uart.h"
#include "u8g.h"
#include <stdio.h>
#include <cstring>
}

class Display
{
public:
	typedef enum Screens : char { MAINSCREEN=0, SECONDSCREEN=1} Screens;
private:
	u8g_t u8g;
	char activeScreen;
	static const uint16_t ENABLE_PIN = GPIO_Pin_14;
	#define DISPLAY_STATUS_BAR_LENGTH 20
	#define DISPLAY_AUDIO_SOURCE_LENGTH 10
	char temp[3][7]; //3 temperatures 7 characters: 99.9°C\0
	bool heatingState; //heating state if is heating
	char statusBar[DISPLAY_STATUS_BAR_LENGTH]; //status bar bottom of screen
	//*******RADIO PART*******
	bool radioState; //if draw radio is enabled
	bool radioStereo; //if radio reception is stereo
	char radioChannel[9]; //radio channel 999.9MHz\0
	char radioSignal; //signal strength.
	//*******END RADIO PART***
	bool audioState; //if draw source is enabled
	char audioSource[DISPLAY_AUDIO_SOURCE_LENGTH]; //Audio source like Bluetooth\0

	//*******SECOND SCREEN****
	#define DISPLAY_SCREEN_TEXT_COUNT 5		//5 lines of 20 characters
	#define DISPLAY_SCREEN_TEXT_LENGTH 20	//5 lines of 20 characters
	#define DISPLAY_SCREEN_LINE_COUNT 4		//provide 4 line buffers
	char screenText[DISPLAY_SCREEN_TEXT_COUNT][DISPLAY_SCREEN_TEXT_LENGTH];
	uint8_t screenHLinesWidth[DISPLAY_SCREEN_LINE_COUNT]; //won't draw if width == 0
	uint8_t screenHLinesX[DISPLAY_SCREEN_LINE_COUNT];
	uint8_t screenHLinesY[DISPLAY_SCREEN_LINE_COUNT];
	uint8_t screenVLinesWidth[DISPLAY_SCREEN_LINE_COUNT]; //won't draw if width == 0
	uint8_t screenVLinesX[DISPLAY_SCREEN_LINE_COUNT];
	uint8_t screenVLinesY[DISPLAY_SCREEN_LINE_COUNT];

	void DrawMainScreen();
	void DrawSecondScreen();
	#define WATERICON_X 0
	#define WATERICON_Y 15
	void DrawWaterIcon();
	#define FROSTICON_X 0
	#define FROSTICON_Y 3
	void DrawFrostIcon();
	#define SIGNALPERCENTAGE_X 114 //114
	#define SIGNALPERCENTAGE_Y 0   //0
	void DrawSignalStrength();
public:
	static Display& getInstance()
	{
		static Display instance;
		return instance;
	}
	void Init();
	void SetDisplayState(const bool enable);
	void Draw();
	void SetActiveScreen(const Screens s);
	void SetTemp(const unsigned char tempNr, const char * text);
	void SetHeatingState(const bool state);
	void SetStatusBar(const char * text);
	void ClearStatusBar();
	void SetAudioState(const bool enabled);
	void SetAudioSource(const char * text);
	void SetRadioState(const bool enabled);
	void SetRadioStereo(const bool stereo);
	void SetRadioChannel(const char * channel);
	void SetRadioSignalstrength(const char strength);
	void WriteText(const unsigned char index, const char * text);
	void WriteHLine(const unsigned char index, const uint8_t x, const uint8_t y, const uint8_t w);
	void WriteVLine(const unsigned char index, const uint8_t x, const uint8_t y, const uint8_t w);
	void ClearSecondScreen();
};


#endif
