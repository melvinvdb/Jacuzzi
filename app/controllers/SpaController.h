#ifndef __SPACONTROLLER_H__
#define __SPACONTROLLER_H__

#include "Keypad.h"
#include "KeypadInterrupt.h"
#include "LEDS.h"
#include "RelayBoard.h"
#include "Display.h"
#include "DS18B20.h"

class SpaController : public KeypadInterrupt
{
private:
#define SPA_MIN_TEMP 10
#define SPA_MAX_TEMP 45
#define SPA_INC_TEMP 0.5f
#define SPA_WATER_PORT		GPIOA
#define SPA_WATER_PIN		GPIO_Pin_4
	Keypad& keypad;
	LEDS& leds;
	RelayBoard& relayBoard;
	Display& display;
	DS18B20 tempBath;
	DS18B20 tempOut;
	bool prevTempBathFound;
	bool tempBathFound;
	bool prevTempOutFound;
	bool tempOutFound;
	float targetTemp;
	bool waterDetected;
	bool stateHeat; 		//heat state
	bool stateCirc; 		//circulation state
	bool statePower;		//power state
	/******RELAY STATES****/
	bool relayJets;
	bool relayGas;
	bool relayHeat;
	bool relayCirc;
	bool relayLED1;
	bool relayLED2;

	void WriteFloatToDisplay(const float f, const char index);
	void CheckTemp(const float bath);
	void SwitchHeater(const bool on);
	bool IsWater();
public:
	SpaController();
	void Init();
	void KeypadKeysPressed(const unsigned short keys, const bool keyshold);
	void Monitor();
	void IncrementTemp();
	void DecrementTemp();
	void ToggleJets();
	void ToggleGas();
	void ToggleHeat();
	void ToggleCirc();
	void ToggleLED1();
	void ToggleLED2();
	void SwitchOff();
	void TogglePower();
};

#endif
