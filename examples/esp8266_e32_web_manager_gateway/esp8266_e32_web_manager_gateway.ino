/*
 * LoRa EByte E32
 * EByte LoRa E32 Manager for esp8266.
 *
 * Created by Renzo Mischianti <renzo.mischianti@gmail.com>
 * License: CC BY-NC-ND 3.0
 *
 * https://www.mischianti.org
 *
 * E32		  ----- WeMos D1 mini
 * M0         ----- D7 (or GND)
 * M1         ----- D6 (or 3.3v)
 * TX         ----- D3 (PullUP)
 * RX         ----- D4 (PullUP)
 * AUX        ----- D5 (PullUP)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h"
#include <ESP8266WiFi.h>        // Include the Wi-Fi library

#include <WebSocketsServer.h>

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>        // Include the mDNS library


#include "LittleFS.h"



#ifndef SERVER_MODE
#include <DNSServer.h>
DNSServer dnsServer;
const byte DNS_PORT = 53;
#endif

// Uncomment to enable printing out nice debug messages.
#define EBYTE_MANAGER_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef EBYTE_MANAGER_DEBUG
	#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
	#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
	#define DEBUG_PRINT(...) {}
	#define DEBUG_PRINTLN(...) {}
#endif

#define RESET_PIN D0

#define HTTP_REST_PORT 8080
ESP8266WebServer httpRestServer(HTTP_REST_PORT);

#define HTTP_PORT 80
ESP8266WebServer httpServer(HTTP_PORT);

#define WS_PORT 8081
#define WS_UPDATE_TIME 10000

WebSocketsServer webSocket = WebSocketsServer(WS_PORT);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);


// ---------- esp8266 pins --------------
//LoRa_E22 e32ttl(RX, TX, AUX, M0, M1);  // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
LoRa_E32 e32ttl(D3, D4, D5, D7, D6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E22 e22ttl(D2, D3); // Config without connect AUX and M0 M1
//LoRa_E32 e32ttl(&Serial, D3, D0, D8); // RX, TX
//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D2, D3); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E22 e22ttl(&mySerial, D5, D7, D6); // AUX M0 M1
// -------------------------------------

// ---------- Arduino pins --------------
//LoRa_E22 e22ttl(4, 5, 3, 7, 6); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
//LoRa_E22 e32ttl(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX
//LoRa_E22 e22ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

// ---------- esp32 pins --------------
//LoRa_E22 e32ttl(&Serial2, 18, 21, 19); //  RX AUX M0 M1

//LoRa_E22 e32ttl(&Serial2, 22, 4, 18, 21, 19, UART_BPS_RATE_9600); //  esp32 RX <-- e22 TX, esp32 TX --> e22 RX AUX M0 M1
// -------------------------------------

void sendCrossOriginHeader();
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void sendTransparentMessage();
void sendFixedMessage();
void sendBroadcastMessage();
void resetModule();
void resetMicrocontroller();
void serverRouting();
void realtimeDataCallbak();

void sendWSMessageOfMessageReceived(bool readSingleMessage);

#define SERVER_MODE

const char* ssid     = "<YOUR-SSID>";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "<YOUR-PASSWD>";     // The password of the Wi-Fi network

// Config for SoftAccessPoint
const char* hostname = "e32dev01";

#ifndef SERVER_MODE
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
#endif

bool readSingleMessage = true;
bool isConnectedWebSocket = false;
bool isConnectedWebSocketAck = false;

void setup() {
	DEBUG_PRINTER.begin(9600);
	delay(500);

#ifdef SERVER_MODE
	WiFi.begin(ssid, password);             // Connect to the network
	  DEBUG_PRINT(F("Connecting to "));
	  DEBUG_PRINT(ssid); DEBUG_PRINTLN(" ...");

	  int i = 0;
	  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
	    delay(1000);
	    DEBUG_PRINT(++i); DEBUG_PRINT(' ');
	  }

	  DEBUG_PRINTLN('\n');
	  DEBUG_PRINTLN("Connection established!");
	  DEBUG_PRINT("IP address:\t");
	  DEBUG_PRINTLN(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
#else
	  DEBUG_PRINT("Setting soft-AP configuration ... ");
		  DEBUG_PRINTLN(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

		  DEBUG_PRINT("Setting soft-AP ... ");
		  bool sap = WiFi.softAP(hostname);
		  DEBUG_PRINTLN(sap ? "Ready" : "Failed!");


		  if(sap == true)
		  {
			  DEBUG_PRINT("Name: ");
			  DEBUG_PRINT(hostname);
			  DEBUG_PRINTLN(F(" Ready"));

			  DEBUG_PRINT("Soft-AP IP address = ");
			  DEBUG_PRINTLN(WiFi.softAPIP());

			  // if DNSServer is started with "*" for domain name, it will reply with
			  // provided IP to all DNS request
			  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

		  }
		  else
		  {
			  DEBUG_PRINTLN(F("Failed!"));
		  }
#endif

	    if (!MDNS.begin(hostname)) {             // Start the mDNS responder for esp8266.local
	    	DEBUG_PRINTLN(F("Error setting up mDNS responder!"));
	    }
	    DEBUG_PRINT(hostname);
	    DEBUG_PRINTLN(F(" --> mDNS responder started"));
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);

	// Startup all pins and UART
	e32ttl.begin();

	DEBUG_PRINTLN(F("Inizializing FS..."));
	if (LittleFS.begin()){
		DEBUG_PRINTLN(F("done."));
	}else{
		DEBUG_PRINTLN(F("fail."));
	}


	restServerRouting();
	httpRestServer.begin();
	DEBUG_PRINTLN(F("REST Server Started"));

	serverRouting();
	httpServer.begin();
	DEBUG_PRINTLN(F("Web Server Started"));

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	DEBUG_PRINTLN(F("WS Server Started"));

#ifdef RESET_PIN
	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);
#endif
}


unsigned long lastRead = millis();

void loop() {
	if (isConnectedWebSocket){
		if (isConnectedWebSocketAck && e32ttl.available()>1){
			sendWSMessageOfMessageReceived(readSingleMessage);
		}
		if (lastRead+WS_UPDATE_TIME<millis()){
			realtimeDataCallbak();
			lastRead = millis();
		}

	}

	httpRestServer.handleClient();
	httpServer.handleClient();

	webSocket.loop();

#ifndef SERVER_MODE
	  dnsServer.processNextRequest();
#endif
}

void restServerRouting() {
//	httpRestServer.header("Access-Control-Allow-Headers: Authorization, Content-Type");
//
    httpRestServer.on(F("/"), HTTP_GET, []() {
    	DEBUG_PRINTLN(F("CALL /"));
    	httpRestServer.send(200, F("text/html"),
            F("Welcome to the Inverter Centraline REST Web Server"));
    });
    httpRestServer.on(F("/configuration"), HTTP_GET, getConfiguration);
    httpRestServer.on(F("/configuration"), HTTP_POST, postConfiguration);
    httpRestServer.on(F("/configuration"), HTTP_OPTIONS, sendCrossOriginHeader);

    httpRestServer.on(F("/reset"), HTTP_GET, resetMicrocontroller);

    httpRestServer.on(F("/resetModule"), HTTP_GET, resetModule);
    httpRestServer.on(F("/moduleInfo"), HTTP_GET, getModuleInfo);

    httpRestServer.on(F("/transparentMessage"), HTTP_POST, sendTransparentMessage);
    httpRestServer.on(F("/transparentMessage"), HTTP_OPTIONS, sendCrossOriginHeader);

    httpRestServer.on(F("/fixedMessage"), HTTP_POST, sendFixedMessage);
    httpRestServer.on(F("/fixedMessage"), HTTP_OPTIONS, sendCrossOriginHeader);

    httpRestServer.on(F("/broadcastMessage"), HTTP_POST, sendBroadcastMessage);
    httpRestServer.on(F("/broadcastMessage"), HTTP_OPTIONS, sendCrossOriginHeader);


}

void setCrossOrigin(){
	httpRestServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
	httpRestServer.sendHeader(F("Access-Control-Max-Age"), F("600"));
	httpRestServer.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
	httpRestServer.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};
void sendCrossOriginHeader(){
	DEBUG_PRINTLN(F("sendCORSHeader"));

	httpRestServer.sendHeader(F("access-control-allow-credentials"), F("false"));

	setCrossOrigin();

	httpRestServer.send(204);
}


void getConfigurationJSON(Configuration configuration){

	DynamicJsonDocument doc(2048);
	JsonObject rootObj = doc.to<JsonObject>();

	JsonObject config = rootObj.createNestedObject("configuration");

	config[F("ADDH")] = configuration.ADDH;
	config[F("ADDL")] = configuration.ADDL;
	config[F("CHAN")] = configuration.CHAN;

	JsonObject option = config.createNestedObject("OPTION");

	option[F("fec")] = configuration.OPTION.fec;
	option[F("fixedTransmission")] = configuration.OPTION.fixedTransmission;
	option[F("ioDriveMode")] = configuration.OPTION.ioDriveMode;
	option[F("transmissionPower")] = configuration.OPTION.transmissionPower;
	option[F("wirelessWakeupTime")] = configuration.OPTION.wirelessWakeupTime;

	JsonObject speed = config.createNestedObject("SPED");

	speed[F("airDataRate")] = configuration.SPED.airDataRate;
	speed[F("uartBaudRate")] = configuration.SPED.uartBaudRate;
	speed[F("uartParity")] = configuration.SPED.uartParity;

	DEBUG_PRINT(F("Stream file..."));
	String buf;
	serializeJson(rootObj, buf);
	httpRestServer.send(200, F("application/json"), buf);
	DEBUG_PRINTLN(F("done."));
}

void getConfiguration() {
	DEBUG_PRINTLN(F("------------------------------ getConfiguration ------------------------------"));

	setCrossOrigin();

	ResponseStructContainer c;
	c = e32ttl.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	DEBUG_PRINTLN(c.status.getResponseDescription());
	DEBUG_PRINTLN(c.status.code);

	printParameters(configuration);

	getConfigurationJSON(configuration);

	c.close();
}
void getModuleInfo() {
	DEBUG_PRINTLN(F("------------------------------ getModuleInfo ------------------------------"));

	setCrossOrigin();

	ResponseStructContainer c;
	c = e32ttl.getModuleInformation();
	// It's important get configuration pointer before all other operation
	ModuleInformation moduleInformation = *(ModuleInformation*) c.data;
	DEBUG_PRINTLN(c.status.getResponseDescription());
	DEBUG_PRINTLN(c.status.code);

	printModuleInformation(moduleInformation);

	DynamicJsonDocument doc(2048);
	JsonObject rootObj = doc.to<JsonObject>();

	JsonObject config = rootObj.createNestedObject("moduleInfo");

	config[F("frequency")] = String(moduleInformation.frequency, HEX);
	config[F("version")] = String(moduleInformation.version, HEX);
	config[F("features")] = String(moduleInformation.features, HEX);

	DEBUG_PRINT(F("Stream file..."));
	String buf;
	serializeJson(rootObj, buf);
	httpRestServer.send(200, F("application/json"), buf);
	DEBUG_PRINTLN(F("done."));

	c.close();
}

void resetMicrocontroller() {
#ifdef RESET_PIN
	digitalWrite(RESET_PIN, LOW);
#endif
}

void resetModule() {
	DEBUG_PRINTLN(F("------------------------------ resetModule ------------------------------"));

	setCrossOrigin();

	ResponseStatus rs;
	rs = e32ttl.resetModule();
	// It's important get configuration pointer before all other operation
	DEBUG_PRINTLN(rs.getResponseDescription());
	DEBUG_PRINTLN(rs.code);

	DEBUG_PRINT(F("==> reset END"));

	DynamicJsonDocument doc(512);

	if (rs.code != E32_SUCCESS) {
	    DEBUG_PRINTLN(F("fail."));
	    httpRestServer.send(400, F("text/html"), String(rs.getResponseDescription()));
	}else{
		JsonObject rootObj = doc.to<JsonObject>();

		JsonObject status = rootObj.createNestedObject("status");

		status[F("code")] = String(rs.code);
		status[F("error")] = rs.code!=1;
		status[F("description")] = rs.getResponseDescription();

		DEBUG_PRINT(F("Stream file..."));
		String buf;
		serializeJson(rootObj, buf);
		if (rs.code!=1){
			httpRestServer.send(500, F("application/json"), buf);
		}else{
			httpRestServer.send(200, F("application/json"), buf);
		}
		DEBUG_PRINTLN(F("done."));

	}
}

void postConfiguration() {
	DEBUG_PRINTLN(F("postConfigFile"));

	setCrossOrigin();

    String postBody = httpRestServer.arg("plain");
    DEBUG_PRINTLN(postBody);

	DynamicJsonDocument doc(2048);
	DeserializationError error = deserializeJson(doc, postBody);
	if (error) {
		// if the file didn't open, print an error:
		DEBUG_PRINT(F("Error parsing JSON "));
		DEBUG_PRINTLN(error.c_str());

		String msg = error.c_str();

		httpRestServer.send(400, F("text/html"), "Error in parsin json body! <br>"+msg);

	}else{
		JsonObject postObj = doc.as<JsonObject>();

		DEBUG_PRINT(F("HTTP Method: "));
		DEBUG_PRINTLN(httpRestServer.method());

        if (httpRestServer.method() == HTTP_POST) {
            if (postObj.containsKey("ADDH") && postObj.containsKey("ADDL") && postObj.containsKey("CHAN") && postObj.containsKey("OPTION")&& postObj.containsKey("SPED") ) {

            	DEBUG_PRINT(F("==> Set config file..."));

//            	JsonObject config = postObj[F("configuration")];

            	DEBUG_PRINT("ADDH");
            	DEBUG_PRINTLN((byte)postObj[F("ADDH")]);


            	Configuration configuration;
            	configuration.ADDH = (byte)postObj[F("ADDH")];
            	configuration.ADDL = (byte)postObj[F("ADDL")];
            	configuration.CHAN = (byte)postObj[F("CHAN")];

				JsonObject option = postObj[F("OPTION")];

            	DEBUG_PRINT("fec");
            	DEBUG_PRINTLN( (FORWARD_ERROR_CORRECTION_SWITCH)(byte)option[F("fec")] );

//				byte optionByte = 0b00000000;
//				bitWrite(bitRead((byte)option[F("fec")], 0))

				configuration.OPTION.fec = (FORWARD_ERROR_CORRECTION_SWITCH)(byte)option[F("fec")];
				configuration.OPTION.fixedTransmission = (FIDEX_TRANSMISSION)(byte)option[F("fixedTransmission")];
				configuration.OPTION.ioDriveMode = (IO_DRIVE_MODE)(byte)option[F("ioDriveMode")];
				configuration.OPTION.transmissionPower = (TRANSMISSION_POWER)(byte)option[F("transmissionPower")];
				configuration.OPTION.wirelessWakeupTime = (WIRELESS_WAKE_UP_TIME)(byte)option[F("wirelessWakeupTime")];

				JsonObject speed = postObj[F("SPED")];

				configuration.SPED.airDataRate = (AIR_DATA_RATE)(byte)speed[F("airDataRate")];
				configuration.SPED.uartBaudRate = (UART_BPS_TYPE)(byte)speed[F("uartBaudRate")];
				configuration.SPED.uartParity = (UART_PARITY)(byte)speed[F("uartParity")];




//            	configuration.ADDH = atoi(config[F("ADDH")]);
//            	configuration.ADDL = atoi(config[F("ADDL")]);
//            	configuration.CHAN = atoi(config[F("CHAN")]);
//
//				JsonObject option = config[F("OPTION")];
//
//				configuration.OPTION.fec = atoi(option[F("fec")]);
//				configuration.OPTION.fixedTransmission = atoi(option[F("fixedTransmission")]);
//				configuration.OPTION.ioDriveMode = atoi(option[F("ioDriveMode")]);
//				configuration.OPTION.transmissionPower = atoi(option[F("transmissionPower")]);
//				configuration.OPTION.wirelessWakeupTime = atoi(option[F("wirelessWakeupTime")]);
//
//				JsonObject speed = config[F("SPED")];
//
//				configuration.SPED.airDataRate = atoi(speed[F("airDataRate")]);
//				configuration.SPED.uartBaudRate = atoi(speed[F("uartBaudRate")]);
//				configuration.SPED.uartParity = atoi(speed[F("uartParity")]);




				printParameters(configuration);

				ResponseStatus rs = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
				DEBUG_PRINTLN(rs.getResponseDescription());
				DEBUG_PRINTLN(rs.code);
				printParameters(configuration);
            	DEBUG_PRINT(F("==> Set config file END"));


            	if (rs.code != E32_SUCCESS) {
            	    DEBUG_PRINTLN(F("fail."));
            	    httpRestServer.send(400, F("text/html"), String(rs.getResponseDescription()));
            	}else{


//            		DEBUG_PRINT(F("Stream file..."));
//            		String buf;
//            		serializeJson(rootObj, buf);
//            		httpRestServer.send(200, F("application/json"), buf);
//            		DEBUG_PRINTLN(F("done."));
            		ResponseStructContainer c;
            		c = e32ttl.getConfiguration();
            		// It's important get configuration pointer before all other operation
            		Configuration configurationRetrieve = *(Configuration*) c.data;
            		DEBUG_PRINTLN(c.status.getResponseDescription());
            		DEBUG_PRINTLN(c.status.code);

            		printParameters(configuration);

            		if (
            				configuration.ADDH==configurationRetrieve.ADDH &&
            				configuration.ADDL==configurationRetrieve.ADDL &&
            				configuration.CHAN==configurationRetrieve.CHAN ){

                		getConfigurationJSON(configurationRetrieve);

                		delay(1000);

                		resetMicrocontroller();
					}else{
						httpRestServer.send(500, F("text/html"), F("Problem on store data on device, try to reset!"));
					}

            		c.close();
            	}
            }
            else {
            	DEBUG_PRINTLN("Configuration not present")
            	httpRestServer.send(204, F("text/html"), F("No data found, or incorrect!"));
            }
        }
    }
}

void sendTransparentMessage() {
	DEBUG_PRINTLN(F("sendTransparentMessage"));

	setCrossOrigin();

    String postBody = httpRestServer.arg("plain");
    DEBUG_PRINTLN(postBody);

	DynamicJsonDocument doc(2048);
	DeserializationError error = deserializeJson(doc, postBody);
	if (error) {
		// if the file didn't open, print an error:
		DEBUG_PRINT(F("Error parsing JSON "));
		DEBUG_PRINTLN(error.c_str());

		String msg = error.c_str();

		httpRestServer.send(400, F("text/html"), "Error in parsin json body! <br>"+msg);

	}else{
		JsonObject postObj = doc.as<JsonObject>();

		DEBUG_PRINT(F("HTTP Method: "));
		DEBUG_PRINTLN(httpRestServer.method());

        if (httpRestServer.method() == HTTP_POST) {
            if (postObj.containsKey("message") ) {

            	DEBUG_PRINT(F("==> readMessage..."));

//            	JsonObject config = postObj[F("configuration")];

            	String message = postObj[F("message")];

				ResponseStatus rs = e32ttl.sendMessage(message+'\0');
				DEBUG_PRINTLN(rs.getResponseDescription());
				DEBUG_PRINTLN(rs.code);
            	DEBUG_PRINT(F("==> Send message END"));


            	if (rs.code != E32_SUCCESS) {
            	    DEBUG_PRINTLN(F("fail."));
            	    httpRestServer.send(400, F("text/html"), String(rs.getResponseDescription()));
            	}else{
            		JsonObject rootObj = doc.to<JsonObject>();

            		JsonObject status = rootObj.createNestedObject("status");

            		status[F("code")] = String(rs.code);
            		status[F("error")] = rs.code!=1;
            		status[F("description")] = rs.getResponseDescription();

            		DEBUG_PRINT(F("Stream file..."));
            		String buf;
            		serializeJson(rootObj, buf);
            		if (rs.code!=1){
            			httpRestServer.send(500, F("application/json"), buf);
            		}else{
            			httpRestServer.send(200, F("application/json"), buf);
            		}
            		DEBUG_PRINTLN(F("done."));

            	}
            }
            else {
            	DEBUG_PRINTLN("Configuration not present")
            	httpRestServer.send(204, F("text/html"), F("No data found, or incorrect!"));
            }
        }
    }
}

void sendFixedMessage() {
	DEBUG_PRINTLN(F("sendFixedMessage"));

	setCrossOrigin();

    String postBody = httpRestServer.arg("plain");
    DEBUG_PRINTLN(postBody);

	DynamicJsonDocument doc(2048);
	DeserializationError error = deserializeJson(doc, postBody);
	if (error) {
		// if the file didn't open, print an error:
		DEBUG_PRINT(F("Error parsing JSON "));
		DEBUG_PRINTLN(error.c_str());

		String msg = error.c_str();

		httpRestServer.send(400, F("text/html"), "Error in parsin json body! <br>"+msg);

	}else{
		JsonObject postObj = doc.as<JsonObject>();

		DEBUG_PRINT(F("HTTP Method: "));
		DEBUG_PRINTLN(httpRestServer.method());

        if (httpRestServer.method() == HTTP_POST) {
            if (postObj.containsKey("message") && postObj.containsKey("CHAN") && postObj.containsKey("ADDL") && postObj.containsKey("ADDH")) {

            	DEBUG_PRINT(F("==> readMessage..."));

//            	JsonObject config = postObj[F("configuration")];

            	String message = postObj[F("message")];
            	byte CHAN = (byte)postObj[F("CHAN")];
            	byte ADDL = (byte)postObj[F("ADDL")];
            	byte ADDH = (byte)postObj[F("ADDH")];

				ResponseStatus rs = e32ttl.sendFixedMessage(ADDH, ADDL, CHAN, message+'\0');
				DEBUG_PRINTLN(rs.getResponseDescription());
				DEBUG_PRINTLN(rs.code);
            	DEBUG_PRINT(F("==> Send message END"));


            	if (rs.code != E32_SUCCESS) {
            	    DEBUG_PRINTLN(F("fail."));
            	    httpRestServer.send(400, F("text/html"), String(rs.getResponseDescription()));
            	}else{
            		JsonObject rootObj = doc.to<JsonObject>();

            		JsonObject status = rootObj.createNestedObject("status");

            		status[F("code")] = String(rs.code);
            		status[F("error")] = rs.code!=1;
            		status[F("description")] = rs.getResponseDescription();

            		DEBUG_PRINT(F("Stream file..."));
            		String buf;
            		serializeJson(rootObj, buf);
            		if (rs.code!=1){
            			httpRestServer.send(500, F("application/json"), buf);
            		}else{
            			httpRestServer.send(200, F("application/json"), buf);
            		}
            		DEBUG_PRINTLN(F("done."));

            	}
            }
            else {
            	DEBUG_PRINTLN("Configuration not present")
            	httpRestServer.send(204, F("text/html"), F("No data found, or incorrect!"));
            }
        }
    }
}

void sendBroadcastMessage() {
	DEBUG_PRINTLN(F("sendBroadcastMessage"));

	setCrossOrigin();

    String postBody = httpRestServer.arg("plain");
    DEBUG_PRINTLN(postBody);

	DynamicJsonDocument doc(2048);
	DeserializationError error = deserializeJson(doc, postBody);
	if (error) {
		// if the file didn't open, print an error:
		DEBUG_PRINT(F("Error parsing JSON "));
		DEBUG_PRINTLN(error.c_str());

		String msg = error.c_str();

		httpRestServer.send(400, F("text/html"), "Error in parsin json body! <br>"+msg);

	}else{
		JsonObject postObj = doc.as<JsonObject>();

		DEBUG_PRINT(F("HTTP Method: "));
		DEBUG_PRINTLN(httpRestServer.method());

        if (httpRestServer.method() == HTTP_POST) {
            if (postObj.containsKey("message") && postObj.containsKey("CHAN")) {

            	DEBUG_PRINT(F("==> readMessage..."));

//            	JsonObject config = postObj[F("configuration")];

            	String message = postObj[F("message")];
            	byte CHAN = (byte)postObj[F("CHAN")];

				ResponseStatus rs = e32ttl.sendBroadcastFixedMessage(CHAN, message+'\0');
				DEBUG_PRINTLN(rs.getResponseDescription());
				DEBUG_PRINTLN(rs.code);
            	DEBUG_PRINT(F("==> Send message END"));


            	if (rs.code != E32_SUCCESS) {
            	    DEBUG_PRINTLN(F("fail."));
            	    httpRestServer.send(400, F("text/html"), String(rs.getResponseDescription()));
            	}else{
            		JsonObject rootObj = doc.to<JsonObject>();

            		JsonObject status = rootObj.createNestedObject("status");

            		status[F("code")] = String(rs.code);
            		status[F("error")] = rs.code!=1;
            		status[F("description")] = rs.getResponseDescription();

            		DEBUG_PRINT(F("Stream file..."));
            		String buf;
            		serializeJson(rootObj, buf);
            		if (rs.code!=1){
            			httpRestServer.send(500, F("application/json"), buf);
            		}else{
            			httpRestServer.send(200, F("application/json"), buf);
            		}
            		DEBUG_PRINTLN(F("done."));

            	}
            }
            else {
            	DEBUG_PRINTLN("Configuration not present")
            	httpRestServer.send(204, F("text/html"), F("No data found, or incorrect!"));
            }
        }
    }
}

void printParameters(struct Configuration configuration) {
	DEBUG_PRINTLN("----------------------------------------");

	DEBUG_PRINT(F("HEAD BIN: "));  DEBUG_PRINT(configuration.HEAD, BIN);DEBUG_PRINT(" ");DEBUG_PRINT(configuration.HEAD, DEC);DEBUG_PRINT(" ");DEBUG_PRINTLN(configuration.HEAD, HEX);
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("AddH BIN: "));  DEBUG_PRINTLN(configuration.ADDH, DEC);
	DEBUG_PRINT(F("AddL BIN: "));  DEBUG_PRINTLN(configuration.ADDL, DEC);
	DEBUG_PRINT(F("Chan BIN: "));  DEBUG_PRINT(configuration.CHAN, DEC); DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.getChannelDescription());
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("SpeedParityBit BIN    : "));  DEBUG_PRINT(configuration.SPED.uartParity, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getUARTParityDescription());
	DEBUG_PRINT(F("SpeedUARTDataRate BIN : "));  DEBUG_PRINT(configuration.SPED.uartBaudRate, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getUARTBaudRate());
	DEBUG_PRINT(F("SpeedAirDataRate BIN  : "));  DEBUG_PRINT(configuration.SPED.airDataRate, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getAirDataRate());

	DEBUG_PRINT(F("OptionTrans BIN       : "));  DEBUG_PRINT(configuration.OPTION.fixedTransmission, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getFixedTransmissionDescription());
	DEBUG_PRINT(F("OptionPullup BIN      : "));  DEBUG_PRINT(configuration.OPTION.ioDriveMode, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getIODroveModeDescription());
	DEBUG_PRINT(F("OptionWakeup BIN      : "));  DEBUG_PRINT(configuration.OPTION.wirelessWakeupTime, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getWirelessWakeUPTimeDescription());
	DEBUG_PRINT(F("OptionFEC BIN         : "));  DEBUG_PRINT(configuration.OPTION.fec, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getFECDescription());
	DEBUG_PRINT(F("OptionPower BIN       : "));  DEBUG_PRINT(configuration.OPTION.transmissionPower, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getTransmissionPowerDescription());

	DEBUG_PRINTLN("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
	DEBUG_PRINTLN("----------------------------------------");
	DEBUG_PRINT(F("HEAD BIN: "));  DEBUG_PRINT(moduleInformation.HEAD, BIN);DEBUG_PRINT(" ");DEBUG_PRINT(moduleInformation.HEAD, DEC);DEBUG_PRINT(" ");DEBUG_PRINTLN(moduleInformation.HEAD, HEX);

	DEBUG_PRINT(F("Frequency.: "));  DEBUG_PRINTLN(moduleInformation.frequency, HEX);
	DEBUG_PRINT(F("Version  : "));  DEBUG_PRINTLN(moduleInformation.version, HEX);
	DEBUG_PRINT(F("Features : "));  DEBUG_PRINTLN(moduleInformation.features, HEX);
	DEBUG_PRINTLN("----------------------------------------");
}

void sendWSMessageOfMessageReceived(bool readSingleMessage){
	DynamicJsonDocument docws(512);
	JsonObject objws = docws.to<JsonObject>();

		ResponseContainer rs;
		if (readSingleMessage){
			DEBUG_PRINTLN("READ MESSAGE UNTIL!");
			rs = e32ttl.receiveMessageUntil();
		}else{
			DEBUG_PRINTLN("READ MESSAGE!");
			rs = e32ttl.receiveMessage();
		}


		objws[F("type")] = "message";
		objws[F("code")] = rs.status.code;
		objws[F("description")] = rs.status.getResponseDescription();

		DEBUG_PRINTLN(rs.status.getResponseDescription());

		if (rs.status.code==1){
			String message = rs.data;

			objws[F("message")] = message;
			DEBUG_PRINTLN(message);

			objws[F("error")] = false;
		}else{
			objws[F("error")] = true;
		}

	String buf;
	serializeJson(objws, buf);

	webSocket.broadcastTXT(buf);

}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	DynamicJsonDocument docwsT(512);
	DeserializationError error;

    switch(type) {
        case WStype_DISCONNECTED:
			webSocket.sendTXT(num, "{\"connection\": false}");

            DEBUG_PRINT(F(" Disconnected "));
            DEBUG_PRINTLN(num, DEC);

            isConnectedWebSocket = false;
            isConnectedWebSocketAck = false;
//            DEBUG_PRINTF_AI(F("[%u] Disconnected!\n"), num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
//                DEBUG_PRINTF_AI(F("[%u] Connected from %d.%d.%d.%d url: %s\n"), num, ip[0], ip[1], ip[2], ip[3], payload);

                DEBUG_PRINT(num);
                DEBUG_PRINT(F("Connected from: "));
                DEBUG_PRINT(ip.toString());
                DEBUG_PRINT(F(" "));
                DEBUG_PRINTLN((char*)payload);

				// send message to client
				webSocket.sendTXT(num, "{\"type\":\"connection\", \"connection\": true, \"simpleMessage\": "+String(readSingleMessage?"true":"false")+"}");

				isConnectedWebSocket = true;

            }
            break;
        case WStype_TEXT:
        	DEBUG_PRINT("NUM -> ");DEBUG_PRINT(num);
        	DEBUG_PRINT("payload -> ");DEBUG_PRINTLN((char*)payload);

        	error = deserializeJson(docwsT, (char*)payload);

        	if (error) {
        		// if the file didn't open, print an error:
        		DEBUG_PRINT(F("Error parsing JSON "));
        		webSocket.broadcastTXT("Error on WS");
        	}else{
        		JsonObject postObjws = docwsT.as<JsonObject>();

        		bool startReceiveDevMsg = postObjws[F("startReceiveDevMsg")];
        		if (startReceiveDevMsg==true){
//					readSingleMessage = postObjws[F("singleMessage")];
					isConnectedWebSocketAck  = true;
					DEBUG_PRINT(F("Start listening messages SM -> "));
					DEBUG_PRINTLN(isConnectedWebSocketAck);

	        		webSocket.broadcastTXT("{\"type\": \"device_msg\", \"receiving\": true}");
        		}else if (startReceiveDevMsg==false){
					isConnectedWebSocketAck  = false;
					DEBUG_PRINT(F("Start listening messages SM -> "));
					DEBUG_PRINTLN(isConnectedWebSocketAck);

					webSocket.broadcastTXT("{\"type\": \"device_msg\", \"receiving\": false}");
        		}

				bool singleMessage = postObjws[F("singleMessage")];
				DEBUG_PRINT(F("Single message -> "));
				DEBUG_PRINTLN(singleMessage);
        		if (singleMessage){
					readSingleMessage = singleMessage;
        		}
        	}

//        	DEBUG_PRINTF_AI(F("[%u] get Text: %s\n"), num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
////        	DEBUG_PRINTF_AI(F("[%u] get binary length: %u\n"), num, length);
//            hexdump(payload, length);
//
//            // send message to client
//            // webSocket.sendBIN(num, payload, length);
//            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        case WStype_PING:
        case WStype_PONG:

//        	DEBUG_PRINTF_AI(F("[%u] get binary length: %u\n"), num, length);
        	DEBUG_PRINT(F("WS : "))
        	DEBUG_PRINT(type)
        	DEBUG_PRINT(F(" - "))
			DEBUG_PRINTLN((char*)payload);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}

String getContentType(String filename){
  if(filename.endsWith(F(".htm"))) 		return F("text/html");
  else if(filename.endsWith(F(".html"))) 	return F("text/html");
  else if(filename.endsWith(F(".css"))) 	return F("text/css");
  else if(filename.endsWith(F(".js"))) 	return F("application/javascript");
  else if(filename.endsWith(F(".json"))) 	return F("application/json");
  else if(filename.endsWith(F(".png"))) 	return F("image/png");
  else if(filename.endsWith(F(".gif"))) 	return F("image/gif");
  else if(filename.endsWith(F(".jpg"))) 	return F("image/jpeg");
  else if(filename.endsWith(F(".ico"))) 	return F("image/x-icon");
  else if(filename.endsWith(F(".xml"))) 	return F("text/xml");
  else if(filename.endsWith(F(".pdf"))) 	return F("application/x-pdf");
  else if(filename.endsWith(F(".zip"))) 	return F("application/x-zip");
  else if(filename.endsWith(F(".gz"))) 	return F("application/x-gzip");
  return F("text/plain");
}

bool handleFileRead(String path){  // send the right file to the client (if it exists)
  DEBUG_PRINT(F("handleFileRead: "));
  DEBUG_PRINTLN(path);

  // Capite portal
 if (
		 path.endsWith("/generate_204") ||  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
		 path.endsWith("/fwlink")  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
 ){
	 path = "/";
 }

  if(path.endsWith("/")) path += F("index.html");           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + F(".gz");

  DEBUG_PRINT(F("Path exist : "));
  DEBUG_PRINTLN(path);

  if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(LittleFS.exists(pathWithGz))                          // If there's a compressed version available
      path += F(".gz");                                         // Use the compressed version
    fs::File file = LittleFS.open(path, "r");                    // Open the file
    size_t sent = httpServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    DEBUG_PRINTLN(String(F("\tSent file: ")) + path + String(F(" of size ")) + sent);
    return true;
  }
  DEBUG_PRINTLN(String(F("\tFile Not Found: ")) + path);
  return false;                                          // If the file doesn't exist, return false
}

void serverRouting() {
	  httpServer.onNotFound([]() {                              // If the client requests any URI
		  DEBUG_PRINTLN(F("On not found"));
	    if (!handleFileRead(httpServer.uri())){                  // send it if it exists
	    	DEBUG_PRINTLN(F("Not found"));
	    	httpServer.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
	    }
	  });

	  DEBUG_PRINTLN(F("Set cache!"));

	  httpServer.serveStatic("/settings.json", LittleFS, "/settings.json","no-cache, no-store, must-revalidate");
	  httpServer.serveStatic("/", LittleFS, "/","max-age=31536000");
}
void sendSimpleWSMessage(String type, String val){
	DynamicJsonDocument docws(512);
	JsonObject objws = docws.to<JsonObject>();

	objws["type"] = type;

	objws["value"] = val;

	String buf;
	serializeJson(objws, buf);

	webSocket.broadcastTXT(buf);
}

void realtimeDataCallbak() {
	String type = F("wifi_rt");
	sendSimpleWSMessage(type, String(WiFi.RSSI()));
}
