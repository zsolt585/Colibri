#include <Adafruit_GFX.h>       // include Adafruit graphics library
#include <Adafruit_ILI9341.h>   // include Adafruit ILI9341 TFT library
#include <AccelStepper.h>       // https://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html

#define STEP_L    A3     // Step signal output - left motor
#define STEP_R    A1     // Step signal output - right motor
#define DIR_L     A2     // Direction signal output - left motor
#define DIR_R     A0     // Direction signal output - right motor
#define SON_L     A5     // Servo ON signal output - left motor
#define SON_R     A4     // Servo ON signal output - right motor
#define ARST_L    A7     // Alarm reset signal output - left motor FUCK
#define ARST_R    A6     // Alarm reset signal output - right motor FUCK

#define RES_L    2      // Servo ready signal input - left motor
#define RES_R    4      // Servo ready signal input - right motor
#define END_L    3      // Servo alarm signal input - left motor
#define END_R    5      // Servo alarm signal input - right motor

#define TFT_CS    8      // TFT CS  pin is connected to arduino pin 8
#define TFT_RST   9      // TFT RST pin is connected to arduino pin 9
#define TFT_DC    10     // TFT DC  pin is connected to arduino pin 10

#define LED1      7      // LED output
#define LED2      7      // LED output
#define BTN1      6      // LED output

union data_union {
  int ints[30];
  byte bytes[60];
};
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
AccelStepper stepperLeft(AccelStepper::DRIVER, STEP_L, DIR_L);
AccelStepper stepperRight(AccelStepper::DRIVER, STEP_R, DIR_R);

const byte START = 0x55;
const byte STOP = 0xAA;
byte bytesReceived = 0;
byte cmd = 0;
byte len = 0;
byte cmdNew = 0;
int oldTime = 0;
int timePassed = 0;
int HeartbeatTime = 0;
union data_union dataRX, dataTX;
unsigned long blinkPeriod = 500;
unsigned long nextBlinkTime = 0;
unsigned long dispPeriod = 20;
unsigned long nextDispTime = 0;
byte partDisp = 1;
int buttonOldState = HIGH;
float default_accel = 200.0;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(RES_L, INPUT_PULLUP);
  pinMode(RES_R, INPUT_PULLUP);
  pinMode(END_L, INPUT_PULLUP);
  pinMode(END_R, INPUT_PULLUP);
  Serial.begin(115200);
  tft.begin();
  drawScreen();
  delay(1000);
  while (Serial.available()) {Serial.read();}
  sendText("System online!\n");
  stepperLeft.setEnablePin(SON_L);
  stepperRight.setEnablePin(SON_R);
  //stepperLeft.setPinsInverted(false, true, false);
  //stepperRight.setPinsInverted(false, true, false);
  stepperLeft.setMaxSpeed(800.0);
  stepperRight.setMaxSpeed(800.0);
  stepperLeft.setMinPulseWidth(20);
  stepperRight.setMinPulseWidth(20);
  stepperLeft.setAcceleration(default_accel);
  stepperRight.setAcceleration(default_accel);
//  stepperLeft.enableOutputs(); // Called again after constructor to set enable pins
//  stepperRight.enableOutputs(); // Called again after constructor to set enable pins
//  stepperLeft.setMinPulseWidth (unsigned int minWidth);
//  stepperRight.setMinPulseWidth (unsigned int minWidth);

  nextBlinkTime = millis();
  nextDispTime = millis();
}

 
void loop(void) {

//innen
  if(digitalRead(END_L) && stepperLeft.distanceToGo() >= 0) {
   stepperLeft.stop();
   stepperLeft.setAcceleration(2200.0); // A gyorsulas valtoztatasaval allithato, milyen gyorsan alljon meg
   stepperLeft.stop();
   stepperLeft.runToPosition();
  }
  else{stepperLeft.setAcceleration(default_accel);}

  if(digitalRead(END_R) && stepperRight.distanceToGo() >= 0) {
   stepperRight.stop();
   stepperRight.setAcceleration(2200.0); 
   stepperRight.stop();
   stepperRight.runToPosition();
  }
  else{stepperRight.setAcceleration(default_accel);}
//idaig 
  timePassed = millis() - oldTime;
  oldTime = millis();

  if (digitalRead(BTN1) == LOW) {
    if( millis() > nextDispTime ) {
    nextDispTime += dispPeriod;
    checkInputs();
    }
  }
 
  if(HeartbeatTime < 2000){
    HeartbeatTime += timePassed;
    //stepperLeft.run();
    //stepperRight.run();
  }
  else if(HeartbeatTime > 2000){
    stepperLeft.stop();
    stepperRight.stop();
  }

  stepperLeft.run();
  stepperRight.run();

  readData();
  if (cmdNew) { processCommand(); }
  blinkLED();
  
//  stepper2.currentPosition()
//  stepper.runToNewPosition(100); blocking
//  stepper.runSpeed(); // Constant speed
//  if (stepper1.distanceToGo() == 0)
//  stepper.stop(); // Stop as fast as possible: sets new target ESTOP?
//  setCurrentPosition (long position) // Zeroing
//  disableOutputs ()
//  enableOutputs ()

}

void drawScreen() { // 320/16=20 240/12=20
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("System ready.");
  tft.println("");
  tft.setTextColor(ILI9341_GREEN);
  tft.println("Servo ready (L): ");
  tft.println("Servo ready (R): ");
  tft.println("SW-end (L): ");
  tft.println("SW-end (R): ");
  tft.println("");
  tft.println("Target pos (L): ");
  tft.println("Target pos (R): ");
  tft.println("Current p. (L): ");
  tft.println("Current p. (R): ");
  tft.println("Distance   (L): ");
  tft.println("Distance   (R): ");
  tft.println("Drive state (L): ");
  tft.println("Drive state (R): ");
}

void checkInputs() {
  char buffer[4];
  if (digitalRead(RES_L)==HIGH){ tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); tft.setCursor(17*12, 2*16); tft.print("YES"); }
  else{                           tft.setTextColor(ILI9341_RED, ILI9341_BLACK);   tft.setCursor(17*12, 2*16); tft.print("NO "); }
  if (digitalRead(RES_R)==HIGH){ tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); tft.setCursor(17*12, 3*16); tft.print("YES"); }
  else{                           tft.setTextColor(ILI9341_RED, ILI9341_BLACK);   tft.setCursor(17*12, 3*16); tft.print("NO "); }
  if (digitalRead(END_L)==HIGH){ tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);   tft.setCursor(17*12, 4*16); tft.print("YES"); }
  else{                           tft.setTextColor(ILI9341_RED, ILI9341_BLACK); tft.setCursor(17*12, 4*16); tft.print("NO "); }
  if (digitalRead(END_R)==HIGH){ tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);   tft.setCursor(17*12, 5*16); tft.print("YES"); }
  else{                           tft.setTextColor(ILI9341_RED, ILI9341_BLACK); tft.setCursor(17*12, 5*16); tft.print("NO "); }

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(16*12, 7*16);  sprintf(buffer, "%4d", stepperLeft.targetPosition());   tft.print(buffer);
  tft.setCursor(16*12, 8*16);  sprintf(buffer, "%4d", stepperRight.targetPosition());  tft.print(buffer);
  tft.setCursor(16*12, 9*16);  sprintf(buffer, "%4d", stepperLeft.currentPosition());  tft.print(buffer);
  tft.setCursor(16*12, 10*16); sprintf(buffer, "%4d", stepperRight.currentPosition()); tft.print(buffer);
  tft.setCursor(16*12, 11*16); sprintf(buffer, "%4d", stepperLeft.distanceToGo());     tft.print(buffer);
  tft.setCursor(16*12, 12*16); sprintf(buffer, "%4d", stepperRight.distanceToGo());    tft.print(buffer);
  
  tft.setCursor(17*12, 13*16); tft.print( stepperLeft.isRunning() ? "RUN" : "RDY");
  tft.setCursor(17*12, 14*16); tft.print( stepperLeft.isRunning() ? "RUN" : "RDY");
}
