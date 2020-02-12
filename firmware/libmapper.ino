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
mapper_signal sigOrientationQ1;
mapper_signal sigOrientationQ2;
mapper_signal sigOrientationQ3;
mapper_signal sigOrientationQ4;
mapper_signal sigMagGyro;
mapper_signal sigMagAccl;
mapper_signal sigMagMagn;
mapper_signal sigButton;

void initLibmapper() {
  dev = mapper_device_new("T-Stick", 0, 0);

  int rawCapsenseMin[16] = { 0 }, rawCapsenseMax[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  float rawGyroMin[1] = { -1.0f }, rawGyroMax[1] = { 1.0f };
  float rawAcclMin[1] = { -1.0f }, rawAcclMax[1] = { 1.0f };
  float rawMagnMin[1] = { -1.0f }, rawMagnMax[1] = { 1.0f };
  float rawFSRMin[1] = { 0.0f }, rawFSRMax[1] = { 1.0f };
  float rawPiezoMin[1] = { 0.0f }, rawPiezoMax[1] = { 1.0f };
  float orientationMin[1] = { -1.0f }, orientationMax[1] = { 1.0f };
  float magMin[1] = { 0.0f }, magMax[1] = { 1.7320508 };  // sqrt(3)
  int buttonMin[1] = { 0 }, buttonMax[1] = { 1 };

  sigRawCapsense = mapper_device_add_output_signal(dev, "raw/capsense", 16, 'i', NULL, rawCapsenseMin, rawCapsenseMax);
  sigRawGyroX = mapper_device_add_output_signal(dev, "raw/gyro/X", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawGyroY = mapper_device_add_output_signal(dev, "raw/gyro/Y", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawGyroZ = mapper_device_add_output_signal(dev, "raw/gyro/Z", 1, 'f', NULL, rawGyroMin, rawGyroMax);
  sigRawAcclX = mapper_device_add_output_signal(dev, "raw/accl/X", 1, 'f', NULL, rawAcclMin, rawAcclMax);
  sigRawAcclY = mapper_device_add_output_signal(dev, "raw/accl/Y", 1, 'f', NULL, rawAcclMin, rawAcclMax);
  sigRawAcclZ = mapper_device_add_output_signal(dev, "raw/accl/Z", 1, 'f', NULL, rawAcclMin, rawAcclMax);
  sigRawMagnX = mapper_device_add_output_signal(dev, "raw/magn/X", 1, 'f', NULL, rawMagnMin, rawMagnMax);
  sigRawMagnY = mapper_device_add_output_signal(dev, "raw/magn/Y", 1, 'f', NULL, rawMagnMin, rawMagnMax);
  sigRawMagnZ = mapper_device_add_output_signal(dev, "raw/magn/Z", 1, 'f', NULL, rawMagnMin, rawMagnMax);
  sigRawFSR = mapper_device_add_output_signal(dev, "raw/fsr", 1, 'f', NULL, rawFSRMin, rawFSRMax);
  sigRawPiezo = mapper_device_add_output_signal(dev, "raw/piezo", 1, 'f', NULL, rawPiezoMin, rawPiezoMax);
  sigOrientationQ1 = mapper_device_add_output_signal(dev, "orientation/q1", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ2 = mapper_device_add_output_signal(dev, "orientation/q2", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ3 = mapper_device_add_output_signal(dev, "orientation/q3", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ4 = mapper_device_add_output_signal(dev, "orientation/q4", 1, 'f', NULL, orientationMin, orientationMax);
  sigMagGyro = mapper_device_add_output_signal(dev, "gyro/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagAccl = mapper_device_add_output_signal(dev, "accl/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagMagn = mapper_device_add_output_signal(dev, "magn/magnitude", 1, 'f', NULL, magMin, magMax);
  sigButton = mapper_device_add_output_signal(dev, "button", 1, 'i', NULL, buttonMin, buttonMax);
}

void updateLibmapper() {
  mapper_device_poll(dev, 0);

  mapper_signal_update(sigRawCapsense, Data.touch16, 16, MAPPER_NOW);
  mapper_signal_update_float(sigRawFSR, Data.fsr);
  mapper_signal_update_float(sigRawPiezo, Data.piezo);
  mapper_signal_update_float(sigRawAcclX, Data.accl[0]);
  mapper_signal_update_float(sigRawAcclY, Data.accl[1]);
  mapper_signal_update_float(sigRawAcclZ, Data.accl[2]);
  mapper_signal_update_float(sigRawGyroX, Data.gyro[0]);
  mapper_signal_update_float(sigRawGyroY, Data.gyro[1]);
  mapper_signal_update_float(sigRawGyroZ, Data.gyro[2]);
  mapper_signal_update_float(sigRawMagnX, Data.magn[0]);
  mapper_signal_update_float(sigRawMagnY, Data.magn[1]);
  mapper_signal_update_float(sigRawMagnZ, Data.magn[2]);
  mapper_signal_update_float(sigOrientationQ1, Data.quat[0]);
  mapper_signal_update_float(sigOrientationQ2, Data.quat[1]);
  mapper_signal_update_float(sigOrientationQ3, Data.quat[2]);
  mapper_signal_update_float(sigOrientationQ4, Data.quat[3]);
  mapper_signal_update_float(sigMagGyro, Data.magGyro);
  mapper_signal_update_float(sigMagAccl, Data.magAccl);
  mapper_signal_update_float(sigMagMagn, Data.magMagn);

  mapper_signal_update_int(sigButton, !buttonState);

  // Missing ypr
}
