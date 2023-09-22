#include "batt.h"

bool FUELGAUGE::init(int designcap, float rsense, float vempty)
{
    Wire.beginTransmission(I2CAddress);
    byte error = Wire.endTransmission();
    if (error == 0) //Device Acknowledged
    {
        bool POR = readReg16Bit(Status)&0x0002;
        if (POR)
        {
            while(readReg16Bit(0x3D)&1) {
                delay(10);
            }
            //Initialise Configuration
            HibCFG = readReg16Bit(0xBA);
            // Exit hibernate mode
            writeReg16Bit(0x60, 0x90);
            writeReg16Bit(0xBA, 0x0);
            writeReg16Bit(0x60, 0x0);

            //EZ Config
            writeReg16Bit(DesignCap, designcap); //Write Design Cap
            writeReg16Bit(dQAcc, designcap/32); //Write dQAcc
            writeReg16Bit(Vempty, vempty); //Write Vempty
            writeReg16Bit(0x46,int(designcap/32)*44138/designcap); //Write dPAcc
            writeReg16Bit(0xDB, 0x8000); //Write ModelCFG

            //Wait until model refresh
            while(readReg16Bit(0xDB)&0x8000) {
                delay(10);
            }
            //Reload original HbCFG value
            writeReg16Bit(0xBA,HibCFG);
            writeReg16Bit(Status,readReg16Bit(Status)&0xFFFD); //reset POR Status            
        }
        // Set current resistor for calculations
        setresistsensor(rsense);  
        return true;
    }
    return false; //device not found
}

void FUELGAUGE::setresistsensor(float rsense) {
    resistSensor = rsense;
    updateMultipliers();

}

void FUELGAUGE::updateMultipliers() {
    capacity_multiplier_mAH = (5e-3)/resistSensor; //refer to row "Capacity"
    current_multiplier_mV = (1.5625e-3)/resistSensor; //refer to row "Current"
}

float FUELGAUGE::getresistsensor() {
    return resistSensor;
}

void FUELGAUGE::getBatteryData() {
    raw_soc = percentage_multiplier * readReg16Bit(RepSOC);
    cooked_soc = readReg16HighBit(RepSOC);
    capacity = capacity_multiplier_mAH * readReg16Bit(RepCap);
    voltage = voltage_multiplier_V * readReg16Bit(VCell);
    current = current_multiplier_mV * readReg16Bit(Current);
    tte = time_multiplier_Hours * readReg16Bit(TimeToEmpty);
    age = time_multiplier_Hours * readReg16Bit(Age);
}
// Private Methods

void FUELGAUGE::writeReg16Bit(uint8_t reg, uint16_t value)
{
  //Write order is LSB first, and then MSB. Refer to AN635 pg 35 figure 1.12.2.5
  Wire.beginTransmission(I2CAddress);
  Wire.write(reg);
  Wire.write( value       & 0xFF); // value low byte
  Wire.write((value >> 8) & 0xFF); // value high byte
  uint8_t last_status = Wire.endTransmission();
}

uint16_t FUELGAUGE::readReg16Bit(uint8_t reg)
{
  uint16_t value = 0;  
  Wire.beginTransmission(I2CAddress); 
  Wire.write(reg);
  uint8_t last_status = Wire.endTransmission(false);
  
  Wire.requestFrom(I2CAddress, (uint8_t) 2); 
  value  = Wire.read();
  value |= (uint16_t)Wire.read() << 8;      // value low byte
  return value;
}

uint16_t FUELGAUGE::readReg16HighBit(uint8_t reg)
{
  uint16_t value = 0;  
  Wire.beginTransmission(I2CAddress); 
  Wire.write(reg);
  uint8_t last_status = Wire.endTransmission(false);
  
  Wire.requestFrom(I2CAddress, (uint8_t) 2); 
  value  = Wire.read();
  return value;
}