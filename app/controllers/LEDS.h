#ifndef __LEDS_H__
#define __LEDS_H__

#include "mcp23017.h"

class LEDS
{
private:
	mcp23017 mcp;
	unsigned short state;
public:
	typedef enum Leds : unsigned short { JET=0x0100, GAS=0x0200, HEAT=0x0400, CIRC=0x0800, POWER=0x1000, LED1=0x2000, LED2=0x4000, NC=0x8000,
										TEMPP=0x0080, TEMPM=0x0040, SOURCE=0x0020, CHP=0x0010, CHM=0x0008, TV=0x0004, VOLP=0x0002, VOLM=0x0001} Leds;
	static LEDS& getInstance()
	{
		static LEDS instance;
		return instance;
	}
	void Init();
	void SwitchLed(LEDS::Leds led, bool enabled);
};


#endif
