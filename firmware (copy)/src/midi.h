
/* To use in any project:
 * - include the file (#include "midi.h")
 * - Adjust definitions accordingly using the struct in this file
 * - When needed, use the definitions by calling the structure (E.g. Pin.led)
 */

#ifndef MIDI_H
#define MIDI_H

  #include <Arduino.h>

  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>
  #include <BLE2902.h>

// MIDI definitions

class Midi {
    private:
      char deviceName[21] = "Default";
      uint8_t channel = 0; // some instances see that as channel 1 (e.g., PD)
      const uint8_t controlChange = 0b011;
      const uint8_t noteOff = 0b000;
      const uint8_t noteOn = 0b001;
      uint8_t midiPacket[5] = {
        0b10000000,  // header
        0b10000000,  // timestamp, not implemented 
        0b00000000,  // status byte
        0b00010100,  // note (or midi_control_number). Default: 20
        0x00000000   // velocity
      };
    public:
      bool initMIDI();
      uint8_t mountStatusByte(uint8_t type, uint8_t channel);
      int mapMIDI(float x, float in_min, float in_max, float out_min, float out_max);
      int mapMIDI(float x, float in_min, float in_max);
      void noteON (uint8_t note, uint8_t velocity);
      void noteOFF (uint8_t note);
      void CC (uint8_t controlNumber, uint8_t value);
      int setDeviceName(const char* name);
      bool setChannel(uint8_t channel);
      uint8_t getChannel();
      void CCbundle ( uint8_t controlNumber1, uint8_t value1, 
                      uint8_t controlNumber2, uint8_t value2, 
                      uint8_t controlNumber3, uint8_t value3,
                      uint8_t controlNumber4, uint8_t value4,
                      uint8_t controlNumber5, uint8_t value5,
                      uint8_t controlNumber6, uint8_t value6,
                      uint8_t controlNumber7, uint8_t value7,
                      uint8_t controlNumber8, uint8_t value8,
                      uint8_t controlNumber9, uint8_t value9,
                      uint8_t controlNumber10, uint8_t value10,
                      uint8_t controlNumber11, uint8_t value11,
                      uint8_t controlNumber12, uint8_t value12,
                      uint8_t controlNumber13, uint8_t value13,
                      uint8_t controlNumber14, uint8_t value14,
                      uint8_t controlNumber15, uint8_t value15);
      void CCbundle9 ( uint8_t controlNumber1, uint8_t value1, 
                      uint8_t controlNumber2, uint8_t value2, 
                      uint8_t controlNumber3, uint8_t value3,
                      uint8_t controlNumber4, uint8_t value4,
                      uint8_t controlNumber5, uint8_t value5,
                      uint8_t controlNumber6, uint8_t value6,
                      uint8_t controlNumber7, uint8_t value7,
                      uint8_t controlNumber8, uint8_t value8,
                      uint8_t controlNumber9, uint8_t value9);
      void CCbundle3 ( uint8_t controlNumber1, uint8_t value1, 
                      uint8_t controlNumber2, uint8_t value2, 
                      uint8_t controlNumber3, uint8_t value3);
      bool status();
};

// The MIDI Service standard UUIDs
  #define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
  #define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"


#endif