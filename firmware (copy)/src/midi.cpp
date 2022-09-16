/* To use in any project:
 * - include the file (#include "midi.h")
 * - Adjust definitions accordingly using the struct in this file
 * - When needed, use the definitions by calling the structure (E.g. Pin.led)
 */

#include "midi.h"

BLECharacteristic *pCharacteristic;
bool MIDIdeviceConnected = false;

// Check connection status
  class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      MIDIdeviceConnected = true; // from main file bool
    };
    void onDisconnect(BLEServer* pServer) {
      MIDIdeviceConnected = false;
    };
  };

bool Midi::status(){
  return MIDIdeviceConnected;
}

// Initializing BLE MIDI  

  bool Midi::initMIDI() {
    BLEDevice::init(Midi::deviceName);
    // Create the BLE Server
      BLEServer *pServer = BLEDevice::createServer();
      pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
      BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));
    // Create a BLE Characteristic
      pCharacteristic = pService->createCharacteristic(
        BLEUUID(CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_WRITE_NR
      );
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
      pCharacteristic->addDescriptor(new BLE2902());
    // Start the service
      pService->start();
    // Start advertising
      BLEAdvertising *pAdvertising = pServer->getAdvertising();
      pAdvertising->addServiceUUID(pService->getUUID());
      pAdvertising->start();

      return true;
  };

// Mount Status byte to send MIDI messages
  uint8_t Midi::mountStatusByte(uint8_t type, uint8_t channel) {
    // Status Byte (8 bits): 1 + type (3 bits) + channel (4 bits)
    uint8_t result = 0b10000000 | (type << 4) | channel;
    return result;
  };

// Functions to change range for sending MIDI messages
  int Midi::mapMIDI(float x, float in_min, float in_max, float out_min, float out_max) {
    float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if (result > out_max) {
      result = out_max;
    };
    if (result < out_min) {
      result = out_min;
    };
    return static_cast<int>(result);
  };
  int Midi::mapMIDI(float x, float in_min, float in_max) {
    int result = (x - in_min) * 127 / (in_max - in_min);
    if (result > 127) {
      result = 127;
    };
    if (result < 0) {
      result = 0;
    };
    return static_cast<int>(result);
  };

// Send note ON
  void Midi::noteON (uint8_t note, uint8_t velocity) {
    midiPacket[2] = mountStatusByte(Midi::noteOn, Midi::channel); // note up, channel
    midiPacket[3] = note;
    midiPacket[4] = velocity;
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  };

// Send note OFF
void Midi::noteOFF (uint8_t note) {
    midiPacket[2] = mountStatusByte(Midi::noteOff, Midi::channel); // note up, channel
    midiPacket[3] = note;
    midiPacket[4] = 0;
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  };

// Send MIDI CC
  void Midi::CC (uint8_t controlNumber, uint8_t value) {
    midiPacket[2] = mountStatusByte(Midi::controlChange, Midi::channel); // CC, channel
    midiPacket[3] = controlNumber;
    midiPacket[4] = value;
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
    pCharacteristic->notify();
  };

  // Send MIDI CC "bundle"

  void Midi::CCbundle ( uint8_t controlNumber1, uint8_t value1, 
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
                        uint8_t controlNumber15, uint8_t value15) {
    uint8_t intMidiPacket[61] = {
      0b10000000,  // header
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber1, value1,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber2, value2, 
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber3, value3,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber4, value4,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber5, value5,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber6, value6,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber7, value7,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber8, value8,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber9, value9,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber10, value10,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber11, value11,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber12, value12,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber13, value13,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber14, value14,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber15, value15
    };
    pCharacteristic->setValue(intMidiPacket, 61);
    pCharacteristic->notify();
  };

  void Midi::CCbundle9 ( uint8_t controlNumber1, uint8_t value1, 
                        uint8_t controlNumber2, uint8_t value2, 
                        uint8_t controlNumber3, uint8_t value3,
                        uint8_t controlNumber4, uint8_t value4,
                        uint8_t controlNumber5, uint8_t value5,
                        uint8_t controlNumber6, uint8_t value6,
                        uint8_t controlNumber7, uint8_t value7,
                        uint8_t controlNumber8, uint8_t value8,
                        uint8_t controlNumber9, uint8_t value9) {
    uint8_t intMidiPacket[37] = {
      0b10000000,  // header
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber1, value1,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber2, value2, 
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber3, value3,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber4, value4,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber5, value5,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber6, value6,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber7, value7,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber8, value8,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber9, value9
    };
    pCharacteristic->setValue(intMidiPacket, 37);
    pCharacteristic->notify();
  };

  void Midi::CCbundle3 (uint8_t controlNumber1, uint8_t value1, 
                        uint8_t controlNumber2, uint8_t value2, 
                        uint8_t controlNumber3, uint8_t value3) {
    uint8_t intMidiPacket[13] = {
      0b10000000,  // header
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber1, value1,
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber2, value2, 
      0b10000000,  // timestamp, not implemented
      mountStatusByte(Midi::controlChange, Midi::channel), // CC, channel
      controlNumber3, value3
    };
    pCharacteristic->setValue(intMidiPacket, 13);
    pCharacteristic->notify();
  };

  int Midi::setDeviceName(const char* name) {
    strlcpy(Midi::deviceName, name, sizeof(Midi::deviceName));
    return 1;
  };

  bool Midi::setChannel(uint8_t channel) {
    Midi::channel = channel;
    return true;
  };
    
  uint8_t Midi::getChannel() {
    return Midi::channel;
  };
