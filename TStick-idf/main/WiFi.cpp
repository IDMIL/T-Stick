#include "TStick.h"

void connectToWifi() {

  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nThe T-Stick will try to connect to the saved network now...");
    // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(tstickSSID);
    WiFi.begin(Tstick.lastConnectedNetwork, Tstick.lastStoredPsk);
    time_now = millis();
    while ( (WiFi.status() != WL_CONNECTED) && (millis() < time_now + waitForConnection) ) {
      Serial.print(".");
      delay(500);
      }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nThe T-Stick is connected to "); Serial.print(WiFi.SSID()); Serial.println(" network");
      
      Serial.print("IP address: "); Serial.println(WiFi.localIP());
      }
    else {Serial.println("Failed to connect to the saved network. Enter setup to connect to a different network.");}
    }
  else {
    Serial.print("The T-Stick is connected to "); Serial.print(WiFi.SSID()); Serial.println(" network");
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    }

  // Disable WiFi power save (huge latency improvements)
  esp_wifi_set_ps(WIFI_PS_NONE);

  oscEndpoint.begin(portLocal);
  Serial.println("Starting UDP (listening to OSC messages)");
  Serial.print("Local port: ");
  #ifdef ESP8266
      Serial.println(oscEndpoint.localPort());
  #else
      Serial.println(portLocal);
  #endif
}


void Wifimanager_portal(char *portal_name, char *portal_password) {

  //WiFiManager
  WiFiManager wifiManager;

  char wifimanagerbuf[50];
  char* copybuf;

  char libmapperCheck[24] = "type=\"checkbox\"";
  char OSCCheck[24] = "type=\"checkbox\"";

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  WiFiManagerParameter wifimanager_APpasswd("APpasswd", "T-Stick password", APpasswdTemp, 15);
  WiFiManagerParameter wifimanager_APpasswdValidate("APpasswdValidate", "Type T-Stick password again", APpasswdValidate, 15);
  WiFiManagerParameter wifimanager_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave Password fields empty</small>");  
  
  snprintf(wifimanagerbuf,(sizeof(wifimanagerbuf)-1),"%s,%s",Tstick.oscIP[0],Tstick.oscIP[1]);
  WiFiManagerParameter wifimanager_oscIP("server", "IP to send OSC messages", wifimanagerbuf, 40);

  snprintf(wifimanagerbuf,(sizeof(wifimanagerbuf)-1),"%i,%i",Tstick.oscPORT[0],Tstick.oscPORT[1]);
  WiFiManagerParameter wifimanager_oscPORT("port", "port to send OSC messages", wifimanagerbuf, 16);

  if (Tstick.libmapper == 1) { strcat(libmapperCheck, " checked"); }
    WiFiManagerParameter wifimanager_libmapper("libmapper", "Libmapper ON/OFF (WiFiManager exit required)", "T", 2, libmapperCheck, WFM_LABEL_AFTER);

  if (Tstick.osc == 1) { strcat(OSCCheck, " checked"); }
    WiFiManagerParameter wifimanager_osc("OSC", "OSC messages ON/OFF (WiFiManager exit required)", "T", 2, OSCCheck, WFM_LABEL_AFTER);
    
  fsrbuf = float(Tstick.FSRoffset) / 4095;
  WiFiManagerParameter wifimanager_FSRoffset("FSRoffset", "FSR offset value (default = 0)", dtostrf(fsrbuf,2,3,wifimanagerbuf), 6);

  // snprintf(wifimanagerbuf,(sizeof(wifimanagerbuf)-1),"%i,%i,%i,%i,%i,%i,%i,%i",
  //     Tstick.touchMask[0],Tstick.touchMask[1],
  //     Tstick.touchMask[2],Tstick.touchMask[3],
  //     Tstick.touchMask[4],Tstick.touchMask[5],
  //     Tstick.touchMask[6],Tstick.touchMask[7]);
      
  WiFiManagerParameter wifimanager_touchMask("touchMask", "Touch Mask capacitive sensing values", wifimanagerbuf, 40);  
  
  WiFiManagerParameter wifimanager_id("id", "T-Stick serial number", itoa(Tstick.id,wifimanagerbuf,10), 6, "readonly");
  WiFiManagerParameter wifimanager_type("type", "T-Stick size (type)", Tstick.type, 4, "readonly");
  WiFiManagerParameter wifimanager_author("author", "T-Stick firmware revision", Tstick.author, 20, "readonly");
  WiFiManagerParameter wifimanager_color("color", "T-Stick color", Tstick.color, 10, "readonly");
  WiFiManagerParameter wifimanager_firmware("firmware", "T-Stick firmware revision", itoa(Tstick.firmware,wifimanagerbuf,10), 10, "readonly");

  //add all your parameters here
  wifiManager.addParameter(&wifimanager_APpasswd);
  wifiManager.addParameter(&wifimanager_APpasswdValidate);
  wifiManager.addParameter(&wifimanager_hint);
  wifiManager.addParameter(&wifimanager_osc);
  wifiManager.addParameter(&wifimanager_oscIP);
  wifiManager.addParameter(&wifimanager_oscPORT);
  wifiManager.addParameter(&wifimanager_libmapper);
  wifiManager.addParameter(&wifimanager_FSRoffset);
  wifiManager.addParameter(&wifimanager_touchMask);
  wifiManager.addParameter(&wifimanager_id);
  wifiManager.addParameter(&wifimanager_type);
  wifiManager.addParameter(&wifimanager_author);
  wifiManager.addParameter(&wifimanager_color);
  wifiManager.addParameter(&wifimanager_firmware);

  if (!wifiManager.startConfigPortal(portal_name, portal_password)) {
    Serial.println("Failed to connect and hit timeout");
    Serial.println("ESP.restart");
    ESP.restart(); //reset and try again, or maybe put it to deep sleep
    }

  //if you get here you have connected to the WiFi
  Serial.println("The T-Stick is connected to the chosen network");

  //read updated T-Stick parameters
  if (wifimanager_APpasswd.getValue()[0] != NULL) {
    if (strcmp(wifimanager_APpasswd.getValue(), wifimanager_APpasswdValidate.getValue()) == 0 ) {  
      strcpy(Tstick.APpasswd, wifimanager_APpasswd.getValue());
      Serial.println("T-Stick AP password changed");
      } 
    else {Serial.println("T-Stick AP passwords don't match! Not saving new password.");}
    }
  else {
    Serial.println("T-Stick AP password fields are blank. Not saving new password.");
    }
  WiFi.SSID().toCharArray(Tstick.lastConnectedNetwork,sizeof(Tstick.lastConnectedNetwork));
  WiFi.psk().toCharArray(Tstick.lastStoredPsk,sizeof(Tstick.lastStoredPsk));
  
  strcpy(wifimanagerbuf,wifimanager_oscIP.getValue());
  copybuf = strtok(wifimanagerbuf,",");
  if (copybuf != NULL) {
    strcpy(Tstick.oscIP[0],copybuf);
  }
  copybuf = strtok(NULL,",");
  if (copybuf == NULL) {
    strcpy(Tstick.oscIP[1],"0.0.0.0");
  }
  else {
    strcpy(Tstick.oscIP[1],copybuf);
  }  

  strcpy(wifimanagerbuf,wifimanager_oscPORT.getValue());
  copybuf = strtok(wifimanagerbuf,",");
  if (copybuf != NULL) {
    Tstick.oscPORT[0] = atoi(copybuf);
  }
  copybuf = strtok(NULL,",");
  if (copybuf != NULL) {
    Tstick.oscPORT[1] = atoi(copybuf);
  }
  
  if (strncmp(wifimanager_libmapper.getValue(), "T", 1) == 0) {
    Tstick.libmapper = 1;
  } 
  else {
    Tstick.libmapper = 0;
  }

  if (strncmp(wifimanager_osc.getValue(), "T", 1) == 0) {
    Tstick.osc = 1;
  } 
  else {
    Tstick.osc = 0;
  }
  
  fsrbuf = atof(wifimanager_FSRoffset.getValue());
  fsrbuf *= 4095;
  Tstick.FSRoffset = fsrbuf;
  
  // strcpy(wifimanagerbuf,wifimanager_touchMask.getValue());
  // copybuf = strtok(wifimanagerbuf,",");
  // if (copybuf != NULL) {
  //   Tstick.touchMask[0] = atoi(copybuf);
  //   copybuf = strtok(NULL,",");
  //   for (byte i=1; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask)[0]); ++i) {
  //     copybuf = strtok(NULL,",");
  //     if (copybuf == NULL) {break;}
  //     Tstick.touchMask[i] = atoi(copybuf);
  //     copybuf = strtok(NULL,",");
  //   }
  // }

  Serial.print("\nThe T-Stick is connected to "); Serial.print(WiFi.SSID()); Serial.println(" network");
  Serial.print("\nlocal ip: "); Serial.println(WiFi.localIP());

  //save the modified T-Stick parameters to Json file
  saveJSON();

  createTstickSSID();
  
  digitalWrite(ledPin, ledStatus);

  // Disable WiFi power save (huge latency improvements)
  esp_wifi_set_ps(WIFI_PS_NONE);
}


void createTstickSSID () {
  itoa(Tstick.id,tstickID,10);
  strcpy (tstickSSID,"T-Stick_");
  strcat (tstickSSID, Tstick.type);
  strcat (tstickSSID,"_");
  strcat (tstickSSID, Tstick.color);
  strcat (tstickSSID,"_");
  strcat (tstickSSID, tstickID);
}

