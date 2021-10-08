
void Wifimanager_init(bool WiFiSerialDebug) {

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json and enabling WiFi32Manager debug
  if (WiFiSerialDebug == true) {
    Serial.println("mounting FS...");
  }

    #if defined(ESP8266)
    #else
      SPIFFS.begin (true);
    #endif

  if (SPIFFS.begin()) {
    if (WiFiSerialDebug == true) {
      Serial.println("mounted file system");
    }
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      if (WiFiSerialDebug == true) {
        Serial.println("reading config file");
      }
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        if (WiFiSerialDebug == true) {
          Serial.println("opened config file");
        }
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        // reading info from Json
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (WiFiSerialDebug == true) {
          json.printTo(Serial);
        }
        if (json.success()) {
          if (WiFiSerialDebug == true) {
            Serial.println("\nparsed json");
          }
          strcpy(oscIP, json["oscIP"]);
          strcpy(oscPORT, json["oscPORT"]);
          strcpy(device, json["device"]);
          strcpy(APpasswd, json["APpasswd"]);
          strcpy(APpasswdTemp, json["APpasswd"]);
          strcpy(APpasswdValidate, json["APpasswdValidate"]);
          strcpy(directSendOSCCHAR, json["directSendOSC"]);
          strcpy(infoTstickCHAR0, json["infoTstick0"]);
          strcpy(infoTstickCHAR1, json["infoTstick1"]);
          strcpy(calibrateCHAR, json["calibratetrig"]);
          strcpy(calibrationDataCHAR0, json["calibrationData0"]);
          strcpy(calibrationDataCHAR1, json["calibrationData1"]);
          strcpy(touchMaskCHAR0, json["touchMaskData0"]);
          strcpy(touchMaskCHAR1, json["touchMaskData1"]);
        } else {
          if (WiFiSerialDebug == true) {
            Serial.println("failed to load json config");
          }
        }
      }
    }
  } else {
    if (WiFiSerialDebug == true) {
      Serial.println("failed to mount FS");
    }
  } //end read configuration from FS json

  // converting OSC IP Address and port
  char_conversion();
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
  unsigned long startedAt = millis();
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis() - startedAt);
  if (WiFiSerialDebug == true) {
    Serial.print("Stored SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("Stored password: ");
    Serial.println(WiFi.psk());
    Serial.print("After waiting "); Serial.print(waited / 1000); Serial.print(" secs in setup() connection result is "); Serial.println(connRes);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("CONNECTION STATUS: Failed to connect up to this point, finishing setup anyway");
      Serial.println("                   If using ESP32, I'll try to connect again later");
    } else {
      Serial.println("");
      Serial.print("local ip: "); Serial.println(WiFi.localIP());
      Serial.print("OSC ip: "); Serial.println(oscEndpointIP);
      Serial.print("OSC port: "); Serial.println(oscEndpointPORT);
    }
  }

  #if defined(ESP32)
    WiFi.SSID().toCharArray(stored_ssid, 20);
    WiFi.psk().toCharArray(stored_psk, 20); 
    if (WiFi.status() != WL_CONNECTED) {
      if (WiFiSerialDebug == true) {Serial.println("ESP32 2nd attempt to connect");}
      WiFi.begin(stored_ssid, stored_psk);
    }
  #endif

  
  oscEndpoint.begin(portLocal);
  if (WiFiSerialDebug == true) {
    Serial.println("Starting UDP");
    Serial.print("Local port: ");
    #ifdef ESP8266
        Serial.println(oscEndpoint.localPort());
    #else
        Serial.println(portLocal);
    #endif
  }
}

void Wifimanager_portal(char *portal_name, char *portal_password, bool shouldSaveConfig, bool WiFiSerialDebug) {

  //WiFiManager
  WiFiManager wifiManager;

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  
  char directSendOSCcheck[24] = "type=\"checkbox\"";
  char calibrateCheck[24] = "type=\"checkbox\"";

  WiFiManagerParameter custom_device("device", "T-Stick name (also AP SSID)", device, 25);
  WiFiManagerParameter custom_hint("<small>Original number: 190 (IDMIL)<br>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");
  //WiFiManagerParameter custom_hint(hinttext);
  WiFiManagerParameter custom_APpasswd("APpasswd", "Access Point SSID password", APpasswdTemp, 15);
  WiFiManagerParameter custom_APpasswdValidate("APpasswdValidate", "Type password again", APpasswdValidate, 15);
  WiFiManagerParameter custom_warning("<small>Be careful: if you forget your new password you'll not be able to connect!</small>");
  WiFiManagerParameter custom_oscIP("server", "IP to send OSC messages", oscIP, 17);
  WiFiManagerParameter custom_oscPORT("port", "port to send OSC messages", oscPORT, 7);
  if (directSendOSC == 1) { strcat(directSendOSCcheck, " checked"); }
    WiFiManagerParameter custom_directSendOSC("directSendOSC", "DirectSend OSC mode (WiFiManager exit required)", "T", 2, directSendOSCcheck, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_infoTstick0("info0", "T-Stick serial number", infoTstickCHAR0, 6);
  WiFiManagerParameter custom_infoTstick1("info1", "T-Stick firmware revision", infoTstickCHAR1, 6);
  if (calibrate == 1) { strcat(calibrateCheck, " checked"); }
    WiFiManagerParameter custom_calibratetrig("calt", "Pressure sensor calibration ON/OFF (WiFiManager exit required)", "T", 2, calibrateCheck, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_calibrationData0("cal0", "Pressure sensor min calibration value (default = 0)", calibrationDataCHAR0, 6);
  WiFiManagerParameter custom_calibrationData1("cal1", "Pressure sensor max calibration value (default = 4095)", calibrationDataCHAR1, 6);
  WiFiManagerParameter custom_touchMaskData0("touchMask0", "Touch Mask capacitive sensing value (1/2)", touchMaskCHAR0, 7);
  WiFiManagerParameter custom_touchMaskData1("touchMask1", "Touch Mask capacitive sensing value (2/2)", touchMaskCHAR1, 7);

  //add all your parameters here
  wifiManager.addParameter(&custom_device);
  wifiManager.addParameter(&custom_hint);
  wifiManager.addParameter(&custom_APpasswd);
  wifiManager.addParameter(&custom_APpasswdValidate);
  wifiManager.addParameter(&custom_warning);
  wifiManager.addParameter(&custom_oscIP);
  wifiManager.addParameter(&custom_oscPORT);
  wifiManager.addParameter(&custom_directSendOSC);
  wifiManager.addParameter(&custom_infoTstick0);
  wifiManager.addParameter(&custom_infoTstick1);
  wifiManager.addParameter(&custom_calibratetrig);
  wifiManager.addParameter(&custom_calibrationData0);
  wifiManager.addParameter(&custom_calibrationData1);
  wifiManager.addParameter(&custom_touchMaskData0);
  wifiManager.addParameter(&custom_touchMaskData1);

  if (!wifiManager.startConfigPortal(portal_name, portal_password)) {
    if (WiFiSerialDebug == true) {
      Serial.println("failed to connect and hit timeout");
    }
    delay(3000);
    ESP.restart(); //reset and try again, or maybe put it to deep sleep
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  if (WiFiSerialDebug == true) {
    Serial.println("Connected to the network");
  }

  //read updated OSC and port parameters
  strcpy(oscIP, custom_oscIP.getValue());
  strcpy(oscPORT, custom_oscPORT.getValue());
  strcpy(device, custom_device.getValue());
  strcpy(APpasswdTemp, custom_APpasswd.getValue());
  strcpy(APpasswdValidate, custom_APpasswdValidate.getValue());
  if (strncmp(custom_directSendOSC.getValue(), "T", 1) == 0) {strcpy(directSendOSCCHAR, one);}
  else {strcpy(directSendOSCCHAR, zero);}
  strcpy(infoTstickCHAR0, custom_infoTstick0.getValue());
  strcpy(infoTstickCHAR1, custom_infoTstick1.getValue());
  if (strncmp(custom_calibratetrig.getValue(), "T", 1) == 0) {strcpy(calibrateCHAR, one);}
  else {strcpy(calibrateCHAR, zero);}
  strcpy(calibrationDataCHAR0, custom_calibrationData0.getValue());
  strcpy(calibrationDataCHAR1, custom_calibrationData1.getValue());
  strcpy(touchMaskCHAR0, custom_touchMaskData0.getValue());
  strcpy(touchMaskCHAR1, custom_touchMaskData1.getValue());

  // validating some fields
  if (strcmp(APpasswdTemp, APpasswdValidate) == 0 ) {
    strcpy(APpasswd, APpasswdTemp);
  }
  else {
    strcpy(APpasswdTemp, APpasswd);
    strcpy(APpasswdValidate, APpasswd);
  }

  if (shouldSaveConfig) {
    save_to_json(DEBUG);  //save the custom parameters to FS
  }
  if (WiFiSerialDebug == true) {
    Serial.println("");
    Serial.print("local ip: "); Serial.println(WiFi.localIP());
    Serial.print("OSC ip: "); Serial.println(oscEndpointIP);
    Serial.print("OSC port: "); Serial.println(oscEndpointPORT);
  }
  char_conversion(); // converting OSC IP Address and port
  digitalWrite(ledPin, LOW);
}


void char_conversion() {
  oscEndpointIP.fromString(oscIP);
  oscEndpointPORT = atoi(oscPORT);
  directSendOSC = atoi(directSendOSCCHAR);
  infoTstick[0] = atoi(infoTstickCHAR0);
  infoTstick[1] = atoi(infoTstickCHAR1);
  calibrate = atoi(calibrateCHAR);
  calibrationData[0] = atoi(calibrationDataCHAR0);
  calibrationData[1] = atoi(calibrationDataCHAR1);
  touchMask[0] = atoi(touchMaskCHAR0);
  touchMask[1] = atoi(touchMaskCHAR1);
}

void save_to_json(bool WiFiSerialDebug) {
  if (WiFiSerialDebug == true) {
    Serial.println("saving config to json");
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["device"] = device;
  json["APpasswd"] = APpasswd;
  json["APpasswdValidate"] = APpasswdValidate;
  json["directSendOSC"] = directSendOSCCHAR;
  json["oscIP"] = oscIP;
  json["oscPORT"] = oscPORT;
  json["infoTstick0"] = infoTstickCHAR0;
  json["infoTstick1"] = infoTstickCHAR1;
  json["calibratetrig"] = calibrateCHAR;
  json["calibrationData0"] = calibrationDataCHAR0;
  json["calibrationData1"] = calibrationDataCHAR1;
  json["touchMaskData0"] = touchMaskCHAR0;
  json["touchMaskData1"] = touchMaskCHAR1;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    if (WiFiSerialDebug == true) {
      Serial.println("failed to open config file for writing");
    }
  }
  //json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  //end save
}
