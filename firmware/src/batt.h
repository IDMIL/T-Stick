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
        Status      = 0x00, //Maintains all flags related to alert thresholds and battery insertion or removal.
        Age         = 0x07, //calculated percentage value of capacity compared to original design capacity.
        Temperature = 0x08, //Temperature of MAX17055 chip
        VCell       = 0x09, //VCell reports the voltage measured between BATT and CSP.
        AvgVCell    = 0x19, //The AvgVCell register reports an average of the VCell register readings. 
        Current     = 0x0A, //Voltage between the CSP and CSN pins, and would need to convert to current
        AvgCurrent  = 0x0B, //The AvgCurrent register reports an average of Current register readings
        RepSOC      = 0x06, //The Reported State of Charge of connected battery. Refer to AN6358 page 23 and 13
        RepCap      = 0x05, //Reported Capacity. Refer to page 23 and 13 of AN6358.
        TimeToEmpty = 0x11, //How long before battery is empty (in ms). Refer to page 24 and 13 of AN6358 
        DesignCap   = 0x18, //Capacity of battery inserted, not typically used for user requested capacity
        Vempty      = 0x3A, //Register for voltage when the battery is empty
        dQAcc      = 0x45 //Register for voltage when the battery is empty
        };
        // raw variables
        uint16_t current = 0;
        float voltage = 0;
        uint16_t capacity = 0;

        // SOC
        uint16_t raw_soc = 0;
        uint16_t cooked_soc = 0; 

        // Learned variables
        uint16_t age = 0;
        uint16_t tte = 0;
        uint16_t chargecycles = 0;
        uint16_t fullcapnorm = 0;


        // methods
        bool init(int designcap, float rsense, float vempty);
        void setresistsensor(float rsense);
        float getresistsensor();
        void updateMultipliers();
        void getBatteryData();

    private:
        //variables
        float resistSensor = 0.01; //default internal resist sensor
        uint8_t I2CAddress = 0x36;
        uint16_t HibCFG = 0;
        
        //Based on "Standard Register Formats" AN6358, figure 1.3. 
        //Multipliers are constants used to multiply register value in order to get final result
        float capacity_multiplier_mAH = (5e-3)/resistSensor; //refer to row "Capacity"
        float current_multiplier_mV = (1.5625e-3)/resistSensor; //refer to row "Current"
        float voltage_multiplier_V = 7.8125e-5; //refer to row "Voltage"
        float time_multiplier_Hours = 5.625/3600.0; //Least Significant Bit= 5.625 seconds, 3600 converts it to Hours. refer to AN6358 pg 13 figure 1.3 in row "Time"
        float percentage_multiplier = 1.0/256.0; //refer to row "Percentage"
        
        //methods
        uint16_t readReg16Bit(uint8_t reg);
        uint16_t readReg16HighBit(uint8_t reg);
        void writeReg16Bit(uint8_t reg, uint16_t value);
};