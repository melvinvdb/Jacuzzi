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
	statePower = true;
	stateRadio = false;
	radioAvailable = radio.init(false); //initialize radio but don't turn it on
	radio.setBand(RADIO_BAND_FM);
	rdsParser.registerForCallback(*this);
	bluetooth.Init(RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13);
}

void EntController::RdsReceiveServiceName(char *name)
{
	printf("RDS SN: %s\r\n", name);
	if (stateAudio == AUDFM)
	{
		display.SetStatusBar(name);
	}
}

void EntController::RdsReceiveText(char *name)
{
	printf("RDS TXT: %s\r\n", name);
}

void EntController::RdsReceiveTime(uint8_t hour, uint8_t minute)
{

}

void EntController::KeypadKeysPressed(const unsigned short keys, const bool keyshold)
{
	//printf("Hello from EntController, long press: %s\r\n", keyshold ? "true" : "false");
	if ((keys | Keypad::SOURCE) == Keypad::SOURCE)
		RotateAudioOutput();
	else if ((keys | Keypad::CHP) == Keypad::CHP)
		ChangeChannel(true, keyshold);
	else if ((keys | Keypad::CHM) == Keypad::CHM)
		ChangeChannel(false, keyshold);
	else if ((keys | Keypad::TV) == Keypad::TV)
		ToggleTV();
	else if ((keys | Keypad::VOLP) == Keypad::VOLP)
		ChangeVolume(true);
	else if ((keys | Keypad::VOLM) == Keypad::VOLM)
		ChangeVolume(false);
	else if ((keys | Keypad::POWER) == Keypad::POWER)
		TogglePower();
}

void EntController::Monitor()
{
	if (tvMoving)
	{
		if ((unsigned long)(GetSysTick() - tvMoveTime) >= SysTickFormatMs(TV_MOVE_DURATION))
		{
			relayBoard.SwitchRelay(RelayBoard::TVDOWN, false);
			relayBoard.SwitchRelay(RelayBoard::TVUP, false);
			tvMoving = false;
		}
	}
	if (audioRotating)
	{
		if ((unsigned long)(GetSysTick() - audioRotateTime) >= SysTickFormatMs(AUDIO_ROTATE_DURATION))
		{
			SetAudioOut(stateAudio);
			audioRotating = false;
		}
	}
	if (stateAudio == AUDFM && radioAvailable)
	{
		uint16_t block1, block2, block3, block4;
		if (radio.checkRDS(&block1, &block2, &block3, &block4))
			rdsParser.processData(block1, block2, block3, block4);
		RADIO_FREQ freq = radio.getFrequency();
		display.SetRadioChannel(freq);
		display.SetRadioState(true);
		RADIO_INFO info;
		radio.getRadioInfo(&info);
		display.SetRadioStereo(info.stereo);
		display.SetRadioSignalstrength(info.rssi);
	}
}

void EntController::ChangeChannel(const bool up, const bool keyHold)
{
	if (stateAudio == AUDFM && radioAvailable)
	{
		if (up)
			radio.seekUp(keyHold);
		else
			radio.seekDown(keyHold);
		display.ClearStatusBar();
	}
	else if (stateAudio == AUDBT)
	{
		if (up)
			bluetooth.PlayNext();
		else
			bluetooth.PlayPrevious();
	}
}

void EntController::ChangeVolume(const bool up)
{
	if (stateAudio == AUDFM && radioAvailable)
	{
		uint8_t vol = radio.getVolume();
		if (up)
			++vol;
		else if (vol > 0)
			--vol;
		radio.setVolume(vol);
	}
	else if (stateAudio == AUDBT)
	{
		if (up)
			bluetooth.VolumeUp();
		else
			bluetooth.VolumeDown();
	}
}

void EntController::RotateAudioOutput()
{
	if (statePower ==  false)
		return;
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
	audioRotateTime = GetSysTick();
	audioRotating = true;
}

void EntController::ToggleTV()
{
	if (statePower ==  false)
		return;
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
		DwtDelayMs(500);
	}
	relayBoard.SwitchRelay((up == true ? RelayBoard::TVUP : RelayBoard::TVDOWN), true);
	leds.SwitchLed(LEDS::SOURCE, up);
	tvMoveTime = GetSysTick();
	tvMoving = true;
}

void EntController::SetAudioOut(const AudioOutput output)
{
	relayBoard.ResetAudio();
	if (output != AUDFM && stateRadio)
	{
		if (radioAvailable)
			radio.setPower(false);
		display.SetRadioState(false);
		display.ClearStatusBar(); //make sure no radio station is visible anymore
		stateRadio = false;
	}
	if (output != AUDBT)
	{
		bluetooth.SetPower(false);
	}
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
		//TODO IPTV control
		relayBoard.SwitchRelay(RelayBoard::IPTVPOWER, true);
		display.SetAudioSource("IPTV");
	}
	else if (output == AUDJACK)
	{
		display.SetAudioSource("JACK in");
	}
	else if (output == AUDFM)
	{
		if (radioAvailable)
		{
			radio.setPower(true);
			radio.setVolume(RADIO_START_VOLUME);
		}
		else
			display.SetStatusBar("No radio found");
		display.SetAudioSource("FM radio");
		stateRadio = true;
	}
	else if (output == AUDBT)
	{
		bluetooth.SetPower(true);
		bluetooth.SetVolume(BLUETOOTH_START_VOLUME);
		display.SetAudioSource("Bluetooth");
	}
	relayBoard.SwitchRelay(static_cast<RelayBoard::Relays>(output), true); 	//enable audio channel
	relayBoard.SwitchRelay(RelayBoard::AMPPOWER, true); 					//enable AMP
	leds.SwitchLed(LEDS::SOURCE, true); 									//enable source led
	display.SetAudioState(true);
}

void EntController::TogglePower()
{
	if (statePower == true)
	{
		if (stateTV == true)
		{
			ToggleTV();
		}
	}
	statePower = !statePower;
}
