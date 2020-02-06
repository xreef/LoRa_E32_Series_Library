/*
 * EBYTE LoRa E32 Series
 * https://www.mischianti.org/category/my-libraries/lora-e32-devices/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LoRa_E32_h
#define LoRa_E32_h

#define M0_PIN	7
#define M1_PIN	8
#define AUX_PIN	A0
#define SOFT_RX_PIN	10
#define SOFT_TX_PIN 11

#ifndef ESP32
	#define ACTIVATE_SOFTWARE_SERIAL
#endif

#ifdef ACTIVATE_SOFTWARE_SERIAL
	#include <SoftwareSerial.h>
#endif

#include <includes/statesNaming.h>

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Uncomment to enable printing out nice debug messages.
// #define LoRa_E32_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef LoRa_E32_DEBUG
	#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
	#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
	#define DEBUG_PRINT(...) {}
	#define DEBUG_PRINTLN(...) {}
#endif

enum MODE_TYPE
{
  MODE_0_NORMAL = 0,
  MODE_1_WAKE_UP = 1,
  MODE_2_POWER_SAVING = 2,
  MODE_3_SLEEP = 3,
  MODE_3_PROGRAM =3,
  MODE_INIT = 0xFF
};

enum PROGRAM_COMMAND
{
  WRITE_CFG_PWR_DWN_SAVE  	= 0xC0,
  READ_CONFIGURATION 		= 0xC1,
  WRITE_CFG_PWR_DWN_LOSE 	= 0xC2,
  READ_MODULE_VERSION   	= 0xC3,
  WRITE_RESET_MODULE     	= 0xC4
};

#pragma pack(push, 1)
struct Speed {
  uint8_t airDataRate : 3; //bit 0-2
	String getAirDataRate() {
		return getAirDataRateDescriptionByParams(this->airDataRate);
	}

  uint8_t uartBaudRate: 3; //bit 3-5
	String getUARTBaudRate() {
		return getUARTBaudRateDescriptionByParams(this->uartBaudRate);
	}

  uint8_t uartParity:   2; //bit 6-7
	String getUARTParityDescription() {
		return getUARTParityDescriptionByParams(this->uartParity);
	}
};

struct Option {
	byte transmissionPower	: 2; //bit 0-1
	String getTransmissionPowerDescription() {
		return getTransmissionPowerDescriptionByParams(this->transmissionPower);
	}

	byte fec       		: 1; //bit 2
	String getFECDescription() {
		return getFECDescriptionByParams(this->fec);
	}

	byte wirelessWakeupTime : 3; //bit 3-5
	String getWirelessWakeUPTimeDescription() {
		return getWirelessWakeUPTimeDescriptionByParams(this->wirelessWakeupTime);
	}

	byte ioDriveMode  		: 1; //bit 6
	String getIODroveModeDescription() {
		return getIODriveModeDescriptionDescriptionByParams(this->ioDriveMode);
	}

	byte fixedTransmission	: 1; //bit 7
	String getFixedTransmissionDescription() {
		return getFixedTransmissionDescriptionByParams(this->fixedTransmission);
	}

};

struct Configuration {
	byte HEAD = 0;
	byte ADDH = 0;
	byte ADDL = 0;
	struct Speed SPED;
	byte CHAN = 0;
	String getChannelDescription() {
		return String(this->CHAN + OPERATING_FREQUENCY) + F("MHz") ;
	}
	struct Option OPTION;
};

struct ModuleInformation {
	byte HEAD = 0;
	byte model = 0;
	byte version = 0;
	byte features = 0;
};

struct ResponseStatus {
	Status code;
	String getResponseDescription() {
		return getResponseDescriptionByParams(this->code);
	}
};

struct ResponseStructContainer {
	void *data;
	ResponseStatus status;
};
struct ResponseContainer {
	String data;
	ResponseStatus status;
};
//struct FixedStransmission {
//		byte ADDL = 0;
//		byte ADDH = 0;
//		byte CHAN = 0;
//		void *message;
//};
#pragma pack(pop)

class LoRa_E32 {
	public:
		#ifdef ACTIVATE_SOFTWARE_SERIAL
			LoRa_E32(byte rxPin, byte txPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
			LoRa_E32(byte rxPin, byte txPin, byte auxPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
			LoRa_E32(byte rxPin, byte txPin, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
		#endif

		LoRa_E32(HardwareSerial* serial, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
		LoRa_E32(HardwareSerial* serial, byte auxPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
		LoRa_E32(HardwareSerial* serial, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);

		#ifdef ESP32
			LoRa_E32(HardwareSerial* serial, byte rxPin, byte txPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, uint32_t serialConfig = SERIAL_8N1);
			LoRa_E32(HardwareSerial* serial, byte rxPin, byte txPin, byte auxPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, uint32_t serialConfig = SERIAL_8N1);
			LoRa_E32(HardwareSerial* serial, byte rxPin, byte txPin, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, uint32_t serialConfig = SERIAL_8N1);
		#endif

		#ifdef ACTIVATE_SOFTWARE_SERIAL
			LoRa_E32(SoftwareSerial* serial, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
			LoRa_E32(SoftwareSerial* serial, byte auxPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
			LoRa_E32(SoftwareSerial* serial, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
		#endif

//		LoRa_E32(byte rxPin, byte txPin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, MODE_TYPE mode = MODE_0_NORMAL);
//		LoRa_E32(HardwareSerial* serial = &Serial, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, MODE_TYPE mode = MODE_0_NORMAL);
//		LoRa_E32(SoftwareSerial* serial, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600, MODE_TYPE mode = MODE_0_NORMAL);

		bool begin();
        Status setMode(uint8_t mode);

		ResponseStructContainer getConfiguration();
		ResponseStatus setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType = WRITE_CFG_PWR_DWN_LOSE);

		ResponseStructContainer getModuleInformation();
        void printParameters(struct Configuration configuration);
        ResponseStatus resetModule();

        ResponseStatus sendMessage(const void *message, const uint8_t size);
        ResponseStructContainer receiveMessage(const uint8_t size);

        ResponseStatus sendMessage(const String message);
        ResponseContainer receiveMessage();

        ResponseStatus sendFixedMessage(byte ADDL, byte ADDH, byte CHAN, const String message);
        ResponseStatus sendBroadcastFixedMessage(byte CHAN, const String message);

        ResponseStatus sendFixedMessage(byte ADDL, byte ADDH, byte CHAN, const void *message, const uint8_t size);
        ResponseStatus sendBroadcastFixedMessage(byte CHAN, const void *message, const uint8_t size );

        ResponseContainer receiveInitialMessage(const uint8_t size);

        int available(unsigned long timeout = 1000);
	private:
		HardwareSerial* hs;

		#ifdef ACTIVATE_SOFTWARE_SERIAL
			SoftwareSerial* ss;
		#endif

		bool isSoftwareSerial = true;

        byte rxPin = 0;
        byte txPin = 0;
        byte auxPin = 0;

        uint32_t serialConfig = SERIAL_8N1;

        byte m0Pin = 0;
        byte m1Pin = 0;

        unsigned long halfKeyloqKey = 0x06660708;
        unsigned long encrypt(unsigned long data);
        unsigned long decrypt(unsigned long data);

        UART_BPS_RATE bpsRate = UART_BPS_RATE_9600;

		struct NeedsStream{

		  template< typename T >
		  void begin( T &t, int baud, uint32_t config ){
			  DEBUG_PRINTLN("Begin ");
			  t.setTimeout(500);
			  t.begin(baud, config);
			  stream = &t;
		  }

		  template< typename T >
		  void begin( T &t, int baud, uint32_t config, int8_t rxPin, int8_t txPin ){
			  DEBUG_PRINTLN("Begin ");
			  t.setTimeout(500);
			  t.begin(baud, config, rxPin, txPin);
			  stream = &t;
		  }

		  void listen(){

		  }


		  Stream *stream;
		};
		NeedsStream serialDef;

        MODE_TYPE mode = MODE_0_NORMAL;

        void managedDelay(unsigned long timeout);
        Status waitCompleteResponse(unsigned long timeout = 1000, unsigned int waitNoAux = 100);
        void flush();
        void cleanUARTBuffer();

        Status sendStruct(void *structureManaged, uint16_t size_);
        Status receiveStruct(void *structureManaged, uint16_t size_);
        void writeProgramCommand(PROGRAM_COMMAND cmd);


};

#endif
