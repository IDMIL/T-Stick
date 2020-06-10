#include "TStick.h"

void updateInstrument() {

  // InstrumentData.touchAll: get the "amount of touch" for the entire capsense
  // normalized between 0 and 1
  InstrumentData.touchAll = touchAverage(RawData.touchStrips, 0, touchStripsSize);

  // InstrumentData.touchTop: get the "amount of touch" for the top part of the capsense
  // normalized between 0 and 1
  InstrumentData.touchTop = touchAverage(RawData.touchStrips, 0, touchSizeEdge);

  // InstrumentData.touchMiddle: get the "amount of touch" for the central part of the capsense
  // normalized between 0 and 1
  InstrumentData.touchMiddle = touchAverage(RawData.touchStrips, (0+touchSizeEdge), (touchStripsSize - touchSizeEdge));

  // InstrumentData.touchBottom: get the "amount of touch" for the botton part of the capsense
  // normalized between 0 and 1
  InstrumentData.touchBottom = touchAverage(RawData.touchStrips, (touchStripsSize-touchSizeEdge), touchStripsSize);

  // Save last blob detection state before reading new data
  for (byte i=0; i < (sizeof(BlobDetection.blobPos)/sizeof(BlobDetection.blobPos[0])); ++i) {
      LastState.blobPos[i] = BlobDetection.blobPos[i];
  }

  // 1D blob detection: used for brush
  BlobDetection = blobDetection1D(RawData.touch,(nCapsenses*2));

  // InstrumentData.brush: direction and intensity of capsense brush motion
  // InstrumentData.rub: intensity of rub motion
  // in ~cm/s (distance between stripes = ~1.5cm)
  for (byte i=0; i < (sizeof(BlobDetection.blobPos)/sizeof(BlobDetection.blobPos[0])); ++i) {        
    float movement = BlobDetection.blobPos[i] - LastState.blobPos[i]; 
    if ( BlobDetection.blobPos[i] == -1 ) {
      InstrumentData.multiBrush[i] = 0;
      InstrumentData.multiRub[i] = 0;
      brushCounter[i] = 0;
    }
    else if (movement == 0) {
      if (brushCounter[i] < 10) {
        brushCounter[i]++;
        // wait some time before dropping the rub/brush values
      }
      else if (InstrumentData.multiBrush[i] < 0.001) {
        InstrumentData.multiBrush[i] = 0;
        InstrumentData.multiRub[i] = 0;
      }
      else {
        InstrumentData.multiBrush[i] = leakyIntegrator(movement*0.15, InstrumentData.multiBrush[i], 0.7, leakyBrushFreq, leakyBrushTimer);
        InstrumentData.multiRub[i] = leakyIntegrator(abs(movement*0.15), InstrumentData.multiRub[i], 0.7, leakyRubFreq, leakyRubTimer);
      }
    }
    else if ( abs(movement) > 1 ) {
      InstrumentData.multiBrush[i] = leakyIntegrator(0, InstrumentData.multiBrush[i], 0.6, leakyBrushFreq, leakyBrushTimer);
    }
    else {
      InstrumentData.multiBrush[i] = leakyIntegrator(movement*0.15, InstrumentData.multiBrush[i], 0.8, leakyBrushFreq, leakyBrushTimer);
      InstrumentData.multiRub[i] = leakyIntegrator(abs(movement*0.15), InstrumentData.multiRub[i], 0.99, leakyRubFreq, leakyRubTimer);
      brushCounter[i] = 0;
    }
  }
  InstrumentData.brush =  arrayAverageZero(InstrumentData.multiBrush,4);
  InstrumentData.rub = arrayAverageZero(InstrumentData.multiRub,4);

  // InstrumentData.shakeXYZ
  for (byte i=0; i<(sizeof(RawData.gyro)/sizeof(RawData.gyro[0])); ++i) {
    if (abs(RawData.gyro[i]) > 0.1) {
      InstrumentData.shakeXYZ[i] = leakyIntegrator(abs(RawData.gyro[i]*0.1), InstrumentData.shakeXYZ[i], 0.6, leakyShakeFreq, leakyShakeTimer[i]);
    }
    else {
      InstrumentData.shakeXYZ[i] = leakyIntegrator(0, InstrumentData.shakeXYZ[i], 0.3, leakyShakeFreq, leakyShakeTimer[i]);
      if (InstrumentData.shakeXYZ[i] < 0.01) {
        InstrumentData.shakeXYZ[i] = 0;
      }
    }
  }

  // InstrumentData.jabXYZ
  if (arrayMax(LastState.gyroXArray,5)-arrayMin(LastState.gyroXArray,5) > 15) {
    InstrumentData.jabXYZ[0] = arrayMax(LastState.gyroXArray,5) - arrayMin(LastState.gyroXArray,5) - 10;
  }
  else {
    InstrumentData.jabXYZ[0] = 0;
  }
  if (arrayMax(LastState.gyroYArray,5)-arrayMin(LastState.gyroYArray,5) > 3) {
    InstrumentData.jabXYZ[1] = arrayMax(LastState.gyroYArray,5) - arrayMin(LastState.gyroYArray,5) - 3;
  }
  else {
    InstrumentData.jabXYZ[1] = 0;
  }
  if (arrayMax(LastState.gyroZArray,5)-arrayMin(LastState.gyroZArray,5) > 10) {
    InstrumentData.jabXYZ[2] = arrayMax(LastState.gyroZArray,5) - arrayMin(LastState.gyroZArray,5) - 10;
  }
  else {
    InstrumentData.jabXYZ[2] = 0;
  }
  

}


float touchAverage (byte * touchArrayStrips, byte firstStrip, byte lastStrip) {
    int sum = 0;
    for (int i = firstStrip; i < lastStrip; ++i)
      sum += touchArrayStrips[i];
      
    return  ((float) sum) / (lastStrip - firstStrip);
}


float arrayAverage (float * Array, byte ArraySize) {
    float sum = 0;
    for (int i = 0; i < ArraySize; ++i)
      sum += Array[i];
      
    return  (sum / ArraySize);
}

float arrayAverageZero (float * Array, byte ArraySize) {
    float sum = 0;
    byte count = 0;
    float output = 0;
    for (int i = 0; i < ArraySize; ++i) {
      if (Array[i] != 0) {
        sum += Array[i];
        count++;
      }
    }
    if (count > 0) {
    output = sum / count; 
    }
      
    return output;
}


blob blobDetection1D (byte * touchArray, byte arraySize) {

    // creating local variables
    blob blobDecect;
    byte tempArray[8];
    int beginBlob = -1; // -1 means it will not count stripes
    byte blobCount = 0;
    for (byte i=0; i < sizeof(blobDecect.blobPos)/sizeof(blobDecect.blobPos[0]); ++i) {
      blobDecect.blobPos[i] = -1;
      blobDecect.blobSize[i] = 0;
    }
    for (byte i=0; i < sizeof(blobDecect.blobArray)/sizeof(blobDecect.blobArray[0]); ++i) {
      blobDecect.blobArray[i] = 0;
    }

    // fixing capsense byte order
    byte order[8] = {1,0,3,2,5,4,7,6};
    for (byte i=0; i < arraySize; ++i) {
      tempArray[i] = touchArray[order[i]];
    }

    // shifting and reading...
    for (byte i=0; i < arraySize*8; ++i) {
      bitShiftArrayL(tempArray, blobDecect.blobArray, arraySize, i);
      if ((blobDecect.blobArray[0] & 128) == 128 && beginBlob == -1) {
          beginBlob = i;
      }
      if ( ((blobDecect.blobArray[0] & 128) == 0 || i == (arraySize*8)-1) && beginBlob != -1) {
          blobDecect.blobPos[blobCount] = (i + beginBlob) / 2;
          blobDecect.blobSize[blobCount] = float(i - beginBlob) / (arraySize * 8);
          beginBlob = -1;
          blobCount++;
        }
    }

    for (byte i=0; i < sizeof(blobDecect.blobArray)/sizeof(blobDecect.blobArray[0]); ++i) {
      blobDecect.blobArray[i] = 0;
    }
    for (byte i=0; i < sizeof(blobDecect.blobPos)/sizeof(blobDecect.blobPos[0]); ++i) {
      if (blobDecect.blobPos[i] != -1) {
        bitWrite(blobDecect.blobArray[blobDecect.blobPos[i]/8], (7-(blobDecect.blobPos[i]%8)), 1);        
      }
      else {
        break;
      }
    }

    return blobDecect; 
}


void printBinary (byte number) {
  byte reading;
  for (int i=7; i >= 0; --i) {
    reading = number >> i;
    Serial.print(reading & 1);
  }
}

void bitShiftArrayL (byte * origArray, byte * shiftedArray, byte arraySize, byte shift) {

  for (byte i=0; i < arraySize; ++i) {
      shiftedArray[i] = origArray[i];
  }

  for (byte k=0; k < shift; ++k) {
      for (byte i=0; i < arraySize; ++i) {
          if ( i == (arraySize-1)) {
              shiftedArray[i] = (shiftedArray[i] << 1);
          }
          else {
              shiftedArray[i] = (shiftedArray[i] << 1) | (shiftedArray[i+1] >> 7);
          }
      }
  }
}


void bitShiftArrayR (byte * origArray, byte * shiftedArray, byte arraySize, byte shift) {
  
  for (byte i=0; i < arraySize; ++i) {
      shiftedArray[i] = origArray[i];
  }

  for (byte k=0; k < shift; ++k) {
      for (int i=arraySize; i >= 0; --i) {
          if ( i == 0) {
              shiftedArray[i] = (shiftedArray[i] >> 1);
          }
          else {
              shiftedArray[i] = (shiftedArray[i] >> 1) | (shiftedArray[i-1] << 7);
          }
      }
  }
}

float arrayMin (float *inputArray, byte arraySize) {
  float output = inputArray[0];
  for (byte i=1; i<arraySize; ++i) {
    output = min(output,inputArray[i]);
  }
  return output;
}

float arrayMax (float *inputArray, byte arraySize) {
  float output = inputArray[0];
  for (byte i=1; i<arraySize; ++i) {
    output = max(output,inputArray[i]);
  }
  return output;
}


// Simple leaky integrator implementation
//
// Create a unsigned long global variable for time counter for each leak implementation (timer)

float leakyIntegrator (float reading, float old_value, float leak, int frequency, unsigned long& timer) {
  float new_value;

  if (frequency == 0) {
    new_value = reading + (old_value * leak);
  }
  else if (millis() - (1000 / frequency) < timer) {  
    new_value = reading + old_value;
  } 
  else {
    new_value = reading + (old_value * leak);
    timer = millis();
  }
  
  return new_value;
}
