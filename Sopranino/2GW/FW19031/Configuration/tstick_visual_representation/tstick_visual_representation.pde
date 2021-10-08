
import oscP5.*;
import netP5.*;

OscP5 oscP5;

// shape parameters
float radius = 65;
float ang = 0; //, //ang2 = 0;
int pts = 8;
float depth = 200;

/* a NetAddress contains the ip address and port number of a remote location in the network. */
NetAddress oscSendAddress; 
NetAddress myAddress;
int myListeningPort = 8000;
int myBroadcastPort = 8888;

// orientation variables
float yaw = 0;
float pitch = 0;
float roll = 0;

int time = millis();

void setup() {

  oscP5 = new OscP5(this, myListeningPort);
  oscSendAddress = new NetAddress("192.168.10.8", myBroadcastPort);

  size(1024, 768, P3D); 
  fill(150);
  smooth();  // comment out with P3D renderer
  noStroke();
  frameRate(30);
}


void draw() {
  
  //TSTICK 1
  background(0);
  directionalLight(166, 166, 196, -60, -60, -60);
  ambientLight(105, 105, 130);
  translate(width/2, height/2, -200);
  rotateY(radians(yaw));
  rotateX(radians(pitch));
  rotateZ(radians(roll));

  //body
  beginShape(QUAD_STRIP); 
  for (int i=0; i<=pts; i++) {
    float  px = cos(radians(ang))*radius;
    float  py = sin(radians(ang))*radius;
    vertex(px, py, depth); 
    vertex(px, py, -depth); 
    ang+=360/pts;
  }
  endShape(); 

  //cap 1
  beginShape(POLYGON); 
  for (int i=0; i<=pts; i++) {
    float  px = cos(radians(ang))*radius;
    float  py = sin(radians(ang))*radius;
    vertex(px, py, depth); 
    ang+=360/pts;
  }
  endShape(); 

  //cap2
  beginShape(POLYGON); 
  for (int i=0; i<=pts; i++) {
    float  px = cos(radians(ang))*radius;
    float  py = sin(radians(ang))*radius;
    vertex(px, py, -depth); 
    ang+=360/pts;
  }
  endShape(); 


  // T-Stick Heartbeat
  if (millis() > time + 800)
  {
    OscMessage heartbeat = new OscMessage("/status");
    heartbeat.add(115);
    oscP5.send(heartbeat, oscSendAddress);
    time = millis();
  }
}


/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* get and print the address pattern and the typetag of the received OscMessage */
  //println("### received an osc message with addrpattern "+theOscMessage.addrPattern()+" and typetag "+theOscMessage.typetag());
  //theOscMessage.print();

  //if (theOscMessage.checkAddrPattern("/orientation/ypr_at")==true) {
  if (theOscMessage.checkAddrPattern("/rawypr")==true) {
    yaw = theOscMessage.get(0).floatValue();
    pitch = theOscMessage.get(1).floatValue();
    roll = theOscMessage.get(2).floatValue();
  }
}
