# T-Stick firmware 200422 - OSC sensor data

#### (Replace XXX for the T-Stick ID number)

## Raw data

/TStick_XXX/raw/capsense, i..., 0--255, ... (1 int per 8 capacitive stripes -- 8 bits)

/TStick_XXX/raw/button/short, i, 0 or 1

/TStick_XXX/raw/button/long, i, 0 or 1

/TStick_XXX/raw/button/double, i, 0 or 1

/TStick_XXX/raw/fsr, i, 0--4095

/TStick_XXX/raw/piezo, i, 0--1023

/TStick_XXX/raw/accl, iii, +/-32767 (integers)

/TStick_XXX/raw/gyro, fff, +/-34.90659 (floats)

/TStick_XXX/raw/magn, iii, +/-32767 (integers)

/raw (IMU data to be send to callibration app)

## Normalized data

/TStick_XXX/norm/fsr, f, 0--1

/TStick_XXX/norm/piezo, f, 0--1

/TStick_XXX/norm/accl, fff, +/-1, +/-1, +/-1

/TStick_XXX/norm/gyro, fff, +/-1, +/-1, +/-1

/TStick_XXX/norm/magn, fff, +/-1, +/-1, +/-1

## Instrument data

/TStick_XXX/orientation, ffff, ?, ? ,? ,?

/TStick_XXX/instrument/ypr, fff, +/-180, +/-90 ,+/-180 (degrees)

/TStick_XXX/instrument/touch/all, f, 0--1

/TStick_XXX/instrument/touch/top, f, 0--1

/TStick_XXX/instrument/touch/middle, f, 0--1

/TStick_XXX/instrument/touch/bottom, f, 0--1

/TStick_XXX/instrument/brush, f, 0--? (~cm/s)

/TStick_XXX/instrument/multibrush, ffff, 0--? (~cm/s)

/TStick_XXX/instrument/rub, f, 0--? (~cm/s)

/TStick_XXX/instrument/multirub, ffff, 0--? (~cm/s)

/TStick_XXX/instrument/shakexyz, fff, 0--?

/TStick_XXX/instrument/jabxyz, fff, 0--?

/TStick_XXX/battery
