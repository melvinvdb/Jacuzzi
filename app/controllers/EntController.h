#ifndef __ENTCONTROLLER_H__
#define __ENTCONTROLLER_H__

#include "Keypad.h"
#include "KeypadInterrupt.h"
#include "LEDS.h"
#include "RelayBoard.h"
#include "Display.h"
#include "delay.h"

class EntController : public KeypadInterrupt
{
private:
	Keypad& keypad;
	LEDS& leds;
	RelayBoard& relayBoard;
	Display& display;
	typedef enum AudioOutput : unsigned short { AUDOFF=0x0000, AUDIPTV=0x0010, AUDJACK=0x0020, AUDFM=0x0040, AUDBT=0x0080 } AudioOutput;

	bool stateTV;
	unsigned long tvMoveTime;
	bool tvMoving;
	AudioOutput stateAudio;
	bool audioRotating;
	unsigned long audioRotateTime;
public:
	EntController();
	void Init();
	void KeypadKeysPressed(const unsigned short keys, const bool keyshold);
	void Monitor();
	void RotateAudioOutput();
	void ToggleTV();
	void MoveTV(const bool up);
	void SetAudioOut(const AudioOutput output);
};

#endif
