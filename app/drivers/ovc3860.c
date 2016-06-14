#include "ovc3860.h"

const char * ovc3860_AT_PLAYPAUSE 	= "AT#MA\r\n";
const char * ovc3860_AT_STOP		= "AT#MC\r\n";
const char * ovc3860_AT_NEXT		= "AT#MD\r\n";
const char * ovc3860_AT_PREVIOUS	= "AT#ME\r\n";
const char * ovc3860_AT_VOLUP		= "AT#VU\r\n";
const char * ovc3860_AT_VOLDOWN		= "AT#VD\r\n";

void ovc3860_init()
{
	USART2_Init(115200); //ovc3860 uart init
}

void ovc3860_playPause()
{
	SetOutputUart(2);
	printf(ovc3860_AT_PLAYPAUSE);
	SetOutputUart(1);
}

void ovc3860_stop()
{
	SetOutputUart(2);
	printf(ovc3860_AT_STOP);
	SetOutputUart(1);
}

void ovc3860_playPrevious()
{
	SetOutputUart(2);
	printf(ovc3860_AT_PREVIOUS);
	SetOutputUart(1);
}

void ovc3860_playNext()
{
	SetOutputUart(2);
	printf(ovc3860_AT_NEXT);
	SetOutputUart(1);
}

void ovc3860_volUp()
{
	SetOutputUart(2);
	printf(ovc3860_AT_VOLUP);
	SetOutputUart(1);
}

void ovc3860_volDown()
{
	SetOutputUart(2);
	printf(ovc3860_AT_VOLDOWN);
	SetOutputUart(1);
}
