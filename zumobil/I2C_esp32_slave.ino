//slave esp32
#include <Wire.h>

//gir enheten en adresse 
int espAddress = 4;

//antall datapoints
const int numReadings = 6;

//beskriver hva de tallene i listen betyr
int right = 2;
int left = 0;
int null = 1;

int krappSving[numReadings];

//funksjon som avleser innkommende data
void receiveEvent(int howMany){
  while(Wire.available()){
    for(int i =0; i < howMany; i++){
      krappSving[i] = Wire.read();
    }
  }
}

void setup() {
  //join i2c bus with address #4
  Wire.begin(espAddress);
  //registers a function to be called when peripheral device recieves a transmission from master device
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  
}

void loop() {
}
