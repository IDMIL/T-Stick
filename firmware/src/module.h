// GuitarAMI Module - WiFi and file system functions
// Edu Meneses
// IDMIL - McGill University (2020)

#ifndef MODULE_H
#define MODULE_H

#include <FS.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <WiFiUdp.h>

#include <DNSServer.h>
#include <AsyncTCP.h> // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

class Module {
  private:
    
  public:
    String wifiScanResults;
    void mountFS();
    void printJSON();
    void scanWiFi(char *deviceName, int mode, char *apPSK, char *wifiSSID, char *wifiPSK);
    void startWifi(char *deviceName, int mode, char *apPSK, char *wifiSSID, char *wifiPSK);
};

#endif