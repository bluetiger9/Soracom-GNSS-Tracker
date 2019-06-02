// based on:
// https://create.arduino.cc/projecthub/Nicholas_N/how-to-use-the-accelerometer-gyroscope-gy-521-6dfc19

#include <Wire.h>
#include <WioLTEforArduino.h>

const int MPU=0x68; 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

WioLTE Wio;

void setup(){
  for (int i = 20; i > 0; --i) {
    SerialUSB.println(i);
    delay(1000);
  }

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply grove.");
  Wio.PowerSupplyGrove(true);
  
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(/*true*/);
  //SerialUSB.begin(9600);
}

void loop(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(/*false*/);
  Wire.requestFrom(MPU,12/*,true*/);  
  AcX=Wire.read()<<8|Wire.read();    
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read();  
  
  SerialUSB.print("Accelerometer: ");
  SerialUSB.print("X = "); SerialUSB.print(AcX);
  SerialUSB.print(" | Y = "); SerialUSB.print(AcY);
  SerialUSB.print(" | Z = "); SerialUSB.println(AcZ); 
  
  SerialUSB.print("Gyroscope: ");
  SerialUSB.print("X = "); SerialUSB.print(GyX);
  SerialUSB.print(" | Y = "); SerialUSB.print(GyY);
  SerialUSB.print(" | Z = "); SerialUSB.println(GyZ);
  SerialUSB.println(" ");
  delay(333);
}
