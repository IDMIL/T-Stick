
void TStickRoutine() {

  if (sendOSC) {
    if (millis() - lastRead > interval)
    {
      lastRead = millis();

      // get the touch values from 2 x CY8C201xx chips
      interTouch[0] = (int)readTouch(I2C_ADDR0) & touchMask[0];
      interTouch[1] = (int)readTouch(I2C_ADDR1) & touchMask[1];
      OSCMessage msg1("/rawcapsense");
      msg1.add(interTouch[0]);
      msg1.add(interTouch[1]);
      //msg1.add((int)touch[0]);
      //msg1.add((int)touch[1]);
      oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
      msg1.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg1.empty();
    }

    if ((micros() - PreviousGyroRead) > GyroDeltaRead) {
      printGyro();  // Print "G: gx, gy, gz"
      PreviousGyroRead = micros();
    }
    if ((micros() - PreviousAccelRead) > AccelDeltaRead) {
      printAccel(); // Print "A: ax, ay, az"
      PreviousAccelRead = micros();
    }
    if ((micros() - PreviousMagRead) > MagDeltaRead) {
      printMag();   // Print "M: mx, my, mz"
      PreviousMagRead = micros();
    }

    //      // quaternion update and coordinate rotation
    //      NowQuat = micros();
    //      deltat = ((NowQuat - lastUpdateQuat)/1000000.0f); // set integration time by time elapsed since last filter update
    //      lastUpdateQuat = NowQuat;
    //      MadgwickQuaternionUpdate(outAccel[0], outAccel[1], outAccel[2], outGyro[0]*PI/180.0f, outGyro[1]*PI/180.0f, outGyro[2]*PI/180.0f, outMag[0], outMag[1], outMag[2]);

    if ((millis() - deltaTransferRate) > dataTransferRate) {
      OSCMessage msg2("/rawgyro");
      msg2.add(outGyro[0]);
      msg2.add(outGyro[1]);
      msg2.add(outGyro[2]);
      oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
      msg2.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg2.empty();

      OSCMessage msg3("/rawaccel");
      msg3.add(outAccel[0]);
      msg3.add(outAccel[1]);
      msg3.add(outAccel[2]);
      oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
      msg3.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg3.empty();

      OSCMessage msg4("/rawmag");
      msg4.add(outMag[0]);
      msg4.add(outMag[1]);
      msg4.add(outMag[2]);
      oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
      msg4.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg4.empty();

      pressure = analogRead(pressurePin);
      if (calibrate == 1) {
        calpressure = constrain(pressure, calibrationData[0], calibrationData[1]);
        calpressure = map(calpressure, calibrationData[0], calibrationData[1], 0, 1024);
      }
      else {
        calpressure = pressure;
      }

      OSCMessage msg5("/rawpressure");
      msg5.add(calpressure);
      oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
      msg5.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg5.empty();
      deltaTransferRate = millis();
    }
  }
  ledBlink();
  then = now;

}
