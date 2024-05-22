#include "Prosjekt.h"
#include <Wire.h>
/*
* Transmits the variable kjoremodus to Zumo32u4 via I2C communication using the wire-library.
* Note: This function must have a period of about 500 ms between each time it is called.
* @param byte zumoaddress: Adress of the Zumo32u4
* @param byte kjøremodus: Variable which stores the driving direction
*/
void wireTransmit(int espaddress, int totalTurns) {
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