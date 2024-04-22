#include "Prosjekt.h"
#include <Wire.h>
/*
* Transmits the variable kjoremodus to Zumo32u4 via I2C communication using the wire-library.
* Note: This function must have a period of about 500 ms between each time it is called.
* @param byte zumoaddress: Adress of the Zumo32u4
* @param byte kjøremodus: Variable which stores the driving direction
*/
void wireTransmit(byte zumoaddress, byte kjoremodus) {
    Wire.beginTransmission(zumoaddress);
    Wire.write("kjøremodus");
    Wire.write(kjoremodus);
    Wire.endTransmission();
}