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

#include "mcp23017.h"
#include "delay.h"

constexpr int mcp23017::IODIR_AB   [2][2];
constexpr int mcp23017::IPOL_AB    [2][2];
constexpr int mcp23017::GPINTEN_AB [2][2];
constexpr int mcp23017::DEFVAL_AB  [2][2];
constexpr int mcp23017::INTCON_AB  [2][2];
constexpr int mcp23017::IOCON_AB   [2][2];
constexpr int mcp23017::GPPU_AB    [2][2];
constexpr int mcp23017::INTF_AB    [2][2];
constexpr int mcp23017::INTCAP_AB  [2][2];
constexpr int mcp23017::GPIO_AB    [2][2];
constexpr int mcp23017::OLAT_AB    [2][2];


void mcp23017::I2C_init()
{
	GPIO_InitTypeDef PORT;

	// Init I2C
	RCC_APB2PeriphClockCmd(gpio_clock,ENABLE);
	PORT.GPIO_Pin = SDA | SCK;
	PORT.GPIO_Mode = GPIO_Mode_AF_OD;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio_port,&PORT);

	I2C_InitTypeDef I2CInit;
	RCC_APB1PeriphClockCmd(i2c_clock,ENABLE); // Enable I2C clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	I2C_DeInit(i2c_port); // I2C reset to initial state
	I2CInit.I2C_Mode = I2C_Mode_I2C; // I2C mode is I2C
	I2CInit.I2C_DutyCycle = I2C_DutyCycle_2; // I2C fast mode duty cycle (WTF is this?)
	I2CInit.I2C_OwnAddress1 = 1; // This device address (7-bit or 10-bit)
	I2CInit.I2C_Ack = I2C_Ack_Disable; // Acknowledgment
	I2CInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // choose 7-bit address for acknowledgment
	I2CInit.I2C_ClockSpeed = 400000;
	I2C_Init(i2c_port,&I2CInit); // Configure I2C
	I2C_Cmd(i2c_port,ENABLE); // Enable I2C


	// WARNING: THERE WILL HANG IF NO RESPOND FROM I2C DEVICE
	while (I2C_GetFlagStatus(i2c_port,I2C_FLAG_BUSY)); // Wait until I2C free
}


/** Read from specified MCP23017 register
*
* @param char address the internal registeraddress of the MCP23017
* @returns data from register 
*/
char mcp23017::Read(const char address)
{
    char value;
    delay_1ms(); //needed for 72mhz cpu clock
    //I2C_AcknowledgeConfig(i2c_port,ENABLE); // Enable I2C acknowledge
    I2C_GenerateSTART(i2c_port,ENABLE); // Send START condition
    while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
    I2C_Send7bitAddress(i2c_port,deviceaddress,I2C_Direction_Transmitter); // Send slave address for READ
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // Wait for EV6
	I2C_SendData(i2c_port,address); // send data
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8

	I2C_GenerateSTART(i2c_port,ENABLE); // Send START condition
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
	I2C_Send7bitAddress(i2c_port,deviceaddress,I2C_Direction_Receiver); // Send slave address for READ
    while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)); // Wait for EV6
    while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_BYTE_RECEIVED)); // Wait for EV7 (Byte received from slave)
    value = I2C_ReceiveData(i2c_port); // Receive byte
    //I2C_AcknowledgeConfig(i2c_port,DISABLE); // Disable I2C acknowledgment
    I2C_GenerateSTOP(i2c_port,ENABLE); // Send STOP condition
    return value;
}

/** Write to specified MCP23017 register
*
* @param char address the internal registeraddress of the MCP23017
*/
void mcp23017::Write(char address, char byte)
{
	delay_1ms(); //needed for 72mhz cpu clock
	//I2C_AcknowledgeConfig(i2c_port,ENABLE); // Enable I2C acknowledge
	I2C_GenerateSTART(i2c_port,ENABLE); // Send START condition
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
	I2C_Send7bitAddress(i2c_port,deviceaddress,I2C_Direction_Transmitter); // Send slave address
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // Wait for EV6

	I2C_SendData(i2c_port,address); // send data
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8
	I2C_SendData(i2c_port,byte); 	// send data
	while (!I2C_CheckEvent(i2c_port,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8

	//I2C_AcknowledgeConfig(i2c_port,DISABLE); // Disable I2C acknowledgment
	I2C_GenerateSTOP(i2c_port,ENABLE); // Send STOP condition
}

/* Init MCP23017
*
* @param
* @returns 
*
*/
void mcp23017::Init(I2C_TypeDef * _i2c_port, uint32_t _i2c_clock, GPIO_TypeDef * _gpio_port, uint32_t _gpio_clock, uint16_t _SCK, uint16_t _SDA, uint16_t _deviceaddress, bool i2cinit)
{
  _bankMode = NOT_BNK;  // This may not be true after software reset without hardware reset !!!
  i2c_port = _i2c_port;
  i2c_clock = _i2c_clock;
  gpio_port = _gpio_port;
  gpio_clock = _gpio_clock;
  SCK = _SCK;
  SDA = _SDA;
  deviceaddress = _deviceaddress;
  
  if (i2cinit)
  {
	  // Init I2C hardware
	  I2C_init();
  }

  // Hardware addressing on, no-autoincrement, 16 bit mode (operations toggle between A and B registers)
  Write(IOCON_AB[_bankMode][PORT_A], (IOCON_BYTE_MODE | IOCON_HAEN ));

}

/** Set I/O direction of specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char direction pin direction (0 = output, 1 = input)
*/
void mcp23017::Direction(const Port port, const char direction)
{
    Write(IODIR_AB[_bankMode][port], direction);
}

/** Set Pull-Up Resistors on specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void mcp23017::ConfigurePullUps(const Port port, const char offOrOn)
{
  Write(GPPU_AB[_bankMode][port], offOrOn);
}

/** Configere the Banked or Non-Banked mode
*
* @param Bank bankMode
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void mcp23017::ConfigureBanked(const Bank bankMode)
{
    if (bankMode == NOT_BNK)
    {
      // Non-Banked sequential registers (default POR)
      // Hardware addressing on, , no-autoincrement, 16 bit mode (operations do toggle between A and B registers)          
      Write(IOCON_AB[_bankMode][PORT_A], (IOCON_BYTE_MODE | IOCON_HAEN ));
      _bankMode = NOT_BNK;
    }  
    else
    {
      // Banked registers
      // Hardware addressing on, no-autoincrement, 8 bit mode           
      Write(IOCON_AB[_bankMode][PORT_A], (IOCON_BANK | IOCON_BYTE_MODE | IOCON_HAEN ));
      _bankMode = BNK;
    }
}


void mcp23017::InterruptEnable(const Port port, const char interruptsEnabledMask)
{
  
  Write(GPINTEN_AB[_bankMode][port], interruptsEnabledMask);
   
}

void mcp23017::MirrorInterrupts(const bool mirror)
{
  char iocon = Read(IOCON_AB[_bankMode][PORT_A]);

  if (mirror)
  {
    iocon = iocon | INTERRUPT_MIRROR_BIT;
  }
  else
  {
    iocon = iocon & ~INTERRUPT_MIRROR_BIT;
  }

  Write(IOCON_AB[_bankMode][PORT_A], iocon);

}

void mcp23017::InterruptPolarity(const Polarity polarity)
{
    char iocon = Read(IOCON_AB[_bankMode][PORT_A]);
    
    if (polarity == ACTIVE_LOW)
    {
        iocon = iocon & ~INTERRUPT_POLARITY_BIT;
    } else
    {
        iocon = iocon | INTERRUPT_POLARITY_BIT;
    }
    Write(IOCON_AB[_bankMode][PORT_A], iocon);
}

void mcp23017::DefaultValue(const Port port, const char valuesToCompare)
{
    
  Write(DEFVAL_AB[_bankMode][port], valuesToCompare);
    
}

void mcp23017::InterruptControl(const Port port, const char interruptControlBits)
{
    
  Write(INTCON_AB[_bankMode][port], interruptControlBits);
    
}

/** Write to specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char byte data to write
*/
void mcp23017::Write(const Port port, const char byte)
{
    Write(OLAT_AB[_bankMode][port], byte);
}
    
/** Read from specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @returns data from Port 
*/
char mcp23017::Read(const Port port)
{
    return Read(GPIO_AB[_bankMode][port]);
}
