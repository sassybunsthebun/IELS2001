#include <Wire.h>
#include <Zumo32U4Encoders.h>
#include <Zumo32U4.h>

/// DEFINE THE INSTANCES OF THE CLASSES IN THE ZUMO32u4 LIBRARY ///
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4OLED display;
Zumo32U4Encoders encoder;

/// VARIABLER FOR LINE FOLLOWING ///
int speed = 220; 
unsigned long previousMillis = 0; 

#define NUM_SENSORS 5 // amount of sensors
unsigned int lineSensorValues[NUM_SENSORS]; //creates an array lineSensorValues with a length of NUM_SENSORS

/// VARIABLES FOR I2C COMMUNICATION ///

byte espaddress = 8; 
byte info = 0; 

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); 
  Serial.begin(9600);
  lineSensors.initFiveSensors(); //merges the line sensors 
  motors.setSpeeds(-100,100); 
  for (byte i = 0; i <= 254; i++){
    lineSensors.calibrate();
  }
  motors.setSpeeds(0,0);
}

void loop()
{
  
  //linjefolging();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    //unsigned long currentMillis = millis();
  //if(currentMillis - previousMillis >= 100){ //reads average sensor value every 5 seconds
    //previousMillis = currentMillis;
    int kjoremodus = Wire.read();   
    Serial.println(kjoremodus);
    if(kjoremodus == 1){
      motors.setSpeeds(50,150);
    }
    else if(kjoremodus == 2){
      motors.setSpeeds(150,50);
    }
    else if(kjoremodus == 3){
      motors.setSpeeds(150,150);
    }
    else if(kjoremodus == 4){
      motors.setSpeeds(-100,-100);
    //}
    delay(100);
  }
  
  //Inne i denne funksjonen kan man sette opp logikk for hvordan bilen skal bevege seg basert på verdien til kjoremodus.
}

//A function that follows a taped up line on the floor using the Zumo32u4's linefollower sensors
void linjefolging(){
  int previousOffset;
  int maxSpeed = speed; 
  int sensorValue = lineSensors.readLine(lineSensorValues);//measures a value between 0 and 4000
  int offset; //offset from 2000
  
  if (sensorValue >= 2020){ 
    offset = (sensorValue - 2000)/5;//the bigger the offset, the shorter the turn of the car 
    maxSpeed -= offset/2;
    unsigned int leftMotor = maxSpeed + offset - previousOffset/4;
    unsigned int rightMotor = maxSpeed - offset + previousOffset/4;
    motors.setSpeeds( leftMotor, rightMotor);
  }
  else if (sensorValue <= 1980) { 
    offset = (2000 - sensorValue)/5;
    
    maxSpeed -= offset/2;

    unsigned int leftMotor = maxSpeed - offset + previousOffset/4;
    unsigned int rightMotor = maxSpeed + offset - previousOffset/4;
    motors.setSpeeds(leftMotor, rightMotor);
  }
  previousOffset = offset;
} 