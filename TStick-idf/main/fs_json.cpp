#include "SPIFFS.h"
#include "TStick.h"

void mountFS() {

  SPIFFS.begin (true);
  
  if (SPIFFS.begin()) {
    Serial.println("\nFile system mounted!");
    } 
  else {
    Serial.println("Failed to mount file system.\n");
    }
}
    
    
void printJSON() {
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


void printVariables() {
  // Serial.println("Printing loaded values (variables):");
  // Serial.print("Tstick.id: "); Serial.println(Tstick.id);
  // Serial.print("Tstick.type: "); Serial.println(Tstick.type);
  // Serial.print("Tstick.author: "); Serial.println(Tstick.author);
  // Serial.print("Tstick.color: "); Serial.println(Tstick.color);
  // Serial.print("Tstick.APpasswd: "); Serial.println(Tstick.APpasswd);
  // Serial.print("Tstick.lastConnectedNetwork: "); Serial.println(Tstick.lastConnectedNetwork);
  // Serial.print("Tstick.lastStoredPsk: "); Serial.println(Tstick.lastStoredPsk);
  // Serial.print("Tstick.firmware: "); Serial.println(Tstick.firmware);
  // Serial.print("Tstick.osc: "); Serial.println(Tstick.osc);
  // Serial.print("Tstick.oscIP #1: "); Serial.println(Tstick.oscIP[0]);
  // Serial.print("Tstick.oscIP #2: "); Serial.println(Tstick.oscIP[1]);
  // Serial.print("Tstick.oscPORT #1: "); Serial.println(Tstick.oscPORT[0]);
  // Serial.print("Tstick.oscPORT #2: "); Serial.println(Tstick.oscPORT[1]);
  // Serial.print("Tstick.libmapper: "); Serial.println(Tstick.libmapper);

  // Serial.print("Tstick.FSRoffset: "); Serial.print(Tstick.FSRoffset);
  // Serial.print(" (normalized: "); Serial.print(float(Tstick.FSRoffset)/4095, 4); ; Serial.println(")");
  // Serial.print("\nTstick.touchMask: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])) ; ++i ){
  //       Serial.print(Tstick.touchMask[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.abias: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.abias)/sizeof(Tstick.abias[0])) ; ++i ){
  //       Serial.print(Tstick.abias[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.mbias: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.mbias)/sizeof(Tstick.mbias[0])) ; ++i ){
  //       Serial.print(Tstick.mbias[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.gbias: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.gbias)/sizeof(Tstick.gbias[0])) ; ++i ){
  //       Serial.print(Tstick.gbias[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.acclcalibration: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.acclcalibration)/sizeof(Tstick.acclcalibration[0])) ; ++i ){
  //       Serial.print(Tstick.acclcalibration[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.magncalibration: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.magncalibration)/sizeof(Tstick.magncalibration[0])) ; ++i ){
  //       Serial.print(Tstick.magncalibration[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.print("\nTstick.gyrocalibration: ");
  //     for( int i = 0 ; i < (sizeof(Tstick.gyrocalibration)/sizeof(Tstick.gyrocalibration[0])) ; ++i ){
  //       Serial.print(Tstick.gyrocalibration[i], 10);
  //       Serial.print(" ");
  //     }
  // Serial.println();
}


void parseJSON() {    
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant/ to compute the capacity.
  const size_t capacity = 4*JSON_ARRAY_SIZE(2) + 3*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(25) + 350;
  DynamicJsonDocument doc(capacity);

  if (SPIFFS.exists("/config.json")) { // file exists, reading and loading
    Serial.println("Reading config file...");
    File configFile = SPIFFS.open("/config.json", "r");
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.println("Failed to read file!\n");
        } 
      else {
        // Copy values from the JsonDocument to the Config
        Tstick.id = doc["id"];
        strlcpy(Tstick.type, doc["type"], sizeof(Tstick.type));
        strlcpy(Tstick.author, doc["author"], sizeof(Tstick.author));
        strlcpy(Tstick.color, doc["color"], sizeof(Tstick.color));
        strlcpy(Tstick.APpasswd, doc["APpasswd"], sizeof(Tstick.APpasswd));
        strlcpy(Tstick.lastConnectedNetwork, doc["lastConnectedNetwork"], sizeof(Tstick.lastConnectedNetwork));
        strlcpy(Tstick.lastStoredPsk, doc["lastStoredPsk"], sizeof(Tstick.lastStoredPsk));
        //Tstick.firmware = doc["firmware"];
        strlcpy(Tstick.oscIP[0], doc["oscIP1"], sizeof(Tstick.oscIP[0]));
        strlcpy(Tstick.oscIP[1], doc["oscIP2"], sizeof(Tstick.oscIP[1]));
        Tstick.oscPORT[0] = doc["oscPORT1"];
        Tstick.oscPORT[1] = doc["oscPORT2"];
        Tstick.libmapper = doc["libmapper"];
        Tstick.osc = doc["osc"];
        Tstick.FSRoffset = doc["FSRoffset"];
        
        // Tstick.touchMask[0] = doc["touchMask0"][0];
        // Tstick.touchMask[1] = doc["touchMask0"][1];
        // Tstick.touchMask[2] = doc["touchMask1"][0];
        // Tstick.touchMask[3] = doc["touchMask1"][1];
        // Tstick.touchMask[4] = doc["touchMask2"][0];
        // Tstick.touchMask[5] = doc["touchMask2"][1];
        // Tstick.touchMask[6] = doc["touchMask3"][0];
        // Tstick.touchMask[7] = doc["touchMask3"][1];
        
        JsonArray abias = doc["abias"];
        Tstick.abias[0] = abias[0];
        Tstick.abias[1] = abias[1];
        Tstick.abias[2] = abias[2];
        
        JsonArray mbias = doc["mbias"];
        Tstick.mbias[0] = mbias[0];
        Tstick.mbias[1] = mbias[1];
        Tstick.mbias[2] = mbias[2];
        
        JsonArray gbias = doc["gbias"];
        Tstick.gbias[0] = gbias[0];
        Tstick.gbias[1] = gbias[1];
        Tstick.gbias[2] = gbias[2];
        
        JsonArray acclcalibration = doc["acclcalibration"];
        Tstick.acclcalibration[0] = acclcalibration[0];
        Tstick.acclcalibration[1] = acclcalibration[1];
        Tstick.acclcalibration[2] = acclcalibration[2];
        Tstick.acclcalibration[3] = acclcalibration[3];
        Tstick.acclcalibration[4] = acclcalibration[4];
        Tstick.acclcalibration[5] = acclcalibration[5];
        Tstick.acclcalibration[6] = acclcalibration[6];
        Tstick.acclcalibration[7] = acclcalibration[7];
        Tstick.acclcalibration[8] = acclcalibration[8];
        
        JsonArray gyrocalibration = doc["gyrocalibration"];
        Tstick.gyrocalibration[0] = gyrocalibration[0];
        Tstick.gyrocalibration[1] = gyrocalibration[1];
        Tstick.gyrocalibration[2] = gyrocalibration[2];
        Tstick.gyrocalibration[3] = gyrocalibration[3];
        Tstick.gyrocalibration[4] = gyrocalibration[4];
        Tstick.gyrocalibration[5] = gyrocalibration[5];
        Tstick.gyrocalibration[6] = gyrocalibration[6];
        Tstick.gyrocalibration[7] = gyrocalibration[7];
        Tstick.gyrocalibration[8] = gyrocalibration[8];
        
        JsonArray magncalibration = doc["magncalibration"];
        Tstick.magncalibration[0] = magncalibration[0];
        Tstick.magncalibration[1] = magncalibration[1];
        Tstick.magncalibration[2] = magncalibration[2];
        Tstick.magncalibration[3] = magncalibration[3];
        Tstick.magncalibration[4] = magncalibration[4];
        Tstick.magncalibration[5] = magncalibration[5];
        Tstick.magncalibration[6] = magncalibration[6];
        Tstick.magncalibration[7] = magncalibration[7];
        Tstick.magncalibration[8] = magncalibration[8];

        configFile.close();

        createTstickSSID();
        
        Serial.println("T-Stick configuration file loaded.\n");
        }
      
    } 
  else {
    Serial.println("Failed to read config file!\n");
    }
}


void saveJSON() {
  
  Serial.println("Saving config to JSON file...");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant/ to compute the capacity.
  const size_t capacity = 4*JSON_ARRAY_SIZE(2) + 3*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(25);
  DynamicJsonDocument doc(capacity);

  // Copy values from Config to the JsonDocument
  doc["id"] = Tstick.id;
  doc["type"] = Tstick.type;
  doc["author"] = Tstick.author;
  doc["color"] = Tstick.color;
  doc["APpasswd"] = Tstick.APpasswd;
  doc["lastConnectedNetwork"] = Tstick.lastConnectedNetwork;
  doc["lastStoredPsk"] = Tstick.lastStoredPsk;
  doc["firmware"] = Tstick.firmware;
  doc["oscIP1"] = Tstick.oscIP[0];
  doc["oscPORT1"] = Tstick.oscPORT[0];
  doc["oscIP2"] = Tstick.oscIP[1];
  doc["oscPORT2"] = Tstick.oscPORT[1];
  doc["libmapper"] = Tstick.libmapper;
  doc["osc"] = Tstick.osc;
  doc["FSRoffset"] = Tstick.FSRoffset;
  
  JsonArray touchMask0 = doc.createNestedArray("touchMask0");
  touchMask0.add(Tstick.touchMask[0]);
  touchMask0.add(Tstick.touchMask[1]);

  JsonArray touchMask1 = doc.createNestedArray("touchMask1");
  touchMask1.add(Tstick.touchMask[2]);
  touchMask1.add(Tstick.touchMask[3]);

  JsonArray touchMask2 = doc.createNestedArray("touchMask2");
  touchMask2.add(Tstick.touchMask[4]);
  touchMask2.add(Tstick.touchMask[5]);

  JsonArray touchMask3 = doc.createNestedArray("touchMask3");
  touchMask3.add(Tstick.touchMask[6]);
  touchMask3.add(Tstick.touchMask[7]);
  
  JsonArray abias = doc.createNestedArray("abias");
  abias.add(Tstick.abias[0]);
  abias.add(Tstick.abias[1]);
  abias.add(Tstick.abias[2]);
  
  JsonArray mbias = doc.createNestedArray("mbias");
  mbias.add(Tstick.mbias[0]);
  mbias.add(Tstick.mbias[1]);
  mbias.add(Tstick.mbias[2]);
  
  JsonArray gbias = doc.createNestedArray("gbias");
  gbias.add(Tstick.gbias[0]);
  gbias.add(Tstick.gbias[1]);
  gbias.add(Tstick.gbias[2]);
  
  JsonArray acclcalibration = doc.createNestedArray("acclcalibration");
  acclcalibration.add(Tstick.acclcalibration[0]);
  acclcalibration.add(Tstick.acclcalibration[1]);
  acclcalibration.add(Tstick.acclcalibration[2]);
  acclcalibration.add(Tstick.acclcalibration[3]);
  acclcalibration.add(Tstick.acclcalibration[4]);
  acclcalibration.add(Tstick.acclcalibration[5]);
  acclcalibration.add(Tstick.acclcalibration[6]);
  acclcalibration.add(Tstick.acclcalibration[7]);
  acclcalibration.add(Tstick.acclcalibration[8]);
  
  JsonArray gyrocalibration = doc.createNestedArray("gyrocalibration");
  gyrocalibration.add(Tstick.gyrocalibration[0]);
  gyrocalibration.add(Tstick.gyrocalibration[1]);
  gyrocalibration.add(Tstick.gyrocalibration[2]);
  gyrocalibration.add(Tstick.gyrocalibration[3]);
  gyrocalibration.add(Tstick.gyrocalibration[4]);
  gyrocalibration.add(Tstick.gyrocalibration[5]);
  gyrocalibration.add(Tstick.gyrocalibration[6]);
  gyrocalibration.add(Tstick.gyrocalibration[7]);
  gyrocalibration.add(Tstick.gyrocalibration[8]);

  JsonArray magncalibration = doc.createNestedArray("magncalibration");
  magncalibration.add(Tstick.magncalibration[0]);
  magncalibration.add(Tstick.magncalibration[1]);
  magncalibration.add(Tstick.magncalibration[2]);
  magncalibration.add(Tstick.magncalibration[3]);
  magncalibration.add(Tstick.magncalibration[4]);
  magncalibration.add(Tstick.magncalibration[5]);
  magncalibration.add(Tstick.magncalibration[6]);
  magncalibration.add(Tstick.magncalibration[7]);
  magncalibration.add(Tstick.magncalibration[8]);

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {Serial.println("Failed to open config file for writing!\n");}
  
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {Serial.println("Failed to write to file");}
  else {Serial.println("JSON file successfully saved!\n");}
  
  configFile.close();
} //end save

