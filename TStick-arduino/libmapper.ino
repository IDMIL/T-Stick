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
mapper_signal sigYaw;
mapper_signal sigPitch;
mapper_signal sigRoll;
mapper_signal sigMagGyro;
mapper_signal sigMagAccl;
mapper_signal sigMagMagn;
mapper_signal sigButton;
mapper_signal sigLongButton;
mapper_signal sigDoubleButton;
mapper_signal sigtouchAll;
mapper_signal sigtouchTop;
mapper_signal sigtouchMiddle;
mapper_signal sigtouchBottom;
mapper_signal sigBrush;
mapper_signal sigRub;
mapper_signal sigMultiBrush;
mapper_signal sigMultiRub;
mapper_signal sigShakeXYZ;
mapper_signal sigJabXYZ;



// TODO: touchAll, touchTop, touchMiddle, touchBottom, brush, rub, multibrush, multirub, shakeXYZ

void initLibmapper() {

  char namespaceBuffer[30];
  snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"TStick_%i",Tstick.id);
  dev = mapper_device_new(namespaceBuffer, 0, 0);

  int rawCapsenseMin[(touchStripsSize)] = { 0 };
  int rawCapsenseMax[(touchStripsSize)];
    for (int i=0; i<(touchStripsSize); ++i) {
      rawCapsenseMax[i] = 1;
    }
  float rawGyroMin[1] = { -34.90659f }, rawGyroMax[1] = { 34.90659f };
  int rawAcclMin[1] = { -32767 }, rawAcclMax[1] = { 32767 };
  int rawMagnMin[1] = { -32767 }, rawMagnMax[1] = { 32767 };
  int rawFSRMin[1] = { 0 }, rawFSRMax[1] = { 4095 };
  int rawPiezoMin[1] = { 0 }, rawPiezoMax[1] = { 1023 };
  float orientationMin[1] = { -1.0f }, orientationMax[1] = { 1.0f };
  float magMin[1] = { 0.0f }, magMax[1] = { 1.7320508f };  // sqrt(3)
  int buttonMin[1] = { 0 }, buttonMax[1] = { 1 };
  float yprMin[1] = { -180.0f }, yprMax[1] = { 180.0f };
  float instTouchMin[1] = { 0.0f }, instTouchMax[1] = { 1.0f };
  float genericMin[1] = { 0.0f }, genericMax[1] = { 100.0f };


  sigRawCapsense = mapper_device_add_output_signal(dev, "raw/capsense", touchStripsSize, 'i', NULL, rawCapsenseMin, rawCapsenseMax);
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
  sigOrientationQ1 = mapper_device_add_output_signal(dev, "orientation/q1", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ2 = mapper_device_add_output_signal(dev, "orientation/q2", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ3 = mapper_device_add_output_signal(dev, "orientation/q3", 1, 'f', NULL, orientationMin, orientationMax);
  sigOrientationQ4 = mapper_device_add_output_signal(dev, "orientation/q4", 1, 'f', NULL, orientationMin, orientationMax);
  sigYaw = mapper_device_add_output_signal(dev, "orientation/yaw", 1, 'f', NULL, yprMin, yprMax);
  sigPitch = mapper_device_add_output_signal(dev, "orientation/pitch", 1, 'f', NULL, yprMin, yprMax);
  sigRoll = mapper_device_add_output_signal(dev, "orientation/roll", 1, 'f', NULL, yprMin, yprMax);
  sigMagGyro = mapper_device_add_output_signal(dev, "gyro/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagAccl = mapper_device_add_output_signal(dev, "accl/magnitude", 1, 'f', NULL, magMin, magMax);
  sigMagMagn = mapper_device_add_output_signal(dev, "magn/magnitude", 1, 'f', NULL, magMin, magMax);
  sigButton = mapper_device_add_output_signal(dev, "button/short", 1, 'i', NULL, buttonMin, buttonMax);
  sigLongButton = mapper_device_add_output_signal(dev, "button/long", 1, 'i', NULL, buttonMin, buttonMax);
  sigDoubleButton = mapper_device_add_output_signal(dev, "button/double", 1, 'i', NULL, buttonMin, buttonMax);
  sigtouchAll = mapper_device_add_output_signal(dev, "instrument/touchall", 1, 'f', NULL, instTouchMin, instTouchMax);
  sigtouchTop = mapper_device_add_output_signal(dev, "instrument/touchtop", 1, 'f', NULL, instTouchMin, instTouchMax);
  sigtouchMiddle = mapper_device_add_output_signal(dev, "instrument/touchmiddle", 1, 'f', NULL, instTouchMin, instTouchMax);
  sigtouchBottom = mapper_device_add_output_signal(dev, "instrument/touchbottom", 1, 'f', NULL, instTouchMin, instTouchMax);
  sigBrush = mapper_device_add_output_signal(dev, "instrument/brush", 1, 'f', NULL, genericMin, genericMax);
  sigRub = mapper_device_add_output_signal(dev, "instrument/rub", 1, 'f', NULL, genericMin, genericMax);
  sigMultiBrush = mapper_device_add_output_signal(dev, "instrument/multibrush", 4, 'f', NULL, genericMin, genericMax);
  sigMultiRub = mapper_device_add_output_signal(dev, "instrument/multirub", 4, 'f', NULL, genericMin, genericMax);
  sigShakeXYZ = mapper_device_add_output_signal(dev, "instrument/shakexyz", 3, 'f', NULL, genericMin, genericMax);
  sigJabXYZ = mapper_device_add_output_signal(dev, "instrument/jabxyz", 3, 'f', NULL, genericMin, genericMax);
}

void updateLibmapper() {
  mapper_device_poll(dev, 0);

  mapper_signal_update(sigRawCapsense, RawData.touchStrips, sizeof(RawData.touchStrips), MAPPER_NOW);
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
  mapper_signal_update_float(sigOrientationQ1, RawData.quat[0]);
  mapper_signal_update_float(sigOrientationQ2, RawData.quat[1]);
  mapper_signal_update_float(sigOrientationQ3, RawData.quat[2]);
  mapper_signal_update_float(sigOrientationQ4, RawData.quat[3]);
  mapper_signal_update_float(sigMagGyro, RawData.magGyro);
  mapper_signal_update_float(sigMagAccl, RawData.magAccl);
  mapper_signal_update_float(sigMagMagn, RawData.magMagn);
  mapper_signal_update_int(sigButton, RawData.buttonShort);
  mapper_signal_update_int(sigLongButton, RawData.buttonLong);
  mapper_signal_update_int(sigDoubleButton, RawData.buttonDouble);
  mapper_signal_update_float(sigYaw, InstrumentData.ypr[0]);
  mapper_signal_update_float(sigPitch, InstrumentData.ypr[1]);
  mapper_signal_update_float(sigRoll, InstrumentData.ypr[2]);
  mapper_signal_update_float(sigtouchAll, InstrumentData.touchAll);
  mapper_signal_update_float(sigtouchTop, InstrumentData.touchTop);
  mapper_signal_update_float(sigtouchMiddle, InstrumentData.touchMiddle);
  mapper_signal_update_float(sigtouchBottom, InstrumentData.touchBottom);
  mapper_signal_update_float(sigBrush, InstrumentData.brush);
  mapper_signal_update_float(sigRub, InstrumentData.rub);
  mapper_signal_update(sigMultiBrush, RawData.touchStrips, sizeof(InstrumentData.multiBrush)/sizeof(InstrumentData.multiBrush[0]), MAPPER_NOW);
  mapper_signal_update(sigMultiRub, RawData.touchStrips, sizeof(InstrumentData.multiRub)/sizeof(InstrumentData.multiRub[0]), MAPPER_NOW);
  mapper_signal_update(sigShakeXYZ, RawData.touchStrips, sizeof(InstrumentData.shakeXYZ)/sizeof(InstrumentData.shakeXYZ[0]), MAPPER_NOW);
  mapper_signal_update(sigJabXYZ, RawData.touchStrips, sizeof(InstrumentData.jabXYZ)/sizeof(InstrumentData.jabXYZ[0]), MAPPER_NOW);
}
