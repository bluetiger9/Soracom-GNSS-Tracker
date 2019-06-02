#include <WioLTEforArduino.h>
#include <Wire.h>
#include <stdio.h>

// Wio LTE
WioLTE Wio;
const int RECEIVE_TIMEOUT = 10000;

// MPU-6050 (GY-521)
const int MPU = 0x68;

void stop() {
  while (true);
}

void setupSerialUSB() {
  // sleep 20s
  for (int i = 20; i > 0; --i) {
    SerialUSB.println(i);
    delay(1000);
  }  
}

void setupWioLTE() {
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  SerialUSB.println("### Power supply GNSS.");
  Wio.PowerSupplyGNSS(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    stop();
  }  
}

void setupGNSS() {
  SerialUSB.println("### Enable GNSS.");
  if (!Wio.EnableGNSS()) {
    SerialUSB.println("### ERROR! ###");
    stop();
  }
}

// MPU-6050 (GY-521)
// based on https://create.arduino.cc/projecthub/Nicholas_N/how-to-use-the-accelerometer-gyroscope-gy-521-6dfc19
void setupAccelerometer() {
  SerialUSB.println("### Power supply grove.");
  Wio.PowerSupplyGrove(true);
  
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(/*true*/);
}

void setupSoracom() {
  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    stop();
  }  
}

// Setup
void setup() {
  setupSerialUSB();
  setupWioLTE();
  setupGNSS();  
  setupAccelerometer();
  setupSoracom();
  
  SerialUSB.println("### Setup completed.");
}

bool getGNSSLocation(double &longitude, double &latitude, double &altitude, struct tm &utc) {
  SerialUSB.println("### Get GNSS location.");
  if (Wio.GetGNSSLocation(&longitude, &latitude, &altitude, &utc)) {
    SerialUSB.print("    long: ");
    SerialUSB.println(longitude, 6);
    SerialUSB.print("    lat: ");
    SerialUSB.println(latitude, 6);
    SerialUSB.print("    altitude: ");
    SerialUSB.println(altitude, 6);
    SerialUSB.print("    utc: ");
    SerialUSB.println(asctime(&utc));
    return true;
    
  } else {
    if (Wio.GetLastError() == WioLTE::E_GNSS_NOT_FIXED) {
      SerialUSB.println("### NOT FIXED. ###");
    } else {
      SerialUSB.println("### ERROR! ###");
    }
    return false;
  }  
}

bool getAccelerometerData(int16_t &AcX, int16_t &AcY, int16_t &AcZ, 
                          int16_t &Tmp, int16_t &GyX, int16_t &GyY, int16_t &GyZ) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(/*false*/);
  Wire.requestFrom(MPU, 12 /*,true*/);  
  AcX = Wire.read() << 8 | Wire.read();    
  AcY = Wire.read() << 8 | Wire.read();  
  AcZ = Wire.read() << 8 | Wire.read();  
  GyX = Wire.read() << 8 | Wire.read();  
  GyY = Wire.read() << 8 | Wire.read();  
  GyZ = Wire.read() << 8 | Wire.read();  
  
  SerialUSB.print("Accelerometer: ");
  SerialUSB.print("X = "); SerialUSB.print(AcX);
  SerialUSB.print(" | Y = "); SerialUSB.print(AcY);
  SerialUSB.print(" | Z = "); SerialUSB.println(AcZ); 
  
  SerialUSB.print("Gyroscope: ");
  SerialUSB.print("X = "); SerialUSB.print(GyX);
  SerialUSB.print(" | Y = "); SerialUSB.print(GyY);
  SerialUSB.print(" | Z = "); SerialUSB.println(GyZ);
  SerialUSB.println(" ");
  
  return true; // TODO: handle errors
}

bool soracomSend(char *data) {
  SerialUSB.println("### Open.");
  int connectId;
  connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    return false;
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

  SerialUSB.println("### Receive.");
  int length;
  length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT);
  if (length < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }
  if (length == 0) {
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
    goto err_close;
  }
  SerialUSB.print("Receive:");
  SerialUSB.print(data);
  SerialUSB.println("");
  return true;

err_close:
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    return false;
  }    
}

// the loop function runs over and over again forever
void loop() {
  // GNSS
  double longitude, latitude, altitude;
  struct tm utc; 
  
  if (!getGNSSLocation(longitude, latitude, altitude, utc)) {
    delay(1000);
    return;
  }

  // Accelerometer
  int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

  if (!getAccelerometerData(AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ)) {
    delay(1000);
    return;  
  }

  // Soracom
  char data[1024];
  
  sprintf(data,
    "{\"lat\":%.6f,\"lng\":%.6f, \"alt\":%.6f, \"acX\":%d, \"acY\":%d, \"acZ\":%d, \"temp\": %d, \"gyX\":%d, \"gyY\":%d, \"gyZ\":%d}",
    latitude, longitude, altitude, AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ);

  if (!soracomSend(data)) {
    delay(1000);
    return;
  }
  
  // sleep
  delay(10000);
}
