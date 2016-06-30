#ifndef __ENTCONTROLLER_H__
#define __ENTCONTROLLER_H__

#include "Keypad.h"
#include "Keys.h"
#include "LEDS.h"
#include "RelayBoard.h"
#include "Display.h"
#include "delay.h"
#include "SI4703.h"
#include "RDSParser.h"
#include "RDSParserInterrupt.h"
#include "OVC3860.h"

class EntController : public Keys, public RDSParserInterrupt
{
private:
	Keypad& keypad;
	LEDS& leds;
	RelayBoard& relayBoard;
	Display& display;
	SI4703 radio;
	RDSParser rdsParser;
	OVC3860 bluetooth;
	typedef enum AudioOutput : unsigned short { AUDOFF=0x0000, AUDIPTV=0x0010, AUDJACK=0x0020, AUDFM=0x0040, AUDBT=0x0080 } AudioOutput;

	bool stateTV;
	unsigned long tvMoveTime;
	bool tvMoving;
	const unsigned long TV_MOVE_DURATION = 10000; //tv move time in ms
	AudioOutput stateAudio;
	bool audioRotating;
	unsigned long audioRotateTime;
	const unsigned long AUDIO_ROTATE_DURATION = 1000; //audio rotate switch delay time
	bool statePower;		//power state
	bool radioAvailable;
	bool stateRadio;
	const uint8_t RADIO_START_VOLUME = 2; //range 0..15
	const uint8_t BLUETOOTH_START_VOLUME = 3; //range 0..14
public:
	EntController();
	void Init();
	void RdsReceiveServiceName(char *name);
	void RdsReceiveText(char *name);
	void RdsReceiveTime(uint8_t hour, uint8_t minute);
	void KeyDataReceived(const unsigned short keys, const bool keyshold);
	void Monitor();
	void ChangeChannel(const bool up, const bool keyHold);
	void ChangeVolume(const bool up);
	void RotateAudioOutput();
	void ToggleTV();
	void MoveTV(const bool up);
	void SetAudioOut(const AudioOutput output);
	void TogglePower();
};

#endif
