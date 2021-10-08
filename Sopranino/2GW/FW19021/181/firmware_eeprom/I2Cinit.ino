
void I2Cinit(){

  //start I2C bus
  Wire.begin();
  Wire.setClockStretchLimit(15000);


  // set pin modes
  pinMode(xres, OUTPUT);
  pinMode(xres1, OUTPUT);

  // Always reset both Capsense. they sometimes get stuck with address.
  digitalWrite(xres1, HIGH);
  delay(200);
  digitalWrite(xres1, LOW);
  delay(200);
  digitalWrite(xres, HIGH);
  //Serial.println("Reset high");
  delay(200);
  digitalWrite(xres, LOW);
  //Serial.println("Reset low");
  delay(200);
  
  // chip #1: put into reset mode so that we can configure chip #2
  digitalWrite(xres, HIGH);
  delay(200);

  // this is for debugging purposes on chip 2
  if (DEBUG){
    Serial.println();
    Serial.println("debug 1");
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7C);
    Wire.endTransmission();
    delay(200);
  
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    Wire.endTransmission();
    delay(200);
  
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7B);
    Wire.endTransmission();
    delay(200);
  
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    Wire.endTransmission();
    delay(200);
  
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7A);
    Wire.endTransmission();
    delay(200);
    
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.print(touch1);
    Wire.endTransmission();
    delay(200);
  }
  
  // CONFIGURE CHIP #2
  // chip #2: switch to setup mode
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(COMMAND_REG);
  Wire.write(0x08);
  Wire.endTransmission();
  
  // chip #2: setup CS_ENABLE0 register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(CS_ENABLE0);
  Wire.write(B00001111);
  Wire.endTransmission();
  
  // chip #2: setup CS_ENABLE1 register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(CS_ENABLE1);
  Wire.write(B00001111);
  Wire.endTransmission();
  
  // chip #2: switch to normal mode
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(COMMAND_REG);
  Wire.write(0x07);
  Wire.endTransmission();
//    
  // chip #2: unlock the I2C_DEV_LOCK register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_DEV_LOCK);
  Wire.write(I2CDL_KEY_UNLOCK, 3);
  Wire.endTransmission();
  
  // chip #2: change the I2C_ADDR_DM register to I2C_ADDR1
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_ADDR_DM);
  Wire.write(I2C_ADDR1_NOPULL);
  Wire.endTransmission();
  
  // chip #2: lock register again for change to take effect
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_DEV_LOCK);
  Wire.write(I2CDL_KEY_LOCK, 3);
  Wire.endTransmission();
  // chip #2 now has the I2C address I2C_ADDR1_NOPULL
//

  // This is for debugging
  if (DEBUG){
    Serial.println();
    Serial.println("debug 2");
    Wire.beginTransmission(I2C_ADDR1_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7C);
    Wire.endTransmission();
    delay(200);
    
    Wire.requestFrom(I2C_ADDR1_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    Wire.endTransmission();
    delay(200);

    Wire.beginTransmission(I2C_ADDR1_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7B);
    Wire.endTransmission();
     delay(200);
   
    Wire.requestFrom(I2C_ADDR1_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    Wire.endTransmission();
    delay(200);

    Wire.beginTransmission(I2C_ADDR1_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7A);
    Wire.endTransmission();
    delay(200);

    Wire.requestFrom(I2C_ADDR1_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.print(touch1);
    Wire.endTransmission();
    delay(200);
  }
  
  // CONFIGURE CHIP #1
  // let the chip #1 wake up again
  digitalWrite(xres, LOW);
  delay(200);
  
  // chip #1: switch to setup mode
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(COMMAND_REG);
  Wire.write(0x08);
  Wire.endTransmission();
  
  // chip #1: setup CS_ENABLE0 register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(CS_ENABLE0);
  Wire.write(B00001111);
  Wire.endTransmission();
  
  // chip #1: setup CS_ENABLE1 register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(CS_ENABLE1);
  Wire.write(B00001111);
  Wire.endTransmission();
  
  // chip #1: switch to normal mode
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(COMMAND_REG);
  Wire.write(0x07);
  Wire.endTransmission();

//    
  // chip #1: unlock the I2C_DEV_LOCK register
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_DEV_LOCK);
  Wire.write(I2CDL_KEY_UNLOCK, 3);
  Wire.endTransmission();
  
  // chip #1: change the I2C_ADDR_DM register to I2C_ADDR0_NOPULL
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_ADDR_DM);
  Wire.write(I2C_ADDR0_NOPULL);
  Wire.endTransmission();
  
  // chip #1: lock register again for change to take effect
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(I2C_DEV_LOCK);
  Wire.write(I2CDL_KEY_LOCK, 3);
  Wire.endTransmission();
  // chip #1 now has the I2C address I2C_ADDR0_NOPULL
//
  // Debugging again
  if (DEBUG){
    Serial.println();
    Serial.println("debug 3");
    Wire.beginTransmission(I2C_ADDR0_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7C);
    Wire.endTransmission();
    delay(200);

    Wire.requestFrom(I2C_ADDR0_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    delay(200);

    Wire.beginTransmission(I2C_ADDR0_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7B);
    Wire.endTransmission();
    delay(200);
    
    Wire.requestFrom(I2C_ADDR0_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.println(touch1);
    Wire.endTransmission();
    delay(200);

    Wire.beginTransmission(I2C_ADDR0_NOPULL);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7A);
    Wire.endTransmission();
    delay(200);
    
    Wire.requestFrom(I2C_ADDR0_NOPULL,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    Serial.print(touch1);
    Wire.endTransmission();
    delay(200);
  }


  // Use the begin() function to initialize the LSM9DS0 library.
  // You can either call it with no parameters (the easy way):
  uint16_t status = dof.begin();
  // Or call it with declarations for sensor scales and data rates:  
  //uint16_t status = dof.begin(dof.G_SCALE_2000DPS, 
  //                            dof.A_SCALE_6G, dof.M_SCALE_2GS);
  
  // begin() returns a 16-bit value which includes both the gyro 
  // and accelerometers WHO_AM_I response. You can check this to
  // make sure communication was successful.
  //Wire.setClock(400000L);
  if (DEBUG) {
    Serial.println();
    Serial.print("LSM9DS0 WHO_AM_I's returned: 0x");
    Serial.println(status, HEX);
    Serial.println("Should be 0x49D4");
    Serial.println();
  }
  // Set data output ranges; choose lowest ranges for maximum resolution
  // Accelerometer scale can be: A_SCALE_2G, A_SCALE_4G, A_SCALE_6G, A_SCALE_8G, or A_SCALE_16G   
  dof.setAccelScale(dof.A_SCALE_4G);
  // Gyro scale can be:  G_SCALE__245, G_SCALE__500, or G_SCALE__2000DPS
  dof.setGyroScale(dof.G_SCALE_245DPS);
  // Magnetometer scale can be: M_SCALE_2GS, M_SCALE_4GS, M_SCALE_8GS, M_SCALE_12GS   
  dof.setMagScale(dof.M_SCALE_2GS);
  
  // Set output data rates  
  // Accelerometer output data rate (ODR) can be: A_ODR_3125 (3.225 Hz), A_ODR_625 (6.25 Hz), A_ODR_125 (12.5 Hz), A_ODR_25, A_ODR_50, 
  //                                              A_ODR_100,  A_ODR_200, A_ODR_400, A_ODR_800, A_ODR_1600 (1600 Hz)
  dof.setAccelODR(dof.A_ODR_25); // Set accelerometer update rate at 200 Hz

  
  // Accelerometer anti-aliasing filter rate can be 50, 194, 362, or 763 Hz
  // Anti-aliasing acts like a low-pass filter allowing oversampling of accelerometer and rejection of high-frequency spurious noise.
  // Strategy here is to effectively oversample accelerometer at 100 Hz and use a 50 Hz anti-aliasing (low-pass) filter frequency
  // to get a smooth ~150 Hz filter update rate
  dof.setAccelABW(dof.A_ABW_50); // Choose lowest filter setting for low noise
  
  // Gyro output data rates can be: 95 Hz (bandwidth 12.5 or 25 Hz), 190 Hz (bandwidth 12.5, 25, 50, or 70 Hz)
  //                                 380 Hz (bandwidth 20, 25, 50, 100 Hz), or 760 Hz (bandwidth 30, 35, 50, 100 Hz)
  dof.setGyroODR(dof.G_ODR_190_BW_125);  // Set gyro update rate to 190 Hz with the smallest bandwidth for low noise

  
  // Magnetometer output data rate can be: 3.125 (ODR_3125), 6.25 (ODR_625), 12.5 (ODR_125), 25, 50, or 100 Hz
  dof.setMagODR(dof.M_ODR_125); // Set magnetometer to update every 80 ms

  
  // Use the FIFO mode to average accelerometer and gyro readings to calculate the biases, which can then be removed from
  // all subsequent measurements.
  dof.calLSM9DS0(gbias, abias);
  delay(1000);

  Wire.setClockStretchLimit(15000);
  delay(1000);

}
