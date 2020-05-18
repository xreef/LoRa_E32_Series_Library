/*
 * LoRa E32-TTL-100
 * Start device, reset or write to the Serial to send a message.
 * https://www.mischianti.org/2019/10/15/lora-e32-device-for-arduino-esp32-or-esp8266-specs-and-basic-usage-part-1/
 *
 * E32-TTL-100----- Arduino UNO
 * M0         ----- GND
 * M1         ----- GND
 * TX         ----- PIN 2 (PullUP)
 * RX         ----- PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h"

// ---------- esp8266 pins --------------
//LoRa_E32 e32ttl(D2, D3, D5, D7, D6);
//LoRa_E32 e32ttl(D2, D3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D2, D3); // RX, TX
//LoRa_E32 e32ttl(&mySerial, D5, D7, D6);
// -------------------------------------

// ---------- Arduino pins --------------
//LoRa_E32 e32ttl(2, 3, 5, 7, 6);
LoRa_E32 e32ttl100(2, 3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(2, 3); // RX, TX
//LoRa_E32 e32ttl(&mySerial, 5, 7, 6);
// -------------------------------------

void setup() {
  Serial.begin(9600);
  delay(500);

  // Startup all pins and UART
  e32ttl100.begin();

//  If you have ever change configuration you must restore It
//	ResponseStructContainer c;
//	c = e32ttl100.getConfiguration();
//	Configuration configuration = *(Configuration*) c.data;
//	Serial.println(c.status.getResponseDescription());
//	configuration.CHAN = 0x17;
//	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
//	e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);

  Serial.println("Hi, I'm going to send message!");

  // Send message
  ResponseStatus rs = e32ttl100.sendMessage("Hello, world?");
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());
}

void loop() {
	// If something available
  if (e32ttl100.available()>1) {
	  // read the String message
	ResponseContainer rc = e32ttl100.receiveMessage();
	// Is something goes wrong print error
	if (rc.status.code!=1){
		rc.status.getResponseDescription();
	}else{
		// Print the data received
		Serial.println(rc.data);
	}
  }
  if (Serial.available()) {
	  String input = Serial.readString();
	  e32ttl100.sendMessage(input);
  }
}

