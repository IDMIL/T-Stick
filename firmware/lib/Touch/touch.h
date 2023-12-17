#ifndef TOUCH_H
#define TOUCH_H

#ifdef touch_TRILL
#include "Trill-touch/trill-touch.h"
TrillTouch touch;
#endif
#ifdef touch_ENCHANTI
#include "Enchanti-touch/enchanti-touch."
EnchantiTouch touch;
#endif
#ifdef touch_IDMIL
#include "Idmil-touch/idmil-touch.h"
#endif

#endif