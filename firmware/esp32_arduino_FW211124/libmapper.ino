#ifdef LIBMAPPER

#include <mapper_cpp.h>
#include <string>

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

  // Signal creation:
  //      mpr_sig_new(
  //          mpr_dev parent, 
  //          mpr_dir dir, 
  //          const char *name, 
  //          int length,
  //          mpr_type type, 
  //          const char *unit, 
  //          void *Min, &
  //          void *max,
  //          int *num_inst, 
  //          mpr_sig_handler *h, 
  //          int events,
  //          0, 0, 0);
                    
lm.sigRawCapsense = mpr_sig_new(dev, MPR_DIR_OUT, "raw/capsense", touchStripsSize, MPR_INT32, NULL, &lm.rawCapsenseMin, &lm.rawCapsenseMax, 0, 0, 0);
lm.sigRawGyroX = mpr_sig_new(dev, MPR_DIR_OUT, "raw/gyro/X", 1, MPR_FLT, NULL, &lm.rawGyroMin, &lm.rawGyroMax, 0, 0, 0);
lm.sigRawGyroY = mpr_sig_new(dev, MPR_DIR_OUT, "raw/gyro/Y", 1, MPR_FLT, NULL, &lm.rawGyroMin, &lm.rawGyroMax, 0, 0, 0);
lm.sigRawGyroZ = mpr_sig_new(dev, MPR_DIR_OUT, "raw/gyro/Z", 1, MPR_FLT, NULL, &lm.rawGyroMin, &lm.rawGyroMax, 0, 0, 0);
lm.sigRawAcclX = mpr_sig_new(dev, MPR_DIR_OUT, "raw/accl/X", 1, MPR_INT32, NULL, &lm.rawAcclMin, &lm.rawAcclMax, 0, 0, 0);
lm.sigRawAcclY = mpr_sig_new(dev, MPR_DIR_OUT, "raw/accl/Y", 1, MPR_INT32, NULL, &lm.rawAcclMin, &lm.rawAcclMax, 0, 0, 0);
lm.sigRawAcclZ = mpr_sig_new(dev, MPR_DIR_OUT, "raw/accl/Z", 1, MPR_INT32, NULL, &lm.rawAcclMin, &lm.rawAcclMax, 0, 0, 0);
lm.sigRawMagnX = mpr_sig_new(dev, MPR_DIR_OUT, "raw/magn/X", 1, MPR_INT32, NULL, &lm.rawMagnMin, &lm.rawMagnMax, 0, 0, 0);
lm.sigRawMagnY = mpr_sig_new(dev, MPR_DIR_OUT, "raw/magn/Y", 1, MPR_INT32, NULL, &lm.rawMagnMin, &lm.rawMagnMax, 0, 0, 0);
lm.sigRawMagnZ = mpr_sig_new(dev, MPR_DIR_OUT, "raw/magn/Z", 1, MPR_INT32, NULL, &lm.rawMagnMin, &lm.rawMagnMax, 0, 0, 0);
lm.sigRawFSR = mpr_sig_new(dev, MPR_DIR_OUT, "raw/fsr", 1, MPR_INT32, NULL, &lm.rawFSRMin, &lm.rawFSRMax, 0, 0, 0);
lm.sigRawPiezo = mpr_sig_new(dev, MPR_DIR_OUT, "raw/piezo", 1, MPR_INT32, NULL, &lm.rawPiezoMin, &lm.rawPiezoMax, 0, 0, 0);
lm.sigOrientationQ1 = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/q1", 1, MPR_FLT, NULL, &lm.orientationMin, &lm.orientationMax, 0, 0, 0);
lm.sigOrientationQ2 = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/q2", 1, MPR_FLT, NULL, &lm.orientationMin, &lm.orientationMax, 0, 0, 0);
lm.sigOrientationQ3 = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/q3", 1, MPR_FLT, NULL, &lm.orientationMin, &lm.orientationMax, 0, 0, 0);
lm.sigOrientationQ4 = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/q4", 1, MPR_FLT, NULL, &lm.orientationMin, &lm.orientationMax, 0, 0, 0);
lm.sigYaw = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/yaw", 1, MPR_FLT, NULL, &lm.yprMin, &lm.yprMax, 0, 0, 0);
lm.sigPitch = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/pitch", 1, MPR_FLT, NULL, &lm.yprMin, &lm.yprMax, 0, 0, 0);
lm.sigRoll = mpr_sig_new(dev, MPR_DIR_OUT, "orientation/roll", 1, MPR_FLT, NULL, &lm.yprMin, &lm.yprMax, 0, 0, 0);
lm.sigMagGyro = mpr_sig_new(dev, MPR_DIR_OUT, "gyro/magnitude", 1, MPR_FLT, NULL, &lm.magMin, &lm.magMax, 0, 0, 0);
lm.sigMagAccl = mpr_sig_new(dev, MPR_DIR_OUT, "accl/magnitude", 1, MPR_FLT, NULL, &lm.magMin, &lm.magMax, 0, 0, 0);
lm.sigMagMagn = mpr_sig_new(dev, MPR_DIR_OUT, "magn/magnitude", 1, MPR_FLT, NULL, &lm.magMin, &lm.magMax, 0, 0, 0);
lm.sigButton = mpr_sig_new(dev, MPR_DIR_OUT, "button/short", 1, MPR_INT32, NULL, &lm.buttonMin, &lm.buttonMax, 0, 0, 0);
lm.sigLongButton = mpr_sig_new(dev, MPR_DIR_OUT, "button/long", 1, MPR_INT32, NULL, &lm.buttonMin, &lm.buttonMax, 0, 0, 0);
lm.sigDoubleButton = mpr_sig_new(dev, MPR_DIR_OUT, "button/double", 1, MPR_INT32, NULL, &lm.buttonMin, &lm.buttonMax, 0, 0, 0);
lm.sigtouchAll = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/touchall", 1, MPR_FLT, NULL, &lm.instTouchMin, &lm.instTouchMax, 0, 0, 0);
lm.sigtouchTop = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/touchtop", 1, MPR_FLT, NULL, &lm.instTouchMin, &lm.instTouchMax, 0, 0, 0);
lm.sigtouchMiddle = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/touchmiddle", 1, MPR_FLT, NULL, &lm.instTouchMin, &lm.instTouchMax, 0, 0, 0);
lm.sigtouchBottom = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/touchbottom", 1, MPR_FLT, NULL, &lm.instTouchMin, &lm.instTouchMax, 0, 0, 0);
lm.sigBrush = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/brush", 1, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
lm.sigRub = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/rub", 1, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
lm.sigMultiBrush = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/multibrush", 4, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
lm.sigMultiRub = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/multirub", 4, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
lm.sigShakeXYZ = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/shakexyz", 3, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
lm.sigJabXYZ = mpr_sig_new(dev, MPR_DIR_OUT, "instrument/jabxyz", 3, MPR_FLT, NULL, &lm.genericMin, &lm.genericMax, 0, 0, 0);
}

void updateLibmapper() {
  mpr_dev_poll(dev, 0);

  // mpr_sig_set_value(
  //     mpr_sig sig, 
  //     mpr_id inst, 
  //     int length,
  //     mpr_type type, 
  //     void *value
  //     );
  
  // mpr_sig_set_value(outputSignal, 0, 1, MPR_FLT, &&seqNumber);
  
  mpr_sig_set_value(lm.sigRawCapsense, 0, sizeof(RawData.touchStrips), MPR_INT32, &RawData.touchStrips);
mpr_sig_set_value(lm.sigRawFSR, 0, 1, MPR_FLT, &RawData.fsr);
mpr_sig_set_value(lm.sigRawPiezo, 0, 1, MPR_INT32, &RawData.piezo);
mpr_sig_set_value(lm.sigRawAcclX, 0, 1, MPR_INT32, &RawData.accl[0]);
mpr_sig_set_value(lm.sigRawAcclY, 0, 1, MPR_INT32, &RawData.accl[1]);
mpr_sig_set_value(lm.sigRawAcclZ, 0, 1, MPR_INT32, &RawData.accl[2]);
mpr_sig_set_value(lm.sigRawGyroX, 0, 1, MPR_FLT, &RawData.gyro[0]);
mpr_sig_set_value(lm.sigRawGyroY, 0, 1, MPR_FLT, &RawData.gyro[1]);
mpr_sig_set_value(lm.sigRawGyroZ, 0, 1, MPR_FLT, &RawData.gyro[2]);
mpr_sig_set_value(lm.sigRawMagnX, 0, 1, MPR_INT32, &RawData.magn[0]);
mpr_sig_set_value(lm.sigRawMagnY, 0, 1, MPR_INT32, &RawData.magn[1]);
mpr_sig_set_value(lm.sigRawMagnZ, 0, 1, MPR_INT32, &RawData.magn[2]);
mpr_sig_set_value(lm.sigOrientationQ1, 0, 1, MPR_FLT, &RawData.quat[0]);
mpr_sig_set_value(lm.sigOrientationQ2, 0, 1, MPR_FLT, &RawData.quat[1]);
mpr_sig_set_value(lm.sigOrientationQ3, 0, 1, MPR_FLT, &RawData.quat[2]);
mpr_sig_set_value(lm.sigOrientationQ4, 0, 1, MPR_FLT, &RawData.quat[3]);
mpr_sig_set_value(lm.sigMagGyro, 0, 1, MPR_FLT, &RawData.magGyro);
mpr_sig_set_value(lm.sigMagAccl, 0, 1, MPR_FLT, &RawData.magAccl);
mpr_sig_set_value(lm.sigMagMagn, 0, 1, MPR_FLT, &RawData.magMagn);
mpr_sig_set_value(lm.sigButton, 0, 1, MPR_INT32, &RawData.buttonShort);
mpr_sig_set_value(lm.sigLongButton, 0, 1, MPR_INT32, &RawData.buttonLong);
mpr_sig_set_value(lm.sigDoubleButton, 0, 1, MPR_INT32, &RawData.buttonDouble);
mpr_sig_set_value(lm.sigYaw, 0, 1, MPR_FLT, &InstrumentData.ypr[0]);
mpr_sig_set_value(lm.sigPitch, 0, 1, MPR_FLT, &InstrumentData.ypr[1]);
mpr_sig_set_value(lm.sigRoll, 0, 1, MPR_FLT, &InstrumentData.ypr[2]);
mpr_sig_set_value(lm.sigtouchAll, 0, 1, MPR_FLT, &InstrumentData.touchAll);
mpr_sig_set_value(lm.sigtouchTop, 0, 1, MPR_FLT, &InstrumentData.touchTop);
mpr_sig_set_value(lm.sigtouchMiddle, 0, 1, MPR_FLT, &InstrumentData.touchMiddle);
mpr_sig_set_value(lm.sigtouchBottom, 0, 1, MPR_FLT, &InstrumentData.touchBottom);
mpr_sig_set_value(lm.sigBrush, 0, 1, MPR_FLT, &InstrumentData.brush);
mpr_sig_set_value(lm.sigRub, 0, 1, MPR_FLT, &InstrumentData.rub);
mpr_sig_set_value(lm.sigMultiBrush, 0, sizeof(InstrumentData.multiBrush)/sizeof(InstrumentData.multiBrush[0]), MPR_FLT, &RawData.touchStrips);
mpr_sig_set_value(lm.sigMultiRub, 0, sizeof(InstrumentData.multiRub)/sizeof(InstrumentData.multiRub[0]), MPR_FLT, &RawData.touchStrips);
mpr_sig_set_value(lm.sigShakeXYZ, 0, sizeof(InstrumentData.shakeXYZ)/sizeof(InstrumentData.shakeXYZ[0]), MPR_FLT, &RawData.touchStrips);
mpr_sig_set_value(lm.sigJabXYZ, 0, sizeof(InstrumentData.jabXYZ)/sizeof(InstrumentData.jabXYZ[0]), MPR_FLT, &RawData.touchStrips);
}

#endif
