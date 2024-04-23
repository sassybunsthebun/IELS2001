#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include "Prosjekt.h" 

/// VARIABLES FOR WIFI-CONNECTION ///

const char* ssid = "Garfield party";
const char* password = "Lasagnalover6969";

/// VARIABLES FOR MQTT COMMUNICATION ///
const char* mqtt_server = "192.168.0.144";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

/// VARIABLES FOR DELAY WITH MILLIS ///

uint32_t timer = millis();
const int interval = 1000;

/// VARIABLES FOR JOYSTICK ///

const int LeftRightPin = 35; // L/R readings are read on pin 3
const int UpDownPin = 32; // U/D readings are read on pin 4
int MidPointBuffer = 300; 
int LeftRightMidPoint = 0; 
int UpDownMidPoint = 0; 
String direction; 

void setup() {
  Serial.begin(115200);
  LeftRightMidPoint = analogRead(LeftRightPin); 
  UpDownMidPoint = analogRead(UpDownPin); 
  connectWiFi(ssid, password); //kobler opp til Wi-Fi
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
}

void loop() {
  int LeftRight = analogRead(LeftRightPin);
  int UpDown = analogRead(UpDownPin); 
  if (UpDown < UpDownMidPoint - MidPointBuffer){
    direction = "backwards"; 
  }
  else if (UpDown > UpDownMidPoint + MidPointBuffer){
    direction = "forwards";
  }
  else if (LeftRight < LeftRightMidPoint - MidPointBuffer){
    direction = "left";
  }
  else if (LeftRight > LeftRightMidPoint + MidPointBuffer){
    direction = "right";
  }
  else {
    direction = "";
  }

  Serial.println(direction); 
  if (millis() - timer > interval) { //sender sensorverdier hvert femte sekund
    timer = millis(); // reset the timer
    // Convert the value to a char array
    char directionString[10];
    direction.toCharArray(directionString, 10);
    Serial.println(directionString);
    client.publish("esp32/output", directionString);
  }

  if (!client.connected()) {
    reconnectMQTT(client);
  }
  client.loop();

  delay(100);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT
  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  // I denne call-back funksjonen kan en lage et system for å endre koden på nettsida 
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
    }
  }
}
