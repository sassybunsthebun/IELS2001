//master zumo
#include <Wire.h>
#include <Zumo32U4.h>

//---------sharp turn----------//

//libraries to include
Zumo32U4Encoders encoders;

//number of datapoints
const int numReadings = 9;

//store data in arrays
int countsLeft[numReadings];
int countsRight[numReadings];
int sharpTurn[numReadings];

int i = 0;

//sharp left turn
int left = 1;
//ok
int null = 2;
//sharp right turn
int right = 3;

unsigned long previousMillis = millis();

//-------i2c------//

//esp32 address
int espAddress = 4;

//----functions----//
//sends data through I2C til esp32
void sendCommand(){
  //begins a transmission to the I2C peripheral with the given address
  Wire.beginTransmission(espAddress);
  //writes data from device (master)
  for(int i =0; i < numReadings; i++){
    Wire.write(sharpTurn[i]);
  }
  //ends a transmission to the peripheral device and transmitts what were queued by write()
  int result = Wire.endTransmission();

  //prints wether the message was sent
  if(result == 0){
    Serial.println("transmission sucessfull");
  }else{
    Serial.print("transmission failed with error code: ");
    Serial.println(result);
  }
}

//-------sharp turn---------//

//calculating amout of ticks on each wheel 
void calculateEncoder() {
  //finds number of ticks
  countsLeft[i] = encoders.getCountsAndResetLeft(); 
  countsRight[i] = encoders.getCountsAndResetRight();

  //converts negative ticks from the left wheel into positive value 
  if(countsLeft[i] < 0){
    countsLeft[i] = -1 * countsLeft[i];
  }
  
  //converts negative ticks from the right wheel into positive value 
  if(countsRight[i] < 0){
    countsRight[i] = -1 * countsRight[i];
  }  
}

//compare number of ticks on each wheel and save result in sharpTurn 
void compareLeftAndRightEncoder(){  
  //if left count is bigger than right count
  if(countsLeft[i] - countsRight[i] >= 300){ // verdien "300" kan endres
    sharpTurn[i] = right;
    Serial.println("krapp sving mot høyre!");
  }

  //if right count is bigger than left count
  else if(countsRight[i] - countsLeft[i] >= 300){ // verdien "300" kan endres
    sharpTurn[i] = left;
    Serial.println("krapp sving mot venstre!");
  }

  //if no big difference
  else{
    sharpTurn[i] = null;
    Serial.println("ok!");
  }
}

void setup() {
  //initialize I2C communication as master
  Wire.begin(); 
  Serial.begin(9600);
}

void loop() {
  //checks every 5 seconds
  if(millis()-previousMillis >= 5000){ //verdien "5000" kan endres
    previousMillis = millis();
    calculateEncoder();
    compareLeftAndRightEncoder();
    i++
  }
  //when the array have 6 new data points, send to esp32
  if(i == numReadings){
    sendCommand();
    i = 0;
  }
}
