#ifdef LIBMAPPER

#include <mapper_cpp.h>
#include <string>
#include <algorithm>
#include<vector>

mapper::Device* dev;

struct Lm {
  int rawCapsenseMin = 0, rawCapsenseMax = 1;
  float rawGyroMin = -34.90659, rawGyroMax = 34.90659;
  int rawAcclMin = -32767, rawAcclMax = 32767;
  int rawMagnMin = -32767, rawMagnMax = 32767;
  int rawFSRMin = 0, rawFSRMax = 4095;
  int rawPiezoMin = 0, rawPiezoMax = 1023;
  float orientationMin = -1.0, orientationMax = 1.0;
  float magMin = 0.0, magMax = 1.7320508;  // sqrt(3)
  int buttonMin = 0, buttonMax = 1;
  float yprMin = -180.0, yprMax = 180.0;
  float instTouchMin = 0.0, instTouchMax = 1.0;
  float genericMin = 0.0, genericMax = 100.0;
  mapper::Signal sigRawCapsense;
  mapper::Signal sigRawGyroX;
  mapper::Signal sigRawGyroY;
  mapper::Signal sigRawGyroZ;
  mapper::Signal sigRawAcclX;
  mapper::Signal sigRawAcclY;
  mapper::Signal sigRawAcclZ;
  mapper::Signal sigRawMagnX;
  mapper::Signal sigRawMagnY;
  mapper::Signal sigRawMagnZ;
  mapper::Signal sigRawFSR;
  mapper::Signal sigRawPiezo;
  mapper::Signal sigOrientationQ1;
  mapper::Signal sigOrientationQ2;
  mapper::Signal sigOrientationQ3;
  mapper::Signal sigOrientationQ4;
  mapper::Signal sigYaw;
  mapper::Signal sigPitch;
  mapper::Signal sigRoll;
  mapper::Signal sigMagGyro;
  mapper::Signal sigMagAccl;
  mapper::Signal sigMagMagn;
  mapper::Signal sigButton;
  mapper::Signal sigLongButton;
  mapper::Signal sigDoubleButton;
  mapper::Signal sigtouchAll;
  mapper::Signal sigtouchTop;
  mapper::Signal sigtouchMiddle;
  mapper::Signal sigtouchBottom;
  mapper::Signal sigBrush;
  mapper::Signal sigRub;
  mapper::Signal sigMultiBrush;
  mapper::Signal sigMultiRub;
  mapper::Signal sigShakeXYZ;
  mapper::Signal sigJabXYZ;
} lm;

void initLibmapper() {

  // Create device for libmapper
  if (WiFi.status() == WL_CONNECTED) {
      std::string lm_name = tstickSSID;
      dev = new mapper::Device(lm_name);
  }

  // output = dev->add_signal(
  //              Direction::OUTGOING, 
  //              "my_output", // a unique name for the signal
  //              4,           // signal's vector length
  //              Type::INT32, // the signal's data type, one of Type::INT32, Type::FLOAT, or Type::DOUBLE
  //              "m/s",       // the signal's unit (optional). Use 0 (without quotes) if not specified
  //              min,         // the signal's minimum value (optional)
  //              max);        // the signal's maximum value (optional)

  int touchlmSize = touchStripsSize*trillamount;
  lm.sigRawCapsense = dev->add_signal(mapper::Direction::OUTGOING, "raw/capsense", touchlmSize, mapper::Type::INT32, 0, &lm.rawCapsenseMin, &lm.rawCapsenseMax);
  lm.sigRawGyroX = dev->add_signal(mapper::Direction::OUTGOING, "raw/gyro/X", 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  lm.sigRawGyroY = dev->add_signal(mapper::Direction::OUTGOING, "raw/gyro/Y", 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  lm.sigRawGyroZ = dev->add_signal(mapper::Direction::OUTGOING, "raw/gyro/Z", 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  lm.sigRawAcclX = dev->add_signal(mapper::Direction::OUTGOING, "raw/accl/X", 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  lm.sigRawAcclY = dev->add_signal(mapper::Direction::OUTGOING, "raw/accl/Y", 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  lm.sigRawAcclZ = dev->add_signal(mapper::Direction::OUTGOING, "raw/accl/Z", 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  lm.sigRawMagnX = dev->add_signal(mapper::Direction::OUTGOING, "raw/magn/X", 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  lm.sigRawMagnY = dev->add_signal(mapper::Direction::OUTGOING, "raw/magn/Y", 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  lm.sigRawMagnZ = dev->add_signal(mapper::Direction::OUTGOING, "raw/magn/Z", 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  lm.sigRawFSR = dev->add_signal(mapper::Direction::OUTGOING, "raw/fsr", 1, mapper::Type::INT32, 0, &lm.rawFSRMin, &lm.rawFSRMax);
  lm.sigRawPiezo = dev->add_signal(mapper::Direction::OUTGOING, "raw/piezo", 1, mapper::Type::INT32, 0, &lm.rawPiezoMin, &lm.rawPiezoMax);
  lm.sigOrientationQ1 = dev->add_signal(mapper::Direction::OUTGOING, "orientation/q1", 1, mapper::Type::FLOAT, 0, &lm.orientationMin, &lm.orientationMax);
  lm.sigOrientationQ2 = dev->add_signal(mapper::Direction::OUTGOING, "orientation/q2", 1, mapper::Type::FLOAT, 0, &lm.orientationMin, &lm.orientationMax);
  lm.sigOrientationQ3 = dev->add_signal(mapper::Direction::OUTGOING, "orientation/q3", 1, mapper::Type::FLOAT, 0, &lm.orientationMin, &lm.orientationMax);
  lm.sigOrientationQ4 = dev->add_signal(mapper::Direction::OUTGOING, "orientation/q4", 1, mapper::Type::FLOAT, 0, &lm.orientationMin, &lm.orientationMax);
  lm.sigYaw = dev->add_signal(mapper::Direction::OUTGOING, "orientation/yaw", 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  lm.sigPitch = dev->add_signal(mapper::Direction::OUTGOING, "orientation/pitch", 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  lm.sigRoll = dev->add_signal(mapper::Direction::OUTGOING, "orientation/roll", 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  lm.sigMagGyro = dev->add_signal(mapper::Direction::OUTGOING, "gyro/magnitude", 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  lm.sigMagAccl = dev->add_signal(mapper::Direction::OUTGOING, "accl/magnitude", 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  lm.sigMagMagn = dev->add_signal(mapper::Direction::OUTGOING, "magn/magnitude", 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  lm.sigButton = dev->add_signal(mapper::Direction::OUTGOING, "button/short", 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  lm.sigLongButton = dev->add_signal(mapper::Direction::OUTGOING, "button/long", 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  lm.sigDoubleButton = dev->add_signal(mapper::Direction::OUTGOING, "button/double", 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  lm.sigtouchAll = dev->add_signal(mapper::Direction::OUTGOING, "instrument/touchall", 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  lm.sigtouchTop = dev->add_signal(mapper::Direction::OUTGOING, "instrument/touchtop", 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  lm.sigtouchMiddle = dev->add_signal(mapper::Direction::OUTGOING, "instrument/touchmiddle", 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  lm.sigtouchBottom = dev->add_signal(mapper::Direction::OUTGOING, "instrument/touchbottom", 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  lm.sigBrush = dev->add_signal(mapper::Direction::OUTGOING, "instrument/brush", 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  lm.sigRub = dev->add_signal(mapper::Direction::OUTGOING, "instrument/rub", 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  lm.sigMultiBrush = dev->add_signal(mapper::Direction::OUTGOING, "instrument/multibrush", 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  lm.sigMultiRub = dev->add_signal(mapper::Direction::OUTGOING, "instrument/multirub", 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  lm.sigShakeXYZ = dev->add_signal(mapper::Direction::OUTGOING, "instrument/shakexyz", 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  lm.sigJabXYZ = dev->add_signal(mapper::Direction::OUTGOING, "instrument/jabxyz", 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
}

void updateLibmapper() {
  mpr_dev_poll(dev, 0);

  std::vector<int> touchStripsVector(RawData.touchStrips, RawData.touchStrips + sizeof(RawData.touchStrips) / sizeof(int));
  std::vector<float> mBrushVector(InstrumentData.multiBrush, InstrumentData.multiBrush + sizeof(InstrumentData.multiBrush) / sizeof(float));
  std::vector<float> mRubVector(InstrumentData.multiRub, InstrumentData.multiRub + sizeof(InstrumentData.multiRub) / sizeof(float));
  std::vector<float> shakeVector(InstrumentData.shakeXYZ, InstrumentData.shakeXYZ + sizeof(InstrumentData.shakeXYZ) / sizeof(float));
  std::vector<float> jabVector(InstrumentData.jabXYZ, InstrumentData.jabXYZ + sizeof(InstrumentData.jabXYZ) / sizeof(float));
  // TODO: incorporate vector into all firmware's logic (stop using int array)
  
  lm.sigRawCapsense.set_value(touchStripsVector);
  lm.sigRawFSR.set_value(RawData.fsr);
  lm.sigRawPiezo.set_value(RawData.piezo);
  lm.sigRawAcclX.set_value(RawData.accl[0]);
  lm.sigRawAcclY.set_value(RawData.accl[1]);
  lm.sigRawAcclZ.set_value(RawData.accl[2]);
  lm.sigRawGyroX.set_value(RawData.gyro[0]);
  lm.sigRawGyroY.set_value(RawData.gyro[1]);
  lm.sigRawGyroZ.set_value(RawData.gyro[2]);
  lm.sigRawMagnX.set_value(RawData.magn[0]);
  lm.sigRawMagnY.set_value(RawData.magn[1]);
  lm.sigRawMagnZ.set_value(RawData.magn[2]);
  lm.sigOrientationQ1.set_value(RawData.quat[0]);
  lm.sigOrientationQ2.set_value(RawData.quat[1]);
  lm.sigOrientationQ3.set_value(RawData.quat[2]);
  lm.sigOrientationQ4.set_value(RawData.quat[3]);
  lm.sigMagGyro.set_value(RawData.magGyro);
  lm.sigMagAccl.set_value(RawData.magAccl);
  lm.sigMagMagn.set_value(RawData.magMagn);
  lm.sigButton.set_value(RawData.buttonShort);
  lm.sigLongButton.set_value(RawData.buttonLong);
  lm.sigDoubleButton.set_value(RawData.buttonDouble);
  lm.sigYaw.set_value(InstrumentData.ypr[0]);
  lm.sigPitch.set_value(InstrumentData.ypr[1]);
  lm.sigRoll.set_value(InstrumentData.ypr[2]);
  lm.sigtouchAll.set_value(InstrumentData.touchAll);
  lm.sigtouchTop.set_value(InstrumentData.touchTop);
  lm.sigtouchMiddle.set_value(InstrumentData.touchMiddle);
  lm.sigtouchBottom.set_value(InstrumentData.touchBottom);
  lm.sigBrush.set_value(InstrumentData.brush);
  lm.sigRub.set_value(InstrumentData.rub);
  lm.sigMultiBrush.set_value(mBrushVector);
  lm.sigMultiRub.set_value(mRubVector);
  lm.sigShakeXYZ.set_value(shakeVector);
  lm.sigJabXYZ.set_value(jabVector);

  // mpr_sig_set_value(
  //     mpr_sig sig, 
  //     mpr_id inst, 
  //     int length,
  //     mpr_type type, 
  //     void *value
  //     );
  
  // mpr_sig_set_value(outputSignal, 0, 1, MPR_FLT, &&seqNumber);
  
//mpr_sig_set_value(lm.sigRawCapsense, 0, sizeof(RawData.touchStrips), MPR_INT32, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigRawFSR, 0, 1, MPR_FLT, &RawData.fsr);
//mpr_sig_set_value(lm.sigRawPiezo, 0, 1, MPR_INT32, &RawData.piezo);
//mpr_sig_set_value(lm.sigRawAcclX, 0, 1, MPR_INT32, &RawData.accl[0]);
//mpr_sig_set_value(lm.sigRawAcclY, 0, 1, MPR_INT32, &RawData.accl[1]);
//mpr_sig_set_value(lm.sigRawAcclZ, 0, 1, MPR_INT32, &RawData.accl[2]);
//mpr_sig_set_value(lm.sigRawGyroX, 0, 1, MPR_FLT, &RawData.gyro[0]);
//mpr_sig_set_value(lm.sigRawGyroY, 0, 1, MPR_FLT, &RawData.gyro[1]);
//mpr_sig_set_value(lm.sigRawGyroZ, 0, 1, MPR_FLT, &RawData.gyro[2]);
//mpr_sig_set_value(lm.sigRawMagnX, 0, 1, MPR_INT32, &RawData.magn[0]);
//mpr_sig_set_value(lm.sigRawMagnY, 0, 1, MPR_INT32, &RawData.magn[1]);
//mpr_sig_set_value(lm.sigRawMagnZ, 0, 1, MPR_INT32, &RawData.magn[2]);
//mpr_sig_set_value(lm.sigOrientationQ1, 0, 1, MPR_FLT, &RawData.quat[0]);
//mpr_sig_set_value(lm.sigOrientationQ2, 0, 1, MPR_FLT, &RawData.quat[1]);
//mpr_sig_set_value(lm.sigOrientationQ3, 0, 1, MPR_FLT, &RawData.quat[2]);
//mpr_sig_set_value(lm.sigOrientationQ4, 0, 1, MPR_FLT, &RawData.quat[3]);
//mpr_sig_set_value(lm.sigMagGyro, 0, 1, MPR_FLT, &RawData.magGyro);
//mpr_sig_set_value(lm.sigMagAccl, 0, 1, MPR_FLT, &RawData.magAccl);
//mpr_sig_set_value(lm.sigMagMagn, 0, 1, MPR_FLT, &RawData.magMagn);
//mpr_sig_set_value(lm.sigButton, 0, 1, MPR_INT32, &RawData.buttonShort);
//mpr_sig_set_value(lm.sigLongButton, 0, 1, MPR_INT32, &RawData.buttonLong);
//mpr_sig_set_value(lm.sigDoubleButton, 0, 1, MPR_INT32, &RawData.buttonDouble);
//mpr_sig_set_value(lm.sigYaw, 0, 1, MPR_FLT, &InstrumentData.ypr[0]);
//mpr_sig_set_value(lm.sigPitch, 0, 1, MPR_FLT, &InstrumentData.ypr[1]);
//mpr_sig_set_value(lm.sigRoll, 0, 1, MPR_FLT, &InstrumentData.ypr[2]);
//mpr_sig_set_value(lm.sigtouchAll, 0, 1, MPR_FLT, &InstrumentData.touchAll);
//mpr_sig_set_value(lm.sigtouchTop, 0, 1, MPR_FLT, &InstrumentData.touchTop);
//mpr_sig_set_value(lm.sigtouchMiddle, 0, 1, MPR_FLT, &InstrumentData.touchMiddle);
//mpr_sig_set_value(lm.sigtouchBottom, 0, 1, MPR_FLT, &InstrumentData.touchBottom);
//mpr_sig_set_value(lm.sigBrush, 0, 1, MPR_FLT, &InstrumentData.brush);
//mpr_sig_set_value(lm.sigRub, 0, 1, MPR_FLT, &InstrumentData.rub);
//mpr_sig_set_value(lm.sigMultiBrush, 0, sizeof(InstrumentData.multiBrush)/sizeof(InstrumentData.multiBrush[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigMultiRub, 0, sizeof(InstrumentData.multiRub)/sizeof(InstrumentData.multiRub[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigShakeXYZ, 0, sizeof(InstrumentData.shakeXYZ)/sizeof(InstrumentData.shakeXYZ[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigJabXYZ, 0, sizeof(InstrumentData.jabXYZ)/sizeof(InstrumentData.jabXYZ[0]), MPR_FLT, &RawData.touchStrips);
}

#endif
