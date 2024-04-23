#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include "Prosjekt.h" //vårt bibliotek for å rydde i koden :) nice 

/// VARIABLES FOR WIFI-CONNECTION ///

const char* ssid = "Garfield party";
const char* password = "Lasagnalover6969";

/// VARIABLER FOR  MELDING GJENNOM WHATSAPP 
String phoneNumber = "+4748230543";
String apiKey = "4833002";
String message = "HEI:)"; //denne kan endres til en mer utfyllende melding senere
/// VARIABLER FOR MQTT ///

/// VARIABLES FOR MQTT COMMUNICATION ///
const char* mqtt_server = "192.168.0.144";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

//modri

/// VARIABLER FOR SENSORAVLESNING ///

const int pressurePin = 33; //connnect the pressure sensor to pin 33
const int numReadings = 6; // amount of datapoints
int temperature[numReadings]; //creates array for temperature readings
int pressure[numReadings]; //creates arrray for pressure readings
int totalTemp;
int totalPressure;

float temperature2 = 2;
float humidity = 0;

// LED Pin
const int ledPin = 4;

/// VARIABLER FOR I2C-KOMMUNIKASJON ///
byte zumoaddress = 4;
byte kjoremodus = 0;

/// VARIABLER FOR GPS-MODUL ///
#define RXD2 16 //RX pin
#define TXD2 17 //TX pin

// what's the name of the hardware serial port?
#define GPSSerial Serial2

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

/// VARIABLES FOR DELAY WITH MILLIS ///

unsigned long previousMillis = 0;
const int interval = 5000;

void setup()
{
  Serial.begin(115200);
  connectWiFi(ssid, password); //kobler opp til Wi-Fi
  sendWhatsAppMessage(message, phoneNumber, apiKey); // sender melding (denne funksjonen brukes da senere i programmet hvor man skal varsle brukeren) 
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
  //pinMode(ledPin, OUTPUT); // for eksempelet i callback-funksjonen
  Wire.begin(); // join i2c bus (address optional for master) //
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready
 // Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  //Serial.println("Setup complete!");
  delay(1000);
  previousMillis = millis();
}

void loop()
{

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){ //reads average sensor value every 5 seconds
    previousMillis = currentMillis;
    readSensorAverage();
  }

  char c = GPS.read(); //Leser GPS-data
  if (GPSECHO)
    if (c) Serial.print(c);
  if (GPS.newNMEAreceived()) {
    Serial.print(GPS.lastNMEA()); 
    if (!GPS.parse(GPS.lastNMEA())) 
      return; 
  }

  if (millis() - previousMillis > interval) { //sender sensorverdier hvert femte sekund
    previousMillis = millis(); // reset the timer
    
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Antenna status: "); Serial.println((int)GPS.antenna);
    }
  }

  if (!client.connected()) {
    reconnectMQTT(client);
  }
  client.loop();
}

//A function that calculates the average value for the pressure and temperature sensors.
void readSensorAverage(){

  for (int i = 0; i < numReadings; i++) {
    temperature[i] = 1; //
    pressure[i] = analogRead(pressurepin);
    totalTemp += temperature[i];
    totalPressure += pressure[i];
  }
  int averageTemp = totalTemp/numReadings;
  int averagePressure = totalPressure/numReadings;
  char pressureString[8];
  averagePressure.toCharArray(pressureString, 8);
  client.publish("esp32/output", pressureString);
  Serial.println(averageTemp); //insert function for sending information to esp32
  Serial.println(averagePressure); //insert function for sending information to esp32
  totalTemp = 0; //resets total value of temperature and pressure  
  totalPressure = 0;
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
    if(messageTemp == "left"){
      kjoremodus = 1; 
      wireTransmit(zumoaddress, kjoremodus);
    }
    else if(messageTemp == "right"){
      kjoremodus = 2; 
      wireTransmit(zumoaddress, kjoremodus);
    }
    else if(messageTemp == "forwards"){
      kjoremodus = 3; 
      wireTransmit(zumoaddress, kjoremodus);
    }
    else if(messageTemp == "backwards"){
      kjoremodus = 4; 
      wireTransmit(zumoaddress, kjoremodus);
    }
  }
}
