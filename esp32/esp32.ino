#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Prosjekt.h" //vårt bibliotek for å rydde i koden :) nice 

/// VARIABLES FOR WIFI-CONNECTION ///

const char* ssid = "NTNU-IOT";
const char* password = "";

/// VARIABLER FOR  MELDING GJENNOM WHATSAPP 
String phoneNumber = "+4748230543";
String apiKey = "4833002";
String message = "HEI:)"; //denne kan endres til en mer utfyllende melding senere
/// VARIABLER FOR MQTT ///

/// VARIABLES FOR MQTT COMMUNICATION ///
const char* mqtt_server = "10.25.17.47";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

const char* messageTopic = "bil1";

/*
const char* mqttUsername = "mashii"
const char* mqttPassword = EtVanskeligPassord69"
const char* clientID = "mashii"
*/

String MQTTmessage;

/// VARIABLER FOR SENSORAVLESNING ///

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

/// VARIABLER FOR I2C-KOMMUNIKASJON ///
byte zumoaddress = 4;
byte espaddress = 8; 
byte kjoremodus = 0;

/// VARIABLER FOR GPS-MODUL ///
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
const int interval = 500;

/// VARIABLES FOR POWER USAGE CALCULATION ///

float basisPower = 17; 
float powerUsage = 0; 

int highAccelerationCount = 0; 
float meterIncrease = 0.0; 
float meterDecrease = 0.0; 
float previousAltitude = 0.0; 
bool heightCheck = false; 
int sharpTurnCount; 

// VARIABLES FOR VELOCITY AND ACCELERATION CALCULATION //

float previousVelocity = 0; 
float acceleration;

void setup()
{
  Serial.begin(115200);

  connectWiFi(ssid, password); //kobler opp til Wi-Fi

  sendWhatsAppMessage(message, phoneNumber, apiKey); // sender melding (denne funksjonen brukes da senere i programmet hvor man skal varsle brukeren) 

  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
  //pinMode(ledPin, OUTPUT); // for eksempelet i callback-funksjonen
  Wire.begin(espaddress); // join i2c bus (address optional for master) //
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready
 // Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  //Serial.println("Setup complete!");
  Wire.onReceive(receiveEvent); 
  delay(1000);
  previousMillis = millis();
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
    sendMQTTMessage(); 
  }
  
  if (!client.connected()) {
    reconnectMQTT(client);
  }
  client.loop();
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
  Serial.println(averageTemp); //insert function for sending information to esp32
  Serial.println(averagePressure); //insert function for sending information to esp32
  totalTemp = 0; //resets total value of temperature and pressure  
  totalPressure = 0;
}

//This function calculates the average acceleration
void averageAcceleration(){
 float velocity = GPS.speed * 51.444; //converts speed from knots to cm/s. 
 acceleration = (velocity - previousVelocity)/interval; 
 float previousVelocity = velocity; 
}

//THis function takes in values received from the MQTT connection 
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
    if(messageTemp == "left"){
      kjoremodus = 1; 
    }
    else if(messageTemp == "right"){
      kjoremodus = 2; 
    }
    else if(messageTemp == "forwards"){
      kjoremodus = 3; 
    }
    else if(messageTemp == "backwards"){
      kjoremodus = 4; 
    }
  } 

  Serial.print(kjoremodus);
    //wireTransmit(zumoaddress, kjoremodus); 
  Wire.beginTransmission(zumoaddress);
  Wire.write(kjoremodus);
  Wire.endTransmission();
  Serial.print("message sent!");
}

//This function counts the amount of sharp turns made by the zumo car
void receiveEvent(int howMany){
  while(Wire.available()){
    for(int i =0; i < howMany; i++){
      sharpTurnCount += Wire.read();
      Serial.println(sharpTurnCount);
    }
  }
}

//This function sends a message with all the values via MQTT
void sendMQTTMessage(){

  String MQTTmessage = String(powerUsage) + " " + String(averageSpeed) + " " + String(meterIncrease) + " " + String(meterDecrease) + " " + String(averageTemp) + " " + String(averageLatitude) + " " + String(averageLongitude) + " " + String(highAccelerationCount) + " " + String(sharpTurnCount); 
  client.publish(messageTopic, MQTTmessage.c_str());
  Serial.print(MQTTmessage);
}

//This function calculates the meter increase or decrease 
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