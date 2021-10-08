
void addFloatArrayToMessage(const float * const v, int size, OSCMessage& m) {
  for (int i = 0; i < size; ++i) m.add(*(v+i));
}

void sendOSC() {
  
  static OSCBundle bundle;
  
  OSCMessage msgCapsense("/raw/capsense");
    msgCapsense.add(Data.touch[0] & Tstick.touchMask[0]);
    msgCapsense.add(Data.touch[1] & Tstick.touchMask[1]);
    bundle.add(msgCapsense);

  OSCMessage msgFsr("/raw/fsr");
    msgFsr.add(Data.fsr);
    bundle.add(msgFsr);

  OSCMessage msgPiezo("/raw/piezo");
    msgPiezo.add(Data.piezo);
    bundle.add(msgPiezo);

  OSCMessage msgAccl("/raw/accl");
    addFloatArrayToMessage(Data.accl, sizeof(Data.accl)/sizeof(Data.accl[0]), msgAccl);
    bundle.add(msgAccl);

  OSCMessage msgGyro("/raw/gyro");
    addFloatArrayToMessage(Data.gyro, sizeof(Data.gyro)/sizeof(Data.gyro[0]), msgGyro);
    bundle.add(msgGyro);
    
  OSCMessage msgMagn("/raw/magn");
    addFloatArrayToMessage(Data.magn, sizeof(Data.magn)/sizeof(Data.magn[0]), msgMagn);
    bundle.add(msgMagn);

  OSCMessage msgRaw("/raw");
    addFloatArrayToMessage(Data.raw, sizeof(Data.raw)/sizeof(Data.raw[0]), msgRaw);
    bundle.add(msgRaw);

  OSCMessage msgQuat("/orientation");
    addFloatArrayToMessage(Data.quat, sizeof(Data.quat)/sizeof(Data.quat[0]), msgQuat);
    bundle.add(msgQuat);

  OSCMessage msgYpr("/ypr");
    addFloatArrayToMessage(Data.ypr, sizeof(Data.ypr)/sizeof(Data.ypr[0]), msgYpr);
    bundle.add(msgYpr);

  oscEndpoint.beginPacket(osc_IP, Tstick.oscPORT);
  bundle.send(oscEndpoint);
  oscEndpoint.endPacket();
  bundle.empty(); 
}

byte receiveOSC() {

  OSCErrorCode error;
  OSCMessage msgReceive;
  int size = oscEndpoint.parsePacket();

  if (size > 0) {
    blinkLED(flickering);
    Serial.println("\nOSC message received");
    while (size--) {
      msgReceive.fill(oscEndpoint.read());
    }
    if (!msgReceive.hasError()) {
      Serial.println("Routing OSC message...");
      msgReceive.dispatch("/calibration/accl/vector", saveAcclVector); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/accl/matrix", saveAcclMatrix); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/gyro/vector", saveGyroVector); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/gyro/matrix", saveGyroMatrix); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/magn/vector", saveMagnVector); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/magn/matrix", saveMagnMatrix); // receive IMU cal values and save to JSON
      
      msgReceive.dispatch("/state/touchMask", receiveTouchMask); // receive touchMask values (doesn't save to JSON)
      msgReceive.dispatch("/state/info", sendInfo); // send back T-Stick current config
      msgReceive.dispatch("/state/json", processJson); // Json file related commands
      msgReceive.dispatch("/state/FSRoffset", receiveFSRoffset); // receive FSRoffset (doesn't save to JSON)
    } else {
      error = msgReceive.getError();
      Serial.print("\nOSC receive error: "); Serial.println(error);
    }
  }
}

void saveAcclVector(OSCMessage &msg) { saveVector(msg, Tstick.abias); }
void saveGyroVector(OSCMessage &msg) { saveVector(msg, Tstick.gbias); }
void saveMagnVector(OSCMessage &msg) { saveVector(msg, Tstick.mbias); }

void saveAcclMatrix(OSCMessage &msg) { saveMatrix(msg, Tstick.acclcalibration); }
void saveGyroMatrix(OSCMessage &msg) { saveMatrix(msg, Tstick.gyrocalibration); }
void saveMagnMatrix(OSCMessage &msg) { saveMatrix(msg, Tstick.magncalibration); }

void saveVector(OSCMessage &msg, float dest[3]) {
  for (byte i = 0; i < 3; i++)
  {
    dest[i] = msg.getFloat(i);
  }
  saveJSON();
}

void saveMatrix(OSCMessage &msg, float dest[9]) {
  for (byte i = 0; i < 9; i++) {
    dest[i] = msg.getFloat(i);
  }
  saveJSON();  
}


void receiveTouchMask(OSCMessage &msg) {
  // message order: Tstick.touchMask[0], Tstick.touchMask[1]
  Tstick.touchMask[0] = (int)msg.getFloat(0);
  Tstick.touchMask[1] = (int)msg.getFloat(1);
  Serial.print("touchMask values received: "); Serial.print(Tstick.touchMask[0]); 
  Serial.print(" "); Serial.println(Tstick.touchMask[1]);
}


void receiveFSRoffset(OSCMessage &msg) {
  // message order: Tstick.FSRoffset
  Tstick.FSRoffset = (float)msg.getFloat(0);
  Serial.print("FSRoffset value received: "); Serial.println(Tstick.FSRoffset);
  Tstick.FSRoffset *= 4095;
  Serial.print("FSRoffset value stored: "); Serial.println(Tstick.FSRoffset);
}


void sendInfo(OSCMessage &msg) {
  // Send back T-Stick current config
  OSCMessage msgInfo("/info");
    msgInfo.add(Tstick.id);
    msgInfo.add(Tstick.firmware);
    msgInfo.add(Tstick.FSRoffset/4095);
    msgInfo.add(Tstick.touchMask[0]);
    msgInfo.add(Tstick.touchMask[1]);
    oscEndpoint.beginPacket(osc_IP, Tstick.oscPORT);
    msgInfo.send(oscEndpoint);
    oscEndpoint.endPacket();
    msgInfo.empty();
}


void processJson(OSCMessage &msg) {
  // "0" to return current JSON file - NOT IMPLEMENTED
  // "1" to save current setup to JSON file
  // "2" to load saved JSON file
  byte command = (int)msg.getFloat(0);
  if (command == 0) {}
  else if (command == 1) {saveJSON();}
  else if (command == 2) {parseJSON();}
}
