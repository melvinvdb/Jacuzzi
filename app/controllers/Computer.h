#pragma once


#include "VariableCallBack.h"
#include "Keys.h"

#ifdef __cplusplus
 extern "C" {
#endif
// Circular buffers for serial TX and RX data - used by interrupt routines
const static int BUFFER_SIZE = 250;
extern char rxBuffer[BUFFER_SIZE+1]; //+1 for null character
// Circular buffer pointers
// volatile makes read-modify-write atomic
extern volatile int rxInIndex;
extern volatile int rxOutIndex;
extern char rxLine[BUFFER_SIZE+1];
extern bool responseAvailable;
#ifdef __cplusplus
 }
#endif

class Keypad;  //forward declare otherwise circuit-include issue
class Display;

class Computer : public Keys
{
public:
	typedef CallBack <Keys, void, const unsigned short, const bool> keyDataReceivedCallback;
	typedef CallBack <Keys, void, const unsigned short> AudioStateChangedCallback;

	static Computer& getInstance()
	{
		static Computer instance;
		return instance;
	}
	Computer();
	void Init();
	bool RegisterForCallback(Keys& callback);
	void CheckIncomingData();
	void SendMessage();
	void KeyDataReceived(const unsigned short keys, const bool keyshold);
	void AudioStateChanged(const unsigned short newState);

private:
	Keypad& keypad;
	Display& display;
	static const char PROTOCOL_VERSION = 0x01;
	static const char INTERRUPT_LIST_LENGTH = 5;
	keyDataReceivedCallback keyDataInterruptList[INTERRUPT_LIST_LENGTH];
	AudioStateChangedCallback audioStateInterruptList[INTERRUPT_LIST_LENGTH];
	int interruptListIndex;
	void ParseMessageStateMain();
	void ParseMessageStateSecond();
	int SearchCharacter(const unsigned int index, const char c);
	void SendMessageStateMain();
	void SendMessageStateSecond(unsigned short key);
};

