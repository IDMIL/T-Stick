// Some high-level gestural descriptors of the GuitarAMI
// using the Touch
// Edu Meneses - 2020 (IDMIL)

#include "instrument_touch.h"

void Instrument_touch::updateInstrument(int *raw_touch) {

    // Get max value, but also use it to check if any touch is pressed
    int instant_maxTouchValue = *std::max_element(raw_touch, raw_touch+touchSize);

    // Touching the T-Stick or not?
    if (instant_maxTouchValue == 0) {
        touchStatus = 0;
    } else {
        touchStatus = 1;
    }

    // Update everything else if user is touching
    if (touchStatus) {
        // We need updated maxTouchValue to normalize touch
        maxTouchValue = std::max(maxTouchValue,instant_maxTouchValue);

        // Touch discretize and normalize
        for (int i=0; i < touchSize; i++) {
            if (raw_touch[i] != 0) {
                touch[i] = 1;
                normTouch[i] = constrain(raw_touch[i] / maxTouchValue, 0, 1);
            }
        }

        // touchAll: get the "amount of touch" for the entire touch sensor
        // normalized between 0 and 1
        touchAll = touchAverage(normTouch, 0, touchSize);

        // touchTop: get the "amount of touch" for the top part of the capsense
        // normalized between 0 and 1
        touchTop = touchAverage(normTouch, 0, touchSizeEdge);

        // touchMiddle: get the "amount of touch" for the central part of the capsense
        // normalized between 0 and 1
        touchMiddle = touchAverage(normTouch, (0+touchSizeEdge), (touchSize - touchSizeEdge));

        // touchBottom: get the "amount of touch" for the botton part of the capsense
        // normalized between 0 and 1
        touchBottom = touchAverage(normTouch, (touchSize-touchSizeEdge), touchSize);

        // Save last blob detection state before reading new data
        for (byte i=0; i < (sizeof(blobPos)/sizeof(blobPos[0])); ++i) {
            lastState_blobPos[i] = blobPos[i];
        }

        // 1D blob detection: used for brush
        blobDetection1D();

        // brush: direction and intensity of capsense brush motion
        // rub: intensity of rub motion
        // in ~cm/s (distance between stripes = ~1.5cm)
        for (byte i=0; i < (sizeof(blobPos)/sizeof(blobPos[0])); ++i) {        
            float movement = blobPos[i] - lastState_blobPos[i]; 
            if ( blobPos[i] == -1 ) {
                multiBrush[i] = 0;
                multiRub[i] = 0;
                brushCounter[i] = 0;
            } else if (movement == 0) {
                if (brushCounter[i] < 10) {
                    brushCounter[i]++;
                    // wait some time before dropping the rub/brush values
                } else if (multiBrush[i] < 0.001) {
                    multiBrush[i] = 0;
                    multiRub[i] = 0;
                } else {
                    multiBrush[i] = leakyIntegrator(movement*0.15, multiBrush[i], 0.7, leakyBrushFreq, leakyBrushTimer);
                    multiRub[i] = leakyIntegrator(abs(movement*0.15), multiRub[i], 0.7, leakyRubFreq, leakyRubTimer);
                }
            } else if ( abs(movement) > 1 ) {
                multiBrush[i] = leakyIntegrator(0, multiBrush[i], 0.6, leakyBrushFreq, leakyBrushTimer);
            } else {
                multiBrush[i] = leakyIntegrator(movement*0.15, multiBrush[i], 0.8, leakyBrushFreq, leakyBrushTimer);
                multiRub[i] = leakyIntegrator(abs(movement*0.15), multiRub[i], 0.99, leakyRubFreq, leakyRubTimer);
                brushCounter[i] = 0;
            }
        }
        brush =  arrayAverageZero(multiBrush,4);
        rub = arrayAverageZero(multiRub,4);
    }
}

float Instrument_touch::touchAverage (float * touchArrayStrips, int firstStrip, int lastStrip) {
    int sum = 0;
    for (int i = firstStrip; i < lastStrip; ++i)
      sum += touchArrayStrips[i];
      
    return  ((float) sum) / (lastStrip - firstStrip);
}

void Instrument_touch::blobDetection1D () {
    blobAmount = 0;
    int sizeCounter = 0;
    int stripe = 0;
    for (int i=0; i<4; i++) {
        blobCenter[i] = 0;
        blobPos[i] = 0;
        blobSize[i] = 0;
    }

    for ( ; stripe<touchSize; stripe++) {
        if (blobAmount < maxBlobs) {
            if (touch[stripe] == 1) { // check for beggining of blob...
                sizeCounter = 1;
                blobPos[blobAmount] = stripe;
                while (touch[stripe+sizeCounter] == 1) { // then keep checking for end
                    sizeCounter++;
                }
                blobSize[blobAmount] = sizeCounter;
                blobCenter[blobAmount] = stripe + (sizeCounter / 2);
                stripe += sizeCounter + 1; // skip stripes already read
                blobAmount++;
            }
        }
    }
}

// void Instrument_touch::blobDetection1D (byte * touchArray, int arraySize) {

//     // creating local variables
//     byte temp_blobArray[8];  // shows the "center" of each array
//     int temp_blobPos[4];     // position (index) of each blob
//     float temp_blobSize[4];  // "size" of each blob
//     byte tempArray[8];
//     int beginBlob = -1; // -1 means it will not count stripes
//     byte blobCount = 0;
//     for (byte i=0; i < sizeof(temp_blobPos)/sizeof(temp_blobPos[0]); ++i) {
//       temp_blobPos[i] = -1;
//       temp_blobSize[i] = 0;
//     }
//     for (byte i=0; i < sizeof(temp_blobArray)/sizeof(temp_blobArray[0]); ++i) {
//       temp_blobArray[i] = 0;
//     }

//     // fixing capsense byte order
//     byte order[8] = {1,0,3,2,5,4,7,6};
//     for (byte i=0; i < arraySize; ++i) {
//       tempArray[i] = touchArray[order[i]];
//     }

//     // shifting and reading...
//     for (byte i=0; i < arraySize*8; ++i) {
//       bitShiftArrayL(tempArray, temp_blobArray, arraySize, i);
//       if ((temp_blobArray[0] & 128) == 128 && beginBlob == -1) {
//           beginBlob = i;
//       }
//       if ( ((temp_blobArray[0] & 128) == 0 || i == (arraySize*8)-1) && beginBlob != -1) {
//           temp_blobPos[blobCount] = (i + beginBlob) / 2;
//           temp_blobSize[blobCount] = float(i - beginBlob) / (arraySize * 8);
//           beginBlob = -1;
//           blobCount++;
//         }
//     }

//     for (byte i=0; i < sizeof(temp_blobArray)/sizeof(temp_blobArray[0]); ++i) {
//       temp_blobArray[i] = 0;
//     }
//     for (byte i=0; i < sizeof(temp_blobPos)/sizeof(temp_blobPos[0]); ++i) {
//       if (temp_blobPos[i] != -1) {
//         //bitWrite(temp_blobArray[temp_blobPos[i]/8], (7-(temp_blobPos[i]%8)), 1);        
//       }
//       else {
//         break;
//       }
//     }

//     for (byte n=0; n<8; n++) {
//         blobArray[n] = temp_blobArray[n];
//     }
//     for (byte n=0; n<4; n++) {
//         blobPos[n] = temp_blobPos[n];
//         blobSize[n] = temp_blobSize[n];
//     }
// }

// Simple leaky integrator implementation
//
// Create a unsigned long global variable for time counter for each leak implementation (timer)

float Instrument_touch::leakyIntegrator (float reading, float old_value, float leak, int frequency, unsigned long& timer) {
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

float Instrument_touch::arrayAverageZero (float * Array, byte ArraySize) {
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

void Instrument_touch::bitShiftArrayL (byte * origArray, byte * shiftedArray, byte arraySize, byte shift) {

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