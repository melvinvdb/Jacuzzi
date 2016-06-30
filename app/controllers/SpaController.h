#ifndef __SPACONTROLLER_H__
#define __SPACONTROLLER_H__

#include "Keypad.h"
#include "Keys.h"
#include "LEDS.h"
#include "RelayBoard.h"
#include "Display.h"
#include "TemperatureSensor.h"

class SpaController : public Keys
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
	TemperatureSensor tempBath;
	TemperatureSensor tempOut;
	unsigned long tempReadTime;
	const unsigned long tempReadInterval = 1000; //ms read interval
	float targetTemp;
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
	bool prevKeypadWorking;

	void WriteFloatToDisplay(const float f, const char index);
	void CheckTemp(const float bath);
	void SwitchHeater(const bool on);
	bool IsWater();
public:
	SpaController();
	void Init();
	void KeyDataReceived(const unsigned short keys, const bool keyshold);
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
