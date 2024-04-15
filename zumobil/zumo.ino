#include <Wire.h>

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop()
{
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void mottaKjoremodus(int howMany)
{
  int kjoremodus = Wire.read();   
  Serial.println(kjoremodus);   
  
  //Inne i denne funskjonen kan man sette opp logikk for hvordan bilen skal bevege seg basert på verdien til kjoremodus.
}

void linjefolging(){
  //sett inn funskjonen for linjefølging her 
}

//sett inn funksjoner (void loops) som leser av de ulike sensorverdiene 