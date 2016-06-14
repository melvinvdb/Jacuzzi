#ifndef __RELAYBOARD_H__
#define __RELAYBOARD_H__

#include "mcp23017.h"

class RelayBoard
{
private:
	mcp23017 relays;
	unsigned short state;
	static const unsigned short safetyReset = 0x1E00;
public:
	typedef enum Relays : unsigned short { LED2=0x0100, HEAT=0x0200, CIRC=0x0400, JET=0x0800, GAS=0x1000, TVDOWN=0x2000, TVUP=0x4000, LED1=0x8000,
												NC=0x0001, TVPOWER=0x0002, AMPPOWER=0x0004, IPTVPOWER=0x0008, AUDIPTV=0x0010, AUDJACK=0x0020, AUDFM=0x0040, AUDBT=0x0080} Relays;
	static RelayBoard& getInstance()
	{
		static RelayBoard instance;
		return instance;
	}
	void Init();
	void SwitchRelay(RelayBoard::Relays relay, bool enabled);
	void ResetToSafety();
};

#endif
