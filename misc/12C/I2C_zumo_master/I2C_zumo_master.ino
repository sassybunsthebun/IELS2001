//master zumo
#include <Wire.h>
#include <Zumo32U4.h>

///VARIABLES FOR DETECTING A SHARP TURN///
Zumo32U4Encoders encoders;

const int numReadings = 6;

int countsLeft[numReadings];
int countsRight[numReadings];
int sharpTurn[numReadings];
int totalTurns; 

int interval = 5000; 
int buffer = 500; 

unsigned long previousMillis = 0; 

///I2C VARIABLES ///

int espaddress = 8;
int zumoaddress = 4; 

void setup() {
  Wire.begin(zumoaddress); 
  Serial.begin(115200);
}

void loop() {
  if(millis() - previousMillis >= interval){
    previousMillis = millis(); 
    calculateEncoders();
  }
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