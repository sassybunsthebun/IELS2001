//slave esp32
#include <Wire.h>

//gir enheten en adresse 
int espaddress = 8;
int zumoaddress = 4; 

//antall datapoints
const int numReadings = 6;

int sharpTurn;

int w = 1; 

//funksjon som avleser innkommende data
void receiveEvent(int howMany){
  while(Wire.available()){
    for(int i =0; i < howMany; i++){
      sharpTurn = Wire.read();
      Serial.println(sharpTurn);
    }
  }
}

void setup() {
  Wire.begin(); //espaddress
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
}

void loop() {
  delay(5000); 
  sendCommand();
}

void sendCommand(){
  Wire.beginTransmission(zumoaddress);
  Wire.write(w);
  int result = Wire.endTransmission();
  if(result == 0){
    Serial.println("transmission sucessfull");
  }else{
    Serial.print("transmission failed with error code: ");
    Serial.println(result);
  }
}
