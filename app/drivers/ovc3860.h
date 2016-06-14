#ifndef _OVC3860_H_
#define _OVC3860_H_

#include <stdio.h>
#include "usart.h"
#include <printf.h>

#ifdef __cplusplus
extern "C"
{
#endif

void ovc3860_init();
void ovc3860_playPause();
void ovc3860_stop();
void ovc3860_playPrevious();
void ovc3860_playNext();
void ovc3860_volUp();
void ovc3860_volDown();

extern const char * ovc3860_AT_PLAYPAUSE;
extern const char * ovc3860_AT_STOP;
extern const char * ovc3860_AT_NEXT;
extern const char * ovc3860_AT_PREVIOUS;
extern const char * ovc3860_AT_VOLUP;
extern const char * ovc3860_AT_VOLDOWN;

#ifdef __cplusplus
}
#endif

#endif
