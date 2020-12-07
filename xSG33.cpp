/*!
 * @file Adafruit_SGP30.cpp
 *
 * @mainpage Adafruit SGP30 gas sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's SGP30 driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit SGP30 breakout: http://www.adafruit.com/products/3709
 *
 * These sensors use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the breakout.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 *
 * @section author Author
 * Written by Ladyada for Adafruit Industries.
 *
 * @section license License
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "xSG33.h"


#if MICROBIT_CODAL
#define BUFFER_TYPE uint8_t*
#else
#define BUFFER_TYPE char*
#endif

#define I2C_DEBUG 1

using namespace pxt;

int response = 0;

/*!
 *  @brief  Instantiates a new SGP30 class
 */
xSG33::xSG33() {}

/*!
 *  @brief  Setups the hardware and detects a valid SGP30. Initializes I2C
 *          then reads the serialnumber and checks that we are talking to an
 * SGP30
 *  @param  theWire
 *          Optional pointer to I2C interface, otherwise use Wire
 *  @param  initSensor
 *          Optional pointer to prevent IAQinit to be called. Used for Deep
 *          Sleep.
 *  @return True if SGP30 found on I2C, False if something went wrong!
 */
bool xSG33::begin() {
	
  uint8_t command[2];
  command[0] = 0x36;
  command[1] = 0x82;
  
  uBit.serial.send("begin command 1\n");
  readWordFromCommand(command, 2, 10, serialnumber, 3);

  uint16_t featureset;
  command[0] = 0x20;
  command[1] = 0x2F;
  
  uBit.serial.send("begin command 2\n");
  readWordFromCommand(command, 2, 10, &featureset, 1);
  // Serial.print("Featureset 0x"); Serial.println(featureset, HEX);
  if ((featureset & 0xF0) != SGP30_FEATURESET)
    return false;

  uBit.serial.send("IAQ init\n");
  IAQinit();

  return true;
}


/*boolean Adafruit_SGP30::softReset(void) {
  uint8_t command[2];
  command[0] = 0x00;
  command[1] = 0x06;
  return readWordFromCommand(command, 2, 10);
}
*/

bool xSG33::IAQinit(void) {
  uint8_t command[2];
  command[0] = 0x20;
  command[1] = 0x03;
  return readWordFromCommand(command, 2, 10);
}


bool xSG33::IAQmeasure(void) {
  uint8_t command[2];
  command[0] = 0x20;
  command[1] = 0x08;
  uint16_t reply[2];
  uBit.serial.send("read TVOC and eCO2\n");
  readWordFromCommand(command, 2, 12, reply, 2);
  TVOC = reply[1];
  eCO2 = reply[0];
  return true;
}


bool xSG33::IAQmeasureRaw(void) {
  uint8_t command[2];
  command[0] = 0x20;
  command[1] = 0x50;
  uint16_t reply[2];
  if (!readWordFromCommand(command, 2, 25, reply, 2))
    return false;
  rawEthanol = reply[1];
  rawH2 = reply[0];
  return true;
}

/*
bool xSG33::getIAQBaseline(uint16_t *eco2_base,
                                       uint16_t *tvoc_base) {
  uint8_t command[2];
  command[0] = 0x20;
  command[1] = 0x15;
  uint16_t reply[2];
  if (!readWordFromCommand(command, 2, 10, reply, 2))
    return false;
  *eco2_base = reply[0];
  *tvoc_base = reply[1];
  return true;
}
*/


/*boolean Adafruit_SGP30::setIAQBaseline(uint16_t eco2_base, uint16_t tvoc_base) {
  uint8_t command[8];
  command[0] = 0x20;
  command[1] = 0x1e;
  command[2] = tvoc_base >> 8;
  command[3] = tvoc_base & 0xFF;
  command[4] = generateCRC(command + 2, 2);
  command[5] = eco2_base >> 8;
  command[6] = eco2_base & 0xFF;
  command[7] = generateCRC(command + 5, 2);

  return readWordFromCommand(command, 8, 10);
}*/

bool xSG33::setHumidity(uint32_t absolute_humidity) {
  if (absolute_humidity > 256000) {
    return false;
  }

  uint16_t ah_scaled =
      (uint16_t)(((uint64_t)absolute_humidity * 256 * 16777) >> 24);
  uint8_t command[5];
  command[0] = 0x20;
  command[1] = 0x61;
  command[2] = ah_scaled >> 8;
  command[3] = ah_scaled & 0xFF;
  command[4] = generateCRC(command + 2, 2);

  return readWordFromCommand(command, 5, 10);
}

/*!
 *  @brief  I2C low level interfacing
 */

bool xSG33::readWordFromCommand(uint8_t command[],
                                         uint8_t commandLength,
                                         uint16_t delayms, uint16_t *readdata,
                                         uint8_t readlen) {

  /*if (!i2c_dev->write(command, commandLength)) {
    return false;
  }*/
  
  response = uBit.i2c.write(SGP30_I2CADDR_DEFAULT << 1, (BUFFER_TYPE)command, commandLength, false);
  
  //reply = uBit.i2c.write(0x76 << 1, (BUFFER_TYPE)command, commandLength, false);
  #ifdef I2C_DEBUG
  uBit.serial.send("Write \n");
  uBit.serial.send(response ? "false" : "true");
  uBit.serial.send("\n");
  #endif

  //delay(delayms);
  
  uBit.sleep(delayms);

  if (readlen == 0)
    return true;

  uint8_t replylen = readlen * (SGP30_WORD_LEN + 1);
  uint8_t replybuffer[replylen];

  /*if (!i2c_dev->read(replybuffer, replylen)) {
    return false;
  }*/
  
  response = uBit.i2c.read(SGP30_I2CADDR_DEFAULT << 1, (BUFFER_TYPE)replybuffer, replylen, false);
  //reply = uBit.i2c.read(0x76 << 1, (BUFFER_TYPE)replybuffer, replylen, false);
  
  #ifdef I2C_DEBUG
  uBit.serial.send("Read \n");
  uBit.serial.send(response ? "false" : "true");
  uBit.serial.send("\n");
  
  uBit.serial.send("VAlue of MICROBIT_OK \n");
  uBit.serial.send(MICROBIT_OK);
  uBit.serial.send("\n");
  
  #endif

  for (uint8_t i = 0; i < readlen; i++) {
    uint8_t crc = generateCRC(replybuffer + i * 3, 2);
#ifdef I2C_DEBUG
    uBit.serial.send("\t\tCRC calced: 0x\n");
	uBit.serial.send("\n");
    uBit.serial.send(crc);
	uBit.serial.send("\n");
    uBit.serial.send(" vs. 0x\n");
    uBit.serial.send(replybuffer[i * 3 + 2]);
	uBit.serial.send("\n");
#endif
    if (crc != replybuffer[i * 3 + 2])
      return false;
    // success! store it
    readdata[i] = replybuffer[i * 3];
    readdata[i] <<= 8;
    readdata[i] |= replybuffer[i * 3 + 1];
#ifdef I2C_DEBUG
    uBit.serial.send("\t\tRead: 0x\n");
    uBit.serial.send(readdata[i]);
	uBit.serial.send("\n");
#endif
  }
  return true;
}

uint8_t xSG33::generateCRC(uint8_t *data, uint8_t datalen) {
  // calculates 8-Bit checksum with given polynomial
  uint8_t crc = SGP30_CRC8_INIT;

  for (uint8_t i = 0; i < datalen; i++) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ SGP30_CRC8_POLYNOMIAL;
      else
        crc <<= 1;
    }
  }
  return crc;
}

namespace sg33
{
	xSG33 IAQ = xSG33();
	
	//%
	bool begin()
	{
		return IAQ.begin();
	}
	
	//%
	bool IAQmeasure()
	{
		return IAQ.IAQmeasure();
	}
	
	//%
	uint16_t getTVOC()
	{
		return IAQ.TVOC;
	}
	
	//%
	uint16_t getCO2()
	{
		return IAQ.eCO2;
	}
}
