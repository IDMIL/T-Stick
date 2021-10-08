// Test sketch to handle WiFi setup for T-Stick
// Alex Nieva
/* Based on  * Property off: www.microcontroller-project.com  *
 * Author      : Usman Ali Butt                               *
 * Date        : October 2015                                 */

bool setupWiFi() {
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP_STA);           //Both in Station and Access Point Mode
  
  if(readNetworkFromEEPROM()) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(Essid.c_str(), Epass.c_str());                         //c_str()
    // Wait for connection
    Serial.print("Connecting");
    int then = millis();
    while ( WiFi.status() != WL_CONNECTED && timeout1check == false) {
      delay ( 500 );
      Serial.print ( "." );
      if (millis() - then > timeout1) {
        timeout1check = true;
        Serial.println("Connection to network on EEPROM failed... Creating Access Point.");
        createAP();
      }
    }
    if (timeout1check == false) {
      Serial.println();
      //delay(5000);                                                    //Wait for IP to be assigned to Module by Router
      IPAddress ip = WiFi.localIP();                                    //Get ESP8266 IP Adress
      Serial.print("Connected to: "); Serial.println(Essid);
      Serial.print("IP address: "); Serial.println(ip);                 //Print Ip on serial monitor or any serial debugger  
      externalNetwork = true; 
      return true;
    }
    else {
      timeout1check = false;
      return true;
    }
  }
  else {
    Serial.println("There was no WiFi info on the EEPROM...");
    createAP();
    return true;
  }
}

bool createAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);
  Serial.println("Creating WiFi Access Point");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.softAPIP());
  externalNetwork = false;
  return true;
}

bool readNetworkFromEEPROM(){
  for (int i = addrNet; i < addrNet + 32; ++i)                        //Reading SSID
    {
      Essid += char(EEPROM.read(i)); 
    }
  for (int i = addrNet + 32; i < addrNet + 96; ++i)                   //Reading Password
    {
      Epass += char(EEPROM.read(i)); 
    }
  if ( Essid[0] != (char)(255) ) {  
    Serial.println(Essid);                                            //Print SSID
    Serial.println(Epass);                                            //Print Password
    return true;
  }
  else {
//    Serial.print("Essid.length: ");
//    Serial.println(Essid.length());
    return false;
  }
}

bool writeNetworkToEEPROM() {
  Serial.println("Writing to EEPROM to be implemented...");

  String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>SopraninoWiFi-181 Configuration Webpage</h1> ";
  s += "<p>Network information written to EEPROM.</p>";
  s += "<br><a href='/'>Return home</a></html>\r\n\r\n";
  server.send(200,"text/html",s);  
}

boolean connectUDP()
{
   boolean state = false;
   if (DEBUG)
   {
      Serial.println("");
      Serial.println("Connecting to UDP");
   }
   if(oscEndpoint.begin(portLocal) == 1)
   {
      if (DEBUG)
         Serial.println("UDP Connection successful");
      state = true;
   }
   else
   {
      if (DEBUG)
         Serial.println("UDP Connection failed");
   }
   return state;
}

