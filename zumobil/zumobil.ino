#include <Wire.h>
#include <Zumo32U4Encoders.h>
#include <Zumo32U4.h>

/// DEFINE THE INSTANCES OF THE CLASSES IN THE ZUMO32U4 LIBRARY ///
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4OLED display;
Zumo32U4Encoders encoders;

/// VARIABLES FOR LINE FOLLOWING ///
int speed = 220; 
unsigned long previousMillis = 0; 

#define NUM_SENSORS 5 // amount of sensors
unsigned int lineSensorValues[NUM_SENSORS]; //creates an array lineSensorValues with a length of NUM_SENSORS

/// VARIABLES FOR DETECTING A SHARP TURN ///

const int numReadings = 6;

int countsLeft[numReadings];
int countsRight[numReadings];
int sharpTurn[numReadings];
int totalTurns; 

int interval = 5000; 
int buffer = 500; 

///I2C VARIABLES ///

int espaddress = 8;
int zumoaddress = 4; 

/// CONTROLLER VARIABLES ///

//bool joyStickMode = false; 
int kjoremodus; 
int controllerInterval = 50; 

void setup()
{
  Wire.begin(); //zumoaddress
  //Wire.onReceive(receiveEvent); 
  Serial.begin(115200);
  lineSensors.initFiveSensors(); //merges the line sensors 
  motors.setSpeeds(-100,100); 
  for (byte i = 0; i <= 254; i++){
    lineSensors.calibrate();
  }
  motors.setSpeeds(0,0);
}

void loop()
{
  if(millis() - previousMillis >= interval){
    previousMillis = millis(); 
    calculateEncoders(); 
  }

  if(millis() - previousMillis >= controllerInterval){
    previousMillis = millis(); 
    calculateEncoders();
    Wire.requestFrom(espaddress, 1); 
  }

 // if (joyStickMode == false) {
  //  linjefolging();
  //}

  while(Wire.available()){
    kjoremodus = Wire.read();
    Serial.println(kjoremodus);
  }
  if(kjoremodus == 1){
    motors.setSpeeds(50,200);
  }
  else if(kjoremodus == 2){
    motors.setSpeeds(200,50);
  }
  else if(kjoremodus == 3){
    motors.setSpeeds(200,200);
  }
  else if(kjoremodus == 4){
    motors.setSpeeds(-150,-150);
  }
  else if(kjoremodus == 5){
    linjefolging(); 
  }
}

/*// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(Wire.available()){
    kjoremodus = Wire.read();
    Serial.println(kjoremodus);
  }
}

*/

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
    motors.setSpeeds(leftMotor, rightMotor);
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

//This function calculates the "ticks" of each wheel to derermine if there is a sharp turn
void calculateEncoders() {
  for(int i =0; i < numReadings; i++){
    countsLeft[i] = abs(encoders.getCountsAndResetLeft()); 
    countsRight[i] = abs(encoders.getCountsAndResetRight());
    if(countsLeft[i] - countsRight[i] >= buffer){ 
      sharpTurn[i] = 1;
    }
    else if(countsRight[i] - countsLeft[i] >= buffer){
      sharpTurn[i] = 1;
    }
    else{
      sharpTurn[i] = 0;
   }
   totalTurns += sharpTurn[i];
  }
  //wireTransmit(espaddress, totalTurns);
  sendCommand(); 
  totalTurns = 0; 
}

//This function sends the total turn value to the ESP32. 
void sendCommand(){
  Wire.beginTransmission(espaddress);
  Wire.write(totalTurns);
  int result = Wire.endTransmission();
  if(result == 0){
    Serial.println("transmission sucessfull");
  }else{
    Serial.print("transmission failed with error code: ");
    Serial.println(result);
  }
}