#include "batt.h"

bool FUELGAUGE::init(fuelgauge_config config, bool reset)
{
    // Get the parameters and save them
    i2c_addr = config.i2c_addr;
    designcap = config.designcap;
    rsense = config.rsense;
    vempty = config.vempty;
    recovery_voltage = config.recovery_voltage;

    // Get Learned parameters
    if (!reset) {
        rcomp = config.rcomp;
        tempco = config.tempco;
        fullcap = config.fullcap;
        fullcapnorm = config.fullcapnorm;
        cycles = config.cycles;
        raw_soc = config.soc;
    }

    Wire.beginTransmission(i2c_addr);
    byte error = Wire.endTransmission();
    if (error == 0) //Device Acknowledged
    {
        bool POR = readReg16Bit(STATUS_REG)&0x0002;
        setresistsensor(rsense);  
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
            // Write Battery capacity
            int reg_cap = designcap / capacity_multiplier_mAH;
            writeReg16Bit(DESIGNCAP_REG, reg_cap); //Write Design Cap
            writeReg16Bit(dQACC_REG, reg_cap/32); //Write dQAcc
            writeReg16Bit(dPACC_REG, 44138/32); //Write dPAcc

            // Set empty voltage and recovery voltage
            // Empty voltage in increments of 10mV
            int reg_vempty = vempty * 100; //empty voltage in 10mV
            int reg_recover = 3.88 *25; //recovery voltage in 40mV increments
            int voltage_settings = (reg_vempty << 7) | reg_recover; 
            writeReg16Bit(VEMPTY_REG, voltage_settings); //Write Vempty
            
            // Set Model Characteristic
            writeReg16Bit(MODELCFG_REG, 0x8000); //Write ModelCFG

            //Wait until model refresh
            while(readReg16Bit(MODELCFG_REG)&0x8000) {
                delay(10);
            }
            //Reload original HbCFG value
            writeReg16Bit(0xBA,HibCFG);
            writeReg16Bit(STATUS_REG,readReg16Bit(STATUS_REG)&0xFFFD); //reset POR Status            
        }
        return true;
    }
    return false; //device not found
}

// Get Fuel Gauge data functions
void FUELGAUGE::getsoc() {
    // Get the State of charge
    raw_soc = readReg16Bit(REPSOC_REG);
    cooked_soc = percentage_multiplier * raw_soc;
}

void FUELGAUGE::getcapacity() {
    // Get Battery Capacity
    capacity = capacity_multiplier_mAH * readReg16Bit(REPCAP_REG);
    saved_designcap = capacity_multiplier_mAH * readReg16Bit(DESIGNCAP_REG);
}

void FUELGAUGE::getvoltage() {
    // Get voltage
    voltage = voltage_multiplier_V * readReg16Bit(VCELL_REG);
    avg_voltage = voltage_multiplier_V * readReg16Bit(AVGVCELL_REG);
}

void FUELGAUGE::getcurrent(){
    // Get current
    current = current_multiplier_mV * readReg16Bit(CURRENT_REG);
    avg_current = current_multiplier_mV * readReg16Bit(AVGCURRENT_REG);
}

void FUELGAUGE::getage() {
    // Get Battery Age
    age = time_multiplier_Hours * readReg16Bit(AGE_REG);
}

void FUELGAUGE::gettte() {
    // Get time to empty
    tte = time_multiplier_Hours * readReg16Bit(TTE_REG);
}

void FUELGAUGE::getparameters() {
    // Get learned parameters
    rcomp = readReg16Bit(RCOMPP0_REG);
    tempco = readReg16Bit(TEMPCO_REG);
    fullcap = readReg16Bit(FULLCAP_REG);
    fullcapnorm = readReg16Bit(FULLCAPNORM_REG);
    cycles = readReg16Bit(CYCLES_REG);
}

void FUELGAUGE::getBatteryData() {
    // Read all the battery data from the registers
    getsoc();
    getcapacity();
    getvoltage();
    getcurrent();
    getage();
    gettte();

    // Get Learned parameters
    getparameters();
}

float FUELGAUGE::getresistsensor() {
    // Output sense resistor value
    return resistSensor;
}

// Private Methods

void FUELGAUGE::setresistsensor(float rsense) {
    resistSensor = rsense;
    updateMultipliers();
}

void FUELGAUGE::updateMultipliers() {
    capacity_multiplier_mAH = (5e-3)/resistSensor; //refer to row "Capacity"
    current_multiplier_mV = (1.5625e-3)/resistSensor; //refer to row "Current"
}


void FUELGAUGE::writeReg16Bit(uint8_t reg, uint16_t value)
{
  //Write order is LSB first, and then MSB. Refer to AN635 pg 35 figure 1.12.2.5
  Wire.beginTransmission(i2c_addr);
  Wire.write(reg);
  Wire.write( value       & 0xFF); // value low byte
  Wire.write((value >> 8) & 0xFF); // value high byte
  uint8_t last_status = Wire.endTransmission();
}

bool FUELGAUGE::writeVerifyReg16Bit(uint8_t reg, uint16_t value)
{
  //Write order is LSB first, and then MSB. Refer to AN635 pg 35 figure 1.12.2.5
  Wire.beginTransmission(i2c_addr);
  Wire.write(reg);
  Wire.write( value       & 0xFF); // value low byte
  Wire.write((value >> 8) & 0xFF); // value high byte
  // Wait a bit
  delay(10);
  int attempt = 0;
  // Verify that the value has been written before moving on
  while (value != readReg16Bit(reg) && attempt < 10) {
    delay(10);
    attempt++;
  };
  uint8_t last_status = Wire.endTransmission();
  
  if (attempt > 10) {
    return false;
  } else {
    return true;
  }
}

uint16_t FUELGAUGE::readReg16Bit(uint8_t reg)
{
  uint16_t value = 0;  
  Wire.beginTransmission(i2c_addr); 
  Wire.write(reg);
  uint8_t last_status = Wire.endTransmission(false);
  
  Wire.requestFrom(i2c_addr, (uint8_t) 2); 
  value  = Wire.read();
  value |= (uint16_t)Wire.read() << 8;      // value low byte
  return value;
}