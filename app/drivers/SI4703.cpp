///
/// \file SI4703.cpp
/// \brief Implementation for the radio library to control the SI4703 radio chip.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014-2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// This library enables the use of the Radio Chip SI4703.
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 05.08.2014 created.


#include "SI4703.h"
#include "radio.h"    // Include the common radio library interface
#include <stdio.h>

// ----- Definitions for the Wire communication

#define SI4703_ADR 0x10 //0b._001.0000 = I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
#define I2C_FAIL_MAX  10 //This is the number of attempts we will try to contact the device before erroring out


// ----- Radio chip specific definitions including the registers

// Use this define to setup European FM specific settings in the chip.
#define IN_EUROPE

#define SI4703_ADDRESS			0x20
#define SI4703_PORT             I2C1
#define SI4703_CLOCK			RCC_APB1Periph_I2C1
#define SI4703_GPIO_PORT		GPIOB
#define SI4703_GPIO_CLOCK       RCC_APB2Periph_GPIOB
#define SI4703_SCK				GPIO_Pin_6 //PB6
#define SI4703_SDA				GPIO_Pin_7 //PB7
#define SI4703_RST				GPIO_Pin_9 //PB9

// Register Definitions -----
#define DEVICEID 0x00
#define CHIPID  0x01
#define POWERCFG  0x02
#define CHANNEL  0x03
#define SYSCONFIG1  0x04
#define SYSCONFIG2  0x05
#define SYSCONFIG3 0x06
#define TEST1 0x07
#define TEST2 0x08
#define BOOT 0x09
#define STATUSRSSI  0x0A
#define READCHAN  0x0B
#define RDSA  0x0C
#define RDSB  0x0D
#define RDSC  0x0E
#define RDSD  0x0F

/* Power configuration */
#define PWR_DSMUTE               15 // Softmute disable (0 = enable (default); 1 = disable)
#define PWR_DMUTE                14 // Mute disable (0 = enable (default); 1 = disable)
#define PWR_MONO                 13 // Mono select (0 = stereo (default); 1 = force mono)
#define PWR_RDSM                 11 // RDS mode (0 = standard (default); 1 = verbose)
#define PWR_SKMODE               10 // Seek mode (0 = wrap band limits and continue (default); 1 = stop at band limit)
#define PWR_SEEKUP                9 // Seek direction (0 = down (default); 1 = up)
#define PWR_SEEK                  8 // Seek (0 = disable (default); 1 = enable)
#define PWR_DISABLE               6 // Powerup disable (0 = enable (default))
#define PWR_ENABLE                0 // Powerup enable (0 = enable (default))
/* Channel */
#define CH_TUNE                  15 // Tune (0 = disable (default); 1 = enable)
/* System configuration #1 */
#define SC1_RDSIEN               15 // RDS interrupt enable (0 = disable (default); 1 = enable)
#define SC1_STCIEN               14 // Seek/Tune complete interrupt enable (0 = disable (default); 1 = enable)
#define SC1_RDS                  12 // RDS enable (0 = disable (default); 1 = enable)
#define SC1_DE                   11 // De-emphasis (0 = 75us, USA (default); 1 = 50us, Europe, Australia, Japan)
#define SC1_AGCD                 10 // AGC disable (0 = AGC enable (default); 1 = AGC disable)
/* System configuration #2 */
#define SC2_SEEKTH_MASK 0xFF00
#define SC2_SEEKTH_MIN  0x0000
#define SC2_SEEKTH_MID  0x1000
#define SC2_SEEKTH_MAX  0x7F00
#define SC2_BAND0                 6 // Band select
#define SC2_BAND1                 7
#define SC2_SPACE0                4 // Channel spacing
#define SC2_SPACE1                5
/* System configuration #3 */
#define SC3_VOLEXT                8 // Extended volume range (0 = disabled (Default); 1 = enabled (decrease the volume by 28dB))
#define SC3_SKSNR_MASK 0x00F0
#define SC3_SKSNR_OFF  0x0000
#define SC3_SKSNR_MIN  0x0010
#define SC3_SKSNR_MID  0x0030
#define SC3_SKSNR_MAX  0x0070

#define SC3_SKCNT_MASK 0x000F
#define SC3_SKCNT_OFF  0x0000
#define SC3_SKCNT_MIN  0x000F
#define SC3_SKCNT_MID  0x0003
#define SC3_SKCNT_MAX  0x0001
/* Test 1 */
#define T1_XOSCEN                15 // Crystal oscillator enable (0 = disable (Default); 1 = enable)
#define T1_WTF                    8 // Datasheet aren't say anything about this, but it's necessary on powerup.
/* Status RSSI */
#define RSSI_RDSR                15 // RDSR is ready (0 = No RDS group ready; 1 = New RDS group ready)
#define RSSI_STC                 14 // Seek/Tune complete (0 = not complete; 1 = complete)
#define RSSI_SFBL                13 // Seek fail/Band limit (0 = Seek successful; 1 = Seek failure/Band limit reached)
#define RSSI_AFCRL               12 // AFC fail (0 = AFC not railed; 1 = AFC railed)
#define RSSI_RDSS                11 // RDS sync (0 = not synchronized; 1 = decoder synchronized)
#define RSSI_ST                   8 // Stereo indicator (0 = mono; 1 = stereo)
#define RSSI_RSSI   0x00FF
/* Some additional constants */
#define SEEK_UP                   0 // Seek up (default)
#define SEEK_DOWN                 1 // Seek down
#define WRAP_ON                   0 // Wrap around band limit enabled (default)
#define WRAP_OFF                  1 // Wrap around band limit disabled


// initialize the extra variables in SI4703
SI4703::SI4703() {
}

// initialize all internals.
bool SI4703::init(const bool switchOn) {
	bool result = false; // no chip found yet.
	//DEBUG_FUNC0("init");


	GPIO_InitTypeDef PORT;

	// Enable peripheral clocks for PortB
	RCC_APB2PeriphClockCmd(SI4703_GPIO_CLOCK, ENABLE);
	// Set RST(PB8) and SDIO(PB7/SDA) as output
	PORT.GPIO_Pin = SI4703_SDA | SI4703_RST;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SI4703_GPIO_PORT,&PORT);
	GPIO_WriteBit(SI4703_GPIO_PORT,SI4703_RST,Bit_RESET); // Pull RST low (Si4703 reset operation)
	GPIO_WriteBit(SI4703_GPIO_PORT,SI4703_SDA,Bit_RESET); // Select 2-wire interface (I2C)
	DwtDelay1ms(); // Just wait
	GPIO_WriteBit(SI4703_GPIO_PORT,SI4703_RST,Bit_SET); // Pull RST high (Si4703 normal operation)
	DwtDelay1ms(); // Give Si4703 some time to rise after reset

	// Init I2C
	RCC_APB2PeriphClockCmd(SI4703_GPIO_CLOCK,ENABLE);
	PORT.GPIO_Pin = SI4703_SDA | SI4703_SCK;
	PORT.GPIO_Mode = GPIO_Mode_AF_OD;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SI4703_GPIO_PORT,&PORT);

	I2C_InitTypeDef I2CInit;
	RCC_APB1PeriphClockCmd(SI4703_CLOCK,ENABLE); // Enable I2C clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	I2C_DeInit(SI4703_PORT); // I2C reset to initial state
	I2CInit.I2C_Mode = I2C_Mode_I2C; // I2C mode is I2C
	I2CInit.I2C_DutyCycle = I2C_DutyCycle_2; // I2C fast mode duty cycle (WTF is this?)
	I2CInit.I2C_OwnAddress1 = 1; // This device address (7-bit or 10-bit)
	I2CInit.I2C_Ack = I2C_Ack_Disable; // Acknowledgment
	I2CInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // choose 7-bit address for acknowledgment
	I2CInit.I2C_ClockSpeed = 400000;
	I2C_Cmd(SI4703_PORT,ENABLE); // Enable I2C
	I2C_Init(SI4703_PORT,&I2CInit); // Configure I2C

	// WARNING: THERE WILL HANG IF NO RESPOND FROM I2C DEVICE
	while (I2C_GetFlagStatus(SI4703_PORT,I2C_FLAG_BUSY)); // Wait until I2C free

	result = _readRegisters(); //Read the current register set
	if (result == false)
		return false;
	//registers[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
	registers[TEST1] = (1 << T1_XOSCEN)|(1 << T1_WTF); //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
	_saveRegisters(); //Update

	DwtDelayMs(500); //Wait for clock to settle - from AN230 page 9

	_readRegisters(); //Read the current register set
	registers[POWERCFG] = (1<<PWR_DMUTE); //Disable Mute
	if (switchOn)
	{
		registers[POWERCFG] |= (1<<PWR_ENABLE)|(0<<PWR_DISABLE);   //Enable the IC
		registers[SYSCONFIG1] |= (1<<SC1_RDS); //Enable RDS
	}

#ifdef IN_EUROPE
	registers[SYSCONFIG1] |= (1<<SC1_DE); // 50us de-emphasis (must be on for Europe, Australia and Japan)
	registers[SYSCONFIG2] &= ~(1<<SC2_BAND1)|(1<<SC2_BAND0); // 87.5-108MHz (USA,Europe)
	registers[SYSCONFIG2] |= (1<<SC2_SPACE0); //100kHz channel spacing for Europe
#else
	registers[SYSCONFIG2] &= ~(1<<SC2_SPACE1 | 1<<SC2_SPACE0) ; //Force 200kHz channel spacing for USA
#endif

	_volume = 1;
	registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
	registers[SYSCONFIG2] |= (_volume & 0x0F); //Set volume
	registers[SYSCONFIG2] |= SC2_SEEKTH_MID; //Set RSSI Seek Threshold.

	// set seek parameters
	registers[SYSCONFIG3] &= ~(SC3_SKSNR_MASK); // Clear seek mask bits
	registers[SYSCONFIG3] |= SC3_SKSNR_MID;     //Set Seek SNR Threshold.

	registers[SYSCONFIG3] &= ~(SC3_SKCNT_MASK); // Clear seek mask bits
	registers[SYSCONFIG3] |= SC3_SKCNT_MID;     //Set Seek FM Impulse Detection Threshold.

	_saveRegisters(); //Update

	DwtDelayMs(110); //Max powerup time, from datasheet page 13
	return(result);
} // init()

void SI4703::setPower(const bool switchOn)
{
	_readRegisters(); //Read the current register set
	if (switchOn)
	{
		registers[POWERCFG] &= ~(1<<PWR_DISABLE); //Enable the IC
		registers[POWERCFG] |= (1<<PWR_ENABLE);   //Enable the IC
		registers[SYSCONFIG1] |= (1<<SC1_RDS); //Enable RDS
	}
	else
	{
		registers[SYSCONFIG1] &= ~(1<<SC1_RDS); //Disable RDS
		registers[POWERCFG] |= (1<<PWR_ENABLE)|(1<<PWR_DISABLE); //Disable the IC
	}
	_saveRegisters(); //Update
}

// ----- Volume control -----

void SI4703::setVolume(uint8_t newVolume)
{
	//DEBUG_FUNC1("setVolume", newVolume);
	if (newVolume > 15) newVolume = 15;
	_readRegisters(); //Read the current register set
	registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
	registers[SYSCONFIG2] |= newVolume; //Set new volume
	_saveRegisters(); //Update
	RADIO::setVolume(newVolume);
} // setVolume()


// Mono / Stereo
void SI4703::setMono(const bool switchOn)
{
	//DEBUG_FUNC1("setMono", switchOn);
	RADIO::setMono(switchOn);
	_readRegisters(); //Read the current register set
	if (switchOn) {
		registers[POWERCFG] |= (1 << PWR_MONO); // set force mono bit
	}
	else {
		registers[POWERCFG] &= ~(1 << PWR_MONO); // clear force mono bit
	} // if
	_saveRegisters();
} // setMono


/// Switch mute mode.
void SI4703::setMute(const bool switchOn)
{
	//DEBUG_FUNC1("setMute", switchOn);
	RADIO::setMute(switchOn);

	if (switchOn) {
		registers[POWERCFG] &= ~(1<<PWR_DMUTE); // clear mute bit
	}
	else {
		registers[POWERCFG] |= (1<<PWR_DMUTE); // set mute bit
	} // if
	_saveRegisters();

} // setMute()


/// Switch soft mute mode.
void SI4703::setSoftMute(const bool switchOn)
{
	//DEBUG_FUNC1("setSoftMute", switchOn);
	RADIO::setSoftMute(switchOn);

	if (switchOn) {
		registers[POWERCFG] &= ~(1<<PWR_DSMUTE); // clear mute bit
	}
	else {
		registers[POWERCFG] |= (1<<PWR_DSMUTE); // set mute bit
	} // if
	_saveRegisters();

} // setSoftMute()



// ----- Band and frequency control methods -----

// tune to new band.
void SI4703::setBand(const RADIO_BAND newBand) {

	if (newBand == RADIO_BAND_FM) {
		RADIO::setBand(newBand);
		_freqLow = 8750;

#ifdef IN_EUROPE
		//Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
		_freqSteps = 10;

#else
		//Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
		_freqSteps = 20;
#endif

  } // if
} // setBand()


/**
* @brief Retrieve the real frequency from the chip after automatic tuning.
* @return RADIO_FREQ the current frequency.
*/
RADIO_FREQ SI4703::getFrequency() {
	_readRegisters();
	int channel = registers[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
	_freq = (channel * _freqSteps) + _freqLow;
	return (_freq);
}  // getFrequency


/**
* @brief Change the frequency in the chip.
* @param newF
* @return void
*/
void SI4703::setFrequency(RADIO_FREQ newF) {
	//DEBUG_FUNC1("setFrequency", newF);
	if (newF < _freqLow)  newF = _freqLow;
	if (newF > _freqHigh) newF = _freqHigh;

	_readRegisters();
	int channel = (newF - _freqLow) / _freqSteps;

	//These steps come from AN230 page 20 rev 0.5
	registers[CHANNEL] &= 0xFE00; //Clear out the channel bits
	registers[CHANNEL] |= channel; //Mask in the new channel
	registers[CHANNEL] |= (1<<CH_TUNE); //Set the TUNE bit to start
	_saveRegisters();
	DwtDelayMs(50); // Gime some time for the Si4703 to tune up
	_waitEnd();
} // setFrequency()


// start seek mode upwards
void SI4703::seekUp(const bool search) {
	//DEBUG_FUNC1("seekUp", toNextSender);
	if (search)
	{
		_seek(true);
	}
	else
	{
		RADIO_FREQ freq = getFrequency();
		freq += _freqSteps;
		setFrequency(freq);
	}
} // seekUp()


// start seek mode downwards
void SI4703::seekDown(const bool search) {
	//DEBUG_FUNC1("seekDown", toNextSender);
	if (search)
	{
		_seek(false);
	}
	else
	{
		RADIO_FREQ freq = getFrequency();
		freq -= _freqSteps;
		setFrequency(freq);
	}
} // seekDown()



// Load all status registers from to the chip
bool SI4703::_readRegisters()
{
	//Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
	//We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
	uint8_t i;
	uint8_t buffer[32]; // 16 of 16-bit registers
	DwtDelay1ms(); //needed for 72mhz cpu clock
	unsigned long time = GetSysTick();

	I2C_AcknowledgeConfig(SI4703_PORT,ENABLE); // Enable I2C acknowledge
	I2C_GenerateSTART(SI4703_PORT,ENABLE); // Send START condition
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_MODE_SELECT)) // Wait for EV5
	{
		if ((unsigned long)(GetSysTick()-time) > SysTickFormatMs(100))
		{
			return false; //timeout device did not respond
		}
	}
	I2C_Send7bitAddress(SI4703_PORT,SI4703_ADDRESS,I2C_Direction_Receiver); // Send slave address for READ
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) // Wait for EV6
	{
		if ((unsigned long)(GetSysTick()-time) > SysTickFormatMs(100))
		{
			return false; //timeout device did not respond
		}
	}
	// Si4703 read start from r0Ah register
	for (i = 0x14; ; i++) {
		if (i == 0x20) i = 0x00;
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_RECEIVED)) // Wait for EV7 (Byte received from slave)
		{
			if ((unsigned long)(GetSysTick()-time) > SysTickFormatMs(100))
			{
				return false; //timeout device did not respond
			}
		}
		buffer[i] = I2C_ReceiveData(SI4703_PORT); // Receive byte
		if (i == 0x13) break;
	}
	I2C_AcknowledgeConfig(SI4703_PORT,DISABLE); // Disable I2C acknowledgment
	I2C_GenerateSTOP(SI4703_PORT,ENABLE); // Send STOP condition
	//while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_RECEIVED)); // Wait for EV7 (Byte received from slave)
	//buffer[i++] = I2C_ReceiveData(SI4703_PORT); // Receive last byte

	for (i = 0; i < 16; i++) {
		registers[i] = (buffer[i<<1] << 8) | buffer[(i<<1)+1];
	}
	return true;
}


// Save writable registers back to the chip
// The registers 02 through 06, containing the configuration
// using the sequential write access mode.
void SI4703::_saveRegisters()
{
	//Write the current 9 control registers (0x02 to 0x07) to the Si4703
	//It's a little weird, you don't write an I2C addres
	//The Si4703 assumes you are writing to 0x02 first, then increments
	uint8_t i;
	DwtDelay1ms(); //needed for 72mhz cpu clock
	I2C_AcknowledgeConfig(SI4703_PORT,ENABLE); // Enable I2C acknowledge
	I2C_GenerateSTART(SI4703_PORT,ENABLE); // Send START condition
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
	I2C_Send7bitAddress(SI4703_PORT,SI4703_ADDRESS,I2C_Direction_Transmitter); // Send slave address
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // Wait for EV6
	for (i = 2; i < 8; i++) {
		I2C_SendData(SI4703_PORT,registers[i] >> 8); // MSB
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8
		I2C_SendData(SI4703_PORT,registers[i] & 0x00ff); // LSB
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8
	}
	I2C_AcknowledgeConfig(SI4703_PORT,DISABLE); // Disable I2C acknowledgment
	I2C_GenerateSTOP(SI4703_PORT,ENABLE); // Send STOP condition
} // _saveRegisters


/// Retrieve all the information related to the current radio receiving situation.
void SI4703::getRadioInfo(RADIO_INFO *info) {
	RADIO::getRadioInfo(info); // all settings to last current settings

	_readRegisters();
	info->active = true; // ???
	if (registers[STATUSRSSI] & RSSI_ST) info->stereo = true;
	info->rssi = registers[STATUSRSSI] & RSSI_RSSI;
	if (registers[STATUSRSSI] & (RSSI_RDSS)) info->rds = true;
	if (registers[STATUSRSSI] & RSSI_STC) info->tuned = true;
	if (registers[POWERCFG] & (1 << PWR_MONO)) info->mono = true;
} // getRadioInfo()


/// Return current audio settings.
void SI4703::getAudioInfo(AUDIO_INFO *info) {
	RADIO::getAudioInfo(info);

	_readRegisters();
	if (! (registers[POWERCFG] & (1<<PWR_DMUTE))) info->mute = true;
	if (! (registers[POWERCFG] & (1<<PWR_DSMUTE))) info->softmute = true;
	info->bassBoost = false; // no bassBoost
	info->volume = registers[SYSCONFIG2] & 0x000F;
} // getAudioInfo()


bool SI4703::checkRDS(uint16_t* block1, uint16_t* block2, uint16_t* block3, uint16_t* block4)
{
	// DEBUG_FUNC0("checkRDS");
	_readRegisters();
	// check for a RDS data set ready
	if(registers[STATUSRSSI] & RSSI_RDSR) {
		*block1 = registers[RDSA];
		*block2 = registers[RDSB];
		*block3 = registers[RDSC];
		*block4 = registers[RDSD];
		return true;
	}
	else
		return false;
} // checkRDS

// ----- Debug functions -----

/*// Send the current values of all registers to the Serial port.
void SI4703::debugStatus()
{
	RADIO::debugStatus();

	_readRegisters();
	// Print all registers for debugging
	for (int x = 0 ; x < 16 ; x++) {
		Serial.print("Reg: 0x0"); Serial.print(x, HEX);
		Serial.print(" = 0x"); _printHex4(registers[x]);
		Serial.println();
	} // for
} // debugStatus*/


/// Seeks out the next available station
void SI4703::_seek(const bool seekUp) {
	uint16_t reg;

	_readRegisters();
  // not wrapping around.
	reg = registers[POWERCFG] & ~((1<<PWR_SKMODE) | (1<<PWR_SEEKUP));

	if (seekUp) reg |= (1<<PWR_SEEKUP); // Set the Seek-up bit

	reg |= (1<<PWR_SEEK); // Start seek now

	// save the registers and start seeking...
	registers[POWERCFG] = reg;
	_saveRegisters();

	_waitEnd();
} // _seek


/// wait until the current seek and tune operation is over.
void SI4703::_waitEnd() {
	//DEBUG_FUNC0("_waitEnd");

	// wait until STC gets high
	while(1) {
	  _readRegisters();
	  if ((registers[STATUSRSSI] & (1<<RSSI_STC)) != 0) break; // Tuning complete
	}

	// DEBUG_VAL("Freq:", getFrequency());
  
	_readRegisters();
	// get the SFBL bit.
	/*if (registers[STATUSRSSI] & RSSI_SFBL)
		DEBUG_STR("Seek limit hit");*/
  
	// end the seek mode
	registers[POWERCFG] &= ~(1<<PWR_SEEK); //Clear the seek if a seek has been requested
	registers[CHANNEL]  &= ~(1<<CH_TUNE); //Clear the tune if a tune has been requested
	_saveRegisters();

	// wait until STC gets down again
	while(1) {
	  _readRegisters();
	  if ((registers[STATUSRSSI] & (1<<RSSI_STC)) == 0) break; // Tuning complete
	}
} // _waitEnd()


// ----- internal functions -----

// The End.


