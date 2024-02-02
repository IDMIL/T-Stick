#include "tstick-sensors.h"
#ifdef touch_TRILL
#include "Trill-touch/trill-touch.h"
TrillTouch touch;
#endif
#ifdef touch_ENCHANTI
#include "Enchanti-touch/enchanti-touch.h"
EnchantiTouch touch;
#endif
#ifdef touch_IDMIL
#include "Idmil-touch/idmil-touch.h"
#endif