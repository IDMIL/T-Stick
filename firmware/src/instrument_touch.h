// Some high-level gestural descriptors of the GuitarAMI
// using the Touch
// Edu Meneses - 2020 (IDMIL)

#ifndef INSTRUMENTTOUCH_H
#define INSTRUMENTINSTRUMENTTOUCH_H

#include <Arduino.h>

class Instrument_touch {
    private:
        int touchSize = 30;
        byte touchSizeEdge = 4; // amount of T-Stick stripes for top and bottom portions of the T-Stick (arbitrary)
        int maxTouchValue;
        float touchAverage (float * touchArrayStrips, int firstStrip, int lastStrip);
        int lastState_blobPos[4];
        struct blob { 
            byte blobArray[8];  // shows the "center" of each array
            int blobPos[4];     // position (index) of each blob
            float blobSize[4];  // "size" of each blob
        } BlobDetection;
        blob blobDetection1D (byte * touchArray, byte arraySize);
        float leakyIntegrator (float reading, float old_value, float leak, int frequency, unsigned long& timer);
        const int leakyBrushFreq = 100; // leaking frequency (Hz)
        unsigned long leakyBrushTimer = 0;
        const int leakyRubFreq = 100;
        unsigned long leakyRubTimer = 0;
        byte brushCounter[4];
        const int leakyShakeFreq = 10;
        unsigned long leakyShakeTimer[3];
        float arrayAverageZero (float * Array, byte ArraySize);
        void bitShiftArrayL (byte * origArray, byte * shiftedArray, byte arraySize, byte shift);
    public:
        void updateInstrument(int *raw_touch);
        byte touchStatus = 0;  // /instrument/touch/status, i..., 0--1, ... (1 per stripe)
        byte touch[30];         // /instrument/touch/touch, i..., 0--1, ... (1 per stripe)
        float normTouch[30];    // /instrument/touch/norm, i..., 0--1, ... (1 per stripe)
        float touchAll;         // /instrument/touch/all, f, 0--1
        float touchTop;         // /instrument/touch/top, f, 0--1
        float touchMiddle;      // /instrument/touch/middle, f, 0--1
        float touchBottom;      // /instrument/touch/bottom, f, 0--1
        float brush;            // /instrument/touch/brush, f, 0--? (~cm/s)
        float multiBrush[4];    // /instrument/touch/brush/multibrush, ffff, 0--? (~cm/s)
        float rub;              // /instrument/touch/rub, f, 0--? (~cm/s)
        float multiRub[4];      // /instrument/touch/rub/multirub, ffff, 0--? (~cm/s)
};

#endif