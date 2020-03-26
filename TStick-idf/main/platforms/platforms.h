#pragma once

#if defined(ESP32)
#include "ESP32.h"
#elif defined(ESP8266)
#include "ESP8266.h"
#else
#error Missing platform.
#endif

#if defined(SOPRANINO)
#include "Sopranino.h"
#elif defined(SOPRANO)
#include "Soprano.h"
#else
#error Missing hardware.
#endif
