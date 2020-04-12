#include "TStick.h"

void addFloatArrayToMessage(const float *const v, int size, OSCMessage &m) {
  for (int i = 0; i < size; ++i) m.add(*(v + i));
}

void sendOSC(char *ip, int32_t port) {
  IPAddress oscIP;

  if (oscIP.fromString(ip) != false) {
    char namespaceBuffer[31];
    static OSCBundle bundle;

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/capsense", Tstick.id);
    OSCMessage msgCapsense(namespaceBuffer);
    for (byte i = 0; i < nCapsenses; ++i) {
      msgCapsense.add(RawData.touch[i][0] & Tstick.touchMask[i][0]);
      msgCapsense.add(RawData.touch[i][1] & Tstick.touchMask[i][1]);
    }
    bundle.add(msgCapsense);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/button/short", Tstick.id);
    OSCMessage msgBtnS(namespaceBuffer);
    msgBtnS.add(RawData.buttonShort);
    bundle.add(msgBtnS);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/button/long", Tstick.id);
    OSCMessage msgBtnL(namespaceBuffer);
    msgBtnL.add(RawData.buttonLong);
    bundle.add(msgBtnL);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/button/double", Tstick.id);
    OSCMessage msgBtnD(namespaceBuffer);
    msgBtnD.add(RawData.buttonDouble);
    bundle.add(msgBtnD);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/fsr", Tstick.id);
    OSCMessage msgFsrR(namespaceBuffer);
    msgFsrR.add(RawData.fsr);
    bundle.add(msgFsrR);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/norm/fsr", Tstick.id);
    OSCMessage msgFsrN(namespaceBuffer);
    msgFsrN.add(NormData.fsr);
    bundle.add(msgFsrN);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/piezo", Tstick.id);
    OSCMessage msgPiezoR(namespaceBuffer);
    msgPiezoR.add(RawData.piezo);
    bundle.add(msgPiezoR);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/norm/piezo", Tstick.id);
    OSCMessage msgPiezoN(namespaceBuffer);
    msgPiezoN.add(NormData.piezo);
    bundle.add(msgPiezoN);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/accl", Tstick.id);
    OSCMessage msgAcclR(namespaceBuffer);
    addFloatArrayToMessage(
        RawData.accl, sizeof(RawData.accl) / sizeof(RawData.accl[0]), msgAcclR);
    bundle.add(msgAcclR);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/norm/accl", Tstick.id);
    OSCMessage msgAcclN(namespaceBuffer);
    addFloatArrayToMessage(NormData.accl,
                           sizeof(NormData.accl) / sizeof(NormData.accl[0]),
                           msgAcclN);
    bundle.add(msgAcclN);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/gyro", Tstick.id);
    OSCMessage msgGyroR(namespaceBuffer);
    addFloatArrayToMessage(
        RawData.gyro, sizeof(RawData.gyro) / sizeof(RawData.gyro[0]), msgGyroR);
    bundle.add(msgGyroR);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/norm/gyro", Tstick.id);
    OSCMessage msgGyroN(namespaceBuffer);
    addFloatArrayToMessage(NormData.gyro,
                           sizeof(NormData.gyro) / sizeof(NormData.gyro[0]),
                           msgGyroN);
    bundle.add(msgGyroN);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/raw/magn", Tstick.id);
    OSCMessage msgMagnR(namespaceBuffer);
    addFloatArrayToMessage(
        RawData.magn, sizeof(RawData.magn) / sizeof(RawData.magn[0]), msgMagnR);
    bundle.add(msgMagnR);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/norm/magn", Tstick.id);
    OSCMessage msgMagnN(namespaceBuffer);
    addFloatArrayToMessage(NormData.magn,
                           sizeof(NormData.magn) / sizeof(NormData.magn[0]),
                           msgMagnN);
    bundle.add(msgMagnN);

    OSCMessage msgRaw("/raw");
    addFloatArrayToMessage(
        RawData.raw, sizeof(RawData.raw) / sizeof(RawData.raw[0]), msgRaw);
    bundle.add(msgRaw);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
             "/TStick_%i/orientation", Tstick.id);
    OSCMessage msgQuat(namespaceBuffer);
    addFloatArrayToMessage(
        RawData.quat, sizeof(RawData.quat) / sizeof(RawData.quat[0]), msgQuat);
    bundle.add(msgQuat);

    snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1), "/TStick_%i/ypr",
             Tstick.id);
    OSCMessage msgYpr(namespaceBuffer);
    addFloatArrayToMessage(
        RawData.ypr, sizeof(RawData.ypr) / sizeof(RawData.ypr[0]), msgYpr);
    bundle.add(msgYpr);

    if (millis() - batteryLastSend > batteryInterval) {
      batteryLastSend = millis();
      snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1),
               "/TStick_%i/battery", Tstick.id);
      OSCMessage msgBattery(namespaceBuffer);
      msgBattery.add(batteryPercentage);
      bundle.add(msgBattery);
    }

    oscEndpoint.beginPacket(oscIP, port);
    bundle.send(oscEndpoint);
    oscEndpoint.endPacket();
    bundle.empty();
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
      msgReceive.dispatch(
          "/state/touchMask",
          receiveTouchMask);  // receive touchMask values (doesn't save to JSON)
      msgReceive.dispatch("/state/info",
                          sendInfo);  // send back T-Stick current config
      msgReceive.dispatch("/state/json",
                          processJson);  // Json file related commands
      msgReceive.dispatch(
          "/state/FSRoffset",
          receiveFSRoffset);  // receive FSRoffset (doesn't save to JSON)
      msgReceive.dispatch("/state/setup", openPortalOSC);  // open portal
      msgReceive.dispatch(
          "/calibration/accl/vector",
          saveIMUaVector);  // receive IMU cal values and save to JSON
      msgReceive.dispatch("/calibration/accl/matrix", saveIMUaMatrix);
      msgReceive.dispatch("/calibration/magn/vector", saveIMUmVector);
      msgReceive.dispatch("/calibration/magn/matrix", saveIMUmMatrix);
      msgReceive.dispatch("/calibration/gyro/vector", saveIMUgVector);
      msgReceive.dispatch("/calibration/gyro/matrix", saveIMUgMatrix);
    } else {
      error = msgReceive.getError();
      Serial.print("\nOSC receive error: ");
      Serial.println(error);
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
  for (byte i = 0; i < (sizeof(Tstick.touchMask) / sizeof(Tstick.touchMask[0]));
       ++i) {
    Tstick.touchMask[i][0] = (int)msg.getFloat(2 * i);
    Tstick.touchMask[i][1] = (int)msg.getFloat((2 * i) + 1);
  }
  Serial.print("touchMask values received: ");
  printf("%i, %i, %i, %i, %i, %i, %i, %i, %i, %i", Tstick.touchMask[0][0],
         Tstick.touchMask[0][1], Tstick.touchMask[1][0], Tstick.touchMask[1][1],
         Tstick.touchMask[2][0], Tstick.touchMask[2][1], Tstick.touchMask[3][0],
         Tstick.touchMask[3][1], Tstick.touchMask[4][0],
         Tstick.touchMask[4][1]);
  Serial.println();
}

void receiveFSRoffset(OSCMessage &msg) {
  // message order: Tstick.FSRoffset
  fsrbuf = (float)msg.getFloat(0);
  Serial.print("FSRoffset value received: ");
  Serial.println(fsrbuf);
  fsrbuf *= 4095;
  Tstick.FSRoffset = fsrbuf;
  Serial.print("FSRoffset value stored: ");
  Serial.println(Tstick.FSRoffset);
}

void sendInfo(OSCMessage &msg) {
  // Send back T-Stick current config
  char namespaceBuffer[30];
  snprintf(namespaceBuffer, (sizeof(namespaceBuffer) - 1), "/TStick_%i/info",
           Tstick.id);
  IPAddress oscIP;
  if (oscIP.fromString(Tstick.oscIP[0])) {
    OSCMessage msgInfo(namespaceBuffer);
    msgInfo.add(Tstick.id);
    msgInfo.add(Tstick.firmware);
    msgInfo.add(float(Tstick.FSRoffset) / 4095);
    for (byte i = 0; i < (sizeof(RawData.touch) / sizeof(RawData.touch[0]));
         ++i) {
      msgInfo.add(Tstick.touchMask[i][0]);
      msgInfo.add(Tstick.touchMask[i][1]);
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
    msgInfo.add(float(Tstick.FSRoffset) / 4095);
    for (byte i = 0; i < (sizeof(RawData.touch) / sizeof(RawData.touch[0]));
         ++i) {
      msgInfo.add(Tstick.touchMask[i][0]);
      msgInfo.add(Tstick.touchMask[i][1]);
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
  if (command == 0) {
  } else if (command == 1) {
    saveJSON();
  } else if (command == 2) {
    parseJSON();
  }
}
