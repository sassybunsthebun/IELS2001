#include <PubSubClient.h>
#include <WiFi.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GPS.h>
#include <Wire.h>

//--------mqtt--------
// WiFi
const char* ssid = "NTNU-IOT";                
const char* wifiPassword = "";

// MQTT
const char* mqttServer = "10.25.18.141"; 

//topics
const char* messageTopic = "message";

const char* mqttUsername = "mashii"; // MQTT username
const char* mqttPassword = "EtVanskeligPassord69"; // MQTT password
const char* clientID = "mashii"; // MQTT client ID

//------I2C------
int espAddress = 4;

//----zumo car----
int right = 3;
int left = 1;
int null = 2;

int sharpTurn[9];
int sharpTurnCount = 0;

//---------sensors--------
// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 32
const uint8_t tempPin = 32;
const uint8_t pressurePin = 33;

//datalists with 4 elements
float pressure[4];
float temperature[4];
float x[4];
float y[4];
float z[4];

//datalists with 9-10 elements
float aks[9];
float velocity[10];

//smooth value (average)
float tempSmoothValue = 0.0;
float pressureSmoothValue = 0.0;
float xSmoothValue = 0.0;
float ySmoothValue = 0.0;
float zSmoothValue = 0.0;
float aksSmoothValue = 0.0;
float velocitySmoothValue = 0.0;

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);	

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

//lists with 4 elements
uint8_t i = 0;

//lists with 9-10 elements
uint8_t j = 0;

//acceleration intervall
float milliSeconds = 1000.0;

//time to send data 10 seconds
//time to check 4 element lists
unsigned long longCheck = 3750;
unsigned long previousLongCheck = millis();

//time to check 9-10 element lists
unsigned long shortCheck = 1500;
unsigned long previousShortCheck = millis();

//when to send message (these are not used in the code, only used in testing)
unsigned long sendTime = 15000;
unsigned long previousSendTime = millis();

//variable for pressure
RTC_DS3231 rtc;

//Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;

// 1883 is the listener port for the Broker
PubSubClient client(mqttServer, 1883, wifiClient);

//----GPS related variables---
#define RXD2 16 //RX pin
#define TXD2 17 //TX pin

// what's the name of the hardware serial port?
#define GPSSerial Serial2

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
#define GPSECHO false

//power usage calculation
float basisPower = 17;
float powerUsage = 0;

//acceleration count
int storAksCount = 0;

//height variables
float meterAbove = 0.0;
float meterBelow = 0.0;
float previousMeter = 0.0;

bool heightCheck = false;

//array with all values
String stringArray[10];
String message = "";

//------functions------

void readPosition(){ 
  x[i] = GPS.latitude; 
  y[i] = GPS.longitude; 
  z[i] = GPS.altitude;
  Serial.print("height: ");
  Serial.println(z[i]);

  Serial.print("x: ");
  Serial.println(x[i]);

  Serial.print("y: ");
  Serial.println(y[i]);
}

void readTemperature(){
  sensors.requestTemperatures(); 
  temperature[i] = sensors.getTempCByIndex(0); 
  Serial.print("i: ");
  Serial.println(i);

  Serial.print("temp: ");
  Serial.println(temperature[i]);
}

//4096
void readPressure(){
  float value = analogRead(pressurePin);
  pressure[i] = map(value, 0, 4095, 0, 1000);
  Serial.print("pressure: ");
  Serial.println(pressure[i]);
}

void read_velocity_and_acceleration(){
  Serial.print("j: ");
  Serial.println(j);
  
  velocity[j] = GPS.speed * 51.4444444; //converts to cm/s
  Serial.print("vel: ");
  Serial.println(velocity[j]);

  if(j > 0){
    float difference = velocity[j]*1000 - velocity[j-1]*1000;
    aks[j-1] = difference / milliSeconds ;
    Serial.print("aks: ");
    Serial.println(aks[j-1]);
  }
}

//---I2C sharp turn---
void receiveEvent(int howMany){
  while(Wire.available()){
    for(int i =0; i < howMany; i++){
      sharpTurn[i] = Wire.read();
      Serial.print("turn: ");
      Serial.println("sharpTurn[i]");
    }
  }
}

void setup(){
  //---I2C---
  //join i2c bus with address #4
  Wire.begin(espAddress);

  //registers a function to be called when peripheral device recieves a transmission from master device
  Wire.onReceive(receiveEvent);

  //define serial2 communication
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(tempPin,INPUT);
  sensors.begin();	// Start up the library

  pinMode(pressurePin,INPUT);
  Serial.begin(115200);
  rtc.begin();
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  delay(300);
  WiFi.begin(ssid, wifiPassword);

  // Wait until the connection is confirmed
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  // Debugging – Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  //---------GPS-en sender ut kommandoer-----------
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate  
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
}

void loop(){
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
    // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  if(millis() - previousShortCheck >= shortCheck){
    previousShortCheck = millis();
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if(GPS.fix){
      read_velocity_and_acceleration();
      //checks height at the very beginning 
      if(!heightCheck){
        heightCheck = true;
        previousMeter = GPS.altitude;
        Serial.print("prev altitude: ");
        Serial.println(previousMeter);
      }
      j++;
    }
  }

  if(millis() - previousLongCheck >= longCheck){
    previousLongCheck = millis();

    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if(GPS.fix){
      readPosition();
      readTemperature();
      readPressure();
      i++;
    }
  }
  //15 seconds have passed send message
  if(j == 10){
    for(int i = 0; i < 10; i++){
      Serial.print("vel: ");
      Serial.println(velocity[i]); //sjekk her!!!!
    }

    for(int i = 0; i < 4; i++){
      Serial.print("x: ");
      Serial.println(x[i]);
    }

    for(int n = 0; n < 4; n++){
      tempSmoothValue += temperature[n];
      pressureSmoothValue += pressure[n];
      xSmoothValue += x[n];
      ySmoothValue += y[n];
      zSmoothValue += z[n];
    }

    for(int n = 0; n < 10; n++){
      velocitySmoothValue += velocity[n];
    }

    for(int n = 0; n < 9; n++){
      if(aks[n] < 0){
        aks[n]/2;
      }
      aksSmoothValue += aks[n];
      if(sharpTurn[n] == 3 or sharpTurn[n] == 1){
        sharpTurnCount += 1;
      }
      if(aks[n] >= 4){
        storAksCount += 1;
      }
      if(aks[n] <= -2){ //m/s 
        storAksCount += 10;
      }
    }
    //calculate average
    tempSmoothValue /= 4;
    pressureSmoothValue /= 4;
    xSmoothValue /= 4;
    ySmoothValue /= 4;
    zSmoothValue /= 4;

    aksSmoothValue /= 9;
    velocitySmoothValue /= 10;

    //calculate meter above and meter below
    if(zSmoothValue - previousMeter < 0){
      meterBelow = abs(zSmoothValue - previousMeter);
      previousMeter = zSmoothValue;
    }
    else if(zSmoothValue - previousMeter >= 0){
      meterAbove = zSmoothValue - previousMeter;
      previousMeter = zSmoothValue; //check if this i correct
    }

    //power usage calculation (her trengs forbedringer)
    //velocity
    powerUsage = powerUsage + basisPower * pow(1.1,0.5*(velocitySmoothValue/100)-40);
    //akselerasjon
    powerUsage = powerUsage + pow((aksSmoothValue/100),3); //tredjegradsfunksjon
    //temp
    powerUsage = powerUsage + (-2*tempSmoothValue + 0.05*pow(tempSmoothValue,2) + 20)/0.15; // funksjon for positiv parabol
    //vekt
    powerUsage = powerUsage + 0.2*pressureSmoothValue;
    //høyde
    powerUsage = powerUsage + (velocitySmoothValue * meterAbove) / 4 - (velocitySmoothValue * meterBelow) / 8;
    //krapp sving
    if(aksSmoothValue < 0 and sharpTurnCount >= 4){
      powerUsage = powerUsage + pow(aksSmoothValue,3)/5;
    }

    //converting each value into string value and placing it into stringArray 
    for(int i = 0; i < 10; i++){
      if(i == 0){
        stringArray[i] = String(powerUsage);
        Serial.print("power: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 1){
        stringArray[i] = String(velocitySmoothValue);
        Serial.print("vel: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 2){
        stringArray[i] = String(meterAbove);
        Serial.print("meter above: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 3){
        stringArray[i] = String(meterBelow);
        Serial.print("meter below: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 4){
        stringArray[i] = String(tempSmoothValue);
        Serial.print("temp: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 5){
        stringArray[i] = String(pressureSmoothValue);
        Serial.print("pressure: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 6){
        stringArray[i] = String(xSmoothValue);
        Serial.print("x: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 7){
        stringArray[i] = String(ySmoothValue);
        Serial.print("y: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 8){
        stringArray[i] = String(storAksCount);
        Serial.print("aks count: ");
        Serial.println(stringArray[i]);
      }

      else if(i == 9){
        stringArray[i] = String(sharpTurnCount);
        Serial.print("turn count: ");
        Serial.println(stringArray[i]);
      }
    }

    //finding size of arrayString
    const int arraySize = sizeof(stringArray) / sizeof(stringArray[0]);

    //concatenate the strings into a single message
    for(int i = 0; i < arraySize; i++){
      message += stringArray[i];
      if(i < arraySize - 1){
        message += " "; //use a comma or any delimeter to seperate values
      }
    }

    Serial.println(message);

    sendMQTT(); 
    j = 0;
    i = 0;

    message = "";

    powerUsage = 0;
    velocitySmoothValue = 0;
    aksSmoothValue = 0;
    meterAbove = 0;
    meterBelow = 0;
    tempSmoothValue = 0;
    pressureSmoothValue = 0;
    xSmoothValue = 0;
    ySmoothValue = 0;
    zSmoothValue = 0;
    storAksCount = 0;
    sharpTurnCount = 0;  
  }
}

//------MQTT------
void sendMQTT(){
  connect_MQTT();
    Serial.setTimeout(2000);
      
    // MQTT can only transmit strings
    const char* messageTopic = "message";

    if (client.publish(messageTopic, message.c_str())) {
      client.publish(messageTopic, message.c_str());
      Serial.println("sendt!");
    }
    else{
      Serial.println("Failed to send. reconnecting to MQTT Broker and trying again");
      client.connect(clientID, mqttUsername, mqttPassword);
      delay(100);
      if (client.publish(messageTopic, message.c_str())) {
        client.publish(messageTopic, message.c_str());
        Serial.println("sendt!");
      }
    }
    client.disconnect(); //disconnect from the MQTT Broker
    
}
    
// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  if (client.connect(clientID, mqttUsername, mqttPassword)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed…");
  } 
}
