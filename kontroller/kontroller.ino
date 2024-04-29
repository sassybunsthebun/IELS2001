#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include "Prosjekt.h" 

/// VARIABLES FOR WIFI-CONNECTION ///

const char* ssid = "NTNU-IOT";
const char* password = "";

/// VARIABLES FOR MQTT COMMUNICATION ///
const char* mqtt_server = "10.25.17.47"; //IP-address of Raspberry Pi

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

/// VARIABLES FOR DELAY WITH MILLIS ///

uint32_t timer = millis();
const int interval = 500;

/// VARIABLES FOR JOYSTICK ///

const int LeftRightPin = 35; // L/R readings are read on pin 3
const int UpDownPin = 32; // U/D readings are read on pin 4
int MidPointBuffer = 200; // buffer for defining the midpoint on the joystick 
int LeftRightMidPoint = 0; 
int UpDownMidPoint = 0; 
String direction; 

/// VARIABLES FOR PUSHBUTTON ///

const int buttonPin = 25; 
int buttonState; 
int lastButtonState = 0; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50; 
int mode = 0; //changes which mode the controller is in. 1 = joystick, 0 = none

void setup() {
  Serial.begin(115200);
  LeftRightMidPoint = analogRead(LeftRightPin); 
  UpDownMidPoint = analogRead(UpDownPin); 
  connectWiFi(ssid, password);
  client.setServer(mqtt_server, 1883); 
  pinMode(buttonPin, INPUT);
}

void loop() {
  int reading = digitalRead(buttonPin); 

  if (reading != lastButtonState) {
    lastDebounceTime = millis(); 
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading; 
      if (buttonState == 1) {
        mode = !mode;
        Serial.println(mode); 
      }
    }
  }

  lastButtonState = reading;

  if (mode == 1){
    joyStickMode(); 
  }
  
  if (!client.connected()) {
    reconnectMQTT(client);
  }
  client.loop();
}

void joyStickMode() {
  if (millis() - timer > interval) {
    timer = millis(); 
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
  char directionString[10];
  direction.toCharArray(directionString, 10);
  client.publish("esp32/output", directionString); //converts the direction to a char array and publishes it to "esp32/output"
  Serial.println(direction); 
  }
}