
//////////////////////////
// Capsense Definitions //
//////////////////////////

#define SENSOR_EN 0x00
#define FSS_EN 0x02
#define SENSITIVITY0 0x08
#define SENSITIVITY1 0x09
#define SENSITIVITY2 0x0A
#define SENSITIVITY3 0x0B
#define DEVICE_ID 0x90    // Should return 0xA05 (returns 2 bytes)
#define FAMILY_ID 0x8F
#define SYSTEM_STATUS 0x8A
#define I2C_ADDR 0x37     // Should return 0x37
#define REFRESH_CTRL 0x52
#define SENSOR_EN 0x00    // We should set it to 0xFF for 16 sensors
#define BUTTON_STAT 0xAA  // Here we red the status of the sensors (2 bytes)
#define CTRL_CMD 0x86     // To configure the Capsense
#define CTRL_CMD_STATUS 0x88
#define CTRL_CMD_ERROR 0x89
#define BUTTON_LBR 0x1F
#define SPO_CFG 0x4C      //CS15 configuration address
#define GPO_CFG 0x40
#define CALC_CRC 0x94
#define CONFIG_CRC 0x7E

void initCapsense() {

  //Wire.begin();
  Wire.setClock(400000);

  if (DEBUG) {
    ///////
    byte infoboard = 0;
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SYSTEM_STATUS);
    Wire.endTransmission();

    // This should be 0 since a configuration other than the factory is loaded.
    Wire.requestFrom(I2C_ADDR, 1);
    if (DEBUG) {
      Serial.print("Wire.available First: ");
      Serial.println(Wire.available());
    }
    infoboard = Wire.read();
    if (DEBUG) {
      Serial.println("This should be 0");
      Serial.print("SYSTEM_STATUS: ");
      Serial.println(infoboard);
    }
    Wire.endTransmission();

    ///////
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(FAMILY_ID);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    if (DEBUG) {
      Serial.print("Wire.available SECOND: ");
      Serial.println(Wire.available());
    }
    infoboard = Wire.read();
    if (DEBUG) {
      Serial.println("This should be 154");
      Serial.print("FAMILY_ID: ");
      Serial.println(infoboard);
    }
    Wire.endTransmission();


    //  Wire.beginTransmission(I2C_ADDR);
    //  Wire.write(DEVICE_ID);
    //  Wire.endTransmission();
    //  //delay(100);
    //
    //  Wire.requestFrom(I2C_ADDR,2);
    //  Serial.print("Wire.available THIRD: ");
    //  Serial.println(Wire.available());
    //  while (Wire.available()) {
    //    byte c = Wire.read();
    //    Serial.println(c);
    //  }
    //delay(100);
    //  Wire.beginTransmission(I2C_ADDR);
    //  Wire.write(REFRESH_CTRL);
    //  Wire.endTransmission();
    //  //delay(100);
    //
    //  Wire.requestFrom(I2C_ADDR,1);
    //  Serial.print("Wire.available sixth: ");
    //  Serial.println(Wire.available());
    //  infoboard = Wire.read();
    //  Serial.print("REFRESH_CTRL VALUE: ");
    //  Serial.println(infoboard);
    //  Wire.endTransmission();
    //delay(100);
  }

  //**********************************
  // Capsense pins configuration
  //
  // Array to enable each of the 16 capsense sensors
  byte sensors[2] = {0xFFu, 0xFFu};

  // Originally the come as 0x00, 0xFF
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SENSOR_EN);
  Wire.write(sensors, 2);
  Wire.endTransmission();

  // Not actually necessary I think.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(FSS_EN);
  //Wire.write(sensors,2);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();

  //  // Sensitivity
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY0);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY1);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY2);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY3);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();

  // Special Purpose Output pin configuration
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SPO_CFG);
  Wire.write(B00010101);
  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SPO_CFG);
  //  Wire.endTransmission();
  //  //delay(100);
  //
  //  Wire.requestFrom(I2C_ADDR,1);
  //  Serial.print("Wire.available SPO_CFG: ");
  //  Serial.println(Wire.available());
  //  infoboard = Wire.read();
  //  Serial.print("SPO_CFG: ");
  //  Serial.println(infoboard,BIN);
  //  Wire.endTransmission();

  // Let's read again the SENSOR_EN just to confirm that nothing has been written yet. To be Deleted
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SENSOR_EN);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  if (DEBUG) {
    Serial.print("Wire.available FOURTH: ");
    Serial.println(Wire.available());
  }
  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    if (DEBUG) {Serial.println(c); }        // print the character
  }

  /*
      WRITING SETTINGS IN CAPSENSE
      - Write via I2C all the commands necessary to configure Capsense. I think the idea here is to have a conditional configuration option in the firmware and only enter this step if newer configuration needs to be loaded. All values are stored in internal flash memory so this could be bypassed in final firmware.
      - Write ‘3’ to CTRL_CMD in order to generate CRC that is written automatically by the chip to CALC_CRC
      - After 220ms or more read CRC from CALC_CRC
      - Write it to CONFIG_CRC
      - Write ‘2’ to CTRL_CMD to compare current CRC and stored one in CALC_CRC (they are the same) and write in internal flash.
      - Wait 220ms or more for command to finish.

  */


  // Send 0x03 to calculate de CRC from the changed configuration.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0x03);
  Wire.endTransmission();

  // This delay is important
  delay(300);

  // Read CRC calculated from the 0x03 command sent
  byte crc[2] = {0, 0};
  int i = 0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CALC_CRC);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read();
    Serial.println(c);         // print the character
    crc[i] = c; // receive a byte as character
    i++;
  }
  Wire.endTransmission();

  // Write CRC to CONFIG_CRC
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CONFIG_CRC);
  Wire.write(crc, 2);
  Wire.endTransmission();

  // Send 0x02 to calculate de CRC from the changed configuration.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0x02);
  Wire.endTransmission();
  delay(300);

  int result = 2;

  while (result != 0) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD_STATUS);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    result = Wire.read();
    if (DEBUG) {
      Serial.print("result CTRL_CMD_STATUS 1: ");
      Serial.println(result);
    }
    if (result == 1) {
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CTRL_CMD_ERROR);
      Wire.endTransmission();

      Wire.requestFrom(I2C_ADDR, 1);
      result = Wire.read();
      if (DEBUG) {
        Serial.print("result CTRL_CMD_ERROR 1: ");
        Serial.println(result);
      }
      Wire.endTransmission();
    }
    Wire.endTransmission();
  }

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(300);

  result = 2;

  while (result != 0) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD_STATUS);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    result = Wire.read();
    if (DEBUG) {
      Serial.print("result CTRL_CMD_STATUS 2: ");
      Serial.println(result);
    }
    if (result == 1) {
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CTRL_CMD_ERROR);
      Wire.endTransmission();

      Wire.requestFrom(I2C_ADDR, 1);
      result = Wire.read();
      if (DEBUG) {
        Serial.print("result CTRL_CMD_ERROR 2: ");
        Serial.println(result);
      }
      Wire.endTransmission();
    }
    Wire.endTransmission();
  }
  delay(500);
}
