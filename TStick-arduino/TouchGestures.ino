// #include <vector>

// bool getNTouch(int n) {
//   return RawData.touch[n / 16][n / 8 % 2];
// }

// void vectorBitwiseAnd(byte output[], byte arrayA[], byte arrayB[], size_t len) {
//   for (int i = 0; i < len; ++i) {
//     output[i] = arrayA[i] & arrayB[i];
//   }
// }

// float bitMean(byte first_bit, byte last_bit) { // calculates mean for a given bit range
//   const float k = 1.0f / (last_bit - first_bit);
//   float mean = 0;
//   for (int i = first_bit; i < last_bit; i++) {
//       if (getNTouch(i)) { // read each bit ...
//         mean += k; // ... and add if its high
//       }
//   }
//   return mean;
// }

// float leakyIntegrator(float reading, float old_value,  float leak) {
//   return reading + (old_value * leak);
// }

// float windowedExtremaMax(float array[], size_t len) {
//   float result = 0;
//   for (int i = 0; i < len; i++) {
//     result = max(result, array[i]);
//   }
//   return result;
// }

// float windowedExtremaMin(float array[], size_t len) {
//   float result = 0;
//   for (int i = 0; i < len; i++) {
//     result = min(result, array[i]);
//   }
//   return result;
// }

// float lowPassFilter(float data, float olddata, float k) {
//   return olddata + ((data - olddata) / k);
// }

// std::vector<Blob> blobDetection() {
//   std::vector<Blob> blobs;
//   // for (byte i = 0; i < TOUCH_SIZE_ALL; ++i) {
//   //   bool t = bitRead(RawData.touch[i / 16][i / 8 % 2], i);
//   //   if (t) {
//   //     Blob b;
//   //     b.idx = i;
//   //     b.size = 0;
//   //     while (bitRead(RawData.touch[(b.idx + b.size) / 8], (b.idx + b.size))) {
//   //       ++b.size;
//   //     } 
//   //     blobs->push_back(b);
//   //   }
//   // }
//   return blobs;
// }

// void readGestures() {
//   static LastStateData lastStateData; 

//   InstrumentData.touchAll = bitMean(0, TOUCH_SIZE_ALL);
//   InstrumentData.touchTop = bitMean(0, TOUCH_SIZE_EDGE);
//   InstrumentData.touchMiddle = bitMean(TOUCH_SIZE_EDGE, (TOUCH_SIZE_ALL - TOUCH_SIZE_EDGE));
//   InstrumentData.touchBottom = bitMean((TOUCH_SIZE_ALL - TOUCH_SIZE_EDGE), TOUCH_SIZE_ALL);

//   byte operationUp[TOUCH_VECTOR_SIZE], operationDown[TOUCH_VECTOR_SIZE];
//   // vectorBitwiseAnd(operationUp, lastStateData.brushUp, RawData.touch, TOUCH_VECTOR_SIZE);
//   // vectorBitwiseAnd(operationDown, lastStateData.brushDown, RawData.touch, TOUCH_VECTOR_SIZE);

//   // const float k = 1 / TOUCH_SIZE_ALL;

//   for (int i = 0; i < TOUCH_SIZE_ALL; i++) {
//       if (bitRead(operationUp[i / 8], i)) { 
//           InstrumentData.brushUp = (TOUCH_SIZE_ALL - i) / TOUCH_SIZE_ALL;
//           // mean_up += k;
//       }
//       if (bitRead(operationDown[i / 8], i)) {
//           InstrumentData.brushDown = (TOUCH_SIZE_ALL - i) / TOUCH_SIZE_ALL;
//           // mean_down += k;
//       }
//   }

//   InstrumentData.brushAmplitude = InstrumentData.brushDown - InstrumentData.brushUp;

//   if (InstrumentData.brushUp > InstrumentData.brushDown) {
//     InstrumentData.brushEnergy = InstrumentData.brushUp;
//   }
//   else {
//     InstrumentData.brushEnergy = InstrumentData.brushDown;
//   }

//   auto blobs = blobDetection();

//   // for (int i = 0; i < TOUCH_VECTOR_SIZE; ++i) {
//   //   lastStateData.brushUp[i] = (RawData.touch[i] << 1) & (~RawData.touch[i]);
//   //   lastStateData.brushDown[i] = (RawData.touch[i] >> 1) & (~RawData.touch[i]);
//   // }
// }

// void printGestures() {
//   if (millis() - serialLastRead > serialInterval) {
//     serialLastRead = millis(); 

//     Serial.println("\nPrinting sensor data: ");
//     Serial.print("RawData.touch: ");
//     printf("%i, %i, %i, %i, %i, %i, %i, %i, %i, %i",
//     RawData.touch[0][0],RawData.touch[0][1],
//     RawData.touch[1][0],RawData.touch[1][1],
//     RawData.touch[2][0],RawData.touch[2][1],
//     RawData.touch[3][0],RawData.touch[3][1],
//     RawData.touch[4][0],RawData.touch[4][1]);
//     Serial.println();
//     Serial.print("InstrumentData.touchAll: "); Serial.println(InstrumentData.touchAll);
//     Serial.print("InstrumentData.touchBottom: "); Serial.println(InstrumentData.touchBottom);
//     Serial.print("InstrumentData.touchMiddle: "); Serial.println(InstrumentData.touchMiddle);
//   }
// }
