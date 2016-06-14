/* MCP23017 - drive the Microchip MCP23017 16-bit Port Extender using I2C
* Copyright (c) 2010 Wim Huiskamp, Romilly Cocking (original version for SPI)
*
* Released under the MIT License: http://mbed.org/license/mit
*
* version 0.2 Initial Release
* version 0.3 Cleaned up
* version 0.4 Fixed problem with _read method
* version 0.5 Added support for 'Banked' access to registers
*/

#ifndef  MCP23017_H
#define  MCP23017_H

#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_i2c.h>
#include <stdbool.h>
#include "delay.h"


class mcp23017
{

public:
typedef enum Polarity { ACTIVE_LOW , ACTIVE_HIGH } Polarity;
typedef enum Port     { PORT_A=0,    PORT_B=1    } Port;
typedef enum Bank     { NOT_BNK=0,   BNK=1       } Bank;

private:
// All register addresses assume IOCON.BANK = 0 (POR default)
static const char IODIRA	= 0x00;
static const char IODIRB	= 0x01;
static const char IPOLA		= 0x02;
static const char IPOLB		= 0x03;
static const char GPINTENA	= 0x04;
static const char GPINTENB	= 0x05;
static const char DEFVALA	= 0x06;
static const char DEFVALB	= 0x07;
static const char INTCONA	= 0x08;
static const char INTCONB	= 0x09;
static const char IOCONA	= 0x0A;
static const char IOCONB	= 0x0B;
static const char GPPUA		= 0x0C;
static const char GPPUB		= 0x0D;
static const char INTFA		= 0x0E;
static const char INTFB		= 0x0F;
static const char INTCAPA	= 0x10;
static const char INTCAPB	= 0x11;
static const char _GPIOA	= 0x12;
static const char _GPIOB	= 0x13;
static const char OLATA		= 0x14;
static const char OLATB		= 0x15;

// The following register addresses assume IOCON.BANK = 1 
static const char IODIRA_BNK	= 0x00;
static const char IPOLA_BNK		= 0x01;
static const char GPINTENA_BNK	= 0x02;
static const char DEFVALA_BNK	= 0x03;
static const char INTCONA_BNK	= 0x04;
static const char IOCONA_BNK	= 0x05;
static const char GPPUA_BNK		= 0x06;
static const char INTFA_BNK		= 0x07;
static const char INTCAPA_BNK	= 0x08;
static const char GPIOA_BNK		= 0x09;
static const char OLATA_BNK		= 0x0A;

static const char IODIRB_BNK	= 0x10;
static const char IPOLB_BNK		= 0x11;
static const char GPINTENB_BNK	= 0x12;
static const char DEFVALB_BNK	= 0x13;
static const char INTCONB_BNK	= 0x14;
static const char IOCONB_BNK	= 0x15;
static const char GPPUB_BNK		= 0x16;
static const char INTFB_BNK		= 0x17;
static const char INTCAPB_BNK	= 0x18;
static const char GPIOB_BNK		= 0x19;
static const char OLATB_BNK		= 0x1A;

// This array allows structured access to Port_A and Port_B registers for both bankModes
static constexpr int IODIR_AB   [2][2] = {  { IODIRA, IODIRB     } , { IODIRA_BNK, IODIRB_BNK     }  } ;
static constexpr int IPOL_AB    [2][2] = {  { IPOLA, IPOLB       } , { IPOLA_BNK, IPOLB_BNK       }  } ;
static constexpr int GPINTEN_AB [2][2] = {  { GPINTENA, GPINTENB } , { GPINTENA_BNK, GPINTENB_BNK }  } ;
static constexpr int DEFVAL_AB  [2][2] = {  { DEFVALA, DEFVALB   } , { DEFVALA_BNK, DEFVALB_BNK   }  } ;
static constexpr int INTCON_AB  [2][2] = {  { INTCONA, INTCONB   } , { INTCONA_BNK, INTCONB_BNK   }  } ;
static constexpr int IOCON_AB   [2][2] = {  { IOCONA, IOCONB     } , { IOCONA_BNK, IOCONB_BNK     }  } ;
static constexpr int GPPU_AB    [2][2] = {  { GPPUA, GPPUB       } , { GPPUA_BNK, GPPUB_BNK       }  } ;
static constexpr int INTF_AB    [2][2] = {  { INTFA, INTFB       } , { INTFA_BNK, INTFB_BNK       }  } ;
static constexpr int INTCAP_AB  [2][2] = {  { INTCAPA, INTCAPB   } , { INTCAPA_BNK, INTCAPB_BNK   }  } ;
static constexpr int GPIO_AB    [2][2] = {  { _GPIOA, _GPIOB     } , { GPIOA_BNK, GPIOB_BNK       }  } ;
static constexpr int OLAT_AB    [2][2] = {  { OLATA, OLATB       } , { OLATA_BNK, OLATB_BNK       }  } ;

// Control settings
static const char IOCON_BANK				= 0x80; // Banked registers for Port A and B
static const char IOCON_BYTE_MODE			= 0x20; // Disables sequential operation, Address Ptr does not increment
													//   If Disabled and Bank = 0, operations toggle between Port A and B registers
													//   If Disabled and Bank = 1, operations do not increment registeraddress
static const char IOCON_HAEN				= 0x08; // Hardware address enable

static const char INTERRUPT_POLARITY_BIT	= 0x02;
static const char INTERRUPT_MIRROR_BIT		= 0x40;

static const char PORT_DIR_OUT				= 0x00;
static const char PORT_DIR_IN				= 0xFF;

Bank _bankMode;
//
//! I2C1
//
I2C_TypeDef * i2c_port;
//
//! RCC_APB1Periph_I2C1
//
uint32_t i2c_clock;
//
//! GPIOB
//
GPIO_TypeDef * gpio_port;
//
//! RCC_APB2Periph_GPIOB
//
uint32_t gpio_clock;
//
//! GPIO_Pin_6
//
uint16_t SCK;
//
//! GPIO_Pin_7
//
uint16_t SDA;
//
//! 0xFF
//
uint16_t deviceaddress;

void I2C_init();
char Read(const char address);
void Write(const char address, const char byte);

public:
/* Init MCP23017
*
* @param
* @returns 
*
*/
void Init(I2C_TypeDef * _i2c_port, uint32_t _i2c_clock, GPIO_TypeDef * _gpio_port, uint32_t _gpio_clock, uint16_t _SCK, uint16_t _SDA, uint16_t _deviceaddress, bool i2cinit);

/** Set I/O direction of specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char direction pin direction (0 = output, 1 = input)
*/
void Direction(const Port port, const char direction);

/** Set Pull-Up Resistors on specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void ConfigurePullUps(const Port port, const char offOrOn);

/** Configere the Banked or Non-Banked mode
*
* @param Bank bankMode
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void ConfigureBanked(const Bank bankMode);


void InterruptEnable(const Port port, const char interruptsEnabledMask);

void MirrorInterrupts(const bool mirror);

void InterruptPolarity(const Polarity polarity);

void DefaultValue(const Port port, const char valuesToCompare);

void InterruptControl(const Port port, const char interruptControlBits);

/** Write to specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char byte data to write
*/
void Write(const Port port, const char byte);
    
/** Read from specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @returns data from Port 
*/
char Read(const Port port);

};

#endif
