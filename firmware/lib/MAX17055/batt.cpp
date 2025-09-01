#include "batt.h"

bool FUELGAUGE::init(fuelgauge_config config, bool reset)
{
    // Print out fuel gauge config
    std::cout << "\n"
              << "    Design Capacity: " << config.designcap << " mAh\n"
              << "    End of Charge Current: " << config.ichg << " mA\n"
              << "    Rsense: " << config.rsense << " mOhm\n"
              << "    Empty Voltage: " << config.vempty << " V\n"
              << "    Recovery Voltage: " << config.recovery_voltage << " V\n" << std::endl;

    // Get the parameters and save them
    i2c_addr = config.i2c_addr;
    designcap = config.designcap;
    ichg = config.ichg;
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
        uint16_t STATUS = readReg16Bit(STATUS_REG);
        uint16_t POR = STATUS&0x0002;
        std::cout << "    Checking status " << "\n"
                  << "    Status read: " << STATUS << "\n"
                  << "    POR flag: " << POR << std::endl;
        
        // Compute multipliers
        updateMultipliers(); 

        // read the registers to ensure the values are correct
        uint16_t raw_design_cap = readReg16Bit(DESIGNCAP_REG);
        uint16_t raw_igchg = readReg16Bit(ICHTERM_REG);

        // compute reg cap
        reg_cap = (designcap * rsense) / base_capacity_multiplier_mAh;
        reg_ichg = (ichg * rsense) / base_current_multiplier_mAh;

        // Compute raw cap and raw end of charge current 
        if ((reg_cap != raw_design_cap) || (reg_ichg != raw_igchg)) {
            reset = true;
        }

        // Check if CGain register was somehow misconfigured
        uint16_t CGAIN = readReg16Bit(0x2E);
        if (CGAIN != 0x400) {
            writeVerifyReg16Bit(0x2E, 0x400);
            std::cout << "    Checking CGAIN" << "\n"
                      << "    CGAIN read: " << CGAIN << "\n"
                      << "    fixing CGAIN" << std::endl;
        }
        // Reset the Fuel Gauge
        if (POR || reset)
        {
            std::cout << "    Initialising Fuel Gauge" << std::endl;
            while(readReg16Bit(0x3D)&1) {
                delay(10);
            }

            std::cout << "    Start up complete" << std::endl;
            //Initialise Configuration
            HibCFG = readReg16Bit(0xBA);
            // Exit hibernate mode
            writeReg16Bit(0x60, 0x90);
            writeReg16Bit(0xBA, 0x0);
            writeReg16Bit(0x60, 0x0);

            //EZ Config
            // Write Battery capacity
            std::cout << "    Writing Capacity" << std::endl;
            writeReg16Bit(DESIGNCAP_REG, reg_cap); //Write Design Cap
            writeReg16Bit(ICHTERM_REG, reg_ichg); // End of charge current
            writeReg16Bit(dQACC_REG, reg_cap/32); //Write dQAcc
            writeReg16Bit(dPACC_REG, 44138/32); //Write dPAcc

            // Set empty voltage and recovery voltage
            // Empty voltage in increments of 10mV
            std::cout << "    Writing Voltage" << std::endl;
            uint16_t reg_vempty = vempty * 100; //empty voltage in 10mV
            uint16_t reg_recover = 3.88 *25; //recovery voltage in 40mV increments
            uint16_t voltage_settings = (reg_vempty << 7) | reg_recover; 
            writeReg16Bit(VEMPTY_REG, voltage_settings); //Write Vempty
            
            // Set Model Characteristic
            writeReg16Bit(MODELCFG_REG, 0x8000); //Write ModelCFG

            //Wait until model refresh
            while(readReg16Bit(MODELCFG_REG)&0x8000) {
                delay(10);
            }
            //Reload original HbCFG value
            writeReg16Bit(0xBA,HibCFG);    

            // Reset Status Register when init function runs
            std::cout << "    Resetting Status" << std::endl;
            STATUS = readReg16Bit(STATUS_REG);
            
            // Get new status
            uint16_t RESET_STATUS = STATUS&0xFFFD;
            std::cout << "    Setting new status: " << RESET_STATUS << std::endl;
            writeVerifyReg16Bit(STATUS_REG,RESET_STATUS); //reset POR Status   

            // Read Status to ensure it has been cleared (for debugging)
            POR = readReg16Bit(STATUS_REG)&0x0002;
            std::cout << "    Status Flag: " << readReg16Bit(STATUS_REG) << "\n"
                    << "    POR Flag: " << POR << std::endl;  
        } else {
            std::cout << "    Loading old config" << std::endl;
        }   
        return true;
    }
    return false; //device not found
}

// Functions for setting design cap and rsense
void FUELGAUGE::setrsense(int resist_sense) {
    // Set current sensing resistor value in mOhms
    rsense = resist_sense;

    // Update the multipiers for capacity and current to adjust for capacity and current.
    updateMultipliers();
}

/// Set design capacity (for use externally)
void FUELGAUGE::setdesigncap(int design_capacity) {
    // Set design capacity in mAh
    designcap = design_capacity;

    // Compute reg cap
    reg_cap = designcap * (rsense / 5);
}

// Get Fuel Gauge data functions
void FUELGAUGE::getsoc() {
    // Get the State of charge
    raw_soc = readReg16Bit(REPSOC_REG);
    rep_soc = percentage_multiplier * raw_soc;
}

void FUELGAUGE::getcapacity() {
    // Get Battery Capacity
    raw_capacity = readReg16Bit(REPCAP_REG);
    rep_capacity = capacity_multiplier_mAH * raw_capacity;
}

void FUELGAUGE::getvoltage() {
    // Get instantaneous voltage
    raw_inst_voltage = readReg16Bit(VCELL_REG);
    rep_inst_voltage = voltage_multiplier_V * raw_inst_voltage;

    // Get Average Voltage
    raw_avg_voltage = readReg16Bit(AVGVCELL_REG);
    rep_avg_voltage = voltage_multiplier_V * raw_avg_voltage;
}

void FUELGAUGE::getcurrent(){
    // Get instantaneous current
    raw_inst_current = readReg16Bit(CURRENT_REG);
    rep_inst_current = current_multiplier_mV * raw_inst_current;

    // Get average current
    raw_avg_current = readReg16Bit(AVGCURRENT_REG);
    rep_avg_current = current_multiplier_mV * raw_avg_current;
}

void FUELGAUGE::getage() {
    // Get Battery Age
    raw_age = readReg16Bit(AGE_REG);
    rep_age = time_multiplier_Hours * int(raw_age);
}

void FUELGAUGE::gettte() {
    // Get time to empty
    raw_tte = readReg16Bit(TTE_REG);
    rep_tte = time_multiplier_Hours * int(raw_tte);
}

void FUELGAUGE::getttf() {
    // Get time to full
    raw_ttf = readReg16Bit(TTF_REG);
    rep_ttf = time_multiplier_Hours * int(raw_ttf);
}

void FUELGAUGE::getparameters() {
    // Get learned parameters
    rcomp = readReg16Bit(RCOMPP0_REG);
    tempco = readReg16Bit(TEMPCO_REG);
    fullcap = readReg16Bit(FULLCAP_REG);
    fullcapnorm = readReg16Bit(FULLCAPNORM_REG);
    uint16_t raw_cycles = readReg16Bit(CYCLES_REG);

    int old_val = (cycles >> 6) & 0x1;
    int cur_val = (raw_cycles >> 6) & 0x1;
    cycles = raw_cycles; 

    // Only save model parameters every 64% change in battery (equal to when bit 6 changes)
    if (old_val != cur_val) {
        save_params = true;
    }
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

void FUELGAUGE::getBatteryStatus() {
    // Get Battery Status
    // Read status register
    raw_status = readReg16Bit(STATUS_REG);

    // Get the 4th bit
    bat_status = raw_status&0x0800;
    bat_status = !bat_status; // battery status 0 when present, must invert
}

void FUELGAUGE::getBatteryInsertion() {
    // Get Battery Insertion
    // Read status register
    raw_status = readReg16Bit(STATUS_REG);

    // Get the 11th bit
    bat_insert = raw_status&0x0800;
    
    // Reset Insertion bit
    writeVerifyReg16Bit(STATUS_REG,raw_status&0xF7FF);
}

void FUELGAUGE::getBatteryRemoval() {
    // Get Battery Insertion
    // Read status register
    raw_status = readReg16Bit(STATUS_REG);

    // Get the 15th bit
    bat_remove = raw_status&0x8000;
    
    // Reset Removal bit
    writeVerifyReg16Bit(STATUS_REG,raw_status&0x7FFF);
}

void FUELGAUGE::getBatteryInfo() {
    // Get whether a battery has been, inserted, removed and if it is present
    getBatteryStatus();
    getBatteryRemoval();
    getBatteryInsertion();
}

float FUELGAUGE::getcapacityLSB() {
    // return the capacity LSB
    return capacity_multiplier_mAH;
}

float FUELGAUGE::getcurrentLSB() {
    // return the current LSB
    return current_multiplier_mV;
}

// Private Methods
void FUELGAUGE::updateMultipliers() {
    // Compute and store new multipliers
    capacity_multiplier_mAH = (base_capacity_multiplier_mAh/rsense); //refer to row "Capacity"
    current_multiplier_mV = (base_current_multiplier_mAh/rsense); //refer to row "Current"

    // Print new multipliers to serial
    std::cout << "    New current multiplier: " << current_multiplier_mV << "\n"
              << "    New capacity multiplier: " << capacity_multiplier_mAH << std::endl; 
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
  int attempt = 0;
  // Verify that the value has been written before moving on
  while ((value != readReg16Bit(reg)) && (attempt < 10)) {
    std::cout << "    Resetting Status ... attempt " << attempt << std::endl;
    //Write the value to the register
    writeReg16Bit(reg, value);
    // Wait a bit
    delay(1);

    //Increase attempt
    attempt++;
  };
  
  if (attempt > 10) {
    return false;
    std::cout << "    Failed to write value" <<std::endl;
  } else {
    std::cout << "    Value successfully written" << std::endl;
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