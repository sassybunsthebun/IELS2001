//master zumo
#include <Wire.h>
#include <Zumo32U4.h>

//---------krapp sving----------//

//hvilke biblioteker vil vi inkludere
Zumo32U4Encoders encoders;

//antall datapoints
const int numReadings = 6;

int countsLeft[numReadings];
int countsRight[numReadings];
int krappSving[numReadings];

int i = 0;

//krapp sving mot vestre
int left = 0;
//ok
int null = 1;
//krapp sving mot høyre
int right = 2;

unsigned long previousMillis = 0;

//-------i2c------//

//gir adresse til esp32
int espAddress = 4;

//funksjon som sender datapakken via I2C til esp32
void sendCommand(){
  //begins a transmission to the I2C peripheral with the given address
  Wire.beginTransmission(espAddress);
  //writes data from device (master)
  for(int i =0; i < numReadings; i++){
    Wire.write(krappSving[i]);
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

//-------krapp sving---------//

//beregner antall ticks på hver hjul 
void calculateEncoder() {
  //finner antall ticks
  countsLeft[i] = encoders.getCountsAndResetLeft(); 
  countsRight[i] = encoders.getCountsAndResetRight();

  //konverterer negative ticks fra vestre hjul til positiv verdi
  if(countsLeft[i] < 0){
    countsLeft[i] = -1 * countsLeft[i];
  }

  //konverterer negative ticks fra høyre hjul til positiv verdi
  if(countsRight[i] < 0){
    countsRight[i] = -1 * countsRight[i];
  }  
}

//funksjon som sammenligner antall ticks på høyre og venstre hjul og lagrer resultatet i listen krappsving
void compareLeftAndRightEncoder(){  
  //dersom vesntre count er høyere enn høyre count
  if(countsLeft[i] - countsRight[i] >= 300){ // verdien "300" kan endres
    krappSving[i] = right;
    Serial.println("krapp sving mot høyre!");
  }

  //dersom høyre count er høyere enn venstre count
  else if(countsRight[i] - countsLeft[i] >= 300){ // verdien "300" kan endres
    krappSving[i] = left;
    Serial.println("krapp sving mot venstre!");
  }

  //dersom forholdet er normalt
  else{
    krappSving[i] = null;
    Serial.println("ok!");
  }

  //when the array have 6 new data points, send to esp32
  if(i == 5){
    sendCommand();
  }

  i = i + 1;

  if(i >= numReadings){
    i = 0;
  }
}

void setup() {
  //initialize I2C communication as master
  Wire.begin(); 

  Serial.begin(9600);

  previousMillis = millis();
}

void loop() {
  //sjekker og lagrer info hver 5. sekund (dette kan endres)
  if(millis()-previousMillis >= 5000){ //verdien "5000" kan endres
    previousMillis = millis();
    calculateEncoder();
    compareLeftAndRightEncoder();
  }
}
