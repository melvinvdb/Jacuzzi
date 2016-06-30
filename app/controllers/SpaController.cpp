#include "SpaController.h"
#include <stdio.h> //printf
SpaController::SpaController() : keypad(Keypad::getInstance()), leds(LEDS::getInstance()), relayBoard(RelayBoard::getInstance()), display(Display::getInstance())
{}

void SpaController::Init()
{
	printf("SpaController: Init() ........\r\n");
	keypad.RegisterForCallback(*this);
	targetTemp = 35.0;
	stateCirc = false;
	stateHeat = false;
	statePower = true;
	relayJets = false;
	relayGas  = false;
	relayHeat = false;
	relayCirc = false;
	relayLED1 = false;
	relayLED2 = false;
	prevKeypadWorking = keypad.IsKeypadWorking();
	if (prevKeypadWorking == false)
		display.SetStatusBar("Controlpanel fault");
	WriteFloatToDisplay(targetTemp, 2);
	GPIO_InitTypeDef PORT;
	// Set LCDEN as output
	PORT.GPIO_Pin = SPA_WATER_PIN;
	PORT.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SPA_WATER_PORT,&PORT);
	tempBath.Init(RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0);
	tempOut.Init(RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_1);
	tempReadTime = GetSysTick();
	leds.SwitchLed(LEDS::POWER, true);
	printf("SpaController: Init() complete\r\n");
}

void SpaController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	//printf("SpaController: KeypadKeysPressed() long press: %s\r\n", keyshold ? "true" : "false");
	//printf("SpaControler DBG: %X\r\n", keys);
	if ((keys | Keypad::JET) == Keypad::JET)
		ToggleJets();
	else if ((keys | Keypad::GAS) == Keypad::GAS)
		ToggleGas();
	else if ((keys | Keypad::HEAT) == Keypad::HEAT)
		ToggleHeat();
	else if ((keys | Keypad::CIRC) == Keypad::CIRC)
		ToggleCirc();
	else if ((keys | Keypad::LED1) == Keypad::LED1)
		ToggleLED1();
	else if ((keys | Keypad::LED2) == Keypad::LED2)
		ToggleLED2();
	else if ((keys | Keypad::TEMPP) == Keypad::TEMPP)
		IncrementTemp();
	else if ((keys | Keypad::TEMPM) == Keypad::TEMPM)
		DecrementTemp();
	else if ((keys | Keypad::POWER) == Keypad::POWER)
		TogglePower();
}

void SpaController::Monitor()
{
	bool water = IsWater();
	//check temperature
	if ((unsigned long)(GetSysTick() - tempReadTime) >= SysTickFormatMs(tempReadInterval))
	{
		printf("Checking temperature\r\n");
		bool tempBathFound = tempBath.ReadTemperature();
		bool tempOutFound = tempOut.ReadTemperature();
		char cTemp[7];
		bool converted = tempBath.GetTemperature(cTemp);
		if (converted || !tempBathFound)
			display.SetTemp(1, cTemp);
		converted = tempOut.GetTemperature(cTemp);
		if (converted || !tempOutFound)
			display.SetTemp(0, cTemp);

		display.SetNoTempAvailable(!tempBathFound);
		if (tempBathFound == false) //a change in temp sensor state
		{
			SwitchOff();
		}
		float tempf = tempBath.GetTemperature();
		if (water && tempBath.TempIsValid())
		{
			CheckTemp(tempf);
		}
		tempBath.StartConversion();
		tempOut.StartConversion();
		tempReadTime = GetSysTick();
	}

	display.SetNoWaterAvailable(!water);
	if (water == false) //no water state
	{
		SwitchOff();
	}

	if (keypad.IsKeypadWorking() != prevKeypadWorking)
	{
		if (keypad.IsKeypadWorking() == false)
			display.SetStatusBar("Controlpanel fault");
		else
			display.ClearStatusBar();
		prevKeypadWorking = keypad.IsKeypadWorking();
	}

}

void SpaController::CheckTemp(const float bath)
{
	if (stateHeat == false)
	{
		if (relayHeat)
			SwitchHeater(false);
		return;
	}
	if (bath < targetTemp && relayHeat == false)
	{
		printf("Switching heater on\r\n");
		SwitchHeater(true);
	}
	else if (bath > targetTemp && relayHeat == true)
	{
		printf("Switching heater off\r\n");
		SwitchHeater(false);
	}
}

void SpaController::SwitchHeater(const bool on)
{
	if (on)
	{
		//enable circ and heater
		relayCirc = true;
		relayHeat = true;
	}
	else //we don't want to heat
	{
		if (relayHeat)
		{
			//disable heating immediately
			relayHeat = false;
		}
		if (stateCirc == false)
		{
			//we don't want to heat and don't want to circ
			relayCirc = false;
		}
	}
	relayBoard.SwitchRelay(RelayBoard::CIRC, relayCirc);
	relayBoard.SwitchRelay(RelayBoard::HEAT, relayHeat);
	display.SetHeatingState(relayHeat);
}

bool SpaController::IsWater()
{
	uint8_t pin = GPIO_ReadInputDataBit(SPA_WATER_PORT, SPA_WATER_PIN);
	return pin == 0 ? true : false; //low == water detected
}

void SpaController::IncrementTemp()
{
	if (targetTemp >= SPA_MAX_TEMP)
		return;
	targetTemp += SPA_INC_TEMP;
	WriteFloatToDisplay(targetTemp, 2);
}

void SpaController::DecrementTemp()
{
	if (targetTemp <= SPA_MIN_TEMP)
		return;
	targetTemp -= SPA_INC_TEMP;
	WriteFloatToDisplay(targetTemp, 2);
}

void SpaController::WriteFloatToDisplay(const float f, const char index)
{
	float tmp = f - (long)f;
	unsigned int fractional = tmp * 10;
	short decimal = (short)f;
	char cTemp[7];
	sprintf(cTemp, "%d.%d°C", decimal, fractional);
	display.SetTemp(index, cTemp);
}

void SpaController::ToggleJets()
{
	if (!IsWater())
		return;
	relayJets = !relayJets;
	relayBoard.SwitchRelay(RelayBoard::JET, relayJets);
	leds.SwitchLed(LEDS::JET, relayJets);
}

void SpaController::ToggleGas()
{
	if (!IsWater())
		return;
	relayGas = !relayGas;
	relayBoard.SwitchRelay(RelayBoard::GAS, relayGas);
	leds.SwitchLed(LEDS::GAS, relayGas);
}

void SpaController::ToggleHeat()
{
	if (!IsWater())
		return;
	stateHeat = !stateHeat;
	leds.SwitchLed(LEDS::HEAT, stateHeat);
}

void SpaController::ToggleCirc()
{
	if (!IsWater())
		return;
	stateCirc = !stateCirc;
	relayBoard.SwitchRelay(RelayBoard::CIRC, stateCirc);
	leds.SwitchLed(LEDS::CIRC, stateCirc);
}

void SpaController::ToggleLED1()
{
	relayLED1 = !relayLED1;
	relayBoard.SwitchRelay(RelayBoard::LED1, relayLED1);
	leds.SwitchLed(LEDS::LED1, relayLED1);
}

void SpaController::ToggleLED2()
{
	relayLED2 = !relayLED2;
	relayBoard.SwitchRelay(RelayBoard::LED2, relayLED2);
	leds.SwitchLed(LEDS::LED2, relayLED2);
}

void SpaController::SwitchOff()
{
	if (stateHeat || stateCirc || relayJets || relayGas || relayHeat || relayCirc)
	{
		stateHeat = false;
		stateCirc = false;
		relayJets = false;
		relayGas  = false;
		relayHeat = false;
		relayCirc = false;
		relayBoard.ResetToSafety();
		display.SetHeatingState(relayHeat);
	}
}

void SpaController::TogglePower()
{
	statePower = !statePower;
	if (statePower == false)
	{
		relayJets = false;
		relayGas  = false;
		relayLED1 = false;
		relayLED2 = false;
		relayBoard.SwitchRelay(RelayBoard::JET, relayJets);
		relayBoard.SwitchRelay(RelayBoard::GAS, relayGas);
		relayBoard.SwitchRelay(RelayBoard::LED1, relayLED1);
		relayBoard.SwitchRelay(RelayBoard::LED2, relayLED2);
		leds.SwitchLed(LEDS::JET, relayJets);
		leds.SwitchLed(LEDS::GAS, relayGas);
		leds.SwitchLed(LEDS::LED1, relayLED1);
		leds.SwitchLed(LEDS::LED2, relayLED2);
		leds.SwitchLed(LEDS::POWER, false);
		display.SetDisplayState(false);
	}
	else
	{
		leds.SwitchLed(LEDS::POWER, true);
		display.SetDisplayState(true);
	}
}
