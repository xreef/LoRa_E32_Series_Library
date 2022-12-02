/*
 * LoRa E32-TTL-100
 * Receive fixed transmission message on channel and wake up.
 * https://www.mischianti.org/2020/01/17/lora-e32-device-for-arduino-esp32-or-esp8266-wor-wake-on-radio-microcontroller-and-new-wemos-d1-mini-shield-part-7/
 *
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- LOW
 * M1         ----- HIGH
 * TX         ----- RX PIN D3 (PullUP)
 * RX         ----- TX PIN D4 (PullUP)
 * AUX        ----- PIN D5
 * VCC        ----- 5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h"
#include <ESP8266WiFi.h>

#define FPM_SLEEP_MAX_TIME           0xFFFFFFF
void callback() {
  Serial.println("Callback");
  Serial.flush();
}

// ---------- esp8266 pins --------------
LoRa_E32 e32ttl(D3, D4, D5);  // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
// -------------------------------------
void printParameters(struct Configuration configuration);
//The setup function is called once at startup of the sketch
void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    delay(100);

    e32ttl.begin();
        e32ttl.setMode(MODE_2_POWER_SAVING);

//  e32ttl.resetModule();
    // After set configuration comment set M0 and M1 to low
    // and reboot if you directly set HIGH M0 and M1 to program
    ResponseStructContainer c;
    c = e32ttl.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
    printParameters(configuration);

    configuration.ADDL = 3;
    configuration.ADDH = 0;
    configuration.CHAN = 0x04;
    configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;

    configuration.OPTION.fec = FEC_1_ON;
    configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
    configuration.OPTION.transmissionPower = POWER_20;

    configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
    printParameters(configuration);

    c.close();
    // ---------------------------
    delay(1000);
    Serial.println();
    Serial.println("Start sleep!");

    //wifi_station_disconnect(); //not needed
    gpio_pin_wakeup_enable(GPIO_ID_PIN(D5), GPIO_PIN_INTR_LOLEVEL);
    wifi_set_opmode(NULL_MODE);
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_set_wakeup_cb(callback);
    wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
    delay(1000);

    Serial.println();
    Serial.println("Start listening!");

}

// The loop function is called in an endless loop
void loop()
{
    if (e32ttl.available()  > 1){
        ResponseContainer rs = e32ttl.receiveMessage();
        // First of all get the data
        String message = rs.data;

        Serial.println(rs.status.getResponseDescription());
        Serial.println(message);
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
