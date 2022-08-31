/*
 * EBYTE LoRa E32
 * Stay in sleep mode and wait a wake up WOR message
 *
 * You must configure the address with 0 3 23 with WOR receiver enable
 * and pay attention that WOR period must be the same of sender
 *
 *
 * https://www.mischianti.org
 *
 * E32		  ----- stm32
 * M0         ----- PB0 (or 3.3v)
 * M1         ----- PB10 (or GND)
 * RX         ----- PA2 TX2 (PullUP)
 * TX         ----- PA3 RX2 (PullUP)
 * AUX        ----- PA0  (PullUP)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */

// with this DESTINATION_ADDL 2 you must set
// WOR SENDER configuration to the other device and
// WOR RECEIVER to this device
#define DESTINATION_ADDL 0x02

#include "Arduino.h"
#include "LoRa_E32.h"
#include "STM32LowPower.h"

void wakedUp() {
	// randomly you can read this, but normally reset block every time the execution
	Serial.println("Wake-Up callback!");
}

void printParameters(struct Configuration configuration);

// ---------- esp8266 pins --------------
//LoRa_E32 e32ttl(RX, TX, AUX, M0, M1);  // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E32 e32ttl(D3, D4, D5, D7, D6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E32 e32ttl(D2, D3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D2, D3); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E32 e32ttl(&mySerial, D5, D7, D6); // AUX M0 M1
// -------------------------------------

// ---------- Arduino pins --------------
//LoRa_E32 e32ttl(4, 5, 3, 7, 6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E32 e32ttl(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E32 e32ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

// ---------- esp32 pins --------------
//LoRa_E32 e32ttl(&Serial2, 15, 21, 19); //  RX AUX M0 M1

//LoRa_E32 e32ttl(&Serial2, 22, 4, 18, 21, 19, UART_BPS_RATE_9600); //  esp32 RX <-- e22 TX, esp32 TX --> e22 RX AUX M0 M1
// -------------------------------------

// ---------------- STM32 --------------------
HardwareSerial Serial2(USART2);   // PA3  (RX)  PA2  (TX)
LoRa_E32 e32ttl(&Serial2, PA0, PB0, PB10); //  RX AUX M0 M1
// -------------------------------------------------

const int pin = SYS_WKUP1;

//The setup function is called once at startup of the sketch
void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    delay(100);

    e32ttl.begin();

	// After set configuration comment set M0 and M1 to low
	// and reboot if you directly set HIGH M0 and M1 to program
	ResponseStructContainer c;
	c = e32ttl.getConfiguration();
	Configuration configuration = *(Configuration*) c.data;
	configuration.ADDL = 0x03;
	configuration.ADDH = 0x00;
	configuration.CHAN = 0x04;
	configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_2000;
	e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	printParameters(configuration);
	c.close();
	// ---------------------------
	e32ttl.setMode(MODE_2_POWER_SAVING);

	// Configure low power
	LowPower.begin();
	// Attach a wakeup interrupt on pin, calling repetitionsIncrease when the device is woken up
	// Last parameter (LowPowerMode) should match with the low power state used: in this example LowPower.sleep()
	LowPower.attachInterruptWakeup(pin, wakedUp, LOW, SLEEP_MODE);

	Serial.println("Start SLEEP mode in ");
	for (int i = 10;i>0;i--) { Serial.print(i); Serial.print(" "); delay(400); } Serial.println( "OK!" );
	delay(1000);

	LowPower.sleep();

	delay(1000);

    Serial.println();
    Serial.println("Wake and start listening!");
}

// The loop function is called in an endless loop
void loop()
{
    if (e32ttl.available()  > 1){
    	Serial.println("Message arrived!");
        ResponseContainer rs = e32ttl.receiveMessage();
        // First of all get the data
        String message = rs.data;

        Serial.println(rs.status.getResponseDescription());
        Serial.println(message);

        e32ttl.setMode(MODE_0_NORMAL);

    	delay(1000);

        e32ttl.sendFixedMessage(0, DESTINATION_ADDL, 0x04, "We have received the message!");
    }
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
