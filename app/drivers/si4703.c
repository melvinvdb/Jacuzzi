#include "si4703.h"

//Read the entire register control set from 0x00 to 0x0F
void si4703_readRegisters(void){
	//Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
	//We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
	uint8_t i;
	uint8_t buffer[32]; // 16 of 16-bit registers
	delay_1ms(); //needed for 72mhz cpu clock

	I2C_AcknowledgeConfig(SI4703_PORT,ENABLE); // Enable I2C acknowledge
	I2C_GenerateSTART(SI4703_PORT,ENABLE); // Send START condition
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
	I2C_Send7bitAddress(SI4703_PORT,SI4703_ADDRESS,I2C_Direction_Receiver); // Send slave address for READ
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)); // Wait for EV6
	// Si4703 read start from r0Ah register
	for (i = 0x14; ; i++) {
		if (i == 0x20) i = 0x00;
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_RECEIVED)); // Wait for EV7 (Byte received from slave)
		buffer[i] = I2C_ReceiveData(SI4703_PORT); // Receive byte
		if (i == 0x13) break;
	}
	I2C_AcknowledgeConfig(SI4703_PORT,DISABLE); // Disable I2C acknowledgment
	I2C_GenerateSTOP(SI4703_PORT,ENABLE); // Send STOP condition
	//while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_RECEIVED)); // Wait for EV7 (Byte received from slave)
	//buffer[i++] = I2C_ReceiveData(SI4703_PORT); // Receive last byte

	for (i = 0; i < 16; i++) {
		si4703_registers[i] = (buffer[i<<1] << 8) | buffer[(i<<1)+1];
	}
}

//Write the current 9 control registers (0x02 to 0x07) to the Si4703
//It's a little weird, you don't write an I2C addres
//The Si4703 assumes you are writing to 0x02 first, then increments
void si4703_updateRegisters() {
	//A write command automatically begins with register 0x02 so no need to send a write-to address
	//First we send the 0x02 to 0x07 control registers
	//In general, we should not write to registers 0x08 and 0x09
	uint8_t i;
	delay_1ms(); //needed for 72mhz cpu clock
	I2C_AcknowledgeConfig(SI4703_PORT,ENABLE); // Enable I2C acknowledge
	I2C_GenerateSTART(SI4703_PORT,ENABLE); // Send START condition
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_MODE_SELECT)); // Wait for EV5
	I2C_Send7bitAddress(SI4703_PORT,SI4703_ADDRESS,I2C_Direction_Transmitter); // Send slave address
	while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // Wait for EV6
	for (i = 2; i < 8; i++) {
		I2C_SendData(SI4703_PORT,si4703_registers[i] >> 8); // MSB
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8
		I2C_SendData(SI4703_PORT,si4703_registers[i] & 0x00ff); // LSB
		while (!I2C_CheckEvent(SI4703_PORT,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Wait for EV8
	}
	I2C_AcknowledgeConfig(SI4703_PORT,DISABLE); // Disable I2C acknowledgment
	I2C_GenerateSTOP(SI4703_PORT,ENABLE); // Send STOP condition
}

//To get the Si4703 into 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
//The Si4703 will be in an unknown state. RST must be controlled
void si4703_init(void)
{
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
	delay_1ms(); // Just wait
	GPIO_WriteBit(SI4703_GPIO_PORT,SI4703_RST,Bit_SET); // Pull RST high (Si4703 normal operation)
	delay_1ms(); // Give Si4703 some time to rise after reset

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

	si4703_readRegisters(); //Read the current register set
	//si4703_registers[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
	si4703_registers[Si4703_TEST1] = (1 << Si4703_T1_XOSCEN)|(1 << Si4703_T1_WTF); // power up crystall
	si4703_updateRegisters(); //Update

	delay_nms(500); //Wait for clock to settle - from AN230 page 9

	si4703_readRegisters(); //Read the current register set
	si4703_registers[Si4703_POWERCFG] = (1<<Si4703_PWR_DMUTE)|(1<<Si4703_PWR_ENABLE);
	si4703_registers[Si4703_SYSCONFIG1] |= (1<<Si4703_SC1_RDS); // Enable RDS
	si4703_registers[Si4703_SYSCONFIG1] |= (1<<Si4703_SC1_DE); // 50us de-emphasis (must be on for Europe, Australia and Japan)
	si4703_registers[Si4703_SYSCONFIG2] &= ~(1<<Si4703_SC2_BAND1)|(1<<Si4703_SC2_BAND0); // 87.5-108MHz (USA,Europe)
    //si4703_registers[Si4703_SYSCONFIG2] |= (1<<Si4703_SC2_BAND1)|(1<<Si4703_SC2_BAND0); // 76-108MHz (Japan wide band)
	si4703_registers[Si4703_SYSCONFIG2] |= (1<<Si4703_SC2_SPACE0); // 100kHz spacing (Europe)
	si4703_registers[Si4703_SYSCONFIG2] &= 0xfff0;
	si4703_registers[Si4703_SYSCONFIG2] |= 0x0001; // minimum volume
	//si4703_registers[Si4703_SYSCONFIG2] |= 0x0002; // volume
	//si4703_registers[Si4703_SYSCONFIG2] |= 0x0007; // medium volume
	//si4703_registers[Si4703_SYSCONFIG2] |= 0x000f; // maximum volume
	//si4703_registers[Si4703_SYSCONFIG3] |= (1<<Si4703_SC3_VOLEXT); // Decrease the volume by 28dB
	si4703_updateRegisters(); //Update

	si4703_initRDS();
	delay_nms(110); //Max powerup time, from datasheet page 13
}

void si4703_setChannel(int channel)
{
  //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
  //97.3 = 0.2 * Chan + 87.5
  //9.8 / 0.2 = 49
  si4703_readRegisters();
  channel *= 10;
  channel -= ((si4703_registers[Si4703_SYSCONFIG2] & ((1<<Si4703_SC2_BAND1) | (1<<Si4703_SC2_BAND0))) == 0) ? 8750 : 7600;
  channel /= 10;

  //These steps come from AN230 page 20 rev 0.5
  si4703_registers[Si4703_CHANNEL] &= 0xfe00; // Clear channel frequency from register
  si4703_registers[Si4703_CHANNEL] |= channel; // Set new channel frequency
  si4703_registers[Si4703_CHANNEL] |= (1<<Si4703_CH_TUNE); // Set TUNE flag
  si4703_updateRegisters();

  delay_nms(50); // Gime some time for the Si4703 to tune up

  //Poll to see if STC is set
  while(1) {
	  si4703_readRegisters();
	  if ((si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_STC)) != 0) break; // Tuning complete
  }

  si4703_readRegisters();
  si4703_registers[Si4703_CHANNEL] &= ~(1<<Si4703_CH_TUNE); // Clear TUNE flag
  si4703_updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
	  si4703_readRegisters();
	  if ((si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_STC)) == 0) break; // Tuning complete
  }
}

//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
int si4703_getChannel() {
	si4703_readRegisters();
	int channel = si4703_registers[Si4703_READCHANNEL] & 0x03ff;
	channel += ((si4703_registers[Si4703_SYSCONFIG2] & ((1<<Si4703_SC2_BAND1) | (1<<Si4703_SC2_BAND0))) == 0) ? 875 : 760;
	return(channel);
}

bool si4703_isStereo() {
	si4703_readRegisters();
	if ((si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_ST)) == 0)
		return false;
	else
		return true;
}

//get signal strength in dBuV.
//dBuV reference:
//54dBuV/m Threshold – This is the level that Ofcom define for all stereo FM radio services – commercial or community.
int si4703_getSignalStrength() {
	si4703_readRegisters();
	return si4703_registers[Si4703_RSSI] & 0x007f;
}

//Seeks out the next available station
//Returns the freq if it made it
//Returns zero if failed
int si4703_seek(uint8_t SeekDirection, uint8_t Wrap) {
	uint32_t _sfbl;
	si4703_readRegisters();
	//Set seek mode wrap bit
	if (Wrap) {
		si4703_registers[Si4703_POWERCFG] |=  (1<<Si4703_PWR_SKMODE); // Band wrap on
	} else {
		si4703_registers[Si4703_POWERCFG] &= ~(1<<Si4703_PWR_SKMODE); // Band wrap off
	}
	//if you disallow wrap, you may want to tune to 87.5 first
	if (SeekDirection) {
		si4703_registers[Si4703_POWERCFG] &= ~(1<<Si4703_PWR_SEEKUP); // Seek up
	} else {
		si4703_registers[Si4703_POWERCFG] |=  (1<<Si4703_PWR_SEEKUP); // Seek down
	}
	si4703_registers[Si4703_POWERCFG] |= (1<<Si4703_PWR_SEEK); // Set seek start bit
	si4703_updateRegisters(); //Seeking will now start

	// Wait for the Si4703 to set STC flag
	//UART_SendStr("Seek...\n");
	while(1) {
		si4703_readRegisters();
		if ((si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_STC)) != 0) break; // Seek complete
		//freq = si4703_getChannel();
		//UART_SendStr("  -->"); UART_SendInt(freq / 10); UART_SendChar('.');
		//UART_SendInt(freq % 10); UART_SendStr("MHz\n");
		delay_nms(50); // <-- Fancy delay, in real this unnecessary
	}

	si4703_readRegisters();
	_sfbl = si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_SFBL); // Store value of SFBL bit
	si4703_registers[Si4703_POWERCFG] &= ~(1<<Si4703_PWR_SEEK); // Reset seek bit (it must be done after seek)
	si4703_updateRegisters();
	delay_1ms(); // <---------------------- Is this necessary?

	// Wait for the Si4703 to clear STC flag
	while(1) {
		si4703_readRegisters();
		if ((si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_STC)) == 0) break; // Tuning complete
		//UART_SendStr("Waiting STC clear...\n");
	}

	if (_sfbl) {
		//UART_SendStr("Seek limit hit.\n");
		return 1;
	}

	//UART_SendStr("Seek completed.\n");
	return 0;
}

void si4703_setVolume(int volume)
{
	si4703_readRegisters(); //Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  si4703_registers[Si4703_SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[Si4703_SYSCONFIG2] |= volume; //Set new volume
  si4703_updateRegisters(); //Update
}

/*uint8_t _last_textAB, _lastTextIDX;
char si4703_RDS1[64 + 2];
char si4703_RDSText[64 + 2];
bool si4703_RDSvalid;*/

void si4703_initRDS(void)
{
	unsigned int i;
	//_last_textAB = 0;
	//_lastTextIDX = 0;
	si4703_PSNvalid = false;
	//si4703_RDSvalid = false;
	for (i = 0; i < 10; i++)
	{
		si4703_PSName1[i] = '\0';
		si4703_PSName2[i] = '\0';
		si4703_programServiceName[i] = '\0';
	}
	/*for (i = 0; i < 64 + 2; i++)
	{
		si4703_RDS1[i] = 0;
		si4703_RDSText[i] = '\0';
	}*/
}

int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

void strcpy1(char dest[], const char source[]) {
	int i = 0;
	while (1) {
		dest[i] = source[i];
		if (dest[i] == '\0') break;
		i++;
	}
}

void si4703_checkRDS(void)
{
	// DEBUG_FUNC0("process");
	uint8_t  idx; // index of rdsText
	char c1, c2, valid;
	uint16_t rdsGroupType;//, rdsTP, rdsPTY;
	unsigned int i;
	//uint8_t _textAB;

	si4703_readRegisters();
	//printf("RDS ready bit check\r\n");
	if(!(si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_RDSR))){
		return;
	}

	// Serial.print('('); Serial.print(block1, HEX); Serial.print(' '); Serial.print(block2, HEX); Serial.print(' '); Serial.print(block3, HEX); Serial.print(' '); Serial.println(block4, HEX);

	if (si4703_registers[Si4703_RDSA] == 0) {
		// reset all the RDS info.
		si4703_initRDS();
	    return;
	} // if

	// analyzing Block 2 si4703_registers[Si4703_RDSB]
	rdsGroupType = 0x0A | ((si4703_registers[Si4703_RDSB] & 0xF000) >> 8) | ((si4703_registers[Si4703_RDSB] & 0x0800) >> 11);
	//rdsTP = (si4703_registers[Si4703_RDSB] & 0x0400);
	//rdsPTY = (si4703_registers[Si4703_RDSB] & 0x0400);

	switch (rdsGroupType) {
	  case 0x0A:
	  case 0x0B:
		  // The data received is part of the Service Station Name
		  idx = 2 * (si4703_registers[Si4703_RDSB] & 0x0003);

		  // new data is 2 chars from block 4
		  c1 = si4703_registers[Si4703_RDSD] >> 8;
		  c2 = si4703_registers[Si4703_RDSD] & 0x00FF;

		  // check that the data was received successfully twice
		  // before publishing the station name

		  if ((si4703_PSName1[idx] == c1) && (si4703_PSName1[idx + 1] == c2)) {
			  // retrieved the text a second time: store to _PSName2
			  si4703_PSName2[idx] = c1;
			  si4703_PSName2[idx + 1] = c2;
			  si4703_PSName2[8] = '\0';

			  if ((idx == 6) && strcmp(si4703_PSName1, si4703_PSName2) == 0) {
				  // simple fix to check for full radio name retrieval, most radio stations are 4 chars minimum anyway
				  valid = 1;
				  for (i = 0; i < 4; i++)
				  {
					  if (si4703_PSName2[i] == '\0')
						  valid = 0;
				  }
				  if (valid == 1 && strcmp(si4703_PSName2, si4703_programServiceName) != 0) {
					  // publish station name
					  strcpy1(si4703_programServiceName, si4703_PSName2);
					  si4703_PSNvalid = true;
				  } // if
			  } // if
		  } // if

		  if ((si4703_PSName1[idx] != c1) || (si4703_PSName1[idx + 1] != c2)) {
			  si4703_PSName1[idx] = c1;
			  si4703_PSName1[idx + 1] = c2;
			  si4703_PSName1[8] = '\0';
			  // Serial.println(_PSName1);
		  } // if
		  break;

	  /*case 0x2A:
	    // The data received is part of the RDS Text.
	    _textAB = (si4703_registers[Si4703_RDSB] & 0x0010);
	    idx = 4 * (si4703_registers[Si4703_RDSB] & 0x000F);

	    if (idx < _lastTextIDX) {
	    	// the existing text might be complete because the index is starting at the beginning again.
	    	// now send it to the possible listener.
	    	strcpy1(si4703_RDSText, si4703_RDS1);
	    }
	    _lastTextIDX = idx;

	    if (_textAB != _last_textAB) {
	      // when this bit is toggled the whole buffer should be cleared.
	      _last_textAB = _textAB;
	      for (i = 0; i < sizeof(si4703_RDS1); i++)
	    	  si4703_RDS1[i] = 0;
	      // Serial.println("T>CLEAR");
	    } // if

	    // new data is 2 chars from block 3
	    si4703_RDS1[idx] = (si4703_registers[Si4703_RDSC] >> 8);     idx++;
	    si4703_RDS1[idx] = (si4703_registers[Si4703_RDSC] & 0x00FF); idx++;

	    // new data is 2 chars from block 4
	    si4703_RDS1[idx] = (si4703_registers[Si4703_RDSD] >> 8);     idx++;
	    si4703_RDS1[idx] = (si4703_registers[Si4703_RDSD] & 0x00FF); idx++;

	    // Serial.print(' '); Serial.println(_RDSText);
	    // Serial.print("T>"); Serial.println(_RDSText);
	    break;

	  case 0x4A:
	    // Clock time and date
	    off = (block4)& 0x3F; // 6 bits
	    mins = (block4 >> 6) & 0x3F; // 6 bits
	    mins += 60 * (((block3 & 0x0001) << 4) | ((block4 >> 12) & 0x0F));

	    // adjust offset
	    if (off & 0x20) {
	      mins -= 30 * (off & 0x1F);
	    } else {
	      mins += 30 * (off & 0x1F);
	    }

	    if ((_sendTime) && (mins != _lastRDSMinutes)) {
	      _lastRDSMinutes = mins;
	      _sendTime(mins / 60, mins % 60);
	    } // if
	    break;*/

	  case 0x6A:
	    // IH
	    break;

	  case 0x8A:
	    // TMC
	    break;

	  case 0xAA:
	    // TMC
	    break;

	  case 0xCA:
	    // TMC
	    break;

	  case 0xEA:
	    // IH
	    break;

	  default:
	    // Serial.print("RDS_GRP:"); Serial.println(rdsGroupType, HEX);
	    break;
	}
}

void si4703_readRDS(char* buffer, long timeout)
{ 
	delay_setSysTick(timeout);
	bool completed[] = {false, false, false, false};
	int completedCount = 0;
	while(completedCount < 4 && delay_getSysTick() > 0) {
		si4703_readRegisters();
		//printf("RDS ready bit check\r\n");
		if(si4703_registers[Si4703_RSSI] & (1<<Si4703_RSSI_RDSR)){

			// ls 2 bits of B determine the 4 letter pairs
			// once we have a full set return
			// if you get nothing after 20 readings return with empty string
			uint16_t b = si4703_registers[Si4703_RDSB];
			int index = b & 0x03;
			if (! completed[index])// && b < 500)
			{
				//printf("RDSB bit SET!!\r\n");
				completed[index] = true;
				completedCount ++;
				char Dh = (si4703_registers[Si4703_RDSD] & 0xFF00) >> 8;
				char Dl = (si4703_registers[Si4703_RDSD] & 0x00FF);
				buffer[index * 2] = Dh;
				buffer[index * 2 +1] = Dl;
				// Serial.print(si4703_registers[RDSD]); Serial.print(" ");
				// Serial.print(index);Serial.print(" ");
				// Serial.write(Dh);
				// Serial.write(Dl);
				// Serial.println();
			}
			delay_nms(40); //Wait for the RDS bit to clear
		}
		else {
			delay_nms(30); //From AN230, using the polling method 40ms should be sufficient amount of time between checks
		}
	}
	if (delay_getSysTick() <= 0) {
		buffer[0] ='\0';
		return;
	}
	buffer[8] = '\0';
}

