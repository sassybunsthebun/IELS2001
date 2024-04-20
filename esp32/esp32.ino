#include <Wire.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include "Prosjekt.h" //vårt bibliotek for å rydde i koden :) nice 

/// VARIABLER FOR WIFI_TILKOBLING ///

const char* ssid = "Garfield party";
const char* password = "Lasagnalover6969";

/// VARIABLER FOR  MELDING GJENNOM WHATSAPP 
// +international_country_code + phone number
// Portugal +351, example: +351912345678
String phoneNumber = "+4748230543";
String apiKey = "4833002";
String message = "HEI:)"; //denne kan endres til en mer utfyllende melding senere
/// VARIABLER FOR MQTT ///

// Legg til IP-adressen til MQTT broker
const char* mqtt_server = "10.25.17.47";

WiFiClient espClient;
PubSubClient client = PubSubClient(espClient);
char msg[50];
int value = 0;

float temperature = 0;
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

/// VARIABLER FOR DELY MED MILLIS ///

uint32_t timer = millis();
const int interval = 5000;

void setup()
{
  Serial.begin(115200);
  connectWiFi(ssid, password); //kobler opp til Wi-Fi
  sendMessages(message, phoneNumber, apiKey); // sender melding (denne funksjonen brukes da senere i programmet hvor man skal varsle brukeren) 
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
  pinMode(ledPin, OUTPUT); // for eksempelet i callback-funksjonen
  Wire.begin(); // join i2c bus (address optional for master) //
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Setup complete!");
  delay(1000);
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
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void loop()
{
  char c = GPS.read(); //Leser GPS-data
  if (GPSECHO)
    if (c) Serial.print(c);
  if (GPS.newNMEAreceived()) {
    Serial.print(GPS.lastNMEA()); 
    if (!GPS.parse(GPS.lastNMEA())) 
      return; 
  }

  if (millis() - timer > interval) { //sender sensorverdier hvert femte sekund
    timer = millis(); // reset the timer
    
    wireTransmit(zumoaddress, kjoremodus); //sender informasjon til zumo bilen med variablen "kjoremodus" (man kan bruke lignende funksjonalitet for å sende data fra zumobilen til esp32.)
    temperature = random(1,10); // sett inn for andre sensorverdier 
    humidity = random(1,99);
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/output", tempString);
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/output", humString);

    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
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
