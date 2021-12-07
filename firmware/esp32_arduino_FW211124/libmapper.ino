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
      dev = new mapper::Device(tstickSSID);

  // output = dev->add_signal(
  //              Direction::OUTGOING, 
  //              "my_output", // a unique name for the signal
  //              4,           // signal's vector length
  //              Type::INT32, // the signal's data type, one of Type::INT32, Type::FLOAT, or Type::DOUBLE
  //              "m/s",       // the signal's unit (optional). Use 0 (without quotes) if not specified
  //              min,         // the signal's minimum value (optional)
  //              max);        // the signal's maximum value (optional)
    
  int touchlmSize = touchStripsSize*trillamount;
  std::string sigRawCapsense_name = "raw/capsense";
  lm.sigRawCapsense = dev->add_signal(mapper::Direction::OUTGOING, sigRawCapsense_name, touchlmSize, mapper::Type::INT32, 0, &lm.rawCapsenseMin, &lm.rawCapsenseMax);
  std::string sigRawGyroX_name = "raw/gyro/X";
  lm.sigRawGyroX = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroX_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawGyroY_name = "raw/gyro/Y";
  lm.sigRawGyroY = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroY_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawGyroZ_name = "raw/gyro/Z";
  lm.sigRawGyroZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroZ_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawAcclX_name = "raw/accl/X";
  lm.sigRawAcclX = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclX_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawAcclY_name = "raw/accl/Y";
  lm.sigRawAcclY = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclY_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawAcclZ_name = "raw/accl/Z";
  lm.sigRawAcclZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclZ_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawMagnX_name = "raw/magn/X";
  lm.sigRawMagnX = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnX_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawMagnY_name = "raw/magn/Y";
  lm.sigRawMagnY = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnY_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawMagnZ_name = "raw/magn/Z";
  lm.sigRawMagnZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnZ_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawFSR_name = "raw/fsr";
  lm.sigRawFSR = dev->add_signal(mapper::Direction::OUTGOING, sigRawFSR_name, 1, mapper::Type::INT32, 0, &lm.rawFSRMin, &lm.rawFSRMax);
  std::string sigYaw_name = "orientation/yaw";
  lm.sigYaw = dev->add_signal(mapper::Direction::OUTGOING, sigYaw_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  std::string sigPitch_name = "orientation/pitch";
  lm.sigPitch = dev->add_signal(mapper::Direction::OUTGOING, sigPitch_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  std::string sigRoll_name = "orientation/roll";
  lm.sigRoll = dev->add_signal(mapper::Direction::OUTGOING, sigRoll_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  std::string sigMagGyro_name = "gyro/magnitude";
  lm.sigMagGyro = dev->add_signal(mapper::Direction::OUTGOING, sigMagGyro_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  std::string sigMagAccl_name = "accl/magnitude";
  lm.sigMagAccl = dev->add_signal(mapper::Direction::OUTGOING, sigMagAccl_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  std::string sigMagMagn_name = "magn/magnitude";
  lm.sigMagMagn = dev->add_signal(mapper::Direction::OUTGOING, sigMagMagn_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  std::string sigButton_name = "button/short";
  lm.sigButton = dev->add_signal(mapper::Direction::OUTGOING, sigButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  std::string sigLongButton_name = "button/long";
  lm.sigLongButton = dev->add_signal(mapper::Direction::OUTGOING, sigLongButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  std::string sigDoubleButton_name = "button/double";
  lm.sigDoubleButton = dev->add_signal(mapper::Direction::OUTGOING, sigDoubleButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  std::string sigtouchAll_name = "instrument/touchall";
  lm.sigtouchAll = dev->add_signal(mapper::Direction::OUTGOING, sigtouchAll_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchTop_name = "instrument/touchtop";
  lm.sigtouchTop = dev->add_signal(mapper::Direction::OUTGOING, sigtouchTop_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchMiddle_name = "instrument/touchmiddle";
  lm.sigtouchMiddle = dev->add_signal(mapper::Direction::OUTGOING, sigtouchMiddle_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchBottom_name = "instrument/touchbottom";
  lm.sigtouchBottom = dev->add_signal(mapper::Direction::OUTGOING, sigtouchBottom_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigBrush_name = "instrument/brush";
  lm.sigBrush = dev->add_signal(mapper::Direction::OUTGOING, sigBrush_name, 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigRub_name = "instrument/rub";
  lm.sigRub = dev->add_signal(mapper::Direction::OUTGOING, sigRub_name, 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigMultiBrush_name = "instrument/multibrush";
  lm.sigMultiBrush = dev->add_signal(mapper::Direction::OUTGOING, sigMultiBrush_name, 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigMultiRub_name = "instrument/multirub";
  lm.sigMultiRub = dev->add_signal(mapper::Direction::OUTGOING, sigMultiRub_name, 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigShakeXYZ_name = "instrument/shakexyz";
  lm.sigShakeXYZ = dev->add_signal(mapper::Direction::OUTGOING, sigShakeXYZ_name, 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigJabXYZ_name = "instrument/jabxyz";
  lm.sigJabXYZ = dev->add_signal(mapper::Direction::OUTGOING, sigJabXYZ_name, 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  }
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
  lm.sigRawAcclX.set_value(RawData.accl[0]);
  lm.sigRawAcclY.set_value(RawData.accl[1]);
  lm.sigRawAcclZ.set_value(RawData.accl[2]);
  lm.sigRawGyroX.set_value(RawData.gyro[0]);
  lm.sigRawGyroY.set_value(RawData.gyro[1]);
  lm.sigRawGyroZ.set_value(RawData.gyro[2]);
  lm.sigRawMagnX.set_value(RawData.magn[0]);
  lm.sigRawMagnY.set_value(RawData.magn[1]);
  lm.sigRawMagnZ.set_value(RawData.magn[2]);
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
