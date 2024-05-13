#include <Adafruit_GPS.h>

//-----------GPS lagrer data i en matrise------------
//antall datapoints
int const numReadings = 6;

//variablene til GPS-matrisen
float x[numReadings]; //x er breddegrad
float y[numReadings]; //y er lengdegrad
float z[numReadings]; //z er høyde
float v[numReadings]; //v er fart

//GPS-matrisen
const int rad = 4;
const int kolonne = numReadings;
float* GPS_matrise[]={x,y,z,v};

int i = 0;

//--------akselerasjon og deakselerasjon---------
//akselerasjonen skal beregnes hvert sekund
float seconds = 1000.0;

//velocity listen brukes bare til å beregne akslerasjonen (dette er grunnet av den skal beregne akselerasjoenn hvert sekund, men farten
//i GPS-matrisen beregnes hvert 10. sekund. Dette kan endres på slik at fart og akselerasjon sendes seperat fra GPS-matrisen)
float aks[numReadings];
float velocity[numReadings+1];

int j = 0;
int k = 0;

uint32_t previousMillis = millis(); 

//--------motta data fra GPS----------
#define RXD2 16 //RX pin
#define TXD2 17 //TX pin

// what's the name of the hardware serial port?
#define GPSSerial Serial2

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

//tar en timestamp ved begynnelsen av programmet
uint32_t timer = millis();

//------funksjon som lagrer data i GPS-matrisen-------
void readSensor(){

  GPS_matrise[0][i] = GPS.latitude; //avleser x posisjon og legger det inn i første variabel

  GPS_matrise[1][i] = GPS.longitude; //avleser y posisjon og legger det inn i andre variabel

  GPS_matrise[2][i] = GPS.altitude; //avleser z posisjon og legger det inn i tredje variabel  

  GPS_matrise[3][i] = GPS.speed * 514.444444; // avleser farten og konverterer det til mm/s som legges inn i fjerde variabel

  if(i == 5){
    Serial.println("send data..."); //sender GPS-verdier med 6 data points for hver variabel (mqtt)
  }

  //går til neste index
  i = i + 1;

  //i settes tilbake til index 0 når den har gått gjennom hele listen
  if(i >= kolonne){
    i = 0;
  }
}
//--------beregner fart og akselerasjon--------
void velocity_and_aks(){

  velocity[j] = (GPS.speed * 514.444444)*1000; //converts to (mm * 10^3)/s 

  //for å kunne beregne akselerasjonen, må velocity listen bestå av en ekstra index
  if(j > 0){
    float average = velocity[j] - velocity[j-1];
    aks[k] = average / seconds ;
    Serial.print("aks: ");
    Serial.println(aks[k]);

    if(k == 5){
      Serial.println("send data..."); //sender akselerasjonslisten (mqtt)
    }

    k = k + 1;

    if(k >= numReadings){
      k = 0;
    }
  }

  j = j + 1;

  if(j >= numReadings + 1){
    j = 0;
  }
}

void setup(){
  //legger til zeros i listen
  for(int m=0; m < rad; m ++){
    for(int n = 0;n < kolonne; n++){
      GPS_matrise[m][n] = 0;
    }
  }
  
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);

  //definerer serial2 kommukasjon 
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  //---------GPS-en sender ut kommandoer-----------
  
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

}

void loop(){
  // read data from the GPS
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  //----------beregner fart og akselerasjon--------
  if(millis() - previousMillis >= 1000){
    previousMillis = millis();

    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix) {
      velocity_and_aks();
    }
  }


  //-----------finner x, y, z og v-------------
  if (millis() - timer > 10000) {
    timer = millis(); // reset the timer

    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix) {
      readSensor(); 
    }
  }
}
