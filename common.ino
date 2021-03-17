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
      dataTX.ints[0] = 0x524F;
      sendCommand(0x51, 2, dataTX.bytes);
      break;
    case 0x12: //set distance
       distanceInMM(dataRX.ints[0]);
      break;
    case 0x13: //Open valve
      dataRX.ints[0];
      break;
    case 0x14: // Get all data
      if (dataRX.ints[0] == 0) { sendPeriod = 0;}
      else if (dataRX.ints[0] < 10) { sendPeriod = 10;}
      else if (dataRX.ints[0] > 10000) { sendPeriod = 10000;}
      else { sendPeriod = dataRX.ints[0];}
//      for(int i = 0; i < 6; i++) { filterLP[i].setFrequency(1/(float(sendPeriod)*1e-3)); }
      break;
    case 0x16: //Set PID params
      
      break;
    case 0x17: //Save PID params
      
      break;
    case 0x18: //Start/Stop motors
      
      break;
    case 0x19: //Emergency stop
      //STOP
      break;
    case 0x20: //Heartbeat
      HeartbeatTime = 0;
      break;
    default:
      sendText("Unknown Command Received!\n");
      break;
  }
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
