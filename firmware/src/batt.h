#include <Arduino.h>
#include <Wire.h>
// Inspired by https://github.com/AwotG/Arduino-MAX17055_Driver
// Simple library for the MAX17055 fuel gauge

// Fuel Gauge Config
struct fuelgauge_config {
    // Initialisation Elements
    uint8_t i2c_addr;
    int designcap;
    float rsense;
    float vempty;
    float recovery_voltage;

    // Learned Parameters
    int soc;
    int rcomp;
    int tempco;
    int fullcap;
    int fullcapnorm;
    int cycles;
};


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
        REPSOC_REG      = 0x06, //The Reported State of Charge of connected battery. 
        REPCAP_REG      = 0x05, //Reported Capacity.
        TTE_REG         = 0x11, //How long before battery is empty (in ms).
        TTF_REG         = 0x20, //How long until the battery is full (in ms)
        DESIGNCAP_REG   = 0x18, //Capacity of battery inserted, not typically used for user requested capacity
        VEMPTY_REG      = 0x3A, //Register for voltage when the battery is empty
        dQACC_REG       = 0x45, //Register for dQAcc
        dPACC_REG       = 0x46, //Register for dPAcc
        MODELCFG_REG    = 0xDB, // Register for MODELCFG
        RCOMPP0_REG     = 0x38, // Register for learned parameter rcomp0, open circuit voltage characterisation
        TEMPCO_REG      = 0x39, // Register for learned parameter tempco, temperature compensation information
        FULLCAP_REG     = 0x10, // Register for learned parameter full capacity
        CYCLES_REG      = 0x17, // Register for learned parameter charge cycles
        FULLCAPNORM_REG = 0x23, // Register for learned parameter full capacity (normalised)
        };

        // Battery Parameters
        // Initialisation Elements
        uint16_t designcap = 2400;
        int rsense = 10;
        float vempty = 3.3;
        float recovery_voltage = 3.88;

        // raw variables
        uint16_t raw_inst_voltage = 0;
        uint16_t raw_avg_voltage = 0;
        uint16_t raw_inst_current = 0;
        uint16_t raw_avg_current = 0;
        uint16_t raw_capacity = 0;

        // reported variables
        float rep_inst_current = 0;
        float rep_avg_current = 0;
        float rep_inst_voltage = 0;
        float rep_avg_voltage = 0;
        int rep_capacity = 0;
        int rep_soc = 0; //
        int rep_age = 0;
        int rep_tte = 0;

        // learned variables
        uint16_t raw_soc = 0;
        uint16_t rcomp = 0;
        uint16_t tempco = 0;
        uint16_t fullcap = 0;
        uint16_t cycles = 0;
        uint16_t fullcapnorm = 0;

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
        float getresistsensor();

        // Save learned parameters
        void getparameters();

    private:
        //variables
        uint8_t i2c_addr = 0x36;
        uint16_t HibCFG = 0;
        
        //Based on "Register Resolutions from MAX17055 Technical Reference" Table 6. 
        float capacity_multiplier_mAH = (5e-6)/rsense; //refer to row "Capacity"
        float current_multiplier_mV = (1.5625e-6)/rsense; //refer to row "Current"
        float voltage_multiplier_V = 7.8125e-5; //refer to row "Voltage"
        float time_multiplier_Hours = 5.625/3600.0; //Least Significant Bit= 5.625 seconds, 3600 converts it to Hours.
        float percentage_multiplier = 1.0/256.0; //refer to row "Percentage"
        
        //methods
        void updateMultipliers();
        uint16_t readReg16Bit(uint8_t reg);
        void writeReg16Bit(uint8_t reg, uint16_t value);
        bool writeVerifyReg16Bit(uint8_t reg, uint16_t value);
};