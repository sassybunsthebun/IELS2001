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
const char* mqtt_server = "10.25.17.47";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

/// VARIABLES FOR DELAY WITH MILLIS ///

uint32_t timer = millis();
const int interval = 200;

/// VARIABLES FOR JOYSTICK ///

const int LeftRightPin = 35; // L/R readings are read on pin 3
const int UpDownPin = 32; // U/D readings are read on pin 4
int MidPointBuffer = 200; 
int LeftRightMidPoint = 0; 
int UpDownMidPoint = 0; 
String direction; 

/// VARIABLES FOR PUSHBUTTON ///

const int buttonPin = 25; 
int buttonState; 
int lastButtonState = 0; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50; 
int modus = 0; 

void setup() {
  Serial.begin(115200);
  LeftRightMidPoint = analogRead(LeftRightPin); 
  UpDownMidPoint = analogRead(UpDownPin); 
  connectWiFi(ssid, password); //kobler opp til Wi-Fi
  client.setServer(mqtt_server, 1883); 
// client.setCallback(callback);
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
        modus = !modus;
        Serial.println(modus); 
      }
    }
  }

  lastButtonState = reading;

  if (modus == 1){
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
  
    Serial.println(direction); 

    char directionString[10];
    direction.toCharArray(directionString, 10);
    Serial.println(directionString);
    client.publish("esp32/output", directionString);
  }

}
/*void callback(char* topic, byte* message, unsigned int length) {
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
*/