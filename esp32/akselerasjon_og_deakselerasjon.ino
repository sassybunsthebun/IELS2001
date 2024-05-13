#include <Adafruit_GPS.h>

//-----------GPS lagre data i matrise funksjon------------
//antall data points
int const numReadings = 6;

//variablene til GPS-en
float x[numReadings];
float y[numReadings];
float z[numReadings];
float v[numReadings];

//GPS-matrisen
const int rad = 4;
const int kolonne = numReadings;
float* GPS_matrise[]={x,y,z,v};

int i = 0;

//--------aks---------
float seconds = 1000.0;

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

//------funksjon som lagrer data i matrisen-------
void readSensor(){

  GPS_matrise[0][i] = GPS.latitude; //avleser x posisjon og legger det inn i første variabel

  GPS_matrise[1][i] = GPS.longitude; //avleser y posisjon og legger det inn i andre variabel

  GPS_matrise[2][i] = GPS.altitude; //avleser z posisjon og legger det inn i tredje variabel  

  GPS_matrise[3][i] = GPS.speed * 514.444444; // avleser farten og konverterer det til mm/s som legges inn i fjerde variabel

  
  //test (printer ut verdiene til de forskjellige listene )
  for(int i = 0; i < rad; i++){
    for(int j = 0; j < kolonne; j++){
      Serial.println(GPS_matrise[i][j]);
    }
  }

  if(i == 5){
    Serial.println("send data..."); //sender GPS-verdier med 6 data points for hver variabel
  }

  //går til neste index
  i = i + 1;

  //i settes tilbake til index 0 når den har gått gjennom hele listen
  if(i >= kolonne){
    i = 0;
  }
}

void velocity_and_aks(){

  velocity[j] = (GPS.speed * 514.444444)*1000; //converts to mm/s
  Serial.print("velocity: ");
  Serial.println(velocity[j]);

  if(j > 0){
    float average = velocity[j] - velocity[j-1];
    aks[k] = average / seconds ;
    Serial.print("aks: ");
    Serial.println(aks[k]);

    if(k == 5){
      Serial.println("send data...");
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
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);

  //definerer serial2 kommukasjon 
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  //---------GPS-en sender ut kommandoer-----------
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

}

void loop(){
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  if(millis() - previousMillis >= 1000){
    previousMillis = millis();

    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix) {
      velocity_and_aks();
    }
  }


  //hvert tiende sekund, lagres verdiene for variablene i GPS-matrisen
  if (millis() - timer > 10000) {
    timer = millis(); // reset the timer

    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);

    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix) {
      readSensor();
      
    }
  }
}