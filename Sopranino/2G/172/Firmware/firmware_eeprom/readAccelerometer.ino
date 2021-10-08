float filterVal;
float filteredVal[3];

void readAccelerometer() 
{
  uint8_t howManyBytesToRead = 6; //6 for all axes
  readFrom( DATAX0, howManyBytesToRead, buff); //read the acceleration data from the ADXL345
  outxyz[0] = (((int)buff[1]) << 8) | buff[0];
  outxyz[1] = (((int)buff[3]) << 8) | buff[2];
  outxyz[2] = (((int)buff[5]) << 8) | buff[4];

  for (int i=0;i<3;i++) {if (outxyz[i]>=0x8000) outxyz[i]=outxyz[i]-0xFFFF-1;} /// calc two's compliment 

  //mapping of data - also rotating coordinates so that matches spatial orientation of the T-Stick sopranino
  acceleration[0] = map(-outxyz[1],-564,564,0,1023);
  acceleration[1] = map(outxyz[0],-564,564,0,1023);
  acceleration[2] = map(outxyz[2],-564,564,0,1023);
  
}

int filter(int data, float filterVal, float smoothedVal){ // Function not in use
  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}


