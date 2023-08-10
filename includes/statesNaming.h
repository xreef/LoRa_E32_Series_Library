#include "Arduino.h"

#ifdef FREQUENCY_433
	#define OPERATING_FREQUENCY 410
#elif defined(FREQUENCY_170)
	#define OPERATING_FREQUENCY 130
#elif defined(FREQUENCY_470)
	#define OPERATING_FREQUENCY 370
#elif defined(FREQUENCY_868)
	#define OPERATING_FREQUENCY 862
#elif defined(FREQUENCY_900)
	#define OPERATING_FREQUENCY 862
#elif defined(FREQUENCY_915)
	#define OPERATING_FREQUENCY 900
#else
	#define OPERATING_FREQUENCY 410
#endif

#define BROADCAST_ADDRESS 0xFF

typedef enum RESPONSE_STATUS {
#ifndef ARDUINO_ARCH_STM32
  SUCCESS = 1,
#endif
  E32_SUCCESS = 1,  
  ERR_E32_UNKNOWN,	/* something shouldn't happened */
  ERR_E32_NOT_SUPPORT,
  ERR_E32_NOT_IMPLEMENT,
  ERR_E32_NOT_INITIAL,
  ERR_E32_INVALID_PARAM,
  ERR_E32_DATA_SIZE_NOT_MATCH,
  ERR_E32_BUF_TOO_SMALL,
  ERR_E32_TIMEOUT,
  ERR_E32_HARDWARE,
  ERR_E32_HEAD_NOT_RECOGNIZED,
  ERR_E32_NO_RESPONSE_FROM_DEVICE,
  ERR_E32_WRONG_UART_CONFIG,
  ERR_E32_PACKET_TOO_BIG
} Status;

static String getResponseDescriptionByParams(byte status){
	switch (status)
	{
	  case E32_SUCCESS:
		return F("Success");
		break;
	  case ERR_E32_UNKNOWN:
		return F("Unknown");
		break;
	  case ERR_E32_NOT_SUPPORT:
		return F("Not support!");
		break;
	  case ERR_E32_NOT_IMPLEMENT:
		return F("Not implement");
		break;
	  case ERR_E32_NOT_INITIAL:
		return F("Not initial!");
		break;
	  case ERR_E32_INVALID_PARAM:
		return F("Invalid param!");
		break;
	  case ERR_E32_DATA_SIZE_NOT_MATCH:
		return F("Data size not match!");
		break;
	  case ERR_E32_BUF_TOO_SMALL:
		return F("Buff too small!");
		break;
	  case ERR_E32_TIMEOUT:
		return F("Timeout!!");
		break;
	  case ERR_E32_HARDWARE:
		return F("Hardware error!");
		break;
	  case ERR_E32_HEAD_NOT_RECOGNIZED:
		return F("Save mode returned not recognized!");
		break;
	  case ERR_E32_NO_RESPONSE_FROM_DEVICE:
		return F("No response from device! (Check wiring)");
		break;
	  case ERR_E32_WRONG_UART_CONFIG:
		return F("Wrong UART configuration! (BPS must be 9600 for configuration)");
		break;
	  case ERR_E32_PACKET_TOO_BIG:
		return F("The device support only 58byte of data transmission!");
		break;
	  default:
		return F("Invalid status!");
	}
}

enum E32_UART_PARITY
{
  MODE_00_8N1 = 0b00,
  MODE_01_8O1 = 0b01,
  MODE_10_8E1 = 0b10,
  MODE_11_8N1 = 0b11
};

static String getUARTParityDescriptionByParams(byte uartParity){
	switch (uartParity)
	{
	  case MODE_00_8N1:
		return F("8N1 (Default)");
		break;
	  case MODE_01_8O1:
		return F("8O1");
		break;
	  case MODE_10_8E1:
		return F("8E1");
		break;
	  case MODE_11_8N1:
		return F("8N1");
		break;
	  default:
		return F("Invalid UART Parity!");
	}
}

enum UART_BPS_TYPE
{
  UART_BPS_1200 = 0b000,
  UART_BPS_2400 = 0b001,
  UART_BPS_4800 = 0b010,
  UART_BPS_9600 = 0b011,
  UART_BPS_19200 = 0b100,
  UART_BPS_38400 = 0b101,
  UART_BPS_57600 = 0b110,
  UART_BPS_115200 = 0b111
};

enum UART_BPS_RATE
{
  UART_BPS_RATE_1200 = 1200,
  UART_BPS_RATE_2400 = 2400,
  UART_BPS_RATE_4800 = 4800,
  UART_BPS_RATE_9600 = 9600,
  UART_BPS_RATE_19200 = 19200,
  UART_BPS_RATE_38400 = 38400,
  UART_BPS_RATE_57600 = 57600,
  UART_BPS_RATE_115200 = 115200
};

static String getUARTBaudRateDescriptionByParams(byte uartBaudRate)
{
	switch (uartBaudRate)
	{
	  case UART_BPS_1200:
		return F("1200bps");
		break;
	  case UART_BPS_2400:
		return F("2400bps");
		break;
	  case UART_BPS_4800:
		return F("4800bps");
		break;
	  case UART_BPS_9600:
		return F("9600bps (default)");
		break;
	  case UART_BPS_19200:
		return F("19200bps");
		break;
	  case UART_BPS_38400:
		return F("38400bps");
		break;
	  case UART_BPS_57600:
		return F("57600bps");
		break;
	  case UART_BPS_115200:
		return F("115200bps");
		break;
	  default:
		return F("Invalid UART Baud Rate!");
	}
}

enum AIR_DATA_RATE
{
  AIR_DATA_RATE_000_03 = 0b000,
  AIR_DATA_RATE_001_12 = 0b001,
  AIR_DATA_RATE_010_24 = 0b010,
  AIR_DATA_RATE_011_48 = 0b011,
  AIR_DATA_RATE_100_96 = 0b100,
  AIR_DATA_RATE_101_192 = 0b101,
  AIR_DATA_RATE_110_192 = 0b110,
  AIR_DATA_RATE_111_192 = 0b111
};


static String getAirDataRateDescriptionByParams(byte airDataRate)
{
	switch (airDataRate)
	{
	  case AIR_DATA_RATE_000_03:
		return F("0.3kbps");
		break;
	  case AIR_DATA_RATE_001_12:
		return F("1.2kbps");
		break;
	  case AIR_DATA_RATE_010_24:
		return F("2.4kbps (default)");
		break;
	  case AIR_DATA_RATE_011_48:
		return F("4.8kbps");
		break;
	  case AIR_DATA_RATE_100_96:
		return F("9.6kbps");
		break;
	  case AIR_DATA_RATE_101_192:
		return F("19.2kbps");
		break;
	  case AIR_DATA_RATE_110_192:
		return F("19.2kbps");
		break;
	  case AIR_DATA_RATE_111_192:
		return F("19.2kbps");
		break;
	  default:
		return F("Invalid Air Data Rate!");
	}
}


enum FIDEX_TRANSMISSION
{
  FT_TRANSPARENT_TRANSMISSION = 0b0,
  FT_FIXED_TRANSMISSION = 0b1
};


static String getFixedTransmissionDescriptionByParams(byte fixedTransmission)
{
	switch (fixedTransmission)
	{
	  case FT_TRANSPARENT_TRANSMISSION:
		return F("Transparent transmission (default)");
		break;
	  case FT_FIXED_TRANSMISSION:
		return F("Fixed transmission (first three bytes can be used as high/low address and channel)");
		break;
	  default:
		return F("Invalid fixed transmission param!");
	}
}


enum IO_DRIVE_MODE
{
  IO_D_MODE_OPEN_COLLECTOR = 0b0,
  IO_D_MODE_PUSH_PULLS_PULL_UPS = 0b1
};


static String getIODriveModeDescriptionDescriptionByParams(byte ioDriveMode)
{
	switch (ioDriveMode)
	{
	  case IO_D_MODE_OPEN_COLLECTOR:
		return F("TXD, RXD, AUX are open-collectors");
		break;
	  case IO_D_MODE_PUSH_PULLS_PULL_UPS:
		return F("TXD, RXD, AUX are push-pulls/pull-ups");
		break;
	  default:
		return F("Invalid IO drive mode!");
	}
}

enum WIRELESS_WAKE_UP_TIME
{
  WAKE_UP_250 = 0b000,
  WAKE_UP_500 = 0b001,
  WAKE_UP_750 = 0b010,
  WAKE_UP_1000 = 0b011,
  WAKE_UP_1250 = 0b100,
  WAKE_UP_1500 = 0b101,
  WAKE_UP_1750 = 0b110,
  WAKE_UP_2000 = 0b111
};


static String getWirelessWakeUPTimeDescriptionByParams(byte wirelessWakeUPTime)
{
	switch (wirelessWakeUPTime)
	{
	  case WAKE_UP_250:
		return F("250ms (default)");
		break;
	  case WAKE_UP_500:
		return F("500ms");
		break;
	  case WAKE_UP_750:
		return F("750ms");
		break;
	  case WAKE_UP_1000:
		return F("1000ms");
		break;
	  case WAKE_UP_1250:
		return F("1250ms");
		break;
	  case WAKE_UP_1500:
		return F("1500ms");
		break;
	  case WAKE_UP_1750:
		return F("1750ms");
		break;
	  case WAKE_UP_2000:
		return F("2000ms");
		break;
	  default:
		return F("Invalid wireless wake-up mode!");
	}
}
enum FORWARD_ERROR_CORRECTION_SWITCH
{
  FEC_0_OFF = 0b0,
  FEC_1_ON = 0b1
};


static String getFECDescriptionByParams(byte fec)
{
	switch (fec)
	{
	  case FEC_0_OFF:
		return F("Turn off Forward Error Correction Switch");
		break;
	  case FEC_1_ON:
		return F("Turn on Forward Error Correction Switch (Default)");
		break;
	  default:
		return F("Invalid FEC param");
	}
}

#ifdef E32_TTL_100
	enum TRANSMISSION_POWER
	{
	  POWER_20 = 0b00,
	  POWER_17 = 0b01,
	  POWER_14 = 0b10,
	  POWER_10 = 0b11

	};

	static String getTransmissionPowerDescriptionByParams(byte transmissionPower)
	{
		switch (transmissionPower)
		{
		  case POWER_20:
			return F("20dBm (Default)");
			break;
		  case POWER_17:
			return F("17dBm");
			break;
		  case POWER_14:
			return F("14dBm");
			break;
		  case POWER_10:
			return F("10dBm");
			break;
		  default:
			return F("Invalid transmission power param");
		}
	}
#elif defined(E32_TTL_500)
	enum TRANSMISSION_POWER
	{
	  POWER_27 = 0b00,
	  POWER_24 = 0b01,
	  POWER_21 = 0b10,
	  POWER_18 = 0b11

	};

	static String getTransmissionPowerDescriptionByParams(byte transmissionPower)
	{
		switch (transmissionPower)
		{
		  case POWER_27:
			return F("27dBm (Default)");
			break;
		  case POWER_24:
			return F("24dBm");
			break;
		  case POWER_21:
			return F("21dBm");
			break;
		  case POWER_18:
			return F("18dBm");
			break;
		  default:
			return F("Invalid transmission power param");
		}
	}
#elif defined(E32_TTL_1W)
	enum TRANSMISSION_POWER
	{
	  POWER_30 = 0b00,
	  POWER_27 = 0b01,
	  POWER_24 = 0b10,
	  POWER_21 = 0b11

	};

	static String getTransmissionPowerDescriptionByParams(byte transmissionPower)
	{
		switch (transmissionPower)
		{
		  case POWER_30:
			return F("30dBm (Default)");
			break;
		  case POWER_27:
			return F("27dBm");
			break;
		  case POWER_24:
			return F("24dBm");
			break;
		  case POWER_21:
			return F("21dBm");
			break;
		  default:
			return F("Invalid transmission power param");
		}
	}
#elif defined(E32_TTL_2W)
	enum TRANSMISSION_POWER
	{
	  POWER_33 = 0b00,
	  POWER_30 = 0b01,
	  POWER_27 = 0b10,
	  POWER_24 = 0b11

	};

	static String getTransmissionPowerDescriptionByParams(byte transmissionPower)
	{
		switch (transmissionPower)
		{
		  case POWER_33:
			return F("33dBm (Default)");
			break;
		  case POWER_30:
			return F("30dBm");
			break;
		  case POWER_27:
			return F("27dBm");
			break;
		  case POWER_24:
			return F("24dBm");
			break;
		  default:
			return F("Invalid transmission power param");
		}
	}
#else
	enum TRANSMISSION_POWER
	{
	  POWER_20 = 0b00,
	  POWER_17 = 0b01,
	  POWER_14 = 0b10,
	  POWER_10 = 0b11

	};

	static String getTransmissionPowerDescriptionByParams(byte transmissionPower)
	{
		switch (transmissionPower)
		{
		  case POWER_20:
			return F("20dBm (Default)");
			break;
		  case POWER_17:
			return F("17dBm");
			break;
		  case POWER_14:
			return F("14dBm");
			break;
		  case POWER_10:
			return F("10dBm");
			break;
		  default:
			return F("Invalid transmission power param");
		}
	}
#endif
