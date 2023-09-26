#include <Arduino.h>
#include <Wire.h>
#include <iostream>

// Inspired by https://github.com/AwotG/Arduino-MAX17055_Driver
// Simple library for the MAX17055 fuel gauge

// Fuel Gauge Config
struct fuelgauge_config {
    // Initialisation Elements
    uint8_t i2c_addr;
    int designcap;
    int rsense;
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
        int designcap = 2400;
        uint16_t reg_cap = designcap *2;
        int rsense = 10;
        float vempty = 3.3;
        float recovery_voltage = 3.88;

        // raw variables
        int16_t raw_inst_current = 0;       // Instantaneous Current (mV/rsense (mOhm))
        int16_t raw_avg_current = 0;        // Average Current (mV/rsense (mOhm))
        uint16_t raw_inst_voltage = 0;      // Instaneous Voltage (0.0078125V/bit)    
        uint16_t raw_avg_voltage = 0;       // Average Voltage (0.0078125V/bit)
        uint16_t raw_capacity = 0;          // Report capacity (mVh/rsense (mOhm))
        uint16_t raw_soc = 0;               // State of Charge (1/256%) [Should be saved for reboot]
        uint16_t raw_age = 0;               // Age of Battery (5.625s/bit)
        uint16_t raw_tte = 0;               // Time till Empty (5.625s/bit)
        uint16_t raw_ttf = 0;               // Time till Full (5.625s/bit)
        uint16_t raw_status = 0;            // Value in Status register

        // reported variables
        float rep_inst_current = 0;         // Instantaneous Current (mA)
        float rep_avg_current = 0;          // Average Current (mA)
        float rep_inst_voltage = 0;         // Instaneous Voltage (V)
        float rep_avg_voltage = 0;          // Average Voltage (V)
        int rep_capacity = 0;               // Report capacity (mAh)
        int rep_soc = 0;                    // State of Charge (%)
        int rep_age = 0;                    // Age of Battery (hours)
        int rep_tte = 0;                    // Time till Empty (hours)
        int rep_ttf = 0;                    // Time till Full (hours)
        
        // Battery info variables
        bool bat_status = false;            // Boolean for if a battery is present in the system
        bool bat_insert = false;            // Boolean for if a battery is inserted in the system
        bool bat_remove = false;            // Boolean for if a battery is removed from the system

        // learned variables
        uint16_t rcomp = 0;                 // Characterisation information for computing open-circuit voltage of cell
        uint16_t tempco = 0;                // Temperature compensation informtion for rcomp0
        uint16_t fullcap = 0;               // Capacity of the battery when full (mAh)
        uint16_t cycles = 0;                // Number of charge cycles
        uint16_t fullcapnorm = 0;           // Capacity of the battery when full, normalised (%)

        // methods
        // Initialise Fuel Gauge
        bool init(fuelgauge_config config, bool reset = true);

        // Get Battery Data (analog meausrements + modelguage outputs)
        void getBatteryData();

        // Get Analog Measurements
        void getvoltage();
        void getcurrent();

        // Get ModelGauge Outputs
        void getsoc();
        void getcapacity();
        void getage();
        void gettte();
        void getttf();

        // Get Battery info (Battery Presence in system, battery removal/insertion)
        void getBatteryInfo();

        // Get Status
        void getBatteryStatus();
        void getBatteryInsertion();
        void getBatteryRemoval();

        // Save learned parameters
        void getparameters();
        
        // Set some properties
        void setrsense(int rsense);
        void setdesigncap(int designcap);
        float getcapacityLSB();
        float getcurrentLSB();

    private:
        //variables
        uint8_t i2c_addr = 0x36;
        uint16_t HibCFG = 0;
        
        //Based on "Register Resolutions from MAX17055 Technical Reference" Table 6. 
        float base_capacity_multiplier_mAh = 5.0;
        float base_current_multiplier_mAh = 1.5625;
        float capacity_multiplier_mAH = 0.5; //refer to row "Capacity" (for 10mOhm)
        float current_multiplier_mV = 0.15625; //refer to row "Current" (for 10mOhm)
        float voltage_multiplier_V = 7.8125e-5; //refer to row "Voltage"
        float time_multiplier_Hours = 5.625/3600.0; //Least Significant Bit= 5.625 seconds, 3600 converts it to Hours.
        float percentage_multiplier = 1.0/256.0; //refer to row "Percentage"
        
        //methods
        void updateMultipliers();
        uint16_t readReg16Bit(uint8_t reg);
        void writeReg16Bit(uint8_t reg, uint16_t value);
        bool writeVerifyReg16Bit(uint8_t reg, uint16_t value);
};