void printAccel()
{
  // To read from the accelerometer, you must first call the
  // readAccel() function. When this exits, it'll update the
  // ax, ay, and az variables with the most current data.

  dof.readAccel();
  rawAccel[0] = dof.calcAccel(dof.ax) - abias[0];
  rawAccel[1] = dof.calcAccel(dof.ay) - abias[1];
  rawAccel[2] = dof.calcAccel(dof.az) - abias[2];

  if (IMUcalibrated) {
    outAccel[0] = R[1][1]*rawAccel[0] + R[1][2]*rawAccel[1] + R[1][3]*rawAccel[2];
    outAccel[1] = R[2][1]*rawAccel[0] + R[2][2]*rawAccel[1] + R[2][3]*rawAccel[2];
    outAccel[2] = R[3][1]*rawAccel[0] + R[3][2]*rawAccel[1] + R[3][3]*rawAccel[2];
  }
  else if (rotateManually) {
    // Rotating manually 45 degrees around Z axis, right-handed, counter-clockwise (clockwise would be -45)
    outAccel[0] = -0.7071*rawAccel[0] + 0.7071*rawAccel[1];
    outAccel[1] = -0.7071*rawAccel[0] - 0.7071*rawAccel[1];
    outAccel[2] = rawAccel[2];    
  }
}

void printGyro()
{
  // To read from the gyroscope, you must first call the
  // readGyro() function. When this exits, it'll update the
  // gx, gy, and gz variables with the most current data.
  dof.readGyro();
  rawGyro[0] = dof.calcGyro(dof.gx) - gbias[0];
  rawGyro[1] = dof.calcGyro(dof.gy) - gbias[1];
  rawGyro[2] = dof.calcGyro(dof.gz) - gbias[2];

  if (IMUcalibrated) {
    outGyro[0] = R[1][1]*rawGyro[0] + R[1][2]*rawGyro[1] + R[1][3]*rawGyro[2];
    outGyro[1] = R[2][1]*rawGyro[0] + R[2][2]*rawGyro[1] + R[2][3]*rawGyro[2];
    outGyro[2] = R[3][1]*rawGyro[0] + R[3][2]*rawGyro[1] + R[3][3]*rawGyro[2];
  }
  else if (rotateManually) {
    // Rotating manually 45 degrees around Z axis, right-handed, counter-clockwise (clockwise would be -45)
    outGyro[0] = -0.7071*rawGyro[0] + 0.7071*rawGyro[1];
    outGyro[1] = -0.7071*rawGyro[0] - 0.7071*rawGyro[1];    
    outGyro[2] = rawGyro[2];
  }
}

void printMag()
{
  // To read from the magnetometer, you must first call the
  // readMag() function. When this exits, it'll update the
  // mx, my, and mz variables with the most current data.
  dof.readMag();

  rawMag[0] = dof.calcMag(dof.mx);
  rawMag[1] = dof.calcMag(dof.my);
  rawMag[2] = dof.calcMag(dof.mz);

  dof.readTemp();
  temperature = 21.0 + (float) dof.temperature/8.; // slope is 8 LSB per degree C, just guessing at the intercept
  
  if (IMUcalibrated) {
    outMag[0] = R[1][1]*rawMag[0] + R[1][2]*rawMag[1] + R[1][3]*rawMag[2];
    outMag[1] = R[2][1]*rawMag[0] + R[2][2]*rawMag[1] + R[2][3]*rawMag[2];
    outMag[2] = R[3][1]*rawMag[0] + R[3][2]*rawMag[1] + R[3][3]*rawMag[2];
  }
  else if (rotateManually) {
    // Rotating manually 45 degrees around Z axis, right-handed, counter-clockwise (clockwise would be -45)
    outMag[0] = -0.7071*rawMag[0] + 0.7071*rawMag[1];
    outMag[1] = -0.7071*rawMag[0] - 0.7071*rawMag[1];
    outMag[2] = rawMag[2];
  }
}

bool calibIMU(){
  unsigned long thenCalib = 0;
  unsigned long nowCalib = 0;
  int i = 0;
  while (i < 10){
    nowCalib = millis();
    if (nowCalib - thenCalib > 25) {
      printAccel();
      printGyro();
      printMag();
      // quaternion update and coordinate rotation
      NowQuat = micros();
      deltat = ((NowQuat - lastUpdateQuat)/1000000.0f); // set integration time by time elapsed since last filter update
      lastUpdateQuat = NowQuat;
      MadgwickQuaternionUpdate(outAccel[0], outAccel[1], outAccel[2], outGyro[0]*PI/180.0f, outGyro[1]*PI/180.0f, outGyro[2]*PI/180.0f, outMag[0], outMag[1], outMag[2]);
      thenCalib = nowCalib;
      i = i+1;
      Serial.println(i);
    }
  }
//  float angle = -acos(q[0])*2; //rotation tare for the origin of coordinates. Negative sign for already inversing the rotation
  q1 [0] = q[0]; Serial.print("q[0]: "); Serial.println(q[0]);
  q1 [1] = -q[1]; Serial.print("q[1]: "); Serial.println(q[1]);
  q1 [2] = -q[2]; Serial.print("q[2]: "); Serial.println(q[2]);
  q1 [3] = -q[3]; Serial.print("q[3]: "); Serial.println(q[3]);

  R[1][1] = 1 - 2*(q1[2]*q1[2] + q1[3]*q1[3]);
  R[1][2] = 2*(q1[1]*q1[2] - q1[3]*q1[0]);
  R[1][3] = 2*(q1[1]*q1[3] + q1[2]*q1[0]);

  R[2][1] = 2*(q1[1]*q1[2] + q1[3]*q1[0]);
  R[2][2] = 1 - 2*(q1[1]*q1[1] + q1[3]*q1[3]);
  R[2][3] = 2*(q1[2]*q1[3] - q1[1]*q1[0]);

  R[3][1] = 2*(q1[1]*q1[3] - q1[2]*q1[0]);
  R[3][2] = 2*(q1[2]*q1[3] + q1[1]*q1[0]);
  R[3][3] = 1 - 2*(q1[1]*q1[1] + q1[2]*q1[2]);
  
  IMUcalibrated = true;
  return true;
}
