/* 
Library for Sparkfun Si4703 breakout board.
Simon Monk. 2011-09-09

This is a library wrapper and a few extras to the excellent code produced
by Nathan Seidle from Sparkfun (Beerware).

Nathan's comments......

Look for serial output at 57600bps.

The Si4703 ACKs the first byte, and NACKs the 2nd byte of a read.

1/18 - after much hacking, I suggest NEVER write to a register without first reading the contents of a chip.
ie, don't updateRegisters without first readRegisters.

If anyone manages to get this datasheet downloaded
http://wenku.baidu.com/view/d6f0e6ee5ef7ba0d4a733b61.html
Please let us know. It seem to be the latest version of the programming guide. It had a change on page 12 (write 0x8100 to 0x07)
that allowed me to get the chip working..

Also, if you happen to find "AN243: Using RDS/RBDS with the Si4701/03", please share. I love it when companies refer to
documents that don't exist.

1/20 - Picking up FM stations from a plane flying over Portugal! Sweet! 93.9MHz sounds a little soft for my tastes,s but
it's in Porteguese.

ToDo:
Display current status (from 0x0A) - done 1/20/11
Add RDS decoding - works, sort of
Volume Up/Down - done 1/20/11
Mute toggle - done 1/20/11
Tune Up/Down - done 1/20/11
Read current channel (0xB0) - done 1/20/11
Setup for Europe - done 1/20/11
Seek up/down - done 1/25/11

The Si4703 breakout does work with line out into a stereo or other amplifier. Be sure to test with different length 3.5mm
cables. Too short of a cable may degrade reception.
*/

#ifndef _SI4703_H_
#define _SI4703_H_

#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_i2c.h>
#include <stdbool.h>
#include "delay.h"

#ifdef __cplusplus
extern "C"
{
#endif

//#include <stdio.h> // printf test

void si4703_init(void);
void si4703_readRegisters(void);
void si4703_updateRegisters(void);
void si4703_setChannel(int channel);  	// 3 digit channel number
int si4703_getChannel();
bool si4703_isStereo();
int si4703_getSignalStrength();
int si4703_seek(uint8_t SeekDirection, uint8_t Wrap);
void si4703_setVolume(int volume); 	// 0 to 15
void si4703_initRDS(void);
void si4703_checkRDS(void);
void si4703_readRDS(char* message, long timeout);
								// message should be at least 9 chars
								// result will be null terminated
								// timeout in milliseconds
char si4703_PSName1[10];
char si4703_PSName2[10];
char si4703_programServiceName[10];
bool si4703_PSNvalid;

#define SI4703_ADDRESS			0x20
uint16_t si4703_registers[16]; // Si4703 registers
#define SI4703_PORT             I2C1
#define SI4703_CLOCK			RCC_APB1Periph_I2C1
#define SI4703_GPIO_PORT		GPIOB
#define SI4703_GPIO_CLOCK       RCC_APB2Periph_GPIOB
#define SI4703_SCK				GPIO_Pin_6 //PB6
#define SI4703_SDA				GPIO_Pin_7 //PB7
#define SI4703_RST				GPIO_Pin_9 //PB9

/* Si4703 registers */
#define Si4703_DEVICEID                 0x00 // Device ID
#define Si4703_CHIPID                   0x01 // Chip ID
#define Si4703_POWERCFG                 0x02 // Power configuration
#define Si4703_CHANNEL                  0x03 // Channel
#define Si4703_SYSCONFIG1               0x04 // System configuration #1
#define Si4703_SYSCONFIG2               0x05 // System configuration #2
#define Si4703_SYSCONFIG3               0x06 // System configuration #3
#define Si4703_TEST1                    0x07 // Test 1
#define Si4703_TEST2                    0x08 // Test 2
#define Si4703_BOOT                     0x09 // Boot configuration
#define Si4703_RSSI                     0x0a // Status RSSI
#define Si4703_READCHANNEL              0x0b // Read channel
#define Si4703_RDSA                     0x0c // RDSA
#define Si4703_RDSB                     0x0d // RDSB
#define Si4703_RDSC                     0x0e // RDSC
#define Si4703_RDSD                     0x0f // RDSD
/* Power configuration */
#define Si4703_PWR_DSMUTE               15 // Softmute disable (0 = enable (default); 1 = disable)
#define Si4703_PWR_DMUTE                14 // Mute disable (0 = enable (default); 1 = disable)
#define Si4703_PWR_MONO                 13 // Mono select (0 = stereo (default); 1 = force mono)
#define Si4703_PWR_RDSM                 11 // RDS mode (0 = standard (default); 1 = verbose)
#define Si4703_PWR_SKMODE               10 // Seek mode (0 = wrap band limits and continue (default); 1 = stop at band limit)
#define Si4703_PWR_SEEKUP                9 // Seek direction (0 = down (default); 1 = up)
#define Si4703_PWR_SEEK                  8 // Seek (0 = disable (default); 1 = enable)
#define Si4703_PWR_DISABLE               6 // Powerup disable (0 = enable (default))
#define Si4703_PWR_ENABLE                0 // Powerup enable (0 = enable (default))
/* Channel */
#define Si4703_CH_TUNE                  15 // Tune (0 = disable (default); 1 = enable)
/* System configuration #1 */
#define Si4703_SC1_RDSIEN               15 // RDS interrupt enable (0 = disable (default); 1 = enable)
#define Si4703_SC1_STCIEN               14 // Seek/Tune complete interrupt enable (0 = disable (default); 1 = enable)
#define Si4703_SC1_RDS                  12 // RDS enable (0 = disable (default); 1 = enable)
#define Si4703_SC1_DE                   11 // De-emphasis (0 = 75us, USA (default); 1 = 50us, Europe, Australia, Japan)
#define Si4703_SC1_AGCD                 10 // AGC disable (0 = AGC enable (default); 1 = AGC disable)
/* System configuration #2 */
#define Si4703_SC2_BAND0                 6 // Band select
#define Si4703_SC2_BAND1                 7
#define Si4703_SC2_SPACE0                4 // Channel spacing
#define Si4703_SC2_SPACE1                5
/* System configuration #3 */
#define Si4703_SC3_VOLEXT                8 // Extended volume range (0 = disabled (Default); 1 = enabled (decrease the volume by 28dB))
/* Test 1 */
#define Si4703_T1_XOSCEN                15 // Crystal oscillator enable (0 = disable (Default); 1 = enable)
#define Si4703_T1_WTF                    8 // Datasheet aren't say anything about this, but it's necessary on powerup.
/* Status RSSI */
#define Si4703_RSSI_RDSR                15 // RDSR is ready (0 = No RDS group ready; 1 = New RDS group ready)
#define Si4703_RSSI_STC                 14 // Seek/Tune complete (0 = not complete; 1 = complete)
#define Si4703_RSSI_SFBL                13 // Seek fail/Band limit (0 = Seek successful; 1 = Seek failure/Band limit reached)
#define Si4703_RSSI_AFCRL               12 // AFC fail (0 = AFC not railed; 1 = AFC railed)
#define Si4703_RSSI_RDSS                11 // RDS sync (0 = not synchronized; 1 = decoder synchronized)
#define Si4703_RSSI_ST                   8 // Stereo indicator (0 = mono; 1 = stereo)
/* Some additional constants */
#define Si4703_SEEK_UP                   0 // Seek up (default)
#define Si4703_SEEK_DOWN                 1 // Seek down
#define Si4703_WRAP_ON                   0 // Wrap around band limit enabled (default)
#define Si4703_WRAP_OFF                  1 // Wrap around band limit disabled

#ifdef __cplusplus
}
#endif


#endif
