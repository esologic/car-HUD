#define PATTERNPANIC 0

#define MESSAGESIZE 3

#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

#define NUMANALOGINPUTPINS 6
int analogInputPins[NUMANALOGINPUTPINS] = {0, 1, 2, 3, 4, 5};

#define NUMDIGITALINPUTPINS 1
int digitalInputPins[NUMDIGITALINPUTPINS] = {4};

#define NUMINPUTPINS NUMANALOGINPUTPINS + NUMDIGITALINPUTPINS
int inputPins[NUMINPUTPINS]; // will hold analogInputPins + digitalInputPins

int (*readPtr[NUMINPUTPINS])(uint8_t); // will hold the read (digitalRead or analogRead or something else) function for a given pin

union message {
    byte rawBytes[MESSAGESIZE]; // holds the two data bytes and the CRC
    byte dataBytes[MESSAGESIZE-1]; // hold only the data bytes, without the CRC
    struct {
        byte M0; // holds the first data byte
    };
    struct {
        byte unused0; // for offsetting DO NOT USE
        byte M1; // holds the second data byte
    };
    struct {
        byte unused1; // for offsetting DO NOT USE
        byte unused2; // for offsetting DO NOT USE
        byte CRC; // holds the checksum
    };
};

union intBytes {
  int i;
  byte bytes[2];
};

void setup() {

  Serial.begin(9600);
  rpiSerial.begin(9600);

  // combine the two arrays of pins
  memcpy(inputPins, analogInputPins, NUMANALOGINPUTPINS * sizeof(int));
  memcpy(inputPins + NUMANALOGINPUTPINS, digitalInputPins, NUMDIGITALINPUTPINS * sizeof(int));

  for (int index = 0; index < NUMANALOGINPUTPINS; index++)
  {
    readPtr[index] = analogRead;
  }

  for (int index = 0; index < NUMDIGITALINPUTPINS; index++)
  {
    pinMode(digitalInputPins[index], INPUT);
    readPtr[index + NUMANALOGINPUTPINS] = digitalRead;
  }
}

void loop() {
    
  if (rpiSerial.available() >= MESSAGESIZE) { // wait for a full MESSAGE to arrive
    
    message masterMessage = ReadMessage(rpiSerial);
    
    byte incomingMasterCRC = CalcCRC(masterMessage.dataBytes, MESSAGESIZE-1);

    if (incomingMasterCRC == masterMessage.CRC) { // if the CRC from the master passes
      
      intBytes reading;
      
      switch (masterMessage.M0) {
        case 0: // read mode -> Send values to master
          reading.i = readPtr[masterMessage.M1](inputPins[masterMessage.M1]);
          break;
        case 1: // write mode -> Set values on arduino
          break;
      }

      message slaveMessage;
      memcpy(slaveMessage.dataBytes, reading.bytes, MESSAGESIZE-1);
      slaveMessage.CRC = CalcCRC(slaveMessage.dataBytes, MESSAGESIZE-1); 
  
      WriteMessage(rpiSerial, slaveMessage);
            
    } else { // Bad master CRC
      Serial.println("Bad CRC");
    }
  }
}

message ReadMessage(SoftwareSerial &port) {
  message incomingMessage;
  port.readBytes(incomingMessage.rawBytes, MESSAGESIZE); // read the raw bytes from the serial port into a message struct
  return incomingMessage;
}

void WriteMessage(SoftwareSerial &port, message m) {
  for (int index = 0; index < MESSAGESIZE; index++) {
    port.write(m.rawBytes[index]);
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
