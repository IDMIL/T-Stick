
void setupI2C(){
   //start I2C bus
  Wire.begin();
  useI2C = true;

  // reset always both capsense. they sometimes get stuck with address.
  digitalWrite(xres1, HIGH);
  delay(200);
  digitalWrite(xres1, LOW);
  delay(200);
  digitalWrite(xres, HIGH);
  delay(200);
  digitalWrite(xres, LOW);
  delay(200);
  
  // chip #1: put into reset mode so that we can configure chip #2
  digitalWrite(xres, HIGH);
  delay(200);

  // this is for debugging purposes
  if (DEBUG){
    byte touch1;
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7C);
    Wire.endTransmission();
    
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    //Serial.println(touch);
    //Wire.endTransmission();
  
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7B);
    Wire.endTransmission();
    
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    //Serial.println(touch);
    //Wire.endTransmission();
  
    Wire.beginTransmission(I2C_ADDR0);
    //Wire.write(COMMAND_REG);
    Wire.write(0x7A);
    Wire.endTransmission();
    
    Wire.requestFrom(I2C_ADDR0,1);
    //while (!Wire.available()) {}
    touch1 = Wire.read();
    //Serial.print(touch);
    //Wire.endTransmission();
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
    Wire.write(I2C_ADDR1);
    Wire.endTransmission();
    
    // chip #2: lock register again for change to take effect
    Wire.beginTransmission(I2C_ADDR0);
    Wire.write(I2C_DEV_LOCK);
    Wire.write(I2CDL_KEY_LOCK, 3);
    Wire.endTransmission();
    // chip #2 now has the I2C address I2C_ADDR1
//

    // This is for debugging
    if (DEBUG){
      byte touch1;
      //Serial.println();
      Wire.beginTransmission(I2C_ADDR1);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7C);
      Wire.endTransmission();
      
      Wire.requestFrom(I2C_ADDR1,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.println(touch);
      //Wire.endTransmission();
  
      Wire.beginTransmission(I2C_ADDR1);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7B);
      Wire.endTransmission();
      
      Wire.requestFrom(I2C_ADDR1,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.println(touch);
      //Wire.endTransmission();
  
          Wire.beginTransmission(I2C_ADDR1);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7A);
      Wire.endTransmission();
      
      Wire.requestFrom(I2C_ADDR1,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.print(touch);
      //Wire.endTransmission();
    }
  
//  // CONFIGURE CHIP #1
//    // let the chip #1 wake up again
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

    // Debugging again
    if(DEBUG){
      byte touch1;
      Serial.println();
      Wire.beginTransmission(I2C_ADDR0);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7C);
      Wire.endTransmission();
  
      Wire.requestFrom(I2C_ADDR0,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.println(touch);
  
      Wire.beginTransmission(I2C_ADDR0);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7B);
      Wire.endTransmission();
      
      Wire.requestFrom(I2C_ADDR0,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.println(touch);
      Wire.endTransmission();
  
      Wire.beginTransmission(I2C_ADDR0);
      //Wire.write(COMMAND_REG);
      Wire.write(0x7A);
      Wire.endTransmission();
      
      Wire.requestFrom(I2C_ADDR0,1);
      //while (!Wire.available()) {}
      touch1 = Wire.read();
      //Serial.print(touch);
      Wire.endTransmission();
    }
    
    // accelerometer setup
    // Put the ADXL345 into +/- 2G range by writing the value 0x01 to the DATA_FORMAT register.
    // FYI: 0x00 = 2G, 0x01 = 4G, 0x02 = 8G, 0x03 = 16G
    // 0x0B puts ADXL345 in full resolution 16G mode.
    // 0x08 puts ADXL345 in full resolution 2G mode.
    //writeTo(DATA_FORMAT, 0x0B);
    
    Wire.beginTransmission(DEVICE);
    Wire.write(DATA_FORMAT);
    Wire.write(0x00);
    Wire.endTransmission();
    
    // Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
    //writeTo(BW_RATE, 0x0B); // 200Hz output rate  
    //writeTo(POWER_CTL, 0x08);
    //writeTo(INT_ENABLE, 0x80);
    Wire.beginTransmission(DEVICE);
    Wire.write(BW_RATE);
    Wire.write(0x0B);
    Wire.endTransmission();

    Wire.beginTransmission(DEVICE);
    Wire.write(POWER_CTL);
    Wire.write(0x08);
    Wire.endTransmission();

    Wire.beginTransmission(DEVICE);
    Wire.write(INT_ENABLE);
    Wire.write(0x80);
    Wire.endTransmission();
}
