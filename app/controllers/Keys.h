#pragma once

//abstract class
class Keys
{
public:
	typedef enum Key : unsigned short { JET=0xFEFF, GAS=0xFDFF, HEAT=0xFBFF, CIRC=0xF7FF, POWER=0xEFFF, LED1=0xDFFF, LED2=0xBFFF, NC=0x7FFF,
										TEMPP=0xFF7F, TEMPM=0xFFBF, SOURCE=0xFFDF, CHP=0xFFEF, CHM=0xFFF7, TV=0xFFFB, VOLP=0xFFFD, VOLM=0xFFFE} Key;

	virtual void KeyDataReceived(const unsigned short keys, const bool keyshold) = 0;
	virtual void AudioStateChanged(const unsigned short newState) = 0;
};
