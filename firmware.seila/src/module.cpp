// T-Stick - WiFi and file system functions
// Edu Meneses
// IDMIL - McGill University (2020)

#include <module.h>

void Module::mountFS() {

  SPIFFS.begin (true);
  
  if (SPIFFS.begin())
    Serial.println("\nFile system mounted!"); 
  else
    Serial.println("Failed to mount file system.\n");
}
    
    
void Module::printJSON() {
  if (SPIFFS.exists("/config.json")) { // file exists, reading and loading
    Serial.println("Reading config file for print...");
    File configFile = SPIFFS.open("/config.json", "r");   // Open file for reading
      if (configFile) {
        Serial.println("Config file:");
        while(configFile.available()){Serial.write(configFile.read());}
        Serial.println("\n");
        }
      else {
        Serial.println("Cannot read config file config.json. File doesn't exist.\n");
        }
  }
}


void Module::scanWiFi(char *deviceName, int mode, char *apPSK, char *wifiSSID, char *wifiPSK) {

  Serial.println("\nSSID scan start");
  bool connectedBefore = false;
  Module::wifiScanResults.clear();

  if (WiFi.status() == WL_CONNECTED) {
    connectedBefore = true;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
  }
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found\n");
  } else {
      Serial.print(n);
      Serial.println(" networks found\n");
      for (int i = 0; i < n; ++i) {
        wifiScanResults += i + 1;
        wifiScanResults += ": ";
        wifiScanResults += WiFi.SSID(i);
        wifiScanResults += " (";
        wifiScanResults += WiFi.RSSI(i);
        wifiScanResults += ")";
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
          wifiScanResults += "(no password required)";
        wifiScanResults += "<br>";
        delay(10);
      }
  }

  if (connectedBefore) {
    connectedBefore = false;
    Module::startWifi(deviceName, mode, apPSK, wifiSSID, wifiPSK);
  }
}


void Module::startWifi(char *deviceName, int mode, char *apPSK, char *wifiSSID, char *wifiPSK) {
  // Modes: 0:STA, 1:AP, 2:MIDI, 3:Setup(STA+AP+WebServer)

  if (mode == 0) {
    WiFi.mode(WIFI_STA);
    Serial.print("The "); Serial.print(deviceName); Serial.println(" WiFi is set to station mode");
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(" Trying to connect to the network now...");
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      WiFi.setHostname(deviceName);
      WiFi.begin(wifiSSID, wifiPSK);
      long time_now = millis();
      while ( (WiFi.status() != WL_CONNECTED) && (millis() < time_now + 8000) ) {
        Serial.print(".");
        delay(100);
      }
      if (WiFi.status() == WL_CONNECTED) {
        //delay(1000);
        Serial.print("\nThe ");Serial.print(deviceName);Serial.print(" is connected to ");Serial.print(WiFi.SSID());
        Serial.print(" network (IP address: ");Serial.print(WiFi.localIP());Serial.println(")");
      } else {
        Serial.println("\nFailed to connect to the network. Enter setup to connect to a different network.");
      }
    }
  }

  if (mode == 1) {
    WiFi.mode(WIFI_AP);
    //WiFi.softAPConfig(AP_local_IP, AP_gateway, AP_subnet);
    Serial.print("The "); Serial.print(deviceName); Serial.println(" WiFi is set to AP mode");
    WiFi.softAP(deviceName, apPSK);
    Serial.println(" Starting AP...");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP: "); Serial.println(myIP);
  }

  if (mode == 2) {
    Serial.print("The "); Serial.print(deviceName); Serial.println(" WiFi is set to OFF (MIDI mode)");
  }

  if (mode == 3) { // setup mode (STA + AP)
    WiFi.mode(WIFI_AP_STA);
    Serial.print("The "); Serial.print(deviceName); Serial.println(" WiFi is set to station+softAP (setup) mode");
    WiFi.softAP(deviceName, apPSK);
    Serial.println(" Starting AP...");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP: "); Serial.println(myIP);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(" Trying to connect to the network now...");
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      WiFi.setHostname(deviceName);
      WiFi.begin(wifiSSID, wifiPSK);
      long time_now = millis();
      while ( (WiFi.status() != WL_CONNECTED) && (millis() < time_now + 8000) ) {
        Serial.print(".");
        delay(100);
      }
      if (WiFi.status() == WL_CONNECTED) {
        //delay(1000);
        Serial.print("\nThe ");Serial.print(deviceName);Serial.print(" is connected to ");Serial.print(WiFi.SSID());
        Serial.print(" network (IP address: ");Serial.print(WiFi.localIP());Serial.println(")");
      }
      if (WiFi.status() == WL_DISCONNECTED)
        Serial.println("\nFailed to connect to the network. Enter setup to connect to a different network.");
    }
  }
  // Disable WiFi power save (huge latency improvements?)
      esp_wifi_set_ps(WIFI_PS_NONE);
}


