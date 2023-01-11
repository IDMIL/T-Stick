# T-Stick OSC sensor data

## firmware 220916

#### (Replace XXX for the T-Stick ID number)

## Raw data

/TStick_XXX/raw/capsense, i..., 0--255, ... (1 int per 8 capacitive stripes -- 8 bits)

/TStick_XXX/raw/fsr, i, 0--4095

/TStick_XXX/raw/accl, fff, +/-24 (floats)

/TStick_XXX/raw/gyro, fff, +/-42 (floats)

/TStick_XXX/raw/magn, iii, +/-0.001 (floats)

## Instrument data

/TStick_XXX/instrument/button/count, i, 0--i

/TStick_XXX/instrument/button/tap, i, 0 or 1

/TStick_XXX/instrument/button/dtap, i, 0 or 1

/TStick_XXX/instrument/button/ttap, i, 0 or 1

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
