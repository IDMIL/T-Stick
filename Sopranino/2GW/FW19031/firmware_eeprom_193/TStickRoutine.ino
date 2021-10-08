


boolean readTouch(){
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i=0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR,2);
  while (Wire.available()) { // slave may send less than requested
//    byte c = Wire.read();
//    temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
    i++;
  }    
  Wire.endTransmission();

  for (int t = 0; t<2; t++){
    if (temp[t] != touch[t]){
      changed = 1;
      touch[t] = temp[t];
    }
  }
  return changed;
}


void TStickRoutine() {
  
if (sendOSC){
      if (millis()-lastRead>touchInterval) {
        lastRead = millis();
        if (readTouch()){
          OSCMessage msg1("/rawcapsense");
          msg1.add((int)touch[0] & touchMask[0]);
          msg1.add((int)touch[1] & touchMask[1]);
          oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
          msg1.send(oscEndpoint);
          oscEndpoint.endPacket();
          msg1.empty();
        }
      }
    
      if ((millis() - deltaTransferRate) > dataTransferRate){

        lsm.read();  /* ask it to read in the data */ 
      
        /* Get a new sensor event */ 
        sensors_event_t a, m, g, temp;
        //sensor_t aRaw, mRaw, gRaw, tempRaw;
      
        lsm.getEvent(&a, &m, &g, &temp); 
        //lsm.getSensor(&aRaw, &mRaw, &gRaw, &tempRaw);
        
        outAccel[0] = a.acceleration.x / 9.80665F;
        outAccel[1] = a.acceleration.y / 9.80665F;
        outAccel[2] = a.acceleration.z / 9.80665F;
        
        outMag[0] = m.magnetic.x;
        outMag[1] = m.magnetic.y;
        outMag[2] = m.magnetic.z;
        
        outGyro[0] = g.gyro.x;
        outGyro[1] = g.gyro.y;
        outGyro[2] = g.gyro.z;
        
  
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
    
        int pressure = analogRead(pressurePin);
        if (calibrate == 1) {
          pressure = map(pressure, calibrationData[0], calibrationData[1], 0, 1024);
          if (pressure < 0) {pressure = 0;} 
         // pressure = constrain(pressure, 0, 4095);
        }
        OSCMessage msg5("/rawpressure");
        msg5.add(pressure);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg5.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg5.empty();
        deltaTransferRate = millis();

        unsigned int piezo = analogRead(piezoPin);
//        if (calibrate == 1) {
//          calibrationData[0] = constrain(min(calibrationData[0], piezo), 0, 4095);
//          calibrationData[1] = constrain(max(calibrationData[1], piezo), 0, 4095);
//        }
//        piezo = constrain(map(piezo, calibrationData[0], calibrationData[1], 0, 4095), 0, 4095);
        OSCMessage msg6("/rawpiezo");
        msg6.add(piezo);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg6.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg6.empty();
        deltaTransferRate = millis();
          
      }
    }  
    ledBlink();
    then = now;

}
