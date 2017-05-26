#define NUMANALOGINPUTPINS 6
#define NUMDIGITALINPUTPINS 1

#define MESSAGESIZE 3

#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

int analogInputPins[NUMANALOGINPUTPINS] = {0, 1, 2, 3, 4, 5};
int digitalInputPins[NUMDIGITALINPUTPINS] = {4}; 

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

  for (int index = 0; index < NUMDIGITALINPUTPINS; index++)
  {
    pinMode(digitalInputPins[index], INPUT);
  }
  
  Serial.begin(9600);
  rpiSerial.begin(9600);
}

void loop() {

  byte messageBuff[MESSAGESIZE]; 
  
  if (rpiSerial.available() >= MESSAGESIZE)
  {
    message masterMessage;
    
    rpiSerial.readBytes(masterMessage.bytes, MESSAGESIZE);

    byte messageBs[2] = {masterMessage.M0, masterMessage.M1};

    byte calcMasterCRC = CalcCRC(messageBs, 2);

    /*
    Serial.print("From Master: ");
    Serial.print("M0: ");
    Serial.print(masterMessage.M0); 
    Serial.print(" M1: ");
    Serial.print(masterMessage.M1);
    Serial.print(" CRC: ");
    Serial.print(masterMessage.CRC);  
    Serial.print(" Calcd CRC: ");
    Serial.print(calcMasterCRC);
    Serial.print(" | ");
    */ 

    if (calcMasterCRC == masterMessage.CRC) {
 
      intBytes reading;
      
      switch (masterMessage.M0)
      {
        case 0: // read mode
          if ((masterMessage.M1 >= 0) && (masterMessage.M1 < NUMANALOGINPUTPINS)) {
              reading.i = analogRead(masterMessage.M1);
          } else if (((masterMessage.M1 >= NUMANALOGINPUTPINS) && (masterMessage.M1 < NUMDIGITALINPUTPINS + NUMANALOGINPUTPINS))) {
              Serial.println(digitalRead(4));
              reading.i = digitalRead(4); // todo
          }
          break;
        
        case 1: // write mode
          break;
      }
      
      byte CRC = CalcCRC(reading.bytes, 2);
  
      rpiSerial.write(reading.bytes[0]);
      rpiSerial.write(reading.bytes[1]);    
      rpiSerial.write(CRC);

      /*
      Serial.print("To Master: ");
      Serial.print("B0: ");
      Serial.print(reading.bytes[0]); 
      Serial.print(" B1: ");
      Serial.print(reading.bytes[1]);
      Serial.print(" CRC: ");
      Serial.print(CRC);  
  
      Serial.println("");
      */ 
      
    } else {
      // Serial.println(" Bad master CRC");
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
