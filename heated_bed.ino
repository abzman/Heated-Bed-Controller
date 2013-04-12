const int clockPin = P1_5;
const int dataPin = P1_6;
const int latchPin = P1_0;
const int enablePin = P2_2;
const int thermistorPin = A3;
const int potPin = A4;
const int fetPin = P2_0;

//Length of time to display a change to the pot in mS
unsigned long  displayPotChangeTime = 1000;

byte digit [10];
const int registers = 2;
byte registerArray [registers];


float max_temp = 200;
int got = 0;
float target_temp = 0;

//Temperature lookup table for 100k Epcos thermistor
const short temptable[][2] = {
	{1, 350},
	{28, 250}, //top rating 250C
	{31, 245},
	{35, 240},
	{39, 235},
	{42, 230},
	{44, 225},
	{49, 220},
	{53, 215},
	{62, 210},
	{71, 205}, //fitted graphically
	{78, 200}, //fitted graphically
	{94, 190},
	{102, 185},
	{116, 170},
	{143, 160},
	{183, 150},
	{223, 140},
	{270, 130},
	{318, 120},
	{383, 110},
	{413, 105},
	{439, 100},
	{484, 95},
	{513, 90},
	{607, 80},
	{664, 70},
	{781, 60},
	{810, 55},
	{849, 50},
	{914, 45},
	{914, 40},
	{935, 35},
	{954, 30},
	{970, 25},
	{978, 22},
	{1008, 3},
	{1023, 0}  //to allow internal 0 degrees C
};

void setup() {                
  pinMode(clockPin, OUTPUT); 
  pinMode(dataPin, OUTPUT); 
  pinMode(latchPin, OUTPUT); 
  pinMode(enablePin, OUTPUT); 
  pinMode(fetPin, OUTPUT);
  analogWrite(enablePin, 170);

  digit [0] = 0b11000000;
  digit [1] = 0b11111001;
  digit [2] = 0b10100100;
  digit [3] = 0b10110000;
  digit [4] = 0b10011001;
  digit [5] = 0b10010010;
  digit [6] = 0b10000011;
  digit [7] = 0b11111000;
  digit [8] = 0b10000000;
  digit [9] = 0b10011000;
  Serial.begin (9600);
}

void sendSerialData (byte registerCount, byte *pValueArray) {
    // Signal to the 595s to listen for data
  digitalWrite (latchPin, LOW);
  
  for (byte reg = registerCount; reg > 0; reg--) {
    byte value = pValueArray [reg - 1];
    for (byte bitMask = 128; bitMask > 0; bitMask >>= 1) {
      digitalWrite (clockPin, LOW);
      digitalWrite (dataPin, value & bitMask ? HIGH : LOW);
      digitalWrite (clockPin, HIGH);
    }
  }
  // Signal to the 595s that I'm done sending
  digitalWrite (latchPin, HIGH);
}  // sendSerialData

void sendint(int value) {
  registerArray[1] = digit[value % 10];
  registerArray[0] = digit[(value / 10) % 10];
  if (value > 99) registerArray[0] = (registerArray[0] & ~0b10000000);
  if (got < target_temp) registerArray[1] = (registerArray[1] & ~0b10000000);
  sendSerialData(registers, registerArray);
}

unsigned long displayTime = 0;

void loop() {
int i = analogRead(thermistorPin);
float new_temp = max_temp * (analogRead(potPin) / 1024.0);
Serial.println(new_temp);

if(abs(new_temp-target_temp) > 5) {
  displayTime = millis() + displayPotChangeTime;
}
target_temp = new_temp;

got = 0;
  for (int j = 0; j < (sizeof(temptable) / sizeof(temptable[0]))-1; ++j) {
    if (i >= temptable[j][0] && i <= temptable[j+1][0]) { //In this range
      got = temptable[j][1];
    }
  }

 if (got < target_temp && got > 3){
  digitalWrite(fetPin, HIGH);
 } else{
   digitalWrite(fetPin, LOW);
 }
  
if (millis() < displayTime) {
  sendint(target_temp);
} else { 
  sendint(got);
}
  delay(10);
}
