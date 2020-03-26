#include "mapper.h"

mapper_device dev;
mapper_signal sigRawCapsense;
mapper_signal sigRawGyroX;
mapper_signal sigRawGyroY;
mapper_signal sigRawGyroZ;
mapper_signal sigRawAcclX;
mapper_signal sigRawAcclY;
mapper_signal sigRawAcclZ;
mapper_signal sigRawMagnX;
mapper_signal sigRawMagnY;
mapper_signal sigRawMagnZ;
mapper_signal sigRawFSR;
mapper_signal sigRawPiezo;
mapper_signal sigQuarternion1;
mapper_signal sigQuarternion2;
mapper_signal sigQuarternion3;
mapper_signal sigQuarternion4;
mapper_signal sigYaw;
mapper_signal sigPitch;
mapper_signal sigRoll;
mapper_signal sigMagGyro;
mapper_signal sigMagAccl;
mapper_signal sigMagMagn;
mapper_signal sigButton;
mapper_signal sigLongButton;
mapper_signal sigDoubleButton;

void initLibmapper() {

  char namespaceBuffer[30];
  snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"TStick_%i",Tstick.id);
  dev = mapper_device_new(namespaceBuffer, 0, 0);

  int rawCapsenseMin[(nCapsenses*16)] = { 0 };
  int rawCapsenseMax[(nCapsenses*16)];
    for (int i=0; i<(nCapsenses*16); ++i) {
      rawCapsenseMax[i] = 1;
    }
  float rawGyroMin[1] = { -34.90659f }, rawGyroMax[1] = { 34.90659f };
  int rawAcclMin[1] = { -32767 }, rawAcclMax[1] = { 32767 };
  int rawMagnMin[1] = { -32767 }, rawMagnMax[1] = { 32767 };
  int rawFSRMin[1] = { 0 }, rawFSRMax[1] = { 4095 };
  int rawPiezoMin[1] = { 0 }, rawPiezoMax[1] = { 1023 };
  float quarternionMin[1] = { -1.0f }, quarternionMax[1] = { 1.0f };
  float magMin[1] = { 0.0f }, magMax[1] = { 1.7320508f };  // sqrt(3)
  int buttonMin[1] = { 0 }, buttonMax[1] = { 1 };
  float yprMin[1] = { -180.0f }, yprMax[1] = { 180.0f };

  sigRawCapsense = mapper_device_add_output_signal(dev, "raw/capsense", (nCapsenses*16), 'i', NULL, rawCapsenseMin, rawCapsenseMax);
  sigRawGyroX = mapper_device_add_output_signal(dev, "raw/gyro/X", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawGyroY = mapper_device_add_output_signal(dev, "raw/gyro/Y", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawGyroZ = mapper_device_add_output_signal(dev, "raw/gyro/Z", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawAcclX = mapper_device_add_output_signal(dev, "raw/accl/X", 1, 'i', NULL, rawAcclMin, rawAcclMax);
  sigRawAcclY = mapper_device_add_output_signal(dev, "raw/accl/Y", 1, 'i', NULL, rawAcclMin, rawAcclMax);
  sigRawAcclZ = mapper_device_add_output_signal(dev, "raw/accl/Z", 1, 'i', NULL, rawAcclMin, rawAcclMax);
  sigRawMagnX = mapper_device_add_output_signal(dev, "raw/magn/X", 1, 'i', NULL, rawMagnMin, rawMagnMax);
  sigRawMagnY = mapper_device_add_output_signal(dev, "raw/magn/Y", 1, 'i', NULL, rawMagnMin, rawMagnMax);
  sigRawMagnZ = mapper_device_add_output_signal(dev, "raw/magn/Z", 1, 'i', NULL, rawMagnMin, rawMagnMax);
  sigRawFSR = mapper_device_add_output_signal(dev, "raw/fsr", 1, 'i', NULL, rawFSRMin, rawFSRMax);
  sigRawPiezo = mapper_device_add_output_signal(dev, "raw/piezo", 1, 'i', NULL, rawPiezoMin, rawPiezoMax);
  sigQuarternion1 = mapper_device_add_output_signal(dev, "quarternion/1", 1, 'f', NULL, quarternionMin, quarternionMax);
  sigQuarternion2 = mapper_device_add_output_signal(dev, "quarternion/2", 1, 'f', NULL, quarternionMin, quarternionMax);
  sigQuarternion3 = mapper_device_add_output_signal(dev, "quarternion/3", 1, 'f', NULL, quarternionMin, quarternionMax);
  sigQuarternion4 = mapper_device_add_output_signal(dev, "quarternion/4", 1, 'f', NULL, quarternionMin, quarternionMax);
  sigYaw = mapper_device_add_output_signal(dev, "euler/yaw", 1, 'f', NULL, yprMin, yprMax);
  sigPitch = mapper_device_add_output_signal(dev, "euler/pitch", 1, 'f', NULL, yprMin, yprMax);
  sigRoll = mapper_device_add_output_signal(dev, "euler/roll", 1, 'f', NULL, yprMin, yprMax);
  sigMagGyro = mapper_device_add_output_signal(dev, "gyro/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagAccl = mapper_device_add_output_signal(dev, "accl/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagMagn = mapper_device_add_output_signal(dev, "magn/magnitude", 1, 'f', NULL, magMin, magMax);
  sigButton = mapper_device_add_output_signal(dev, "button/short", 1, 'i', NULL, buttonMin, buttonMax);
  sigLongButton = mapper_device_add_output_signal(dev, "button/long", 1, 'i', NULL, buttonMin, buttonMax);
  sigDoubleButton = mapper_device_add_output_signal(dev, "button/double", 1, 'i', NULL, buttonMin, buttonMax);
}

void updateLibmapper() {
  mapper_device_poll(dev, 0);

  int touchtemp[(nCapsenses*16)];
  byte counter = 0;
  byte indexStrip = 0;
  for (byte i=0; i < (nCapsenses*8); ++i) {
    touchtemp[i] = bitRead(RawData.touch[i/16][indexStrip],(7-(i%8)));
    if (++counter >= 8) {
      if (indexStrip==0){
        indexStrip=1;
        }
      else {
        indexStrip=0;
        }
      counter = 0;
    }
  }

  mapper_signal_update(sigRawCapsense, touchtemp, sizeof(touchtemp), MAPPER_NOW);
  mapper_signal_update_int(sigRawFSR, RawData.fsr);
  mapper_signal_update_int(sigRawPiezo, RawData.piezo);
  mapper_signal_update_int(sigRawAcclX, RawData.accl[0]);
  mapper_signal_update_int(sigRawAcclY, RawData.accl[1]);
  mapper_signal_update_int(sigRawAcclZ, RawData.accl[2]);
  mapper_signal_update_float(sigRawGyroX, RawData.gyro[0]);
  mapper_signal_update_float(sigRawGyroY, RawData.gyro[1]);
  mapper_signal_update_float(sigRawGyroZ, RawData.gyro[2]);
  mapper_signal_update_int(sigRawMagnX, RawData.magn[0]);
  mapper_signal_update_int(sigRawMagnY, RawData.magn[1]);
  mapper_signal_update_int(sigRawMagnZ, RawData.magn[2]);
  mapper_signal_update_float(sigQuarternion1, RawData.quat[0]);
  mapper_signal_update_float(sigQuarternion2, RawData.quat[1]);
  mapper_signal_update_float(sigQuarternion3, RawData.quat[2]);
  mapper_signal_update_float(sigQuarternion4, RawData.quat[3]);
  mapper_signal_update_float(sigMagGyro, RawData.magGyro);
  mapper_signal_update_float(sigMagAccl, RawData.magAccl);
  mapper_signal_update_float(sigMagMagn, RawData.magMagn);
  mapper_signal_update_int(sigButton, RawData.buttonShort);
  mapper_signal_update_int(sigLongButton, RawData.buttonLong);
  mapper_signal_update_int(sigDoubleButton, RawData.buttonDouble);
  mapper_signal_update_float(sigYaw, RawData.ypr[0]);
  mapper_signal_update_float(sigPitch, RawData.ypr[1]);
  mapper_signal_update_float(sigRoll, RawData.ypr[2]);
}
