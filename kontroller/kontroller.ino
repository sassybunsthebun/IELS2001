const int LeftRightPin = 35; // L/R readings are read on pin 3
const int UpDownPin = 32; // U/D readings are read on pin 4
int MidPointBuffer = 300; 
int LeftRightMidPoint = 0; 
int UpDownMidPoint = 0; 

void setup() {
  Serial.begin(9600);
  LeftRightMidPoint = analogRead(LeftRightPin); 
  UpDownMidPoint = analogRead(UpDownPin); 
}

void loop() {
  int LeftRight = analogRead(LeftRightPin);
  int UpDown = analogRead(UpDownPin); 
  if (UpDown < UpDownMidPoint - MidPointBuffer){
    Serial.println("backwards");
  }
  if (UpDown > UpDownMidPoint + MidPointBuffer){
    Serial.println("forwards");
  }
  if (LeftRight < LeftRightMidPoint - MidPointBuffer){
    Serial.println("left");
  }
  if (LeftRight > LeftRightMidPoint + MidPointBuffer){
    Serial.println("right");
  }
}
