#include <PubSubClient.h>
#include <WiFi.h>
#include <RTClib.h>

// WiFi
const char* ssid = "Telia-2G-7C1911";                
const char* wifiPassword = "BPUt5DKnn9bY";

// MQTT
const char* mqttServer = "192.168.0.84"; 
const char* pressureTopic = "Pressure";

const char* mqttUsername = "mashii"; // MQTT username
const char* mqttPassword = "EtVanskeligPassord69"; // MQTT password
const char* clientID = "mashii"; // MQTT client ID

const uint8_t pressurePin = 33;

int pressurePoints[10] = {};
int smoothValue;

uint8_t j = 0;
int dataMilli = millis();

RTC_DS3231 rtc;

//Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;

// 1883 is the listener port for the Broker
PubSubClient client(mqttServer, 1883, wifiClient);

void setup()
{
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
}

void loop()
{
  DateTime now = rtc.now();
  //Serial.println(analogRead(pressurePin));

  if (millis() - dataMilli > 10000)
  {
    j++;
    int value = analogRead(pressurePin);
    pressurePoints[j] = value;
    Serial.print(j);
    Serial.print(": ");
    Serial.println(value);
    dataMilli = millis();
  }
  if (j == 10){
    smoothValue = 0;
    for (int i = 0;i<10;i++){
      smoothValue += pressurePoints[i];
    }
    smoothValue /= 10;
    Serial.print(smoothValue);
    sendMQTT(); 
    j = 0;
  }
}


void sendMQTT(){
  connect_MQTT();
    Serial.setTimeout(2000);
      
      // MQTT can only transmit strings
    String stringValue = String(smoothValue);
    
    if (client.publish(pressureTopic, stringValue.c_str())) {
    Serial.println("sent!");
  }
  else {
    Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqttUsername, mqttPassword);
    delay(100); // This delay ensures that client.publish doesn’t clash with the client.connect call
    client.publish(pressureTopic, stringValue.c_str());
  }
  client.disconnect();  // disconnect from the MQTT broker
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