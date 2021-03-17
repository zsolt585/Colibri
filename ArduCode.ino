#include <Filters.h>
#include <Servo.h>

#define LED1  7 // LED1 output
#define LED2  8 // LED2 output

union data_union {
  int ints[30];
  byte bytes[60];
};
Servo servo[4];
FilterOnePole filterLP[6];
const byte START = 0x55;
const byte STOP = 0xAA;
const byte servoPins[] = {3, 4, 5, 6}; // CW
const byte analPins[] = {A0, A1, A2, A3, A4, A5};
int analInputs[6];
int servoPWM = 0;
byte bytesReceived = 0;
byte cmd = 0;
byte len = 0;
byte cmdNew = 0;
int oldTime = 0;
int timePassed = 0;
int HeartbeatTime = 0;
union data_union dataRX, dataTX;
unsigned long sendPeriod = 0;
unsigned long nextSendTime = 0;
unsigned long blinkPeriod = 500;
unsigned long nextBlinkTime = 0;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  analogReference(EXTERNAL);
  Serial.begin(115200);
  for(int i = 0; i < 4; i++) { servo[i].attach(servoPins[i]); }
  for(int i = 0; i < 4; i++) { servo[i].writeMicroseconds(1000); }
  for(int i = 0; i < 6; i++) { filterLP[i].setFilter(LOWPASS, 1.0, 0.0); }
  delay(1000);
  while (Serial.available()) {Serial.read();}
  sendText("System online!\n");
}

void loop() {
  for (int i = 0; i <6; i++) {
    analInputs[i] = analogRead(analPins[i]);
    filterLP[i].input(analInputs[i]);
  }
  servoPWM = analogRead(A5);
  servoPWM = map(servoPWM, 0, 1023, 1000, 2000);
  for(int n = 0; n < 4; n++) { servo[n].writeMicroseconds(servoPWM); }
  if (sendPeriod){
    if( millis() > nextSendTime ) {
      nextSendTime += sendPeriod;
      for (int i = 0; i <4; i++) { dataTX.ints[i] = analInputs[i]; }
      for (int i = 4; i <8; i++) { dataTX.ints[i] = round(filterLP[i-4].output()); }
      for (int i = 8; i <12; i++) { dataTX.ints[i] = round(distanceInMM(filterLP[i-8].output())); }
      for (int i = 12; i <16; i++) { dataTX.ints[i] = 0; }
      sendCommand(0x52, 32, dataTX.bytes);
    }
  }

  timePassed = millis() - oldTime;
  oldTime = millis();

  if(HeartbeatTime < 2000){
    HeartbeatTime += timePassed;
  }
  else if(HeartbeatTime > 2000){
   //anything
  }
  
  readData();
  if (cmdNew) { processCommand(); }
  blinkLED();
}

float distanceInMM(float distanceRAW) {
  float distanceMM = 299.88 * pow((distanceRAW*3.3)/1023, -1.173);
//  float distanceMM = 299.88 * pow(map(round(distanceRAW), 0, 1023, 0, 3300)/1000.0, -1.173);
//  distanceCM = 29.988 * pow(map(median, 0, 1023, 0, 5000)/1000.0, -1.173);
  return distanceMM;
}
