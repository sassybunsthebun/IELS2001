#include <Wire.h>

///VARIABLES FOR READING THE CAR SENSORS///

const int numReadings = 6; // amount of datapoints
int temperature[numReadings]; //creates array for temperature readings
int pressure[numReadings]; //creates arrray for pressure readings
unsigned long previousMillis = 0;
int totalTemp;
int totalPressure;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
  previousMillis = millis();
}

void loop()
{
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= 5000){ //reads average sensor value every 5 seconds
    previousMillis = currentMillis;
    readSensorAverage();
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= 100){ //reads average sensor value every 5 seconds
    previousMillis = currentMillis;
      int kjoremodus = Wire.read();   
    Serial.println(kjoremodus);   
  }
  
  //Inne i denne funksjonen kan man sette opp logikk for hvordan bilen skal bevege seg basert på verdien til kjoremodus.
}

//A function that calculates the average value for the pressure and temperature sensors.
void readSensorAverage(){

  for (int i = 0; i < numReadings; i++) {
    temperature[i] = 1; //analogRead(...)
    pressure[i] = 2; //analogRead(...)
    totalTemp += temperature[i];
    totalPressure += pressure[i];
  }
  int averageTemp = totalTemp/numReadings;
  int averagePressure = totalPressure/numReadings;
  Serial.println(averageTemp); //insert function for sending information to esp32
  Serial.println(averagePressure); //insert function for sending information to esp32
  totalTemp = 0; //resets total value of temp and pressure function 
  totalPressure = 0;
}

//A function that follows a taped up line on the floor using the Zumo32u4's linefollower sensors
void linjefolging(){
  //sett inn funskjonen for linjefølging her 
}

//sett inn funksjoner (void loops) som leser av de ulike sensorverdiene 