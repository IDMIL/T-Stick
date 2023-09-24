#include <Arduino.h>
#include <Wire.h>
// Inspired by https://github.com/AwotG/Arduino-MAX17055_Driver
// Simple library for the MAX17055 fuel gauge

class FUELGAUGE
{
    public:
        // register addresses 
        enum regAddr
        {
        STATUS_REG      = 0x00, //Maintains all flags related to alert thresholds and battery insertion or removal.
        AGE_REG         = 0x07, //calculated percentage value of capacity compared to original design capacity.
        TEMP_REG        = 0x08, //Temperature of MAX17055 chip
        VCELL_REG       = 0x09, //VCell reports the voltage measured between BATT and CSP.
        AVGVCELL_REG    = 0x19, //The AvgVCell register reports an average of the VCell register readings. 
        CURRENT_REG     = 0x0A, //Voltage between the CSP and CSN pins, and would need to convert to current
        AVGCURRENT_REG  = 0x0B, //The AvgCurrent register reports an average of Current register readings
        REPSOC_REG      = 0x06, //The Reported State of Charge of connected battery. Refer to AN6358 page 23 and 13
        REPCAP_REG      = 0x05, //Reported Capacity. Refer to page 23 and 13 of AN6358.
        TTE_REG         = 0x11, //How long before battery is empty (in ms). Refer to page 24 and 13 of AN6358 
        DESIGNCAP_REG   = 0x18, //Capacity of battery inserted, not typically used for user requested capacity
        VEMPTY_REG      = 0x3A, //Register for voltage when the battery is empty
        dQACC_REG       = 0x45, //Register for dQAcc
        dPACC_REG       = 0x46, //Register for dPAcc
        MODELCFG_REG    = 0xDB, // Register for MODELCFG
        RCOMPP0_REG     = 0x38, // Register for learned parameter rcomp0
        TEMPCO_REG      = 0x39, // Register for learned parameter tempco
        FULLCAP_REG     = 0x10, // Register for learned parameter full capacity
        CYCLES_REG      = 0x17, // Register for learned parameter charge cycles
        FULLCAPNORM_REG = 0x23, // Register for learned parameter full capacity (normalised)
        };
        // Fuel Gauge Config
        struct fuelgauge_config {
            // Initialisation Elements
            uint8_t i2c_addr = 0x36;
            int designcap = 2400;
            int rsense = 0.01;
            int vempty = 3.3;
            int recovery_voltage = 3.88;

            // Learned Parameters
            int soc = 0;
            int rcomp = 0;
            int tempco = 0;
            int fullcap = 0;
            int fullcapnorm = 0;
            int cycles = 0;
        };
        // Battery Parameters
        // Initialisation Elements
        uint8_t i2c_addr = 0x36;
        int designcap = 2400;
        int rsense = 0.01;
        int vempty = 3.3;
        int recovery_voltage = 3.88;

        // raw variables
        float current = 0;
        float avg_current = 0;
        float voltage = 0;
        float avg_voltage = 0;
        int capacity = 0;
        int designcap = 0;

        // learned variables
        int rcomp = 0;
        int tempco = 0;
        int fullcap = 0;
        int cycles = 0;
        int fullcapnorm = 0;

        // SOC, age and time to empty
        int raw_soc = 0; // State of charge
        int cooked_soc = 0; //
        int age = 0;
        int tte = 0;

        // methods
        // Initialise Fuel Gauge
        bool init(fuelgauge_config config, bool reset = true);
        
        // Get properties
        void getsoc();
        void getcapacity();
        void getvoltage();
        void getcurrent();
        void getage();
        void gettte();
        void getBatteryData();

        // Save learned parameters
        void getparameters();

    private:
        //variables
        float resistSensor = 0.01; //default internal resist sensor
        uint8_t i2c_addr = 0x36;
        uint16_t HibCFG = 0;
        
        //Based on "Register Resolutions from MAX17055 Technical Reference" Table 6. 
        float capacity_multiplier_mAH = (5e-3)/resistSensor; //refer to row "Capacity"
        float current_multiplier_mV = (1.5625e-3)/resistSensor; //refer to row "Current"
        float voltage_multiplier_V = 7.8125e-5; //refer to row "Voltage"
        float time_multiplier_Hours = 5.625/3600.0; //Least Significant Bit= 5.625 seconds, 3600 converts it to Hours. refer to AN6358 pg 13 figure 1.3 in row "Time"
        float percentage_multiplier = 1.0/256.0; //refer to row "Percentage"
        
        //methods
        void setresistsensor(float rsense);
        void updateMultipliers();
        uint16_t readReg16Bit(uint8_t reg);
        void writeReg16Bit(uint8_t reg, uint16_t value);
        bool writeVerifyReg16Bit(uint8_t reg, uint16_t value);
};