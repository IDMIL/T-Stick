
#include "TStick.h"

void addFloatArrayToMessage(const float * const v, int size, OSCMessage& m) {
  for (int i = 0; i < size; ++i) m.add(*(v+i));
}

void sendOSC(char* ip,int32_t port) {

  IPAddress oscIP;

  if (oscIP.fromString(ip) != false) {
      char namespaceBuffer[40];
      static OSCBundle bundleRaw;
      static OSCBundle bundleInstrument;
    
      // snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/capsense",Tstick.id);
      // OSCMessage msgCapsense(namespaceBuffer);
      //   for (byte i=0; i < nCapsenses*2; ++i) {
      //     msgCapsense.add(RawData.touch[i] & Tstick.touchMask[i]);
      //   }
      //   bundleRaw.add(msgCapsense);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/button/short",Tstick.id);
      OSCMessage msgBtnS(namespaceBuffer);
        msgBtnS.add(RawData.buttonShort);
        bundleRaw.add(msgBtnS);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/button/long",Tstick.id);
      OSCMessage msgBtnL(namespaceBuffer);
        msgBtnL.add(RawData.buttonLong);
        bundleRaw.add(msgBtnL);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/button/double",Tstick.id);
      OSCMessage msgBtnD(namespaceBuffer);
        msgBtnD.add(RawData.buttonDouble);
        bundleRaw.add(msgBtnD);
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/fsr",Tstick.id);
      OSCMessage msgFsrR(namespaceBuffer);
        msgFsrR.add(RawData.fsr);
        bundleRaw.add(msgFsrR);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/norm/fsr",Tstick.id);
      OSCMessage msgFsrN(namespaceBuffer);
        msgFsrN.add(NormData.fsr);
        bundleRaw.add(msgFsrN);
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/piezo",Tstick.id);
      OSCMessage msgPiezoR(namespaceBuffer);
        msgPiezoR.add(RawData.piezo);
        bundleRaw.add(msgPiezoR);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/norm/piezo",Tstick.id);
      OSCMessage msgPiezoN(namespaceBuffer);
        msgPiezoN.add(NormData.piezo);
        bundleRaw.add(msgPiezoN);
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/accl",Tstick.id);
      OSCMessage msgAcclR(namespaceBuffer);
        addFloatArrayToMessage(RawData.accl, sizeof(RawData.accl)/sizeof(RawData.accl[0]), msgAcclR);
        bundleRaw.add(msgAcclR);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/norm/accl",Tstick.id);
      OSCMessage msgAcclN(namespaceBuffer);
        addFloatArrayToMessage(NormData.accl, sizeof(NormData.accl)/sizeof(NormData.accl[0]), msgAcclN);
        bundleRaw.add(msgAcclN);
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/gyro",Tstick.id);
      OSCMessage msgGyroR(namespaceBuffer);
        addFloatArrayToMessage(RawData.gyro, sizeof(RawData.gyro)/sizeof(RawData.gyro[0]), msgGyroR);
        bundleRaw.add(msgGyroR);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/norm/gyro",Tstick.id);
      OSCMessage msgGyroN(namespaceBuffer);
        addFloatArrayToMessage(NormData.gyro, sizeof(NormData.gyro)/sizeof(NormData.gyro[0]), msgGyroN);
        bundleRaw.add(msgGyroN);
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/raw/magn",Tstick.id);
      OSCMessage msgMagnR(namespaceBuffer);
        addFloatArrayToMessage(RawData.magn, sizeof(RawData.magn)/sizeof(RawData.magn[0]), msgMagnR);
        bundleRaw.add(msgMagnR);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/norm/magn",Tstick.id);
      OSCMessage msgMagnN(namespaceBuffer);
        addFloatArrayToMessage(NormData.magn, sizeof(NormData.magn)/sizeof(NormData.magn[0]), msgMagnN);
        bundleRaw.add(msgMagnN);
    
      OSCMessage msgRaw("/raw");
        addFloatArrayToMessage(RawData.raw, sizeof(RawData.raw)/sizeof(RawData.raw[0]), msgRaw);
        bundleRaw.add(msgRaw);
        
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/orientation",Tstick.id);
      OSCMessage msgQuat(namespaceBuffer);
        addFloatArrayToMessage(RawData.quat, sizeof(RawData.quat)/sizeof(RawData.quat[0]), msgQuat);
        bundleRaw.add(msgQuat);

      oscEndpoint.beginPacket(oscIP,port);
      bundleRaw.send(oscEndpoint);
      oscEndpoint.endPacket();
      bundleRaw.empty(); 
    
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/ypr",Tstick.id);
      OSCMessage msgYpr(namespaceBuffer);
        addFloatArrayToMessage(InstrumentData.ypr, sizeof(InstrumentData.ypr)/sizeof(InstrumentData.ypr[0]), msgYpr);
        bundleInstrument.add(msgYpr);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/touch/all",Tstick.id);
      OSCMessage msgTouchAll(namespaceBuffer);
        msgTouchAll.add(InstrumentData.touchAll);
        bundleInstrument.add(msgTouchAll);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/touch/top",Tstick.id);
      OSCMessage msgTouchTop(namespaceBuffer);
        msgTouchTop.add(InstrumentData.touchTop);
        bundleInstrument.add(msgTouchTop);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/touch/middle",Tstick.id);
      OSCMessage msgtouchMiddle(namespaceBuffer);
        msgtouchMiddle.add(InstrumentData.touchMiddle);
        bundleInstrument.add(msgtouchMiddle);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/touch/bottom",Tstick.id);
      OSCMessage msgtouchBottom(namespaceBuffer);
        msgtouchBottom.add(InstrumentData.touchBottom);
        bundleInstrument.add(msgtouchBottom);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/brush",Tstick.id);
      OSCMessage msgBrush(namespaceBuffer);
        msgBrush.add(InstrumentData.brush);
        bundleInstrument.add(msgBrush);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/multibrush",Tstick.id);
      OSCMessage msgMultiBrush(namespaceBuffer);
        for (byte i=0; i < sizeof(InstrumentData.multiBrush)/sizeof(InstrumentData.multiBrush[0]); ++i) {
          msgMultiBrush.add(InstrumentData.multiBrush[i]);
        }
        bundleInstrument.add(msgMultiBrush);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/rub",Tstick.id);
      OSCMessage msgRub(namespaceBuffer);
        msgRub.add(InstrumentData.rub);
        bundleInstrument.add(msgRub);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/multirub",Tstick.id);
      OSCMessage msgMultiRub(namespaceBuffer);
        for (byte i=0; i < sizeof(InstrumentData.multiRub)/sizeof(InstrumentData.multiRub[0]); ++i) {
          msgMultiRub.add(InstrumentData.multiRub[i]);
        }
        bundleInstrument.add(msgMultiRub);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/shakexyz",Tstick.id);
      OSCMessage msgShakeXYZ(namespaceBuffer);
        for (byte i=0; i < sizeof(InstrumentData.shakeXYZ)/sizeof(InstrumentData.shakeXYZ[0]); ++i) {
          msgShakeXYZ.add(InstrumentData.shakeXYZ[i]);
        }
        bundleInstrument.add(msgShakeXYZ);

      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/instrument/jabxyz",Tstick.id);
      OSCMessage msgJabXYZ(namespaceBuffer);
        for (byte i=0; i < sizeof(InstrumentData.jabXYZ)/sizeof(InstrumentData.jabXYZ[0]); ++i) {
          msgJabXYZ.add(InstrumentData.jabXYZ[i]);
        }
        bundleInstrument.add(msgJabXYZ);

      if (millis() - batteryLastSend > batteryInterval) {
        batteryLastSend = millis(); 
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/battery",Tstick.id);
        OSCMessage msgBattery(namespaceBuffer);
          msgBattery.add(batteryPercentage);
          bundleInstrument.add(msgBattery);
      }
    
      oscEndpoint.beginPacket(oscIP,port);
      bundleInstrument.send(oscEndpoint);
      oscEndpoint.endPacket();
      bundleInstrument.empty(); 
    }
}

void receiveOSC() {

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
      msgReceive.dispatch("/state/touchMask", receiveTouchMask); // receive touchMask values (doesn't save to JSON)
      msgReceive.dispatch("/state/info", sendInfo); // send back T-Stick current config
      msgReceive.dispatch("/state/json", processJson); // Json file related commands
      msgReceive.dispatch("/state/FSRoffset", receiveFSRoffset); // receive FSRoffset (doesn't save to JSON)
      msgReceive.dispatch("/state/setup", openPortalOSC); // open portal
      msgReceive.dispatch("/calibration/accl/vector", saveIMUaVector); // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/accl/matrix", saveIMUaMatrix);
      msgReceive.dispatch("/calibration/magn/vector", saveIMUmVector);
      msgReceive.dispatch("/calibration/magn/matrix", saveIMUmMatrix);
      msgReceive.dispatch("/calibration/gyro/vector", saveIMUgVector);
      msgReceive.dispatch("/calibration/gyro/matrix", saveIMUgMatrix);
    } else {
      error = msgReceive.getError();
      Serial.print("\nOSC receive error: "); Serial.println(error);
    }
  }
}

void openPortalOSC(OSCMessage &msg) {
  if ((int)msg.getFloat(0) == 1) {
   Wifimanager_portal(tstickSSID, Tstick.APpasswd); 
  }
}

void saveIMUaVector(OSCMessage &msg) {
  // message order: vector[3]
  for (byte i = 0; i < 3; ++i) {
    Tstick.abias[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void saveIMUgVector(OSCMessage &msg) {
  // message order: vector[3]
  for (byte i = 0; i < 3; ++i) {
    Tstick.gbias[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void saveIMUmVector(OSCMessage &msg) {
  // message order: vector[3]
  for (byte i = 0; i < 3; ++i) {
    Tstick.mbias[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void saveIMUaMatrix(OSCMessage &msg) {
  // message order: vector[9]
  for (byte i = 0; i < 9; ++i) {
    Tstick.acclcalibration[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void saveIMUmMatrix(OSCMessage &msg) {
  // message order: vector[9]
  for (byte i = 0; i < 9; ++i) {
    Tstick.magncalibration[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void saveIMUgMatrix(OSCMessage &msg) {
  // message order: vector[9]
  for (byte i = 0; i < 9; ++i) {
    Tstick.gyrocalibration[i] = msg.getFloat(i);
  }
  saveJSON();  
}

void receiveTouchMask(OSCMessage &msg) {
  // message order: Tstick.touchMask[0][0], [0][1], [1][0], [1][1], ...
  // for (byte i=0; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])); ++i ) {
  //   Tstick.touchMask[i] = (int)msg.getFloat(2*i);
  // }
  // Serial.print("touchMask values received: ");
  // for (byte i=0; i < sizeof(Tstick.touchMask); ++i) {
  //   Serial.print(Tstick.touchMask[i]); Serial.print(" ");
  // }
  // Serial.println();
}

void receiveFSRoffset(OSCMessage &msg) {
  // message order: Tstick.FSRoffset
  fsrbuf = (float)msg.getFloat(0);
  Serial.print("FSRoffset value received: "); Serial.println(fsrbuf);
  fsrbuf *= 4095;
  Tstick.FSRoffset = fsrbuf;
  Serial.print("FSRoffset value stored: "); Serial.println(Tstick.FSRoffset);
}


void sendInfo(OSCMessage &msg) {
  // Send back T-Stick current config
  char namespaceBuffer[30];
  snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/TStick_%i/info",Tstick.id);
  IPAddress oscIP;  
  if (oscIP.fromString(Tstick.oscIP[0])) {
    OSCMessage msgInfo(namespaceBuffer);
    msgInfo.add(Tstick.id);
    msgInfo.add(Tstick.firmware);
    msgInfo.add(float(Tstick.FSRoffset)/4095);
    for (byte i=0; i < (sizeof(RawData.touch)/sizeof(RawData.touch[0])); ++i) {
      msgInfo.add(Tstick.touchMask[i]);
    }
    oscEndpoint.beginPacket(oscIP, Tstick.oscPORT[0]);
    msgInfo.send(oscEndpoint);
    oscEndpoint.endPacket();
    msgInfo.empty();
  }
  if (oscIP.fromString(Tstick.oscIP[1])) {
    OSCMessage msgInfo(namespaceBuffer);
    msgInfo.add(Tstick.id);
    msgInfo.add(Tstick.firmware);
    msgInfo.add(float(Tstick.FSRoffset)/4095);
    for (byte i=0; i < (sizeof(RawData.touch)/sizeof(RawData.touch[0])); ++i) {
      msgInfo.add(Tstick.touchMask[i]);
    }
    oscEndpoint.beginPacket(oscIP, Tstick.oscPORT[1]);
    msgInfo.send(oscEndpoint);
    oscEndpoint.endPacket();
    msgInfo.empty();
  }
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
