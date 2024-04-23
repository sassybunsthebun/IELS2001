#include "Prosjekt.h"
#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>

/*
* Connects to WPA2 secured WiFi network 
* Note: Blocks until WiFi has successfully connected
* @param const char* ssid: SSID of network to connect to 
* @param const char* password: Password of network to connect to 
*/
void connectWiFi(const char* ssid, const char* password) { // connect and wait for wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

/* 
* Sends WhatsAppMessage via CallMeBot
* @param String& message: the message being sent
* @param String& phoneNumber: phone number of the receiver (with country code)
* @param String apiKey: API key from CallMeBot
*/
void sendWhatsAppMessage(String& message, String& phoneNumber, String& apiKey) {
  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);
  //CallMeBot expects an URL-encoded form
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }
}

//void sendWarningMessage(String& message, String& phoneNumber, String& apiKey){
//  sendWhatsAppMessage("Warning! You have reached 80%!", phoneNumber, apiKey);
//}

/*
* A function for reconnecting to MQTT if MQTT connection fails
* Note: Blocks until MQTT has successfully connected
* @param PubSubClient& client: client for PubSubClient class
*/
void reconnectMQTT(PubSubClient& client) {
    while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
* Transmits the variable kjoremodus to Zumo32u4 via I2C communication using the wire-library.
* Note: This function must have a period of about 500 ms between each time it is called.
* @param byte zumoaddress: Adress of the Zumo32u4
* @param byte kjøremodus: Variable which stores the driving direction
*/
void wireTransmit(byte zumoaddress, byte kjoremodus) {
    Wire.beginTransmission(zumoaddress);
    Wire.write(kjoremodus);
    Serial.print("message sent!");
    Wire.endTransmission();
}