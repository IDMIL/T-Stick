
byte OSCMsgReceive()
{
  boolean changed = 0;
  OSCMessage msgRec;
  int size = oscEndpoint.parsePacket();
  //Serial.println(size);

  if (size>0){
    if (DEBUG){
      Serial.printf("Received %d bytes from %s, port %d\n", size, oscEndpoint.remoteIP().toString().c_str(), oscEndpoint.remotePort());
    }
    while (size--){
      msgRec.fill(oscEndpoint.read());
    }
    if (!msgRec.hasError()){
      changed = 1;  
      msgRec.route("/status",fromHost);
    }
  }
  return changed;
}

void fromHost(OSCMessage &msg, int addrOffset) {
  int S = msg.size();
  if (DEBUG) {
    int L = msg.getDataLength(0);
    char type = msg.getType(0);    
    Serial.print("Size total msg: "); Serial.println(S);     
    Serial.print("Size of msg: "); Serial.println(L); 
    Serial.print("Type of msg: "); Serial.println(type);
  }
  for (int i=0; i<S; i++) 
  {
    bufferFromHost[i] = msg.getInt(i);
  }
}
