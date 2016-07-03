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

class Keys; //forward declare otherwise circuit-include issue

class Computer
{
public:
	typedef CallBack <Keys, void, const unsigned short, const bool> InterruptCallback;

	static Computer& getInstance()
	{
		static Computer instance;
		return instance;
	}
	void Init();
	bool RegisterForCallback(Keys& callback);

	void CheckIncomingData();

private:
	static const char INTERRUPT_LIST_LENGTH = 5;
	InterruptCallback interruptList[INTERRUPT_LIST_LENGTH];
	int interruptListIndex;
};

