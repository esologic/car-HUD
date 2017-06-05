#define PATTERNPANIC 0

#define NUMANALOGINPUTPINS 6
#define NUMDIGITALINPUTPINS 1
#define NUMINPUTPINS 7

#define MESSAGESIZE 3

#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

int analogInputPins[NUMANALOGINPUTPINS] = {0, 1, 2, 3, 4, 5};
int digitalInputPins[NUMDIGITALINPUTPINS] = {4};
int inputPins[NUMINPUTPINS] = {0, 1, 2, 3, 4, 5, 4};

int (*readPtr[NUMINPUTPINS])(uint8_t) = {analogRead, analogRead, analogRead, analogRead, analogRead, analogRead, digitalRead}; // array of function pointers to the read function of the given pin

union message {
    byte bytes[MESSAGESIZE];
    struct {
        byte M0;
    };
    struct {
        byte unused0; // for offsetting DO NOT USE
        byte M1;
    };
    struct {
        byte unused1; // for offsetting DO NOT USE
        byte unused2; // for offsetting DO NOT USE
        byte CRC;
    };
};

union intBytes {
  int i;
  byte bytes[2];
};

void setup() {
  Serial.begin(9600);
  rpiSerial.begin(9600);
}

void loop() {

  byte messageBuff[MESSAGESIZE]; 
  
  if (rpiSerial.available() >= MESSAGESIZE)
  {
    message masterMessage;
    
    rpiSerial.readBytes(masterMessage.bytes, MESSAGESIZE);

    byte incomingDataBytes[2] = {masterMessage.M0, masterMessage.M1};

    byte incomingMasterCRC = CalcCRC(incomingDataBytes, 2);

 
    Serial.print("From Master: ");
    Serial.print("M0: ");
    Serial.print(masterMessage.M0); 
    Serial.print(" M1: ");
    Serial.print(masterMessage.M1);
    Serial.print(" CRC: ");
    Serial.print(masterMessage.CRC);  
    Serial.print(" Calcd CRC: ");
    Serial.print(incomingMasterCRC);
    Serial.print(" | ");
    

    if (incomingMasterCRC == masterMessage.CRC) { // if the CRC from the master passes
 
      intBytes reading;
      
      switch (masterMessage.M0)
      {
        case 0: // read mode -> Send values to master
          reading.i = readPtr[masterMessage.M1](inputPins[masterMessage.M1]);
          break;
        
        case 1: // write mode -> Set values on arduino
          break;
      }
      
      byte CRC = CalcCRC(reading.bytes, 2);
  
      rpiSerial.write(reading.bytes[0]);
      rpiSerial.write(reading.bytes[1]);    
      rpiSerial.write(CRC);

      
      Serial.print("To Master: ");
      Serial.print("B0: ");
      Serial.print(reading.bytes[0]); 
      Serial.print(" B1: ");
      Serial.print(reading.bytes[1]);
      Serial.print(" CRC: ");
      Serial.print(CRC);  
      Serial.println("");
      
      
    } else { // Bad master CRC
      Serial.println("Bad CRC");
    }
  }
}

byte CalcCRC(byte byteArray[], int numBytes) {
  byte b = 0;
  for (int index = 0; index < numBytes; index++)
  {
    b = b + byteArray[index];
  }
  return b;
}
