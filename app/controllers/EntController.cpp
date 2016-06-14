#include "EntController.h"
#include <stdio.h> //printf

EntController::EntController() : keypad(Keypad::getInstance()), leds(LEDS::getInstance()), relayBoard(RelayBoard::getInstance()), display(Display::getInstance())
{}

void EntController::Init()
{
	keypad.RegisterForCallback(*this);
	stateTV = false;
	tvMoving = false;
	stateAudio = AUDOFF;
	audioRotating = false;
}

void EntController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	//printf("Hello from EntController, long press: %s\r\n", keyshold ? "true" : "false");
	if ((keys | Keypad::SOURCE) == Keypad::SOURCE)
		printf("SOURCE\r\n");
	else if ((keys | Keypad::CHP) == Keypad::CHP)
		printf("CH+\r\n");
	else if ((keys | Keypad::CHM) == Keypad::CHM)
		printf("CH-\r\n");
	else if ((keys | Keypad::TV) == Keypad::TV)
		printf("TV\r\n");
	else if ((keys | Keypad::VOLP) == Keypad::VOLP)
		printf("VOL+\r\n");
	else if ((keys | Keypad::VOLM) == Keypad::VOLM)
		printf("VOL-\r\n");
	else if ((keys | Keypad::POWER) == Keypad::POWER)
		printf("POWER\r\n");
}

void EntController::Monitor()
{
	if (tvMoving)
	{
		//TODO check millis
	}
	if (audioRotating)
	{
		//TODO check millis
	}

}

void EntController::RotateAudioOutput()
{
	switch (stateAudio)
	{
	case AUDOFF:
		stateAudio = AUDIPTV;
		break;
	case AUDIPTV:
		stateAudio = AUDBT;
		break;
	case AUDBT:
		stateAudio = AUDFM;
		break;
	case AUDFM:
		stateAudio = AUDJACK;
		break;
	case AUDJACK:
		stateAudio = AUDOFF;
		break;
	}
	audioRotating = true;
	//TODO set millis
}

void EntController::ToggleTV()
{
	stateTV = !stateTV;
	MoveTV(stateTV);
	if (stateTV)
		SetAudioOut(AUDIPTV); //force switch to IPTV
	else
		SetAudioOut(AUDOFF); //force switch audio off
}

void EntController::MoveTV(const bool up)
{
	if (tvMoving)
	{
		relayBoard.SwitchRelay(RelayBoard::TVDOWN, false);
		relayBoard.SwitchRelay(RelayBoard::TVUP, false);
		delay_nms(500);
	}
	relayBoard.SwitchRelay((up == true ? RelayBoard::TVUP : RelayBoard::TVDOWN), true);
	leds.SwitchLed(LEDS::SOURCE, up);
	tvMoving = true;
	//TODO set millis
}

void EntController::SetAudioOut(const AudioOutput output)
{
	relayBoard.ResetAudio();
	stateAudio = output;
	if (output == AUDOFF)
	{
		relayBoard.SwitchRelay(RelayBoard::AMPPOWER, false);	//set AMP off
		display.SetAudioState(false);
		leds.SwitchLed(LEDS::SOURCE, false);
		return;
	}
	if (output == AUDIPTV)
	{
		relayBoard.SwitchRelay(RelayBoard::IPTVPOWER, true);
		display.SetAudioSource("IPTV");
	}
	else if (output == AUDJACK)
	{
		display.SetAudioSource("JACK in");
	}
	else if (output == AUDFM)
	{
		//TODO FM radio
		display.SetAudioSource("FM radio");
	}
	else if (output == AUDBT)
	{
		//TODO xs3868 control
		display.SetAudioSource("Bluetooth");
	}
	relayBoard.SwitchRelay(static_cast<RelayBoard::Relays>(output), true); 	//enable audio channel
	relayBoard.SwitchRelay(RelayBoard::AMPPOWER, true); 					//enable AMP
	leds.SwitchLed(LEDS::SOURCE, true); 									//enable source led
	display.SetAudioState(true);
}
