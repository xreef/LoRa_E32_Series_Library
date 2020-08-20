/*
 * LoRa E32-TTL-100
 * Receive fixed transmission message on channel.
 * https://www.mischianti.org/2019/12/28/lora-e32-device-for-arduino-esp32-or-esp8266-wor-wake-on-radio-the-microcontroller-also-and-new-arduino-shield-part-6/
 *
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send) 7 (To dinamically manage)
 * M1         ----- 3.3v (To config) GND (To send) 6 (To dinamically manage)
 * TX         ----- RX PIN 3 (PullUP)
 * RX         ----- TX PIN 4 (PullUP & Voltage divider)
 * AUX        ----- PIN 2 (PullUP)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LowPower.h"

#include "LoRa_E32.h"
// ---------- Arduino pins --------------
//LoRa_E32 e32ttl(4, 5, 3, 7, 6);
LoRa_E32 e32ttl(4, 5, 3); // Config without connect M0 M1
// Use pin 2 as wake up pin

const int wakeUpPin = 3;

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
//LoRa_E32 e32ttl(&mySerial, 3, 7, 6);
// -------------------------------------
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void wakeUp()
{
	Serial.println("WAKE!");
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(1);
}

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(9600);
	while (!Serial) {
	    ; // wait for serial port to connect. Needed for native USB
    }
	delay(100);

	e32ttl.begin();

//	e32ttl.resetModule();
	// After set configuration comment set M0 and M1 to low
	// and reboot if you directly set HIGH M0 and M1 to program
//	ResponseStructContainer c;
//	c = e32ttl.getConfiguration();
//	Configuration configuration = *(Configuration*) c.data;
//	configuration.ADDL = 3;
//	configuration.ADDH = 0;
//	configuration.CHAN = 0x04;
//	configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
//	configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
//
//    configuration.OPTION.fec = FEC_1_ON;
//    configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
//    configuration.OPTION.transmissionPower = POWER_20;
//
//    configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
//    configuration.SPED.uartBaudRate = UART_BPS_9600;
//    configuration.SPED.uartParity = MODE_00_8N1;
//
//	e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
//	printParameters(configuration);
//  c.close();
	// ---------------------------
	Serial.println();
	Serial.println("Start listening!");
    e32ttl.setMode(MODE_2_POWER_SAVING);

    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(1, wakeUp, LOW);

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

	Serial.println("OK listening!");


}

// The loop function is called in an endless loop
void loop()
{
	if (e32ttl.available()  > 1){
		ResponseContainer rs = e32ttl.receiveMessage();
        // First of all get the data
		String message = rs.data;
		Serial.print("Received --> ");
		Serial.print(rs.status.getResponseDescription());
		Serial.print(" - ");
		Serial.println(message);
	}
	delay(125);
}

void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, DEC);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, DEC);
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

	Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
	Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
	Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
	Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
	Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

	Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN);Serial.print(" ");Serial.print(moduleInformation.HEAD, DEC);Serial.print(" ");Serial.println(moduleInformation.HEAD, HEX);

	Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}
