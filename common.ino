void readData() { // Read new command
  if (Serial.available() == 0) { return; }
  if (Serial.read() != START) {error();}
  while (Serial.available() == 0); cmd = Serial.read();
  while (Serial.available() == 0); len = Serial.read();
  bytesReceived = Serial.readBytes(dataRX.bytes, len);
  if (bytesReceived!= len) {error();}
  while (Serial.available() == 0); if (Serial.read() != STOP) {error();}
  cmdNew = true;
// 0x55 0x11 0x02 0x21 0x43 0xAA
}

void processCommand() { // Process nem command
  cmdNew = false;
  switch (cmd) {
    case 0x10: // Request ping
      dataTX.ints[0] = 0x1234;
      sendCommand(0x50, 2, dataTX.bytes);
      break;
    case 0x11: // Who are you
      dataTX.ints[0] = 0x5749;
      sendCommand(0x51, 2, dataTX.bytes);
      break;
    case 0x19: //Emergency stop
      stepperLeft.stop();
      stepperRight.stop();
      break;
    case 0x20: //Heartbeat
      HeartbeatTime = 0;
      break;
    case 0x21: // Set position
      stepperLeft.moveTo(((long)dataRX.ints[0])*8);
      stepperRight.moveTo(((long)dataRX.ints[1])*8);
      break;
    case 0x22: // Get state
      dataTX.ints[0] = 0x0000;
      sendCommand(0x54, 2, dataTX.bytes);
      break;
    case 0x23: // Homing
      Homing();
      break;
    default:
      sendText("Unknown Command Received!\n");
      break;
  }
}

void Homing(){
  stepperLeft.setSpeed(800.0);
  while(digitalRead(END_L) == LOW){
    stepperLeft.runSpeed();
  }
  stepperLeft.stop();
  stepperLeft.setCurrentPosition(0);
  dataTX.ints[0] = 0x0001;
  sendCommand(0x56, 2, dataTX.bytes);

 /* stepperRight.setSpeed(800.0);
  while(digitalRead(ALRM_R) == LOW){
    stepperRight.runSpeed();
  }
  stepperRight.stop();
  stepperRight.setCurrentPosition(0);
  dataTX.ints[0] = 0x0002;
  sendCommand(0x56, 2, dataTX.bytes);*/
}

void sendCommand(byte cmd, byte len, byte data[]) {
  Serial.write(START);
  Serial.write(cmd);
  Serial.write(len);
  Serial.write(data, len);
  Serial.write(STOP);
}

void sendText(String text) {
  Serial.write(START);
  Serial.write(0x53);
  Serial.write(text.length());
  text.toCharArray(dataTX.bytes, text.length()+1);
  Serial.write(dataTX.bytes, text.length());
  Serial.write(STOP);
}

void blinkLED() {
  if( millis() > nextBlinkTime ) {
    nextBlinkTime += blinkPeriod;
    digitalWrite(LED1, !digitalRead(LED1));
  }
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void error() {
  sendText("Communication error.\n");
  sendText("Restarting in 5 s...\n");
  for (int i = 0; i <50; i++) { digitalWrite(LED2, !digitalRead(LED2)); delay(100); }
  resetFunc();  //call reset
}
