
void connectToWifi() {

  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nThe T-Stick will try to connect to the saved network now...");
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

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  
  char calibrateCheck[24] = "type=\"checkbox\"";

  WiFiManagerParameter wifimanager_device("device", "\nT-Stick name (also AP SSID)", Tstick.device, 25);
  WiFiManagerParameter wifimanager_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");
  WiFiManagerParameter wifimanager_APpasswd("APpasswd", "T-Stick password", APpasswdTemp, 15);
  WiFiManagerParameter wifimanager_APpasswdValidate("APpasswdValidate", "Type T-Stick password again", APpasswdValidate, 15);
  WiFiManagerParameter wifimanager_warning("<small>Be careful: if you forget your new password you'll not be able to connect!</small>");
  WiFiManagerParameter wifimanager_oscIP("server", "IP to send OSC messages", Tstick.oscIP, 17);
  WiFiManagerParameter wifimanager_oscPORT("port", "port to send OSC messages", itoa(Tstick.oscPORT,wifimanagerbuf,10), 7);
  if (Tstick.FSRcalibration == 1) { strcat(calibrateCheck, " checked"); }
    WiFiManagerParameter wifimanager_calibratetrig("calt", "FSR \"calibration\" ON/OFF (WiFiManager exit required)", "T", 2, calibrateCheck, WFM_LABEL_AFTER);
  WiFiManagerParameter wifimanager_FSRcalibrationValue0("cal0", "FSR min calibration value (default = 0)", itoa(Tstick.FSRcalibrationValues[0],wifimanagerbuf,10), 6);
  WiFiManagerParameter wifimanager_FSRcalibrationValue1("cal1", "FSR max calibration value (default = 4095)", itoa(Tstick.FSRcalibrationValues[1],wifimanagerbuf,10), 6);
  WiFiManagerParameter wifimanager_touchMask0("touchMask0", "Touch Mask capacitive sensing value (1/2)", itoa(Tstick.touchMask[0],wifimanagerbuf,10), 7);
  WiFiManagerParameter wifimanager_touchMask1("touchMask1", "Touch Mask capacitive sensing value (2/2)", itoa(Tstick.touchMask[1],wifimanagerbuf,10), 7);
  WiFiManagerParameter wifimanager_id("id", "T-Stick serial number", itoa(Tstick.id,wifimanagerbuf,10), 6);
  WiFiManagerParameter wifimanager_firmware("firmware", "T-Stick firmware revision", itoa(Tstick.firmware,wifimanagerbuf,10), 6);

  //add all your parameters here
  wifiManager.addParameter(&wifimanager_device);
  wifiManager.addParameter(&wifimanager_hint);
  wifiManager.addParameter(&wifimanager_APpasswd);
  wifiManager.addParameter(&wifimanager_APpasswdValidate);
  wifiManager.addParameter(&wifimanager_warning);
  wifiManager.addParameter(&wifimanager_oscIP);
  wifiManager.addParameter(&wifimanager_oscPORT);
  wifiManager.addParameter(&wifimanager_calibratetrig);
  wifiManager.addParameter(&wifimanager_FSRcalibrationValue0);
  wifiManager.addParameter(&wifimanager_FSRcalibrationValue1);
  wifiManager.addParameter(&wifimanager_touchMask0);
  wifiManager.addParameter(&wifimanager_touchMask1);
  wifiManager.addParameter(&wifimanager_id);
  wifiManager.addParameter(&wifimanager_firmware);

  if (!wifiManager.startConfigPortal(portal_name, portal_password)) {
    Serial.println("Failed to connect and hit timeout");
    Serial.println("ESP.restart");
    ESP.restart(); //reset and try again, or maybe put it to deep sleep
    }

  //if you get here you have connected to the WiFi
  Serial.println("The T-Stick is connected to the chosen network");

  //read updated T-Stick parameters
  strcpy(Tstick.device, wifimanager_device.getValue());
  if (wifimanager_APpasswd.getValue()[0] != NULL) {
    if (strcmp(wifimanager_APpasswd.getValue(), wifimanager_APpasswdValidate.getValue()) == 0 ) {  
      strcpy(Tstick.APpasswd, wifimanager_APpasswd.getValue());
      Serial.println("T-Stick AP password changed");
      } else {Serial.println("T-Stick AP passwords don't match! Not saving new password.");}
    }
  else {
    Serial.println("T-Stick AP password fields are blank. Not saving new password.");
    }
  strcpy(Tstick.oscIP, wifimanager_oscIP.getValue());
  osc_IP.fromString(Tstick.oscIP);
  Tstick.oscPORT = atoi(wifimanager_oscPORT.getValue());
  Tstick.id = atoi(wifimanager_id.getValue());
  Tstick.firmware = atoi(wifimanager_firmware.getValue());
  if (strncmp(wifimanager_calibratetrig.getValue(), "T", 1) == 0) {Tstick.FSRcalibration = 1;} else {Tstick.FSRcalibration = 0;}
  Tstick.FSRcalibrationValues[0] = atoi(wifimanager_FSRcalibrationValue0.getValue());
  Tstick.FSRcalibrationValues[1] = atoi(wifimanager_FSRcalibrationValue1.getValue());
  Tstick.touchMask[0] = atoi(wifimanager_touchMask0.getValue());
  Tstick.touchMask[1] = atoi(wifimanager_touchMask1.getValue());
  WiFi.SSID().toCharArray(Tstick.lastConnectedNetwork,sizeof(Tstick.lastConnectedNetwork));
  WiFi.psk().toCharArray(Tstick.lastStoredPsk,sizeof(Tstick.lastStoredPsk));

  Serial.print("\nThe T-Stick is connected to "); Serial.print(WiFi.SSID()); Serial.println(" network");
  Serial.print("\nlocal ip: "); Serial.println(WiFi.localIP());

  //save the modified T-Stick parameters to Json file
  saveJSON();
  
  digitalWrite(ledPin, ledStatus);
}
