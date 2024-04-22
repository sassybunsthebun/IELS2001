#include <Wire.h>

///VARIABLES FOR READING THE CAR SENSORS///
void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
}

void loop()
{

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

//A function that follows a taped up line on the floor using the Zumo32u4's linefollower sensors
void linjefolging(){
  //sett inn funskjonen for linjefølging her 
}

//sett inn funksjoner (void loops) som leser av de ulike sensorverdiene 