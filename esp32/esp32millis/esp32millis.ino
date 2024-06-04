#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Prosjekt.h"

/// VARIABLES FOR WIFI-CONNECTION ///

const char* ssid = "Garfield party";
const char* password = "Lasagnalover6969";

/// VARIABLER FOR MESSAGE VIA WHATSAPP 
String phoneNumber = "+4748230543";
String apiKey = "4833002";
String message = "Du har nå brukt 90% av kvoten din!"; 

/// VARIABLES FOR MQTT COMMUNICATION ///
const char* mqtt_server = "192.168.0.144";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

const char* messageTopic = "bil1";

String MQTTmessage;

/// VARIABLES FOR SENSOR READING ///

#define ONE_WIRE_BUS 32
const int tempPin = 32; // connect the temperature sensor to pin 32
const int pressurePin = 33; //connnect the pressure sensor to pin 33
const int numReadings = 6; // amount of datapoints
int temperature[numReadings]; //creates array for temperature readings
int pressure[numReadings]; //creates array for pressure readings
int averageTemp;
int averagePressure;
int totalTemp;
int totalPressure;

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire device
DallasTemperature sensors(&oneWire); // Pass oneWire reference to DallasTemperature library

/// VARIABLES FOR I2C-COMMUNICATION ///
byte zumoaddress = 4;
byte espaddress = 8; 
byte drivingmode;

/// VARIABLES FOR GPS-MODULE ///
#define RXD2 16 //RX pin
#define TXD2 17 //TX pin

#define GPSSerial Serial2
Adafruit_GPS GPS(&GPSSerial); // Connect to the GPS on the hardware port
#define GPSECHO false

float latitude[numReadings];
float longitude[numReadings]; 
int altitude[numReadings];
int speed[numReadings];
float averageLatitude; 
float averageLongitude; 
int averageAltitude; 
int averageSpeed; 
float totalLatitude;
float totalLongitude; 
float totalAltitude; 
float totalSpeed; 

/// VARIABLES FOR DELAY WITH MILLIS ///

unsigned long previousMillis = 0;
const int interval = 5000;

/// VARIABLES FOR POWER USAGE CALCULATION ///

float basisPower = 17; 
float powerUsage = 0; 

int highAccelerationCount; 
float meterIncrease; 
float meterDecrease; 
float previousAltitude; 
bool heightCheck = false; 
int sharpTurnCount; 

// VARIABLES FOR VELOCITY AND ACCELERATION CALCULATION //

float previousVelocity = 0; 
float acceleration;

void setup()
{
  Serial.begin(115200);
  Wire.begin(espaddress); //sets the ESP32 address for I2C communication
  Wire.onReceive(receiveEvent); // sets the function for when the ESP32 receives an event
  Wire.onRequest(requestEvent); // sets the function for then the Zumo32u4 requests an event 
  connectWiFi(ssid, password); //connects to WiFi
  client.setServer(mqtt_server, 1883); // sets the raspberry pi as the MQTT server
  client.setCallback(callback); // sets the callback function for MQTT communication
  delay(1000); // a short delay before the main loop executes
}

void loop()
{
  
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
    // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  if(millis() - previousMillis >= interval){ 
    previousMillis = millis();
    readSensorAverage();
    if (GPS.fix) {
      readGPSAverage();
      averageAcceleration(); 
      calculateMeterIncreaseAndDecrease();
      calculatePowerUsage();
    }
    sendMQTTMessage(); //if the GPS is not working, only the other sensors will be read and sent via MQTT. 
    //If everything is working, everything will be sent. This is to avoid potential loss of data should the GPS lose its fix. 
  }

  if (!client.connected()) {
    reconnectMQTT(client); //reconnects if the connection is lost 
  }
  client.loop();
}

//This function takes in values received from the MQTT connection 
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
  if (String(topic) == "esp32/output") {
    if(messageTemp == "left"){
      drivingmode = 1; 
    }
    else if(messageTemp == "right"){
      drivingmode = 2; 
    }
    else if(messageTemp == "forwards"){
      drivingmode = 3; 
    }
    else if(messageTemp == "backwards"){
      drivingmode = 4; 
    }
    else if(messageTemp == "auto"){
      drivingmode = 5; 
    }
  if (String(topic) == "warning") {
    sendWhatsAppMessage(message, phoneNumber, apiKey); //sends a warning message
  }
  Serial.print(drivingmode);
}

//This function finds the average latitude, longitude, altitude and speed
void readGPSAverage(){

  for (int i = 0; i < numReadings; i++) {
    latitude[i] = GPS.latitude; 
    longitude[i] = GPS.longitude;
    altitude[i] = GPS.altitude; 
    speed[i] = GPS.speed * 51.444; //converts from knots of speed to cm/s
    totalLatitude += latitude[i]; 
    totalLongitude += longitude[i];
    totalAltitude += altitude[i]; 
    totalSpeed += speed[i];
  }
  float averageLatitude = totalLatitude/numReadings; 
  float averageLongitude = totalLongitude/numReadings; 
  int averageAltitude = totalAltitude/numReadings; 
  int averageSpeed = totalSpeed/numReadings; 
  totalLongitude = 0;
  totalAltitude = 0; 
  totalSpeed = 0;
}

//A function that calculates the average value for the pressure and temperature sensors.
void readSensorAverage(){

  for (int i = 0; i < numReadings; i++) {
    temperature[i] = sensors.getTempCByIndex(0);  
    pressure[i] = map(analogRead(pressurePin), 0, 4095, 0, 1000); //maps the value from 0 - 1 tons of weight
    totalTemp += temperature[i];
    totalPressure += pressure[i]; 
  }
  Serial.println(averageTemp);
  Serial.println(averagePressure);
  totalTemp = 0; //resets total value of temperature and pressure  
  totalPressure = 0;
}

//This function calculates the average acceleration
void averageAcceleration(){
 float velocity = GPS.speed * 51.444; //converts speed from knots to cm/s. 
 acceleration = (velocity - previousVelocity)/interval; 
 float previousVelocity = velocity; 
}

//This function counts the amount of sharp turns made by the Zumo32u4
void receiveEvent(int howMany){
  while(Wire.available()){
    sharpTurnCount += Wire.read();
  }
}

void requestEvent(){
  Wire.write(drivingmode); //if the Zumo32u4 requests a byte, the ESP32 will send it. 
}

//This function calculates the altitutude increase or decrease 
void calculateMeterIncreaseAndDecrease(){
    if(averageAltitude - previousAltitude < 0){
      meterDecrease = abs(averageAltitude - previousAltitude);
      previousAltitude = averageAltitude;
    }
    else if(averageAltitude - previousAltitude >= 0){
      meterIncrease = averageAltitude - previousAltitude;
      previousAltitude = averageAltitude; 
    }
}

//This function calculates the power usage
void calculatePowerUsage(){
  powerUsage += (basisPower * pow(1.1,0.5*(averageSpeed/100)-40));
  powerUsage += (pow((acceleration/100),3));
  powerUsage += ((-2*averageTemp + 0.05*pow(averageTemp,2) + 20)/0.15);
  powerUsage += (0.2*averagePressure);
  powerUsage += ((averageSpeed * meterIncrease) / 4 - (averageSpeed * meterDecrease) / 8);
}

//This function sends a message with all the values via MQTT 
void sendMQTTMessage(){

  String MQTTmessage = String(powerUsage) + " " + String(averageSpeed) + " " + String(meterIncrease) + " " + String(meterDecrease) + " " + String(averageTemp) + " " + String(averageLatitude) + " " + String(averageLongitude) + " " + String(highAccelerationCount) + " " + String(sharpTurnCount); 
  client.publish(messageTopic, MQTTmessage.c_str());
  int result = Wire.endTransmission();
  if(result == 0){
    Serial.println("transmission sucessfull");
  }else{
    Serial.print("transmission failed with error code: ");
    Serial.println(result);
  }
  Serial.print(MQTTmessage);
}