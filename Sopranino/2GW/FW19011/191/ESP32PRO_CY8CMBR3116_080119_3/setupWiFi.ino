void setupWiFi(){
    // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  connectUDP();

  oscEndpointIP.fromString("192.168.4.2");
  oscEndpointPORT = 8000;
  
  //server.begin();
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
